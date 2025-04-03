#ifndef MOVE_GENERATOR_H
#define MOVE_GENERATOR_H

#include "board.h"
#include "move.h"

#include <iostream>
#include <array>
#include <vector>
#include <set>

class MoveGenerator {

private:

std::set<Move> generated_moves;

public:
    // Constructor
    // MoveGenerator();

    // Generate all valid moves given a board
    void generateMoves(int currPlayer, const Board& currentBoard);

    // store the pieces, their orientation, the direction
    const std::set<Move>& getGeneratedMoves() const;

    // Converts Pieces and Direciton into Move Notation
    // static std::string encodeNotation(std::vector<std::string>& pieces, std::string& direction);

    void printMoves() const;

private:
    bool potentialPushPositionValid(int i, int j, const Board& currentBoard);
};

#endif // MOVE_GENERATOR_H