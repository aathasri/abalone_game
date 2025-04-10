#include "minimax.h"
#include "move_generator.h"
#include "heuristic_calculator.h"
#include "move.h"         // For Move and MoveUndo types
#include "board.h"
#include <algorithm>
#include <limits>
#include <iostream>
#include <future>
#include <thread>
#include <vector>
#include <set>
#include <mutex>
#include <chrono>

// --------------------------------------------------------------------------
// Helper: Checks whether a move is "noisy" (i.e. a capture/push move).
// This implementation applies the move, compares the opponent's piece count 
// before and after, and then undoes the move.
static bool isCaptureMove(Board& board, const Move &move, int currentPlayer) {
    int initialOpponentPieces = (currentPlayer == 1)
                                  ? board.getNumPlayerTwoPieces()
                                  : board.getNumPlayerOnePieces();
    MoveUndo undo;
    board.makeMove(move, undo);
    int newOpponentPieces = (currentPlayer == 1)
                            ? board.getNumPlayerTwoPieces()
                            : board.getNumPlayerOnePieces();
    board.unmakeMove(undo);
    return (newOpponentPieces < initialOpponentPieces);
}

// --------------------------------------------------------------------------
// Quiescence search (interruptible version).
// This version no longer imposes a strict maximum quiescence depth so that
// quiescence search can extend evaluation beyond the iterative deepening limit.
static int quiescenceSearch(Board& board, int currentPlayer, bool isMaximizing,
                            int alpha, int beta, std::atomic<size_t>& nodeCount,
                            std::atomic<bool>& stopSearch, int qDepth = 0) {
    if(stopSearch.load())
        return isMaximizing ? alpha : beta;
    
    ++nodeCount;
    HeuristicCalculator hcalc;
    int standPat = hcalc.calculateHeuristic(board);

    if (isMaximizing) {
        if (standPat >= beta) return beta;
        if (standPat > alpha) alpha = standPat;
    } else {
        if (standPat <= alpha) return alpha;
        if (standPat < beta) beta = standPat;
    }

    MoveGenerator moveGen;
    moveGen.generateMoves(currentPlayer, board);
    const std::set<Move>& allMoves = moveGen.getGeneratedMoves();
    std::set<Move> captureMoves;
    int innerCounter = 0;
    for (const Move &move : allMoves) {
        if(stopSearch.load())
            break;
        if (isCaptureMove(board, move, currentPlayer))
            captureMoves.insert(move);
        if (++innerCounter % 2 == 0)
            std::this_thread::yield();
    }
    if (captureMoves.empty())
        return standPat;

    for (const Move &move : captureMoves) {
        if(stopSearch.load())
            return isMaximizing ? alpha : beta;
        MoveUndo undo;
        board.makeMove(move, undo);
        int score = quiescenceSearch(board, 3 - currentPlayer, !isMaximizing,
                                     alpha, beta, nodeCount, stopSearch, qDepth + 1);
        board.unmakeMove(undo);
        if (isMaximizing) {
            if (score > alpha) alpha = score;
            if (alpha >= beta) return beta;
        } else {
            if (score < beta) beta = score;
            if (beta <= alpha) return alpha;
        }
    }
    return isMaximizing ? alpha : beta;
}

// --------------------------------------------------------------------------
// Constructor for Minimax.
Minimax::Minimax(int maxDepth, int timeLimitSeconds, int bufferTimeSeconds)
    : maxDepth(maxDepth),
      transpositionTable(64),
      nodeCount(0),
      timeLimitSeconds(timeLimitSeconds),
      bufferTimeSeconds(bufferTimeSeconds),
      stopSearch(false)
{
}

// --------------------------------------------------------------------------
// Minimax with alpha-beta pruning (interruptible version).
int Minimax::minimax(Board& board, int depth, int currentPlayer,
                     bool isMaximizing, int alpha, int beta) {
    if(stopSearch.load())
        return isMaximizing ? alpha : beta;

    ++nodeCount;
    HeuristicCalculator hcalc;
    uint64_t hash = board.getZobristHash();

    int p1Marbles = board.getNumPlayerOnePieces();
    int p2Marbles = board.getNumPlayerTwoPieces();
    if (p1Marbles <= 8)
        return 100000;
    if (p2Marbles <= 8)
        return -100000;

    TTEntry entry;
    if (transpositionTable.lookup(hash, entry) && entry.depth >= depth) {
        if (entry.flag == BoundType::EXACT)
            return entry.score;
        else if (entry.flag == BoundType::LOWER && entry.score >= beta)
            return entry.score;
        else if (entry.flag == BoundType::UPPER && entry.score <= alpha)
            return entry.score;
    }

    if (depth == 0) {
        int score = quiescenceSearch(board, currentPlayer, isMaximizing,
                                     alpha, beta, nodeCount, stopSearch);
        transpositionTable.insert(hash, { score, depth, isMaximizing, BoundType::EXACT, Move() });
        return score;
    }

    MoveGenerator moveGen;
    moveGen.generateMoves(currentPlayer, board);
    std::set<Move> moves = moveGen.getGeneratedMoves();
    if (moves.empty()) {
        int score = hcalc.calculateHeuristic(board);
        transpositionTable.insert(hash, { score, depth, isMaximizing, BoundType::EXACT, Move() });
        return score;
    }

    struct MoveOrderChild {
        Move move;
        int heuristic;
    };
    std::vector<MoveOrderChild> childOrder;
    childOrder.reserve(moves.size());
    
    Move ttBestMove;
    if (transpositionTable.lookup(hash, entry)) {
        ttBestMove = entry.bestMove;
    }
    
    int counter = 0;
    for (const auto& m : moves) {
        if(stopSearch.load())
            return isMaximizing ? alpha : beta;
        MoveUndo undo;
        board.makeMove(m, undo);
        int h = hcalc.calculateHeuristic(board);
        board.unmakeMove(undo);
        childOrder.push_back({ m, h });
        if (++counter % 2 == 0) {
            std::this_thread::yield();
            if(stopSearch.load())
                return isMaximizing ? alpha : beta;
        }
    }
    if (ttBestMove.getSize() > 0) {
        auto it = std::find_if(childOrder.begin(), childOrder.end(),
                     [&ttBestMove](const MoveOrderChild& child) { return child.move == ttBestMove; });
        if (it != childOrder.end()) {
            std::iter_swap(childOrder.begin(), it);
        }
    }
    
    std::sort(childOrder.begin() + (ttBestMove.getSize() > 0 ? 1 : 0), childOrder.end(),
              [isMaximizing](const MoveOrderChild& a, const MoveOrderChild& b) {
                  return isMaximizing ? (a.heuristic > b.heuristic)
                                      : (a.heuristic < b.heuristic);
              });

    int originalAlpha = alpha;
    int bestScore = isMaximizing ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();
    Move bestMoveFound;
    counter = 0;
    for (auto& child : childOrder) {
        if(stopSearch.load())
            break;
        MoveUndo undo;
        board.makeMove(child.move, undo);
        int result = minimax(board, depth - 1, 3 - currentPlayer, !isMaximizing, alpha, beta);
        board.unmakeMove(undo);
        if (isMaximizing) {
            if (result > bestScore) {
                bestScore = result;
                bestMoveFound = child.move;
            }
            alpha = std::max(alpha, bestScore);
        } else {
            if (result < bestScore) {
                bestScore = result;
                bestMoveFound = child.move;
            }
            beta = std::min(beta, bestScore);
        }
        if (++counter % 2 == 0) {
            std::this_thread::yield();
            if(stopSearch.load())
                break;
        }
        if (beta <= alpha)
            break;
    }

    BoundType flag = BoundType::EXACT;
    if (bestScore <= originalAlpha)
        flag = BoundType::UPPER;
    else if (bestScore >= beta)
        flag = BoundType::LOWER;
    transpositionTable.insert(hash, { bestScore, depth, isMaximizing, flag, bestMoveFound });
    return bestScore;
}

// --------------------------------------------------------------------------
// Helper function to check if the allotted time has expired.
static inline bool timeExpired(const std::chrono::steady_clock::time_point& start,
                               double limitSeconds) {
    using namespace std::chrono;
    auto now = steady_clock::now();
    double elapsed = duration_cast<milliseconds>(now - start).count() / 1000.0;
    return (elapsed >= limitSeconds);
}

// --------------------------------------------------------------------------
// Iterative deepening search with a time limit (minus a buffer), cancellation,
// and nonblocking waits on futures to cancel outstanding tasks.
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

    // Effective time limit (total allotted time minus a safety buffer)
    double effectiveTimeLimit = static_cast<double>(timeLimitSeconds - bufferTimeSeconds);
    auto startTime = std::chrono::steady_clock::now();

    int searchDepth = 1;
    double lastDepthTimeMs = 0.0;
    const double baseGrowthFactor = 2.0; // Base multiplier (to be refined)
    
    // Maintain a history of search times (in milliseconds)
    std::vector<double> depthTimes;
    
    while (!timeExpired(startTime, effectiveTimeLimit) && searchDepth <= maxDepth) {
        // Estimate next depth time based on weighted average of the last few depths.
        if (!depthTimes.empty()) {
            // Use the last up to 3 depth times.
            int count = std::min(static_cast<int>(depthTimes.size()), 3);
            double weightedSum = 0.0;
            double weightTotal = 0.0;
            // For example, assign weights exponentially: recent depth gets highest weight.
            for (int i = 0; i < count; ++i) {
                // i = 0 -> oldest, i = count-1 -> most recent.
                // Let’s use: weight = 2^(i), so that the most recent depth (highest index) gets highest weight.
                double weight = std::pow(2.0, i);
                weightedSum += weight * depthTimes[depthTimes.size() - count + i];
                weightTotal += weight;
            }
            double averageLastDepthMs = weightedSum / weightTotal;
            // Multiply by a base growth factor
            double estimatedNextDepthTimeMs = averageLastDepthMs * baseGrowthFactor;
            
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

        // Rest of your code ordering root moves and parallel searches
        // [--- Same as your code ---]
        struct MoveOrderItem {
            Move move;
            int heuristic;
        };
        std::vector<MoveOrderItem> ordering;
        ordering.reserve(allMoves.size());
        for (const Move& mv : allMoves) {
            if (stopSearch.load())
                break;
            MoveUndo undo;
            board.makeMove(mv, undo);
            int quickScore = hcalc.calculateHeuristic(board);
            board.unmakeMove(undo);
            ordering.push_back({mv, quickScore});
        }
        std::sort(ordering.begin(), ordering.end(),
                  [](const MoveOrderItem& a, const MoveOrderItem& b) {
                      return a.heuristic > b.heuristic;
                  });
    
        // Create tasks for parallel computation (same as your original code)
        unsigned int numCores = 14;
        size_t desiredGroupCount = numCores;
        size_t totalMoves = ordering.size();
        size_t groupSize = totalMoves / desiredGroupCount;
        ThreadPool pool(numCores);
        std::vector<std::future<std::pair<int, Move>>> futures;
    
        for (size_t groupStart = 0; groupStart < totalMoves; groupStart += groupSize) {
            size_t groupEnd = std::min(groupStart + groupSize, totalMoves);
            futures.push_back(pool.enqueue([this, &ordering, groupStart, groupEnd, &board, currentPlayer, searchDepth, startTime, effectiveTimeLimit]() -> std::pair<int, Move> {
                int groupBestScore = std::numeric_limits<int>::min();
                Move groupBestMove;
                int localCounter = 0;
                for (size_t i = groupStart; i < groupEnd; ++i) {
                    if (timeExpired(startTime, effectiveTimeLimit) || stopSearch.load())
                        break;
                    const Move& candidate = ordering[i].move;
                    Board boardCopy = board;
                    MoveUndo undo;
                    boardCopy.makeMove(candidate, undo);
                    int score = minimax(boardCopy, searchDepth - 1, 1, false,
                                        std::numeric_limits<int>::min(),
                                        std::numeric_limits<int>::max());
                    boardCopy.unmakeMove(undo);
                    if (score > groupBestScore) {
                        groupBestScore = score;
                        groupBestMove = candidate;
                    }
                    if (++localCounter % 2 == 0)
                        std::this_thread::yield();
                }
                return std::make_pair(groupBestScore, groupBestMove);
            }));
        }
    
        int localBestScore = std::numeric_limits<int>::min();
        Move localBestMove;
        bool futuresTimedOut = false;
        for (auto & fut : futures) {
            while (fut.wait_for(std::chrono::milliseconds(2)) == std::future_status::timeout) {
                if (timeExpired(startTime, effectiveTimeLimit)) {
                    futuresTimedOut = true;
                    break;
                }
            }
            if (futuresTimedOut)
                break;
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
        lastDepthTimeMs = std::chrono::duration<double, std::milli>(depthEndTime - depthStartTime).count();
        // Store the elapsed time for this depth.
        depthTimes.push_back(lastDepthTimeMs);
    
        std::cout << "Depth " << searchDepth << " completed in " 
                  << lastDepthTimeMs << " ms. Best score: " 
                  << localBestScore << "\n";
    
        ++searchDepth;
    }
    
    stopSearch.store(true);
    
    std::cout << "Final best score: " << bestScoreOverall << "\n";
    std::cout << "Total nodes searched: " << nodeCount.load() << "\n";
    if (bestMoveOverall.getSize() == 0)
        std::cout << "No valid move found!\n";
    return bestMoveOverall;
}