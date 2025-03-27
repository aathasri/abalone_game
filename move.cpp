#include "move.h"
#include <algorithm>

Move::Move(MoveDirection dir) : positions{}, size(0), direction(dir) {}

void Move::addPosition(int col, int row) {
    if (size < 3) {
        positions[size++] = {col, row};
    }
}

void Move::copyMovePositions(Move m) {
    positions = m.positions;
    size = m.size;
}

bool Move::operator<(const Move& other) const {
    if (size != other.size)
        return size < other.size;

    if (direction != other.direction)
        return direction < other.direction;

    std::array<std::pair<int, int>, 3> sortedA = positions;
    std::array<std::pair<int, int>, 3> sortedB = other.positions;
    std::sort(sortedA.begin(), sortedA.begin() + size);
    std::sort(sortedB.begin(), sortedB.begin() + other.size);

    for (int i = 0; i < size; ++i) {
        if (sortedA[i] != sortedB[i])
            return sortedA[i] < sortedB[i];
    }

    return false;
}

void Move::printString() const {
    std::cout << direction << ": ";
    for (int i = 0; i < size; ++i) {
        std::cout << "(" << positions[i].first << "," << positions[i].second << ")";
        if (i < size - 1) std::cout << ", ";
    }
    std::cout << std::endl;
}