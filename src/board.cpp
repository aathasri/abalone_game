#include "board.h"
#include <iostream>
#include <unordered_map>

BoardArray loadStartingBoard(const std::string& startingPosition) {
    BoardArray board;
    board.fill(EMPTY);

    const std::vector<std::string> standardBlack = {
        "A1", "A2", "A3", "A4", "A5",
        "B1", "B2", "B3", "B4", "B5",
        "C1", "C2", "C3"
    };

    const std::vector<std::string> standardWhite = {
        "I5", "I6", "I7", "I8", "I9",
        "H5", "H6", "H7", "H8", "H9",
        "G7", "G8", "G9"
    };

    const std::vector<std::string> belgianBlack = {
        "A3", "A4", "A5", "B3", "B4", "C4",
        "C5", "D5", "E5"
    };

    const std::vector<std::string> belgianWhite = {
        "I7", "I8", "I9", "H7", "H8", "G7",
        "G6", "F6", "E5"
    };

    const std::vector<std::string> germanBlack = {
        "A1", "A2", "A3", "B1", "B2", "C1",
        "C2", "D1", "E1"
    };

    const std::vector<std::string> germanWhite = {
        "I9", "I8", "I7", "H9", "H8", "G9",
        "G8", "F9", "E9"
    };

    auto placeMarbles = [&](const std::vector<std::string>& positions, CellState color) {
        for (const auto& pos : positions) {
            int idx = posToIdx.at(pos);
            board[idx] = color;
        }
    };

    if (startingPosition == "standard") {
        placeMarbles(standardBlack, BLACK);
        placeMarbles(standardWhite, WHITE);
    } else if (startingPosition == "belgian_daisy") {
        placeMarbles(belgianBlack, BLACK);
        placeMarbles(belgianWhite, WHITE);
    } else if (startingPosition == "german_daisy") {
        placeMarbles(germanBlack, BLACK);
        placeMarbles(germanWhite, WHITE);
    }

    return board;
}

void printBoard(const BoardArray& board) {
    std::cout << "Board (linearized):\n";
    for (int i = 0; i < 61; ++i) {
        char c = (board[i] == BLACK) ? 'B' : (board[i] == WHITE) ? 'W' : '.';
        std::cout << c << ' ';
        if ((i + 1) % 10 == 0) std::cout << '\n';
    }
    std::cout << "\n";
}
