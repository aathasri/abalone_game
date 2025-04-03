#ifndef BOARD_H
#define BOARD_H

#include <iostream>
#include <array>
#include <vector>
#include <map>
#include <sstream>
#include <random>
#include <chrono>

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
    static std::map<std::pair<int, int>, int> coordToIndex; // <-- initialize when creating board for different board configuations
    // Maps back from index to coordinate (to use in logic)
    static std::vector<std::pair<int, int>> indexToCoord; // <-- initialize when creating board for different board configuations

    static std::array<std::array<std::array<uint64_t, 3>, COLS>, ROWS> zobristTable;
    static bool zobristInitialized;

    static bool staticMappingInitialized;


public:
    // Constructor
    Board(const std::array<std::array<int, COLS>, ROWS>& gameboard);

    // Getters
    // TODO - MAKE GETTERS CONSTANT
    const std::array<std::array<int, COLS>, ROWS>& getBoard() const;
    const std::vector<std::vector<bool>>& getAdjacencyMatrix() const;
    static const std::map<std::pair<int, int>, int>& getCoordToIndex();
    const std::vector<std::pair<int, int>>& getIndexToCoord() const;
    const int& getNumPlayerOnePieces() const;
    const int& getNumPlayerTwoPieces() const;
    

    // Function to place pieces on the board
    void placePieces(const std::vector<std::string>& pieces);

    // Apply a valid move to the board
    void applyMove(const Move& move);

    // Validate a position on board
    const bool validPosition(const int letterIndex, const int numberIndex) const;

    // For Hashing - transposition table
    static void initZobrist();
    uint64_t getZobristHash() const;
    bool operator==(const Board& other) const;


    // Printing
    void printPieces() const;
    void printMatrix() const;
    void printBoard() const;

    // TEMP (For Testing) - Creates a list of pieces from a string
    static std::vector<std::string> stringToList(const std::string& pieces);

private:

    void moveOnePiece(const Move& move);

    void movePiecesInline(const Move& move);

    void movePiecesSideStep(const Move& move);

    void initializeAdjacencyMatrix(); // builds the adjacency matrix

    void updateAdjacencyForMove(const std::vector<std::pair<int, int>>& oldPositions,
                                const std::vector<std::pair<int, int>>& newPositions);

    // helper to remove/update a node's connections
    void updateAdjacencyAt(int idx);

    void initializeStaticAdjacencyMapping();

};

struct BoardHasher {
    std::size_t operator()(const Board& b) const {
        return static_cast<std::size_t>(b.getZobristHash());
    }
};

#endif // BOARD_H