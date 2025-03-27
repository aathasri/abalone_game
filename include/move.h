// move.h
#pragma once
#include "ai.h"
#include <vector>
#include <string>

enum MoveType {
    INLINE,
    SIDESTEP
};

struct Move {
    std::vector<int> marbles;  // 1–3 marble indices
    std::string direction;     // "NE", "SW", etc.
    MoveType type;

    bool operator==(const Move& other) const {
        return marbles == other.marbles && direction == other.direction && type == other.type;
    }
};

std::vector<Move> generateLegalMoves(const BoardArray& board, CellState player);
BoardArray applyMove(const BoardArray& board, const Move& move);
std::string moveToString(const Move& move);
