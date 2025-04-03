// minimax.h
#ifndef MINIMAX_H
#define MINIMAX_H

#include "board.h"
#include "move.h"
#include "board_generator.h"
#include "heuristic_calculator.h"
#include "move_generator.h"

#include <unordered_map>
#include <limits>
#include <algorithm>
#include <iostream>

class Minimax {
public:
    // Constructor to set the maximum search depth
    Minimax(int maxDepth);

    // Finds the best move for the current player
    Move findBestMove(const Board& board, int currentPlayer);

private:
    int maxDepth;

    struct TTEntry {
        int score;
        int depth;
        bool isMaxNode;
    };

    std::unordered_map<Board, TTEntry, BoardHasher> transpositionTable;

    // Minimax recursive evaluation
    int minimax(const Board& board, int depth, int currentPlayer, bool isMaximizing, int alpha, int beta);
};

#endif // MINIMAX_H