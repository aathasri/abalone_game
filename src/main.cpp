#include "board.h"
#include "move.h"
#include "move_generator.h"
#include "board_generator.h"
#include "heuristic_calculator.h"
#include "minimax.h"  // include your minimax class

#include <iostream>
#include <vector>
#include <set>
#include <chrono>

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

    std::cout << "Current Board:\n";
    gameBoard.printBoard();

    // Run Minimax to get the best move
    Minimax ai(4);
    int currentPlayer = 1;

    auto start = std::chrono::high_resolution_clock::now();

    Move bestMove = ai.findBestMove(gameBoard, currentPlayer);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "\nBest Move Selected:\n";
    bestMove.printString();

    std::cout << "Minimax took " << duration.count() << " ms\n";

    // Apply move and show new board
    gameBoard.applyMove(bestMove);

    std::cout << "\nBoard After Move:\n";
    gameBoard.printBoard();

    return 0;
}