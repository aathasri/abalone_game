// minimax.h
#ifndef MINIMAX_H
#define MINIMAX_H

#include "board.h"
#include "threadpool.h"
#include <map>
#include <mutex>


struct TTEntry {
    int score;
    int depth;
    bool isMaxNode;
};

class Minimax {
public:
    Minimax(int maxDepth);
    Move findBestMove(Board& board, int currentPlayer);
    int minimax(Board& board, int depth, int currentPlayer,
                bool isMaximizing, int alpha, int beta);
private:
    int maxDepth;

    // Transposition table is shared among threads so we need a mutex.
    std::map<Board, TTEntry> transpositionTable;
    std::mutex ttMutex; // Protects transpositionTable
};

#endif // MINIMAX_H