#include "move_direction.h"

const std::array<std::pair<int, int>, static_cast<int>(MoveDirection::COUNT)> directionArray = {
    std::make_pair(0, -1),   // W
    std::make_pair(1, 0),    // NW
    std::make_pair(1, 1),    // NE
    std::make_pair(0, 1),    // E
    std::make_pair(-1, 0),   // SE
    std::make_pair(-1, -1)   // SW
};

std::vector<std::pair<MoveDirection, MoveDirection>> MoveDirectionHelper::getPerpendiculars(MoveDirection dir) {
    switch (dir) {
        case MoveDirection::E:
        case MoveDirection::W:
            return {{MoveDirection::NW, MoveDirection::SW}, 
                    {MoveDirection::NE, MoveDirection::SE}};
        case MoveDirection::NW:
        case MoveDirection::SE:
            return {{MoveDirection::W, MoveDirection::E}};
        case MoveDirection::NE:
        case MoveDirection::SW:
            return {{MoveDirection::W, MoveDirection::E}};
        default:
            return {};
    }
}

std::ostream& operator<<(std::ostream& os, MoveDirection dir) {
    switch (dir) {
        case MoveDirection::W:  return os << "West";
        case MoveDirection::NW: return os << "North West";
        case MoveDirection::NE: return os << "North East";
        case MoveDirection::E:  return os << "East";
        case MoveDirection::SE: return os << "South East";
        case MoveDirection::SW: return os << "South West";
    }
    return os << "Unknown";
}