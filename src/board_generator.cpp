#include "board_generator.h"

void BoardGenerator::generateBoards(Board& currBoard, std::set<Move>& moves)
{
    for (Move m : moves) {
        Board b = currBoard;
        b.applyMove(m);
        GeneratedBoards.push_back(b);
    }
}

std::vector<Board> BoardGenerator::getGeneratedBoards()
{
    return GeneratedBoards;
}

void BoardGenerator::printBoards()
{
    for (Board b : GeneratedBoards) {
        b.printPieces();
    }
}
