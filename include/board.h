#ifndef BOARD_H
#define BOARD_H

#include <iostream>
#include <array>
#include <vector>
#include <map>

const int ROWS = 9;
const int COLS = 9;

const std::map<char, int> playerCharMap = {
    {'b', 1},
    {'w', 2}
};

const std::map<char, int> printMap = {
    {1, 'b'},
    {2, 'w'},
    {0, '0'},
    {-1, 'X'}
};

class Board {
private:
    std::array<std::array<int, COLS>, ROWS> gameboard;

public:
    // Constructor
    Board(const std::array<std::array<int, COLS>, ROWS>& gameboard);

    std::array<std::array<int, COLS>, ROWS>& getBoard();


    // Function to place pieces on the board
    void placePieces(const std::vector<std::string>& pieces);

    // Move a piece E
    void moveE(std::vector<std::string>& pieces);

    // Move a piece NE
    void moveNE(std::vector<std::string>& pieces);

    // Move a piece NW
    void moveNW(std::vector<std::string>& pieces);

    // Move a piece W
    void moveW(std::vector<std::string>& pieces);
    
    // Move a piece SW
    void moveSW(std::vector<std::string>& pieces);

    // Move a piece SE
    void moveSE(std::vector<std::string>& pieces);

    // Function to print the matrix
    void printMatrix() const;

    // Function to print the board
    void printBoard() const;
    
    // Function to turn board into string or pieces
    std::string toString() const;

    bool validPosition(const int letterIndex, const int numberIndex);

    // Creates a list of pieces from a string
    static std::vector<std::string> stringToList(const std::string& pieces);

private:

    bool isHorizintal(const std::vector<std::string>& pieces);

    bool isBackwardSloping(const std::vector<std::string>& pieces);

    bool isForwardSloping(const std::vector<std::string>& pieces);

    void moveOnePiece(const std::string& move);

    void movePiecesInline(const std::vector<std::string>& pieces, const std::string& move);

    void movePieceSideStep(const std::vector<std::string>& pieces, const std::string& move);

    void sortMovePieces(std::vector<std::string>& pieces);
};

#endif // BOARD_H