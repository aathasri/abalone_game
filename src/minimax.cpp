// minimax.cpp (optimized with heuristic caching)

#include "minimax.h"
#include "move_generator.h"
#include "heuristic_calculator.h"
#include <limits>
#include <iostream>
#include <thread>
#include <future>
#include <cmath>
#include <algorithm>

Minimax::Minimax(int maxDepth, int timeLimitSeconds, int bufferTimeSeconds)
    : maxDepth(maxDepth),
      transpositionTable(64),
      timeLimitSeconds(timeLimitSeconds),
      bufferTimeSeconds(bufferTimeSeconds),
      stopSearch(false) {}

int Minimax::minimax(Board& board, int depth, int currentPlayer, bool isMaximizing, int alpha, int beta) {
    if (stopSearch.load()) return isMaximizing ? alpha : beta;

    ++nodeCount;
    uint64_t hash = board.getZobristHash();

    TTEntry entry;
    if (transpositionTable.lookup(hash, entry) && entry.depth >= depth) {
        if (entry.flag == BoundType::EXACT) return entry.score;
        if (entry.flag == BoundType::LOWER && entry.score >= beta) return entry.score;
        if (entry.flag == BoundType::UPPER && entry.score <= alpha) return entry.score;
    }

    int p1 = board.getNumPlayerOnePieces();
    int p2 = board.getNumPlayerTwoPieces();
    if (p1 <= 8) return 100000;
    if (p2 <= 8) return -100000;

    if (depth == 0) {
        HeuristicCalculator hcalc;
        int score = hcalc.calculateHeuristic(board);
        transpositionTable.insert(hash, { score, depth, isMaximizing, BoundType::EXACT, Move() });
        return score;
    }

    MoveGenerator moveGen;
    moveGen.generateMoves(currentPlayer, board);
    std::set<Move> moves = moveGen.getGeneratedMoves();

    if (moves.empty()) {
        HeuristicCalculator hcalc;
        int score = hcalc.calculateHeuristic(board);
        transpositionTable.insert(hash, { score, depth, isMaximizing, BoundType::EXACT, Move() });
        return score;
    }

    std::vector<std::pair<Move, int>> orderedMoves;
    for (const auto& m : moves) {
        MoveUndo undo;
        board.makeMove(m, undo);
        uint64_t childHash = board.getZobristHash();
        int heuristic;
        TTEntry childEntry;
        if (transpositionTable.lookup(childHash, childEntry)) {
            heuristic = childEntry.score;
        } else {
            HeuristicCalculator hcalc;
            heuristic = hcalc.calculateHeuristic(board);
            transpositionTable.insert(childHash, { heuristic, 0, isMaximizing, BoundType::EXACT, Move() });
        }
        board.unmakeMove(undo);
        orderedMoves.push_back({ m, heuristic });
    }

    std::sort(orderedMoves.begin(), orderedMoves.end(), [isMaximizing](const auto& a, const auto& b) {
        return isMaximizing ? a.second > b.second : a.second < b.second;
    });

    int originalAlpha = alpha;
    int bestScore = isMaximizing ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();
    Move bestMove;

    for (auto& [m, _] : orderedMoves) {
        if (stopSearch.load()) break;
        MoveUndo undo;
        board.makeMove(m, undo);
        int score = minimax(board, depth - 1, 3 - currentPlayer, !isMaximizing, alpha, beta);
        board.unmakeMove(undo);

        if (isMaximizing) {
            if (score > bestScore) {
                bestScore = score;
                bestMove = m;
            }
            alpha = std::max(alpha, bestScore);
        } else {
            if (score < bestScore) {
                bestScore = score;
                bestMove = m;
            }
            beta = std::min(beta, bestScore);
        }
        if (beta <= alpha) break;
    }

    BoundType flag = BoundType::EXACT;
    if (bestScore <= originalAlpha) flag = BoundType::UPPER;
    else if (bestScore >= beta) flag = BoundType::LOWER;

    transpositionTable.insert(hash, { bestScore, depth, isMaximizing, flag, bestMove });
    return bestScore;
}

static inline bool timeExpired(const std::chrono::steady_clock::time_point& start, double limitSeconds) {
    using namespace std::chrono;
    auto now = steady_clock::now();
    double elapsed = duration_cast<milliseconds>(now - start).count() / 1000.0;
    return (elapsed >= limitSeconds);
}

Move Minimax::findBestMove(Board& board, int currentPlayer) {
    HeuristicCalculator hcalc;
    Move bestMoveOverall;
    int bestScoreOverall = std::numeric_limits<int>::min();

    if (currentPlayer != 2) {
        std::cerr << "[ERROR] AI must be player 2!\n";
        return Move();
    }

    transpositionTable.clear();
    nodeCount = 0;
    stopSearch.store(false);

    MoveGenerator moveGen;
    moveGen.generateMoves(currentPlayer, board);
    std::set<Move> allMoves = moveGen.getGeneratedMoves();
    std::cout << "Root: Generated " << allMoves.size() << " moves\n";
    if (allMoves.empty()) {
        std::cout << "No moves available at root!\n";
        return Move();
    }

    double effectiveTimeLimit = static_cast<double>(timeLimitSeconds - bufferTimeSeconds);
    auto startTime = std::chrono::steady_clock::now();

    int searchDepth = 1;
    double lastDepthTimeMs = 0.0;
    const double baseGrowthFactor = 2.0;
    std::vector<double> depthTimes;

    while (!timeExpired(startTime, effectiveTimeLimit) && searchDepth <= maxDepth) {
        if (!depthTimes.empty()) {
            int count = std::min(static_cast<int>(depthTimes.size()), 3);
            double weightedSum = 0.0;
            double weightTotal = 0.0;
            for (int i = 0; i < count; ++i) {
                double weight = std::pow(2.0, i);
                weightedSum += weight * depthTimes[depthTimes.size() - count + i];
                weightTotal += weight;
            }
            double avg = weightedSum / weightTotal;
            double estimatedNextDepthTimeMs = avg * baseGrowthFactor;

            auto now = std::chrono::steady_clock::now();
            double elapsedMs = std::chrono::duration<double, std::milli>(now - startTime).count();
            double remainingMs = effectiveTimeLimit * 1000.0 - elapsedMs;
            if (estimatedNextDepthTimeMs > remainingMs) {
                std::cout << "Not starting depth " << searchDepth 
                          << " because estimated time (" << estimatedNextDepthTimeMs 
                          << " ms) exceeds remaining time (" << remainingMs << " ms).\n";
                break;
            }
        }

        std::cout << "Starting search at depth " << searchDepth << "...\n";
        auto depthStartTime = std::chrono::steady_clock::now();

        struct MoveOrderItem {
            Move move;
            int heuristic;
        };
        std::vector<MoveOrderItem> ordering;
        ordering.reserve(allMoves.size());
        for (const Move& mv : allMoves) {
            if (stopSearch.load()) break;
            MoveUndo undo;
            board.makeMove(mv, undo);
            int score = hcalc.calculateHeuristic(board);
            board.unmakeMove(undo);
            ordering.push_back({mv, score});
        }
        std::sort(ordering.begin(), ordering.end(), [](const auto& a, const auto& b) {
            return a.heuristic > b.heuristic;
        });

        unsigned int numCores = 14;
        size_t desiredGroupCount = numCores;
        size_t totalMoves = ordering.size();
        size_t groupSize = (totalMoves + desiredGroupCount - 1) / desiredGroupCount;

        std::vector<std::shared_ptr<Board>> boardPool;
        for (unsigned int i = 0; i < numCores; ++i)
            boardPool.emplace_back(std::make_shared<Board>(board));

        ThreadPool pool(numCores);
        std::vector<std::future<std::pair<int, Move>>> futures;

        for (size_t groupStart = 0, groupId = 0; groupStart < totalMoves; groupStart += groupSize, ++groupId) {
            size_t groupEnd = std::min(groupStart + groupSize, totalMoves);
            auto boardPtr = boardPool[groupId % boardPool.size()];

            futures.push_back(pool.enqueue([this, groupStart, groupEnd, boardPtr, currentPlayer, searchDepth, startTime, effectiveTimeLimit, &ordering]() -> std::pair<int, Move> {
                Board& threadBoard = *boardPtr;
                int groupBestScore = std::numeric_limits<int>::min();
                Move groupBestMove;

                for (size_t i = groupStart; i < groupEnd; ++i) {
                    if (timeExpired(startTime, effectiveTimeLimit) || stopSearch.load())
                        break;
                    const Move& candidate = ordering[i].move;
                    MoveUndo undo;
                    threadBoard.makeMove(candidate, undo);
                    int score = minimax(threadBoard, searchDepth - 1, 1, false,
                                        std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
                    threadBoard.unmakeMove(undo);
                    if (score > groupBestScore) {
                        groupBestScore = score;
                        groupBestMove = candidate;
                    }
                }
                return std::make_pair(groupBestScore, groupBestMove);
            }));
        }

        int localBestScore = std::numeric_limits<int>::min();
        Move localBestMove;
        bool futuresTimedOut = false;
        for (auto& fut : futures) {
            while (fut.wait_for(std::chrono::milliseconds(2)) == std::future_status::timeout) {
                if (timeExpired(startTime, effectiveTimeLimit)) {
                    futuresTimedOut = true;
                    break;
                }
            }
            if (futuresTimedOut) break;
            auto [score, move] = fut.get();
            if (score > localBestScore) {
                localBestScore = score;
                localBestMove = move;
            }
        }

        if (timeExpired(startTime, effectiveTimeLimit) || futuresTimedOut) {
            std::cout << "Time expired during depth " << searchDepth << "\n";
            break;
        }

        bestMoveOverall = localBestMove;
        bestScoreOverall = localBestScore;

        auto depthEndTime = std::chrono::steady_clock::now();
        double depthDurationMs = std::chrono::duration<double, std::milli>(depthEndTime - depthStartTime).count();
        depthTimes.push_back(depthDurationMs);

        std::cout << "Depth " << searchDepth << " completed in " 
                  << depthDurationMs << " ms. Best score: " << localBestScore << "\n";

        ++searchDepth;
    }

    stopSearch.store(true);

    std::cout << "Final best score: " << bestScoreOverall << "\n";
    std::cout << "Total nodes searched: " << nodeCount.load() << "\n";
    if (bestMoveOverall.getSize() == 0)
        std::cout << "No valid move found!\n";
    return bestMoveOverall;
}