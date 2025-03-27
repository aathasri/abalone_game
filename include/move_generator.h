#ifndef MOVE_GENERATOR_H
#define MOVE_GENERATOR_H

#include "board.h"
#include "move.h"
#include "move_direction.h"

#include <iostream>
#include <array>
#include <vector>
#include <set>

class MoveGenerator {

public:
    // Constructor
    // MoveGenerator();

    // Generate all valid moves given a board
    std::set<Move> generateMoves(int turnColour, Board currentBoard);

    // store the pieces, their orientation, the direction


    // Converts Pieces and Direciton into Move Notation
    // static std::string encodeNotation(std::vector<std::string>& pieces, std::string& direction);

    static void printMoves(const std::set<Move>& m);

private:
    // find pieces that are beside empty space or opponent
    

};

#endif // MOVE_GENERATOR_H