#ifndef BOARD_H
#define BOARD_H

#include <iostream>
#include <array>
#include <vector>
#include <map>
#include <sstream>
#include <random>
#include <chrono>
#include <tuple>

#include "move.h"

const int ROWS = 9;
const int COLS = 9;

// TODO: Adjust based on actual color mapping in your application
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
    // --------------------------------------------------------------------------------
    // Data Members
    // --------------------------------------------------------------------------------
    std::array<std::array<int, COLS>, ROWS> gameboard;

    int numPlayerOnePieces;
    int numPlayerTwoPieces;

    // Instead of a bool adjacency matrix, we store adjacency as a list of neighbors:
    // adjacencyList[i] is a list of indices adjacent to cell i.
    std::vector<std::vector<int>> adjacencyList;

    // Maps from (row, col) to linear index i
    std::map<std::pair<int, int>, int> coordToIndex;
    // Maps back from index i to (row, col)
    std::vector<std::pair<int, int>> indexToCoord;

    // For Zobrist hashing
    static std::array<std::array<std::array<uint64_t, 3>, COLS>, ROWS> zobristTable;
    static bool zobristInitialized;

public:
    // --------------------------------------------------------------------------------
    // Constructor
    // --------------------------------------------------------------------------------
    // We now require adjacencyList, coordToIndex, indexToCoord from outside.
    // We assume the "shape" of the board won't change, so we only build them once.
    Board(const std::array<std::array<int, COLS>, ROWS>& matrix,
          const std::vector<std::vector<int>>& adjacencyList,
          const std::map<std::pair<int, int>, int>& coordToIndex,
          const std::vector<std::pair<int, int>>& indexToCoord);

    // --------------------------------------------------------------------------------
    // Static method to produce adjacency data (list + maps) for a given board shape.
    // --------------------------------------------------------------------------------
    static std::tuple<
        std::vector<std::vector<int>>,
        std::map<std::pair<int,int>, int>,
        std::vector<std::pair<int,int>>
    >
    createAdjacencyData(const std::array<std::array<int, COLS>, ROWS> &matrix);

    // --------------------------------------------------------------------------------
    // Getters
    // --------------------------------------------------------------------------------
    const std::array<std::array<int, COLS>, ROWS>& getBoard() const;
    const std::vector<std::vector<int>>& getAdjacencyList() const;  // New name
    const std::map<std::pair<int, int>, int>& getCoordToIndex() const;
    const std::vector<std::pair<int, int>>& getIndexToCoord() const;
    const int& getNumPlayerOnePieces() const;
    const int& getNumPlayerTwoPieces() const;

    // --------------------------------------------------------------------------------
    // Board Setup
    // --------------------------------------------------------------------------------
    void placePieces(const std::vector<std::string>& pieces);

    // --------------------------------------------------------------------------------
    // Moves
    // --------------------------------------------------------------------------------
    void applyMove(const Move& move);

    // --------------------------------------------------------------------------------
    // Helpers
    // --------------------------------------------------------------------------------
    const bool validPosition(const int letterIndex, const int numberIndex) const;

    // --------------------------------------------------------------------------------
    // Zobrist Hashing
    // --------------------------------------------------------------------------------
    static void initZobrist();
    uint64_t getZobristHash() const;
    bool operator==(const Board& other) const;

    // --------------------------------------------------------------------------------
    // Printing
    // --------------------------------------------------------------------------------
    void printPieces() const;
    void printMatrix() const;
    void printBoard() const;

    // --------------------------------------------------------------------------------
    // TEMP (For Testing) - Creates a list of pieces from a string
    // --------------------------------------------------------------------------------
    static std::vector<std::string> stringToList(const std::string& pieces);

private:
    // --------------------------------------------------------------------------------
    // Helper move methods
    // --------------------------------------------------------------------------------
    void moveOnePiece(const Move& move);
    void movePiecesInline(const Move& move);
    void movePiecesSideStep(const Move& move);
};

struct BoardHasher {
    std::size_t operator()(const Board& b) const {
        return static_cast<std::size_t>(b.getZobristHash());
    }
};

#endif // BOARD_H