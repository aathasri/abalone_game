#include "minimax.h"
#include "move_generator.h"
#include "heuristic_calculator.h"
#include "threadpool.h" // Ensure the filename case matches.
#include <algorithm>
#include <limits>
#include <iostream>
#include <future>
#include <vector>
#include <set>
#include <thread>

// Constructor: initializes maxDepth and clears the transposition table.
Minimax::Minimax(int maxDepth)
    : maxDepth(maxDepth)
{
    // Optionally clear the TT, even though we're not using it.
    transpositionTable.clear();
}

// The recursive minimax function with alpha-beta pruning.
int Minimax::minimax(Board& board, int depth, int currentPlayer,
                     bool isMaximizing, int alpha, int beta)
{
    HeuristicCalculator hcalc;

    // Terminal state check: assume win if opponent's marble count is <=8,
    // lose if AI's marble count is <=8.
    int p1Marbles = board.getNumPlayerOnePieces(); // Opponent
    int p2Marbles = board.getNumPlayerTwoPieces();   // AI
    if (p1Marbles <= 8)
        return 100000; // AI wins.
    if (p2Marbles <= 8)
        return -100000; // AI loses.

    // Transposition table lookup disabled for testing.
    /*
    {
        std::lock_guard<std::mutex> lock(ttMutex);
        auto it = transpositionTable.find(board);
        if (it != transpositionTable.end()) {
            const TTEntry& entry = it->second;
            if (entry.depth >= depth && entry.isMaxNode == isMaximizing)
                return entry.score;
        }
    }
    */

    if (depth == 0) {
        int score = hcalc.calculateHeuristic(board);
        // Transposition table update disabled.
        /*
        {
            std::lock_guard<std::mutex> lock(ttMutex);
            transpositionTable[board] = { score, depth, isMaximizing };
        }
        */
        return score;
    }

    // Generate moves for the current player.
    MoveGenerator moveGen;
    moveGen.generateMoves(currentPlayer, board);
    std::set<Move> moves = moveGen.getGeneratedMoves();
    if (moves.empty()) {
        int score = hcalc.calculateHeuristic(board);
        // Transposition table update disabled.
        /*
        {
            std::lock_guard<std::mutex> lock(ttMutex);
            transpositionTable[board] = { score, depth, isMaximizing };
        }
        */
        return score;
    }

    // Order moves using a quick heuristic evaluation.
    struct MoveOrderChild {
        Move move;
        int heuristic;
    };
    std::vector<MoveOrderChild> childOrder;
    childOrder.reserve(moves.size());
    for (const auto& m : moves) {
        MoveUndo undo;
        board.makeMove(m, undo);
        int h = hcalc.calculateHeuristic(board);
        board.unmakeMove(undo);
        childOrder.push_back({ m, h });
    }
    std::sort(childOrder.begin(), childOrder.end(),
              [isMaximizing](const MoveOrderChild& a, const MoveOrderChild& b) {
                  return isMaximizing ? (a.heuristic > b.heuristic)
                                      : (a.heuristic < b.heuristic);
              });

    int bestScore = isMaximizing
                    ? std::numeric_limits<int>::min()
                    : std::numeric_limits<int>::max();

    // Recursively evaluate child moves.
    for (auto& child : childOrder) {
        MoveUndo undo;
        board.makeMove(child.move, undo);
        int result = minimax(board, depth - 1, 3 - currentPlayer, !isMaximizing, alpha, beta);
        
        // --- Debug printing: Print the move being tested and its score ---
#ifdef DEBUG_MINIMAX
        std::cout << "[DEBUG] Testing move: ";
        child.move.printString();
        std::cout << " -> Heuristic: " << result << std::endl;
#endif
        // --- End Debug printing ---

        board.unmakeMove(undo);
        if (isMaximizing) {
            bestScore = std::max(bestScore, result);
            alpha = std::max(alpha, bestScore);
        } else {
            bestScore = std::min(bestScore, result);
            beta = std::min(beta, bestScore);
        }
        if (beta <= alpha)
            break; // Prune this branch.
    }

    // Transposition table update disabled.
    /*
    {
        std::lock_guard<std::mutex> lock(ttMutex);
        transpositionTable[board] = { bestScore, depth, isMaximizing };
    }
    */
    return bestScore;
}

Move Minimax::findBestMove(Board& board, int currentPlayer)
{
    HeuristicCalculator hcalc;
    Move finalBestMove;
    int finalBestScore = std::numeric_limits<int>::min();

    if (currentPlayer != 2) {
        std::cerr << "[ERROR] AI must be player 2!\n";
        return Move(); // Return an invalid move if called with the wrong player.
    }
    
    // Clear the transposition table for a fresh search.
    transpositionTable.clear();

    // Generate root moves.
    MoveGenerator moveGen;
    moveGen.generateMoves(currentPlayer, board);
    std::set<Move> allMoves = moveGen.getGeneratedMoves();
    std::cout << "Root: Generated " << allMoves.size() << " moves\n";
    if (allMoves.empty()) {
        std::cout << "No moves available at root!\n";
        return Move();
    }

    // Order moves using a quick heuristic.
    struct MoveOrderItem {
        Move move;
        int heuristic;
    };
    std::vector<MoveOrderItem> ordering;
    ordering.reserve(allMoves.size());
    for (const Move& mv : allMoves) {
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

    // Group moves into batches so that we don’t launch one task per candidate.
    unsigned int numCores = std::thread::hardware_concurrency();
    if(numCores == 0) {
        numCores = 2; // fallback
    }
    size_t totalMoves = ordering.size();
    size_t groupSize = (totalMoves + numCores - 1) / numCores; // ceil(totalMoves/numCores)
    std::cout << "number of cores:" << numCores << std::endl;
    // Create a thread pool with number of threads equal to the number of cores.
    ThreadPool pool(numCores);
    std::vector<std::future<std::pair<int, Move>>> futures;

    // For each group, create one task that evaluates that batch of moves.
    for (size_t groupStart = 0; groupStart < totalMoves; groupStart += groupSize) {
        size_t groupEnd = std::min(groupStart + groupSize, totalMoves);
        // Capture the current group indices and a copy of the board for isolation.
        futures.push_back(pool.enqueue([this, &hcalc, &ordering, groupStart, groupEnd, &board, currentPlayer]() -> std::pair<int, Move> {
            int groupBestScore = std::numeric_limits<int>::min();
            Move groupBestMove;
            // Process each candidate in this group sequentially.
            for (size_t i = groupStart; i < groupEnd; ++i) {
                const Move& candidate = ordering[i].move;
                Board boardCopy = board;  // Copy for independent evaluation
                MoveUndo undo;
                boardCopy.makeMove(candidate, undo);
                int score = minimax(boardCopy, maxDepth - 1, 1,
                                    false, // Next player minimizes (player 1)
                                    std::numeric_limits<int>::min(),
                                    std::numeric_limits<int>::max());
                // No need to unmake move on boardCopy since it's local.
                if (score > groupBestScore) {
                    groupBestScore = score;
                    groupBestMove = candidate;
                }
            }
            return std::make_pair(groupBestScore, groupBestMove);
        }));
    }

    // Collect results from all groups.
    for (auto& fut : futures) {
        auto [score, move] = fut.get();
        if (score > finalBestScore) {
            finalBestScore = score;
            finalBestMove = move;
        }
    }

    std::cout << "Best score: " << finalBestScore << "\n";
    if (finalBestMove.getSize() == 0) {
        std::cout << "No valid move found!\n";
    }
    return finalBestMove;
}