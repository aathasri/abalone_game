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

// Map for your player colours:
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

/**
 * Storing the data needed to undo a move.
 * We'll record changes to:
 *  - occupant in any cells that changed (oldVal -> newVal)
 *  - piece counts
 */
struct MoveUndo {
    struct CellChange {
        int row;
        int col;
        int oldVal;
        int newVal;
    };

    // occupant changes
    std::vector<CellChange> changes;

    // old piece counts
    int oldNumPlayerOne;
    int oldNumPlayerTwo;

    // new piece counts
    int newNumPlayerOne;
    int newNumPlayerTwo;

    MoveUndo() 
        : oldNumPlayerOne(0), oldNumPlayerTwo(0),
          newNumPlayerOne(0), newNumPlayerTwo(0) {}
};

class Board {

private:
    // --------------------------------------------------------------------------------
    // Data Members
    // --------------------------------------------------------------------------------
    std::array<std::array<int, COLS>, ROWS> gameboard;

    int numPlayerOnePieces;
    int numPlayerTwoPieces;

    // Adjacency list
    std::vector<std::vector<int>> adjacencyList;
    // Maps from (row, col) to linear index
    std::map<std::pair<int, int>, int> coordToIndex;
    // Maps from index to (row, col)
    std::vector<std::pair<int, int>> indexToCoord;

    // Zobrist
    static std::array<std::array<std::array<uint64_t, 3>, COLS>, ROWS> zobristTable;
    static bool zobristInitialized;

public:
    // --------------------------------------------------------------------------------
    // Constructor
    // --------------------------------------------------------------------------------
    Board(const std::array<std::array<int, COLS>, ROWS>& matrix,
          const std::vector<std::vector<int>>& adjacencyList,
          const std::map<std::pair<int,int>,int>& coordToIndex,
          const std::vector<std::pair<int,int>>& indexToCoord);

    // --------------------------------------------------------------------------------
    // Static method to produce adjacency data
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
    const std::vector<std::vector<int>>& getAdjacencyList() const;
    const std::map<std::pair<int, int>, int>& getCoordToIndex() const;
    const std::vector<std::pair<int, int>>& getIndexToCoord() const;
    const int& getNumPlayerOnePieces() const;
    const int& getNumPlayerTwoPieces() const;

    // --------------------------------------------------------------------------------
    // Board Setup
    // --------------------------------------------------------------------------------
    void placePieces(const std::vector<std::string>& pieces);

    // --------------------------------------------------------------------------------
    // Mutation in-place (Make/Unmake)
    // --------------------------------------------------------------------------------
    
    void applyMove(const Move& move);
    // Applies 'move' to this board, recording all changes into 'undo'
    void makeMove(const Move& move, MoveUndo& undo);
    // Reverts changes from 'undo'
    void unmakeMove(const MoveUndo& undo);

    // --------------------------------------------------------------------------------
    // Helpers
    // --------------------------------------------------------------------------------
    bool validPosition(int row, int col) const;

    // --------------------------------------------------------------------------------
    // Zobrist Hashing
    // --------------------------------------------------------------------------------
    static void initZobrist();
    uint64_t getZobristHash() const;
    bool operator==(const Board& other) const;

    // multithreading
    bool operator<(const Board& other) const;


    // --------------------------------------------------------------------------------
    // Printing
    // --------------------------------------------------------------------------------
    void printPieces() const;
    void printMatrix() const;
    void printBoard() const;

    // --------------------------------------------------------------------------------
    // TEMP: stringToList
    // --------------------------------------------------------------------------------
    static std::vector<std::string> stringToList(const std::string& pieces);

private:
    // Helper that records occupant changes in 'undo'
    void recordCellChange(int row, int col, int oldVal, int newVal, MoveUndo& undo);

    // Sub-helpers for move logic
    void moveOnePiece(const Move& move, MoveUndo& undo);
    void movePiecesInline(const Move& move, MoveUndo& undo);
    void movePiecesSideStep(const Move& move, MoveUndo& undo);
};

// For storing boards in e.g. an unordered_map
struct BoardHasher {
    std::size_t operator()(const Board& b) const {
        return static_cast<std::size_t>(b.getZobristHash());
    }
};

#endif // BOARD_H