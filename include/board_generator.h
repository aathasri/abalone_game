#ifndef BOARD_GENERATOR_H
#define BOARD_GENERATOR_H

#include "board.h"
#include "move.h"
#include "move_generator.h"

#include <iostream>
#include <array>
#include <vector>
#include <set>

class BoardGenerator {

private:

std::vector<Board> GeneratedBoards;

public:
    void generateBoards(Board& currBoard, std::set<Move>& moves);

    std::vector<Board> getGeneratedBoards();

    void printBoards();

private:
    
};

#endif // BOARD_GENERATOR_H