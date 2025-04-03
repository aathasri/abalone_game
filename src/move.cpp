#include "move.h"
#include <algorithm>

/** 
 * --------------------------------------------------------------------------------
 * Move Direction
 * --------------------------------------------------------------------------------
 */ 

// Print direction enum
std::ostream& operator<<(std::ostream& os, MoveDirection dir) {
    switch (dir) {
        case MoveDirection::W:  return os << "West";
        case MoveDirection::NW: return os << "North West";
        case MoveDirection::NE: return os << "North East";
        case MoveDirection::E:  return os << "East";
        case MoveDirection::SE: return os << "South East";
        case MoveDirection::SW: return os << "South West";
        default:                 return os << "Unknown";
    }
}



/** 
 * --------------------------------------------------------------------------------
 * Direction Helper
 * --------------------------------------------------------------------------------
 */ 

const std::array<std::pair<int, int>, static_cast<int>(MoveDirection::COUNT)> DirectionHelper::directionArray = {
    std::make_pair(0, -1),   // W
    std::make_pair(-1, 0),    // NW
    std::make_pair(-1, 1),    // NE
    std::make_pair(0, 1),    // E
    std::make_pair(1, 0),   // SE
    std::make_pair(1, -1)   // SW
};

const std::pair<int, int>& DirectionHelper::getDelta(MoveDirection dir) {
    return directionArray[static_cast<int>(dir)];
}

const std::pair<int, int>& DirectionHelper::getDelta(int dir) {
    return directionArray[dir];
}

std::vector<std::pair<MoveDirection, MoveDirection>> DirectionHelper::getPerpendiculars(MoveDirection dir) {
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



/** 
 * --------------------------------------------------------------------------------
 * Move Type
 * --------------------------------------------------------------------------------
 */ 

// Print Move Type Enum
std::ostream& operator<<(std::ostream& os, MoveType type) {
    switch (type) {
        case MoveType::SIDESTEP:    return os << "s";
        case MoveType::INLINE:      return os << "i";
        default:                    return os << "Unknown";
    }
}



/** 
 * --------------------------------------------------------------------------------
 * Move Class
 * --------------------------------------------------------------------------------
 */ 

 Move::Move() {}

 Move::Move(MoveType typ, MoveDirection dir)
 : type(typ), direction(dir), size(0), positions{} {}

 void Move::addPosition(int col, int row) {
    if (size >= 3) return;

    std::pair<int, int> newPos = {col, row};

    auto insertAt = size; // default to append at end

    if (type == MoveType::INLINE) {
        // Inline logic: use directional projection
        auto [dx, dy] = DirectionHelper::getDelta(direction);

        auto projection = [dx, dy](std::pair<int, int> pos) {
            return pos.first * dx + pos.second * dy;
        };

        while (insertAt > 0 && projection(positions[insertAt - 1]) < projection(newPos)) {
            positions[insertAt] = positions[insertAt - 1];
            --insertAt;
        }
    } else if (type == MoveType::SIDESTEP) {
        // Sidestep logic: sort lexicographically (row, col)
        while (insertAt > 0 && positions[insertAt - 1] > newPos) {
            positions[insertAt] = positions[insertAt - 1];
            --insertAt;
        }
    }

    positions[insertAt] = newPos;
    ++size;
}

void Move::copyMovePositions(const Move& m) {
    positions = m.positions;
    size = m.size;
}

bool Move::operator<(const Move& other) const {
    if (type != other.type)
        return type < other.type;

    if (size != other.size)
        return size < other.size;

    if (direction != other.direction)
        return direction < other.direction;

    for (int i = 0; i < size; ++i) {
        if (positions[i] != other.positions[i])
            return positions[i] < other.positions[i];
    }

    return false;
}

void Move::printString() const {
    std::cout << type << " - ";
    for (int i = 0; i < size; ++i) {
        std::cout << "(" << positions[i].first << "," << positions[i].second << ")";
        if (i < size - 1) std::cout << ", ";
    }
    std::cout << " - " << direction << std::endl;
}

std::ostream& operator<<(std::ostream& os, const Move& move) {
    os << move.type << " - ";
    for (int i = 0; i < move.size; ++i) {
        os << "(" << move.positions[i].first << "," << move.positions[i].second << ")";
        if (i < move.size - 1) os << ", ";
    }
    os << " - " << move.direction;
    return os;
}