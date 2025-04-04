#include "board.h"

/**
 * --------------------------------------------------------------------------------
 * Static: Create Adjacency Data (List Version)
 * --------------------------------------------------------------------------------
 *
 * For a given 9×9 matrix, we build:
 *   1) coordToIndex (mapping from valid (row,col) to a linear index i)
 *   2) indexToCoord (reverse mapping from i to (row,col))
 *   3) adjacencyList, where adjacencyList[i] is a list of the neighbor indices
 */
std::tuple<
    std::vector<std::vector<int>>,
    std::map<std::pair<int,int>, int>,
    std::vector<std::pair<int,int>>
>
Board::createAdjacencyData(const std::array<std::array<int, COLS>, ROWS> &matrix)
{
    std::map<std::pair<int,int>,int> coordToIndexLocal;
    std::vector<std::pair<int,int>> indexToCoordLocal;

    // 1) Build coordToIndex / indexToCoord for valid cells (non -1)
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

    // 2) Initialize adjacency list
    int N = indexToCoordLocal.size();
    std::vector<std::vector<int>> adjacencyListLocal(N);

    // The 6 hex directions
    std::vector<std::pair<int,int>> directions = {
        {-1, 0},  // up
        {-1, 1},  // up-right
        {0, 1},   // right
        {1, 0},   // down
        {1, -1},  // down-left
        {0, -1}   // left
    };

    // 3) For each valid cell, find neighbors and push them into adjacencyListLocal[i]
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

    return std::make_tuple(adjacencyListLocal, coordToIndexLocal, indexToCoordLocal);
}

/** 
 * --------------------------------------------------------------------------------
 * Constructor
 * --------------------------------------------------------------------------------
 */ 
Board::Board(const std::array<std::array<int, COLS>, ROWS>& matrix,
             const std::vector<std::vector<int>>& adjacencyList,
             const std::map<std::pair<int, int>, int>& coordToIndex,
             const std::vector<std::pair<int, int>>& indexToCoord)
    : gameboard(matrix),
      adjacencyList(adjacencyList),
      coordToIndex(coordToIndex),
      indexToCoord(indexToCoord),
      numPlayerOnePieces(14),  // or however many you want initially
      numPlayerTwoPieces(14)
{
    // Any other initialization you want...
}

/** 
 * --------------------------------------------------------------------------------
 * Getters
 * --------------------------------------------------------------------------------
 */
const std::array<std::array<int, COLS>, ROWS>& Board::getBoard() const
{
    return gameboard;
}

const std::vector<std::vector<int>>& Board::getAdjacencyList() const
{
    return adjacencyList;
}

const std::map<std::pair<int, int>, int>& Board::getCoordToIndex() const
{
    return coordToIndex;
}

const std::vector<std::pair<int, int>>& Board::getIndexToCoord() const
{
    return indexToCoord;
}

const int& Board::getNumPlayerOnePieces() const
{
    return numPlayerOnePieces;
}

const int& Board::getNumPlayerTwoPieces() const
{
    return numPlayerTwoPieces;
}

/** 
 * --------------------------------------------------------------------------------
 * Place Pieces
 * --------------------------------------------------------------------------------
 * 
 * We reset the piece counts, then place each piece from the string vector. 
 */
void Board::placePieces(const std::vector<std::string> &pieces)
{
    numPlayerOnePieces = 0;
    numPlayerTwoPieces = 0;

    for (const std::string& p : pieces) {
        int letterAxis = 'I' - p[0];  // e.g., 'I' - 'I' = 0
        int numberAxis = p[1] - '1';  // e.g., '2' - '1' = 1
        char colour = p[2];
        if (Board::validPosition(letterAxis, numberAxis)) {
            gameboard[letterAxis][numberAxis] = playerColourMap.at(colour);
            if (playerColourMap.at(colour) == 1) {
                numPlayerOnePieces++;
            } else {
                numPlayerTwoPieces++;
            }
        } else {
            std::cerr << "Error: Pieces cannot be placed on gameboard" << std::endl;
        }
    }
}

/** 
 * --------------------------------------------------------------------------------
 * Apply Move
 * --------------------------------------------------------------------------------
 */
void Board::applyMove(const Move& move)
{
    if (move.size == 1) {
        moveOnePiece(move);
    } else if (move.type == MoveType::INLINE) {
        movePiecesInline(move);
    } else if (move.type == MoveType::SIDESTEP) {
        movePiecesSideStep(move);
    }
}

/** 
 * --------------------------------------------------------------------------------
 * Move Helpers
 * --------------------------------------------------------------------------------
 */
void Board::moveOnePiece(const Move& move)
{
    int oldPosLetIndex = move.positions[0].first;
    int oldPosNumIndex = move.positions[0].second;
    int currPlayer = gameboard[oldPosLetIndex][oldPosNumIndex];

    auto [dx, dy] = DirectionHelper::getDelta(move.direction);
    int newPosLetIndex = oldPosLetIndex + dx;
    int newPosNumIndex = oldPosNumIndex + dy;

    gameboard[newPosLetIndex][newPosNumIndex] = currPlayer;
    gameboard[oldPosLetIndex][oldPosNumIndex] = 0;
}

void Board::movePiecesInline(const Move& move)
{
    auto [dx, dy] = DirectionHelper::getDelta(move.getDirection());
    int pushCount = (move.getSize() == 2) ? 1 : 2;
    int leadCol = move.getPosition(0).first;
    int leadRow = move.getPosition(0).second;

    // Push opponent pieces
    for (int i = pushCount - 1; i >= 0; --i) {
        int oppCol = leadCol + dx * (i + 1);
        int oppRow = leadRow + dy * (i + 1);
        int newCol = oppCol + dx;
        int newRow = oppRow + dy;

        if (!validPosition(oppCol, oppRow))
            continue;

        int oppPlayer = gameboard[oppCol][oppRow];
        if (oppPlayer != 1 && oppPlayer != 2) 
            continue;

        if (!validPosition(newCol, newRow)) {
            // Pushed off the board
            if (oppPlayer == 1) {
                numPlayerOnePieces--;
            } else {
                numPlayerTwoPieces--;
            }
            gameboard[oppCol][oppRow] = 0;
        } else {
            // Move that piece forward
            gameboard[newCol][newRow] = oppPlayer;
            gameboard[oppCol][oppRow] = 0;
        }
    }

    // Move the current player's pieces
    for (int i = 0; i < move.getSize(); ++i) {
        int col = move.getPosition(i).first;
        int row = move.getPosition(i).second;

        int newCol = col + dx;
        int newRow = row + dy;

        int player = gameboard[col][row];

        gameboard[newCol][newRow] = player;
        gameboard[col][row] = 0;
    }
}

void Board::movePiecesSideStep(const Move& move)
{
    auto [dx, dy] = DirectionHelper::getDelta(move.direction);

    for (int i = 0; i < move.size; ++i) {
        int oldPosLetIndex = move.positions[i].first;
        int oldPosNumIndex = move.positions[i].second;

        int newPosLetIndex = oldPosLetIndex + dx;
        int newPosNumIndex = oldPosNumIndex + dy;

        gameboard[newPosLetIndex][newPosNumIndex] = gameboard[oldPosLetIndex][oldPosNumIndex];
        gameboard[oldPosLetIndex][oldPosNumIndex] = 0;
    }
}

/** 
 * --------------------------------------------------------------------------------
 * Validate a position on the board
 * --------------------------------------------------------------------------------
 */
const bool Board::validPosition(const int letterIndex, const int numberIndex) const
{
    return (letterIndex >= 0 && letterIndex < ROWS) 
        && (numberIndex >= 0 && numberIndex < COLS)
        && (gameboard[letterIndex][numberIndex] >= 0);
}

/** 
 * --------------------------------------------------------------------------------
 * Zobrist Hashing
 * --------------------------------------------------------------------------------
 */
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
    // For the simplest equality, just compare gameboard contents.
    return gameboard == other.gameboard;
}

/** 
 * --------------------------------------------------------------------------------
 * Printing
 * --------------------------------------------------------------------------------
 */
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

/**
 * Prints a matrix-like representation of the gameboard.
 */
void Board::printMatrix() const {
    for (const auto& row : gameboard) {
        for (int cell : row) {
            char state = printMap.at(cell);
            std::cout << state << " ";
        }
        std::cout << std::endl;
    }
}

/**
 * Prints a rough 2D "hex" style board layout (just for debugging).
 */
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

/**
 * Creates a list of all game pieces from a single string that encodes them.
 */
std::vector<std::string> Board::stringToList(const std::string &pieces)
{
    std::vector<std::string> result;
    result.reserve(pieces.size() / 4 + 1);

    for (size_t i = 0; i < pieces.size(); i += 4) {
        result.emplace_back(pieces, i, 3);
    }

    return result;
}