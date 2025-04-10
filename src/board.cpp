#include "board.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <sstream>
#include <tuple>
#include <random>

// --------------------------------------------------------------------------------
// Static: Create Adjacency Data
// --------------------------------------------------------------------------------
std::tuple<
    std::vector<std::vector<int>>,
    std::map<std::pair<int,int>, int>,
    std::vector<std::pair<int,int>>
>
Board::createAdjacencyData(const std::array<std::array<int, COLS>, ROWS>& matrix)
{
    std::map<std::pair<int,int>, int> coordToIndexLocal;
    std::vector<std::pair<int,int>> indexToCoordLocal;

    int index = 0;
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            if (matrix[i][j] != -1) {
                coordToIndexLocal[{i, j}] = index;
                indexToCoordLocal.push_back({i, j});
                index++;
            }
        }
    }

    int N = indexToCoordLocal.size();
    std::vector<std::vector<int>> adjacencyListLocal(N);

    std::vector<std::pair<int,int>> directions = {
        {0, -1}, {-1, 0}, {-1, 1},
        {0, 1},  {1, 0}, {1, -1}
    };

    for (int i = 0; i < N; ++i) {
        auto [row, col] = indexToCoordLocal[i];
        for (auto [dr, dc] : directions) {
            int nr = row + dr;
            int nc = col + dc;
            auto neighbor = std::make_pair(nr, nc);
            if (coordToIndexLocal.count(neighbor)) {
                int neighborIdx = coordToIndexLocal.at(neighbor);
                adjacencyListLocal[i].push_back(neighborIdx);
            }
        }
    }

    return { adjacencyListLocal, coordToIndexLocal, indexToCoordLocal };
}

// --------------------------------------------------------------------------------
// Constructor
// --------------------------------------------------------------------------------
Board::Board(const std::array<std::array<int, COLS>, ROWS>& matrix,
             const std::vector<std::vector<int>>& adjList,
             const std::map<std::pair<int,int>, int>& cToIdx,
             const std::vector<std::pair<int,int>>& idxToCoord)
    : gameboard(matrix),
      numPlayerOnePieces(14),  // Adjust as needed.
      numPlayerTwoPieces(14),
      // Create shared pointers for invariant data.
      adjacencyList(std::make_shared<std::vector<std::vector<int>>>(adjList)),
      coordToIndex(std::make_shared<std::map<std::pair<int,int>, int>>(cToIdx)),
      indexToCoord(std::make_shared<std::vector<std::pair<int,int>>>(idxToCoord))
{
    // Optionally recalc pieces from matrix if needed.
}

// --------------------------------------------------------------------------------
// Custom Copy Constructor
// --------------------------------------------------------------------------------
// This copy constructor copies the mutable board state while sharing the invariant data.
Board::Board(const Board& other)
    : gameboard(other.gameboard),
      numPlayerOnePieces(other.numPlayerOnePieces),
      numPlayerTwoPieces(other.numPlayerTwoPieces),
      adjacencyList(other.adjacencyList),
      coordToIndex(other.coordToIndex),
      indexToCoord(other.indexToCoord)
{
    // No deep copying of invariant data.
}

// --------------------------------------------------------------------------------
// Getters
// --------------------------------------------------------------------------------
const std::array<std::array<int, COLS>, ROWS>& Board::getBoard() const {
    return gameboard;
}

const std::vector<std::vector<int>>& Board::getAdjacencyList() const {
    return *adjacencyList;
}

const std::map<std::pair<int, int>, int>& Board::getCoordToIndex() const {
    return *coordToIndex;
}

const std::vector<std::pair<int, int>>& Board::getIndexToCoord() const {
    return *indexToCoord;
}

const int& Board::getNumPlayerOnePieces() const {
    return numPlayerOnePieces;
}

const int& Board::getNumPlayerTwoPieces() const {
    return numPlayerTwoPieces;
}

// --------------------------------------------------------------------------------
// Board Setup: placePieces
// --------------------------------------------------------------------------------
void Board::placePieces(const std::vector<std::string>& pieces)
{
    numPlayerOnePieces = 0;
    numPlayerTwoPieces = 0;

    for (const std::string& p : pieces) {
        int row = 'I' - p[0];
        int col = p[1] - '1';
        char colour = p[2];

        if (validPosition(row, col)) {
            gameboard[row][col] = playerColourMap.at(colour);
            if (playerColourMap.at(colour) == 1) {
                numPlayerOnePieces++;
            } else {
                numPlayerTwoPieces++;
            }
        } else {
            std::cerr << "Error placing piece: " << p << std::endl;
        }
    }
}

// --------------------------------------------------------------------------------
// Mutation Functions: applyMove, makeMove, unmakeMove, recordCellChange, etc.
// --------------------------------------------------------------------------------
void Board::applyMove(const Move& move)
{
    MoveUndo dummyUndo;
    makeMove(move, dummyUndo);
}

void Board::makeMove(const Move& move, MoveUndo& undo)
{
    undo.oldNumPlayerOne = numPlayerOnePieces;
    undo.oldNumPlayerTwo = numPlayerTwoPieces;

    if (move.size == 1) {
        moveOnePiece(move, undo);
    }
    else if (move.type == MoveType::INLINE) {
        movePiecesInline(move, undo);
    }
    else if (move.type == MoveType::SIDESTEP) {
        movePiecesSideStep(move, undo);
    }

    undo.newNumPlayerOne = numPlayerOnePieces;
    undo.newNumPlayerTwo = numPlayerTwoPieces;
}

void Board::unmakeMove(const MoveUndo& undo)
{
    numPlayerOnePieces = undo.oldNumPlayerOne;
    numPlayerTwoPieces = undo.oldNumPlayerTwo;

    for (auto it = undo.changes.rbegin(); it != undo.changes.rend(); ++it) {
        const auto& cc = *it;
        gameboard[cc.row][cc.col] = cc.oldVal;
    }
}

void Board::recordCellChange(int row, int col, int oldVal, int newVal, MoveUndo& undo)
{
    MoveUndo::CellChange cc;
    cc.row = row;
    cc.col = col;
    cc.oldVal = oldVal;
    cc.newVal = newVal;
    undo.changes.push_back(cc);
}

// --------------------------------------------------------------------------------
// Sub-helpers for Moves (moveOnePiece, movePiecesInline, movePiecesSideStep)
// --------------------------------------------------------------------------------
void Board::moveOnePiece(const Move& move, MoveUndo& undo)
{
    int oldR = move.positions[0].first;
    int oldC = move.positions[0].second;
    int currPlayer = gameboard[oldR][oldC];

    auto [dx, dy] = DirectionHelper::getDelta(move.direction);
    int newR = oldR + dx;
    int newC = oldC + dy;

    recordCellChange(oldR, oldC, currPlayer, 0, undo);
    recordCellChange(newR, newC, gameboard[newR][newC], currPlayer, undo);

    gameboard[oldR][oldC] = 0;
    gameboard[newR][newC] = currPlayer;
}

void Board::movePiecesInline(const Move& move, MoveUndo& undo)
{
    auto [dx, dy] = DirectionHelper::getDelta(move.getDirection());
    int numFriendly = move.getSize();
    int leadR = move.getPosition(0).first;
    int leadC = move.getPosition(0).second;
    int currPlayer = gameboard[leadR][leadC];
    int oppPlayer = (currPlayer == 1) ? 2 : 1;

    int maxPush = (numFriendly == 2) ? 1 : 2;
    std::vector<std::pair<int, int>> oppPositions;
    int oppCount = 0;
    for (int i = 1; i <= maxPush; i++) {
        int oppR = leadR + dx * i;
        int oppC = leadC + dy * i;
        if (!validPosition(oppR, oppC))
            break;
        if (gameboard[oppR][oppC] == oppPlayer) {
            oppPositions.push_back({oppR, oppC});
            oppCount++;
        } else {
            break;
        }
    }
    int actualPushCount = (oppCount > 0 && numFriendly > oppCount) ? std::min(oppCount, maxPush) : 0;
    for (int i = actualPushCount - 1; i >= 0; i--) {
        int oppR = oppPositions[i].first;
        int oppC = oppPositions[i].second;
        int newR = oppR + dx;
        int newC = oppC + dy;
        if (!validPosition(newR, newC)) {
            recordCellChange(oppR, oppC, oppPlayer, 0, undo);
            gameboard[oppR][oppC] = 0;
            if (oppPlayer == 1)
                numPlayerOnePieces--;
            else
                numPlayerTwoPieces--;
        } else {
            recordCellChange(oppR, oppC, oppPlayer, 0, undo);
            recordCellChange(newR, newC, gameboard[newR][newC], oppPlayer, undo);
            gameboard[newR][newC] = oppPlayer;
            gameboard[oppR][oppC] = 0;
        }
    }

    std::vector<std::pair<std::pair<int,int>, std::pair<int,int>>> friendlyMoves;
    for (int i = 0; i < numFriendly; i++) {
        int r = move.getPosition(i).first;
        int c = move.getPosition(i).second;
        int newR = r + dx;
        int newC = c + dy;
        friendlyMoves.push_back({{r, c}, {newR, newC}});
    }
    for (auto &m : friendlyMoves) {
        auto &src = m.first;
        auto &dst = m.second;
        recordCellChange(src.first, src.second, currPlayer, 0, undo);
        recordCellChange(dst.first, dst.second, gameboard[dst.first][dst.second], currPlayer, undo);
    }
    for (auto &m : friendlyMoves) {
        auto &src = m.first;
        auto &dst = m.second;
        gameboard[dst.first][dst.second] = currPlayer;
        gameboard[src.first][src.second] = 0;
    }
}

void Board::movePiecesSideStep(const Move& move, MoveUndo& undo)
{
    auto [dx, dy] = DirectionHelper::getDelta(move.direction);
    
    std::vector<std::pair<std::pair<int,int>, std::pair<int,int>>> moves;
    for (int i = 0; i < move.size; i++) {
        int r = move.getPosition(i).first;
        int c = move.getPosition(i).second;
        int newR = r + dx;
        int newC = c + dy;
        moves.push_back({{r, c}, {newR, newC}});
    }
    for (auto &m : moves) {
        auto &src = m.first;
        auto &dst = m.second;
        int val = gameboard[src.first][src.second];
        recordCellChange(src.first, src.second, val, 0, undo);
        recordCellChange(dst.first, dst.second, gameboard[dst.first][dst.second], val, undo);
    }
    for (auto &m : moves) {
        auto &src = m.first;
        auto &dst = m.second;
        gameboard[dst.first][dst.second] = gameboard[src.first][src.second];
        gameboard[src.first][src.second] = 0;
    }
}

// --------------------------------------------------------------------------------
// Validation and Zobrist Hashing
// --------------------------------------------------------------------------------
bool Board::validPosition(int row, int col) const
{
    return (row >= 0 && row < ROWS) 
        && (col >= 0 && col < COLS)
        && (gameboard[row][col] >= 0);
}

bool Board::zobristInitialized = false;
std::array<std::array<std::array<uint64_t, 3>, COLS>, ROWS> Board::zobristTable;

void Board::initZobrist() {
    std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<uint64_t> dist;

    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            for (int p = 0; p < 3; ++p) {
                zobristTable[i][j][p] = dist(rng);
            }
        }
    }
    zobristInitialized = true;
}

uint64_t Board::getZobristHash() const {
    if (!zobristInitialized) {
        initZobrist();
    }

    uint64_t hash = 0;
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            int piece = gameboard[i][j];
            if (piece >= 0) {
                hash ^= zobristTable[i][j][piece];
            }
        }
    }
    return hash;
}

bool Board::operator==(const Board& other) const {
    return gameboard == other.gameboard;
}

bool Board::operator<(const Board& other) const {
    return gameboard < other.gameboard;
}

// --------------------------------------------------------------------------------
// Printing Functions
// --------------------------------------------------------------------------------
void Board::printPieces() const
{
    std::ostringstream blackOss;
    std::ostringstream whiteOss;
    bool firstBlack = true;
    bool firstWhite = true;

    for (int i = ROWS - 1; i >= 0; --i) {
        for (int j = 0; j < COLS; ++j) {
            int cell = gameboard[i][j];
            char state = printMap.at(cell);
            char letterIndex = 'I' - i;
            int numberIndex = 1 + j;

            if (state == 'b') {
                if (!firstBlack) blackOss << ",";
                blackOss << letterIndex << numberIndex << state;
                firstBlack = false;
            } 
            else if (state == 'w') {
                if (!firstWhite) whiteOss << ",";
                whiteOss << letterIndex << numberIndex << state;
                firstWhite = false;
            }
        }
    }

    std::ostringstream finalOss;
    finalOss << blackOss.str();
    if (!whiteOss.str().empty()) {
        if (!blackOss.str().empty()) finalOss << ",";
        finalOss << whiteOss.str();
    }

    std::cout << finalOss.str() << ": "
              << numPlayerOnePieces << ", "
              << numPlayerTwoPieces << std::endl;
}

void Board::printMatrix() const {
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            char state = printMap.at(gameboard[i][j]);
            std::cout << state << " ";
        }
        std::cout << std::endl;
    }
}

void Board::printBoard() const {
    int middleIndex = (ROWS + 1) / 2;
    for (int i = 0; i < ROWS; ++i) {
        int leadingSpaces = std::abs((i + 1) - middleIndex) % middleIndex;
        std::cout << std::string(leadingSpaces, ' ');

        for (int j = 0; j < COLS; ++j) {
            int cell = gameboard[i][j];
            if (cell > -1) {
                char colour = printMap.at(cell);
                std::cout << colour << " ";
            }
        }
        std::cout << std::endl;
    }
}

std::vector<std::string> Board::stringToList(const std::string& pieces)
{
    std::vector<std::string> result;
    result.reserve(pieces.size() / 4 + 1);

    for (size_t i = 0; i < pieces.size(); i += 4) {
        result.emplace_back(pieces, i, 3);
    }
    return result;
}