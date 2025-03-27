#ifndef MOVE_H
#define MOVE_H

#include "move_type.h"
#include "move_direction.h"

#include <iostream>
#include <array>
#include <vector>

struct Move {
    std::array<std::pair<int, int>, 3> positions;
    int size = 0;
    MoveDirection direction;

    Move(MoveDirection dir);

    void addPosition(int col, int row);
    void copyMovePositions(Move m);
    bool operator<(const Move& other) const;
    void printString() const;
};

class MoveValidator {
public:
    static bool ValidateMove(const MoveType type, const std::vector<std::string>& pieces, const MoveDirection direction);
};

#endif // MOVE_H