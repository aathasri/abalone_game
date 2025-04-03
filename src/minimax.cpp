#include "minimax.h"


Minimax::Minimax(int maxDepth) : maxDepth(maxDepth) {}

Move Minimax::findBestMove(const Board& board, int currentPlayer) {
    Move bestMove;
    int bestScore = std::numeric_limits<int>::min();

    MoveGenerator moveGen;
    moveGen.generateMoves(currentPlayer, board);
    std::set<Move> allMoves = moveGen.getGeneratedMoves();

    std::vector<std::pair<Move, int>> moveScores;
    for (const Move& move : allMoves) {
        auto tempBoard = std::make_unique<Board>(board);
        tempBoard->applyMove(move);
        int score = HeuristicCalculator::calculateHeuristic(*tempBoard);
        moveScores.emplace_back(move, score);
    }

    std::sort(moveScores.begin(), moveScores.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    for (const auto& [move, _] : moveScores) {
        auto child = std::make_unique<Board>(board);
        child->applyMove(move);

        int score = minimax(*child, maxDepth - 1, 3 - currentPlayer, false,
                            std::numeric_limits<int>::min(), std::numeric_limits<int>::max());

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }
    }

    return bestMove;
}

int Minimax::minimax(const Board& board, int depth, int currentPlayer, bool isMaximizing,
                     int alpha, int beta) {
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

    std::vector<std::pair<std::unique_ptr<Board>, int>> scoredChildren;
    for (const auto& move : moves) {
        auto child = std::make_unique<Board>(board);
        child->applyMove(move);
        int h = HeuristicCalculator::calculateHeuristic(*child);
        scoredChildren.emplace_back(std::move(child), h);
    }

    std::sort(scoredChildren.begin(), scoredChildren.end(),
              [isMaximizing](const auto& a, const auto& b) {
                  return isMaximizing ? a.second > b.second : a.second < b.second;
              });

    int bestScore = isMaximizing ? std::numeric_limits<int>::min()
                                 : std::numeric_limits<int>::max();

    for (const auto& [child, _] : scoredChildren) {
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
