#ifndef MOVE_DIRECTION_H
#define MOVE_DIRECTION_H

#include <utility>   // For std::pair
#include <array>
#include <ostream>   // For std::ostream
#include <vector>

enum class MoveDirection {
    W,
    NW,
    NE,
    E,
    SE,
    SW,
    COUNT
};

namespace MoveDirectionHelper {
    std::vector<std::pair<MoveDirection, MoveDirection>> getPerpendiculars(MoveDirection dir);
}

// Declaration only — definition goes in .cpp
std::ostream& operator<<(std::ostream& os, MoveDirection dir);

// Option 1: Declare externally and define in .cpp
extern const std::array<std::pair<int, int>, static_cast<int>(MoveDirection::COUNT)> directionArray;

#endif // MOVE_DIRECTION_H