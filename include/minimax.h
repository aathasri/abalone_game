#ifndef MINIMAX_H
#define MINIMAX_H

#include "board.h"
#include "move.h"
#include "heuristic_calculator.h"
#include <memory>
#include <set>
#include <map>
#include <vector>

struct TTEntry {
    int score;
    int depth;
    bool isMaxNode;
};

class Minimax {
public:
    Minimax(int maxDepth);
    Move findBestMove(const Board& board, int currentPlayer);

private:
    int minimax(const Board& board, int depth, int currentPlayer,
                bool isMaximizing, int alpha, int beta);

    int maxDepth;
    std::unordered_map<Board, TTEntry, BoardHasher> transpositionTable;
};

#endif