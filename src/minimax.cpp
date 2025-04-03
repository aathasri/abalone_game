#include "minimax.h"
#include "heuristic_calculator.h"
#include "move_generator.h"
#include "board_generator.h"
#include <iostream>
#include <limits>
#include <algorithm>
#include <memory>

Minimax::Minimax(int maxDepth) : maxDepth(maxDepth) {}

Move Minimax::findBestMove(const Board& board, int currentPlayer) {
    std::cout << "[Minimax] Starting search for best move (depth " << maxDepth << ") for player " << currentPlayer << std::endl;

    Move bestMove;
    int bestScore = std::numeric_limits<int>::min();

    MoveGenerator moveGen;
    moveGen.generateMoves(currentPlayer, board);
    std::set<Move> allMoves = moveGen.getGeneratedMoves();
    std::cout << "[Minimax] Generated " << allMoves.size() << " moves" << std::endl;

    BoardGenerator generator;
    generator.generateBoards(board, allMoves);
    const auto& children = generator.getGeneratedBoards();
    std::cout << "[Minimax] Generated " << children.size() << " child boards" << std::endl;

    auto moveIt = allMoves.begin();
    for (size_t i = 0; i < children.size() && moveIt != allMoves.end(); ++i, ++moveIt) {
        int score = minimax(*children[i], maxDepth - 1, 3 - currentPlayer, false,
                            std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
        if (score > bestScore) {
            bestScore = score;
            bestMove = *moveIt;
        }
    }

    return bestMove;
}

int Minimax::minimax(const Board& board, int depth, int currentPlayer, bool isMaximizing,
                     int alpha, int beta) {
    std::cout << "[Minimax] Entering depth " << depth << " | Player: " << currentPlayer
              << " | Maximizing: " << isMaximizing << std::endl;

    auto it = transpositionTable.find(board);
    if (it != transpositionTable.end()) {
        const TTEntry& entry = it->second;
        if (entry.depth >= depth && entry.isMaxNode == isMaximizing) {
            return entry.score;
        }
    }

    if (depth == 0) {
        int score = HeuristicCalculator::calculateHeuristic(board);
        transpositionTable[board] = {score, depth, isMaximizing};
        return score;
    }

    MoveGenerator moveGen;
    moveGen.generateMoves(currentPlayer, board);
    std::set<Move> moves = moveGen.getGeneratedMoves();

    if (moves.empty()) {
        int score = HeuristicCalculator::calculateHeuristic(board);
        transpositionTable[board] = {score, depth, isMaximizing};
        return score;
    }

    BoardGenerator generator;
    generator.generateBoards(board, moves);
    const auto& children = generator.getGeneratedBoards();

    int bestScore = isMaximizing ? std::numeric_limits<int>::min()
                                 : std::numeric_limits<int>::max();

    for (const auto& child : children) {
        int result = minimax(*child, depth - 1, 3 - currentPlayer, !isMaximizing, alpha, beta);

        if (isMaximizing) {
            bestScore = std::max(bestScore, result);
            alpha = std::max(alpha, bestScore);
        } else {
            bestScore = std::min(bestScore, result);
            beta = std::min(beta, bestScore);
        }

        if (beta <= alpha) break;
    }

    transpositionTable[board] = {bestScore, depth, isMaximizing};
    return bestScore;
}