#include "board.h"
#include "move_generator.h"


#include <iostream>
#include <vector>

int main() {

    std::array<std::array<int, COLS>, ROWS> blankBoard = {{
        {-1, -1, -1, -1,  0,  0,  0,  0,  0},
        {-1, -1, -1,  0,  0,  0,  0,  0,  0},
        {-1, -1,  0,  0,  0,  0,  0,  0,  0},
        {-1,  0,  0,  0,  0,  0,  0,  0,  0},
        { 0,  0,  0,  0,  0,  0,  0,  0,  0},
        { 0,  0,  0,  0,  0,  0,  0,  0, -1},
        { 0,  0,  0,  0,  0,  0,  0, -1, -1},
        { 0,  0,  0,  0,  0,  0, -1, -1, -1},
        { 0,  0,  0,  0,  0, -1, -1, -1, -1}
    }};


    Board gameBoard(blankBoard);  // Create Board object

    
    std::string piecesString = "C5b,D5b,E4b,E5b,E6b,F5b,F6b,F7b,F8b,G6b,H6b,C3w,C4w,D3w,D4w,D6w,E7w,F4w,G5w,G7w,G8w,G9w,H7w,H8w,H9w";

    std::vector<std::string> boardPieces = Board::stringToList(piecesString);

    gameBoard.placePieces(boardPieces);

    gameBoard.printBoard(); 
 
    MoveGenerator generator;

    std::set<Move> m = generator.generateMoves(1, gameBoard);

    std::cout << m.size() << std::endl;

    MoveGenerator::printMoves(m);

      // Call method to print the board


    return 0;
}