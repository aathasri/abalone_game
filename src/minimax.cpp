#include "minimax.h"

Minimax::Minimax(int maxDepth) : maxDepth(maxDepth) {}

Move Minimax::findBestMove(const Board& board, int currentPlayer) {
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

    std::vector<MoveNode> moveNodes;
    for (const Move& move : allMoves) {
        auto tempBoard = std::make_unique<Board>(board);
        tempBoard->applyMove(move);
        int score = HeuristicCalculator::calculateHeuristic(*tempBoard);
        moveNodes.push_back({move, std::move(tempBoard), score});
    }

    std::sort(moveNodes.begin(), moveNodes.end(),
              [](const MoveNode& a, const MoveNode& b) { return a.heuristic > b.heuristic; });

    for (auto& node : moveNodes) {
        int score = minimax(*node.board, maxDepth - 1, 3 - currentPlayer, false,
                            std::numeric_limits<int>::min(), std::numeric_limits<int>::max());

        if (score > bestScore) {
            bestScore = score;
            bestMove = node.move;
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

    struct ChildNode {
        std::unique_ptr<Board> board;
        int heuristic;
    };

    std::vector<ChildNode> children;
    for (const auto& move : moves) {
        auto child = std::make_unique<Board>(board);
        child->applyMove(move);
        int h = HeuristicCalculator::calculateHeuristic(*child);
        children.push_back({std::move(child), h});
    }

    std::sort(children.begin(), children.end(),
              [isMaximizing](const ChildNode& a, const ChildNode& b) {
                  return isMaximizing ? a.heuristic > b.heuristic : a.heuristic < b.heuristic;
              });

    int bestScore = isMaximizing ? std::numeric_limits<int>::min()
                                 : std::numeric_limits<int>::max();

    for (auto& child : children) {
        int result = minimax(*child.board, depth - 1, 3 - currentPlayer, !isMaximizing, alpha, beta);

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
