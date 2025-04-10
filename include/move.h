#ifndef MOVE_H
#define MOVE_H

#include <iostream>
#include <array>
#include <vector>
#include <utility>
#include <ostream>

enum class MoveDirection {
    W,
    NW,
    NE,
    E,
    SE,
    SW,
    COUNT
};

std::ostream& operator<<(std::ostream& os, MoveDirection dir);

class DirectionHelper {
private:
    static const std::array<std::pair<int, int>, static_cast<int>(MoveDirection::COUNT)> directionArray;
public:
    static const std::pair<int, int>& getDelta(MoveDirection dir);
    static const std::pair<int, int>& getDelta(int dir);
    static std::vector<std::pair<MoveDirection, MoveDirection>> getPerpendiculars(MoveDirection dir);
};

enum class MoveType {
    SIDESTEP,
    INLINE,
    COUNT
};

class Move {
    friend class Board;  // If Board needs access to private members.
private:
    MoveType type;
    MoveDirection direction;
    int size;
    std::array<std::pair<int, int>, 3> positions;

public:
    Move();
    Move(MoveType type, MoveDirection direction);

    void addPosition(int col, int row);
    void copyMovePositions(const Move& other);
    bool operator<(const Move& other) const;

    // New: Equality operator.
    bool operator==(const Move& other) const {
        if (type != other.type)
            return false;
        if (direction != other.direction)
            return false;
        if (size != other.size)
            return false;
        for (int i = 0; i < size; ++i) {
            if (positions[i] != other.positions[i])
                return false;
        }
        return true;
    }
    
    // Optional: Inequality operator.
    bool operator!=(const Move& other) const {
        return !(*this == other);
    }

    const int& getSize() const { return size; }
    const MoveType& getType() const { return type; }
    const MoveDirection& getDirection() const { return direction; }
    const std::pair<int, int>& getPosition(int index) const { return positions[index]; }

    void printString() const;

    friend std::ostream& operator<<(std::ostream& os, const Move& move);
};

#endif // MOVE_H