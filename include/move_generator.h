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
    void generateMoves(int turnColour, Board& currentBoard);

    // store the pieces, their orientation, the direction
    std::set<Move> getGeneratedMoves();

    // Converts Pieces and Direciton into Move Notation
    // static std::string encodeNotation(std::vector<std::string>& pieces, std::string& direction);

    void printMoves();

private:
    bool potentialPushPositionValid(int i, int j, Board& currentBoard);
};

#endif // MOVE_GENERATOR_H