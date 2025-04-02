#ifndef BOARD_H
#define BOARD_H

#include <iostream>
#include <array>
#include <vector>
#include <map>
#include <sstream>

#include "move.h"

const int ROWS = 9;
const int COLS = 9;

const std::map<char, int> playerColourMap = {
    {'b', 1},
    {'w', 2}
};

const std::map<int, char> printMap = {
    {playerColourMap.at('b'), 'b'},
    {playerColourMap.at('w'), 'w'},
    {0, '0'},
    {-1, 'X'}
};

class Board {

private:
    std::array<std::array<int, COLS>, ROWS> gameboard;

    int numPlayerOnePieces;
    int numPlayerTwoPieces;

    // The adjacency matrix: adjacencyMatrix[i][j] is true if i and j are adjacent
    std::vector<std::vector<bool>> adjacencyMatrix;

    // Maps a board coordinate (i, j) to a linear index (for the matrix)
    std::map<std::pair<int, int>, int> coordToIndex;

    // Maps back from index to coordinate (to use in logic)
    std::vector<std::pair<int, int>> indexToCoord;

public:
    // Constructor
    Board(const std::array<std::array<int, COLS>, ROWS>& gameboard);

    // Getters
    // TODO - MAKE GETTERS CONSTANT
    const std::array<std::array<int, COLS>, ROWS>& getBoard()const;
    const std::vector<std::vector<bool>>& getAdjacencyMatrix() const;
    const std::vector<std::pair<int, int>>& getIndexToCoord() const;
    int getNumPlayerOnePieces() const;
    int getNumPlayerTwoPieces() const;

    // Function to place pieces on the board
    void placePieces(const std::vector<std::string>& pieces);

    // Apply a valid move to the board
    void applyMove(const Move move);

    // Validate a position on board
    bool validPosition(const int letterIndex, const int numberIndex);

    // Printing
    void printPieces() const;
    void printMatrix() const;
    void printBoard() const;

    // TEMP (For Testing) - Creates a list of pieces from a string
    static std::vector<std::string> stringToList(const std::string& pieces);

private:

    void moveOnePiece(const Move move);

    void movePiecesInline(const Move move);

    void movePiecesSideStep(const Move move);

    void initializeAdjacencyMatrix(); // builds the adjacency matrix

    void updateAdjacencyForMove(const std::vector<std::pair<int, int>>& oldPositions,
                                const std::vector<std::pair<int, int>>& newPositions);

    // helper to remove/update a node's connections
    void updateAdjacencyAt(int idx);

    // bool isHorizintal(const std::vector<std::string>& pieces);

    // bool isBackwardSloping(const std::vector<std::string>& pieces);

    // bool isForwardSloping(const std::vector<std::string>& pieces);

    // // Move a piece E
    // void moveE(std::vector<std::string>& pieces);

    // // Move a piece NE
    // void moveNE(std::vector<std::string>& pieces);

    // // Move a piece NW
    // void moveNW(std::vector<std::string>& pieces);

    // // Move a piece W
    // void moveW(std::vector<std::string>& pieces);
    
    // // Move a piece SW
    // void moveSW(std::vector<std::string>& pieces);

    // // Move a piece SE
    // void moveSE(std::vector<std::string>& pieces);

    // void moveOnePiece(const std::string& move);

    // void movePiecesInline(const std::vector<std::string>& pieces, const std::string& move);

    // void movePieceSideStep(const std::vector<std::string>& pieces, const std::string& move);

    // void sortMovePieces(std::vector<std::string>& pieces);
};

#endif // BOARD_H