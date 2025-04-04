#include "minimax.h"
#include "move_generator.h"
#include "heuristic_calculator.h"
#include <algorithm>
#include <limits>
#include <memory>

Minimax::Minimax(int maxDepth)
    : maxDepth(maxDepth)
{
}

Move Minimax::findBestMove(const Board& board, int currentPlayer) {
    // Create a local instance of your heuristic
    HeuristicCalculator hcalc;

    Move bestMove;
    int bestScore = std::numeric_limits<int>::min();

    MoveGenerator moveGen;
    moveGen.generateMoves(currentPlayer, board);
    std::set<Move> allMoves = moveGen.getGeneratedMoves();

    struct MoveNode {
        Move move;
        std::unique_ptr<Board> board;
        int heuristic;
    };

    // Generate child boards and evaluate them
    std::vector<MoveNode> moveNodes;
    for (const Move& move : allMoves) {
        auto tempBoard = std::make_unique<Board>(board);
        tempBoard->applyMove(move);

        // Use the instance 'hcalc' to compute the heuristic
        int score = hcalc.calculateHeuristic(*tempBoard);

        moveNodes.push_back({move, std::move(tempBoard), score});
    }

    // Sort them in descending order of heuristic
    std::sort(moveNodes.begin(), moveNodes.end(),
              [](const MoveNode& a, const MoveNode& b) {
                  return a.heuristic > b.heuristic;
              });

    for (auto& node : moveNodes) {
        // Evaluate deeper with minimax
        int score = minimax(*node.board, maxDepth - 1, 3 - currentPlayer,
                            /*isMaximizing=*/false,
                            std::numeric_limits<int>::min(),
                            std::numeric_limits<int>::max());
        if (score > bestScore) {
            bestScore = score;
            bestMove = node.move;
        }
    }

    return bestMove;
}

int Minimax::minimax(const Board& board, int depth, int currentPlayer,
                     bool isMaximizing, int alpha, int beta) 
{
    // Create a local instance of your heuristic
    HeuristicCalculator hcalc;

    // Check the transposition table
    auto it = transpositionTable.find(board);
    if (it != transpositionTable.end()) {
        const TTEntry& entry = it->second;
        if (entry.depth >= depth && entry.isMaxNode == isMaximizing) {
            return entry.score;
        }
    }

    // Base case
    if (depth == 0) {
        int score = hcalc.calculateHeuristic(board);
        transpositionTable[board] = {score, depth, isMaximizing};
        return score;
    }

    // Generate possible moves
    MoveGenerator moveGen;
    moveGen.generateMoves(currentPlayer, board);
    std::set<Move> moves = moveGen.getGeneratedMoves();

    // If no moves, evaluate heuristic
    if (moves.empty()) {
        int score = hcalc.calculateHeuristic(board);
        transpositionTable[board] = {score, depth, isMaximizing};
        return score;
    }

    // Build child boards
    struct ChildNode {
        std::unique_ptr<Board> board;
        int heuristic; // might not be strictly necessary if we reorder them
    };

    std::vector<ChildNode> children;
    for (const auto& move : moves) {
        auto child = std::make_unique<Board>(board);
        child->applyMove(move);
        // Evaluate child board quickly for sorting
        int h = hcalc.calculateHeuristic(*child);
        children.push_back({std::move(child), h});
    }

    // Sort children so we expand in best-first order
    std::sort(children.begin(), children.end(),
              [isMaximizing](const ChildNode& a, const ChildNode& b) {
                  return isMaximizing ? (a.heuristic > b.heuristic)
                                      : (a.heuristic < b.heuristic);
              });

    int bestScore = isMaximizing ? std::numeric_limits<int>::min()
                                 : std::numeric_limits<int>::max();

    // Recurse
    for (auto& child : children) {
        int result = minimax(*child.board, depth - 1, 3 - currentPlayer,
                             !isMaximizing, alpha, beta);

        if (isMaximizing) {
            bestScore = std::max(bestScore, result);
            alpha = std::max(alpha, bestScore);
        } else {
            bestScore = std::min(bestScore, result);
            beta = std::min(beta, bestScore);
        }

        // Alpha-Beta cutoff
        if (beta <= alpha) {
            break;
        }
    }

    // Store to transposition table
    transpositionTable[board] = {bestScore, depth, isMaximizing};
    return bestScore;
}