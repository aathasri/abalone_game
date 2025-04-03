#include "board.h"

/** 
 * --------------------------------------------------------------------------------
 * Constructor & Getters
 * --------------------------------------------------------------------------------
 */ 
Board::Board(const std::array<std::array<int, COLS>, ROWS>& matrix)
    : gameboard(matrix), numPlayerOnePieces(14), numPlayerTwoPieces(14) 
{
    initializeAdjacencyMatrix();
    initializeStaticAdjacencyMapping();
}

const std::array<std::array<int, COLS>, ROWS>& Board::getBoard() const
{
    return gameboard;
}

const std::vector<std::vector<bool>> &Board::getAdjacencyMatrix() const
{
    return adjacencyMatrix;
}

const std::map<std::pair<int, int>, int>& Board::getCoordToIndex() {
    return coordToIndex;
}

const std::vector<std::pair<int, int>>& Board::getIndexToCoord() const {
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
 * TEMP - For testing
 */
void Board::placePieces(const std::vector<std::string> &pieces)
{
    numPlayerOnePieces = 0;
    numPlayerTwoPieces = 0;
    for (const std::string& p : pieces) {
        int letterAxis = 'I' - p[0];
        int numberAxis = p[1] - '1';
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

    initializeAdjacencyMatrix();
}

/** 
 * --------------------------------------------------------------------------------
 * Apply Moves
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

void Board::moveOnePiece(const Move& move)
{
    int oldPosLetIndex = move.positions[0].first;
    int oldPosNumIndex = move.positions[0].second;
    int currPlayer = gameboard[oldPosLetIndex][oldPosNumIndex];
    int newPosLetIndex = oldPosLetIndex + DirectionHelper::getDelta(move.direction).first;
    int newPosNumIndex = oldPosNumIndex + DirectionHelper::getDelta(move.direction).second;
    gameboard[newPosLetIndex][newPosNumIndex] = currPlayer;
    gameboard[oldPosLetIndex][oldPosNumIndex] = 0;

    updateAdjacencyForMove({{oldPosLetIndex, oldPosNumIndex}}, {{newPosLetIndex, newPosNumIndex}});
}

void Board::movePiecesInline(const Move& move)
{
    auto [dx, dy] = DirectionHelper::getDelta(move.getDirection());

    std::vector<std::pair<int, int>> oldPositions;
    std::vector<std::pair<int, int>> newPositions;

    // STEP 1: Push opponent marble(s) first
    int pushCount = move.getSize() == 2 ? 1 : 2;

    int leadCol = move.getPosition(0).first;
    int leadRow = move.getPosition(0).second;

    for (int i = pushCount - 1; i >= 0; --i) {
        int oppCol = leadCol + dx * (i + 1);
        int oppRow = leadRow + dy * (i + 1);
        int newCol = oppCol + dx;
        int newRow = oppRow + dy;

        if (!validPosition(oppCol, oppRow)) continue; // invalid cell (not playable)

        int oppPlayer = gameboard[oppCol][oppRow];
        if (oppPlayer != 1 && oppPlayer != 2) continue; // not a piece, skip

        oldPositions.push_back({oppCol, oppRow});

        if (!validPosition(newCol, newRow)) {
            // Pushed off the board
            if (oppPlayer == 1) {
                numPlayerOnePieces--;
            } else if (oppPlayer == 2) {
                numPlayerTwoPieces--;
            }
            gameboard[oppCol][oppRow] = 0; // remove from board
        } else {
            // Pushed to a new cell
            gameboard[newCol][newRow] = oppPlayer;
            gameboard[oppCol][oppRow] = 0;
            newPositions.push_back({newCol, newRow});
        }
    }

    // STEP 2: Move current player's marbles
    for (int i = 0; i < move.getSize(); ++i) {
        int col = move.getPosition(i).first;
        int row = move.getPosition(i).second;

        int newCol = col + dx;
        int newRow = row + dy;

        int player = gameboard[col][row];

        gameboard[newCol][newRow] = player;
        gameboard[col][row] = 0;

        oldPositions.push_back({col, row});
        newPositions.push_back({newCol, newRow});
    }

    // STEP 3: Update adjacency matrix
    updateAdjacencyForMove(oldPositions, newPositions);
}


void Board::movePiecesSideStep(const Move& move)
{
    std::vector<std::pair<int, int>> oldPositions, newPositions;

    for (int i = 0; i < move.size; ++i) {
        int oldPosLetIndex = move.positions[i].first;
        int oldPosNumIndex = move.positions[i].second;
        
        int newPosLetIndex = oldPosLetIndex + DirectionHelper::getDelta(move.direction).first;
        int newPosNumIndex = oldPosNumIndex + DirectionHelper::getDelta(move.direction).second;
        
        oldPositions.emplace_back(oldPosLetIndex, oldPosNumIndex);
        newPositions.emplace_back(newPosLetIndex, newPosNumIndex);
    
        gameboard[newPosLetIndex][newPosNumIndex] = gameboard[oldPosLetIndex][oldPosNumIndex];
        gameboard[oldPosLetIndex][oldPosNumIndex] = 0;
    }

    updateAdjacencyForMove(oldPositions, newPositions);
}

/** 
 * --------------------------------------------------------------------------------
 * Adjacency Matrix
 * --------------------------------------------------------------------------------
 */ 

void Board::initializeAdjacencyMatrix()
{
    coordToIndex.clear();
    indexToCoord.clear();

    int index = 0;
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            if (gameboard[i][j] != -1) {
                coordToIndex[{i, j}] = index;
                indexToCoord.push_back({i, j});
                index++;
            }
        }
    }

    int N = indexToCoord.size();
    adjacencyMatrix.assign(N, std::vector<bool>(N, false));

    std::vector<std::pair<int, int>> directions = {
        {-1, 0}, {-1, 1}, {0, 1},
        {1, 0}, {1, -1}, {0, -1}
    };

    for (int idx = 0; idx < N; ++idx) {
        auto [i, j] = indexToCoord[idx];
        for (auto [di, dj] : directions) {
            auto neighbor = std::make_pair(i + di, j + dj);
            if (coordToIndex.count(neighbor)) {
                int neighborIdx = coordToIndex.at(neighbor);
                adjacencyMatrix[idx][neighborIdx] = true;
            }
        }
    }
}

void Board::updateAdjacencyForMove(const std::vector<std::pair<int, int>> &oldPositions, const std::vector<std::pair<int, int>> &newPositions)
{
    for (const auto& pos : oldPositions) {
        if (coordToIndex.count(pos)) {
            updateAdjacencyAt(coordToIndex[pos]);
        }
    }

    for (const auto& pos : newPositions) {
        if (coordToIndex.count(pos)) {
            updateAdjacencyAt(coordToIndex[pos]);
        }
    }
}

void Board::updateAdjacencyAt(int idx)
{
    auto [i, j] = indexToCoord[idx];
    for (int k = 0; k < indexToCoord.size(); ++k) {
        adjacencyMatrix[idx][k] = false;
        adjacencyMatrix[k][idx] = false;
    }

    std::vector<std::pair<int, int>> directions = {
        {-1, 0}, {-1, 1}, {0, 1},
        {1, 0}, {1, -1}, {0, -1}
    };

    for (auto [di, dj] : directions) {
        auto neighbor = std::make_pair(i + di, j + dj);
        if (coordToIndex.count(neighbor)) {
            int neighborIdx = coordToIndex[neighbor];
            adjacencyMatrix[idx][neighborIdx] = true;
            adjacencyMatrix[neighborIdx][idx] = true;
        }
    }
}

std::map<std::pair<int, int>, int> Board::coordToIndex;
std::vector<std::pair<int, int>> Board::indexToCoord;

bool Board::staticMappingInitialized = false;


void Board::initializeStaticAdjacencyMapping() {
    if (staticMappingInitialized) return;

    int index = 0;
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            if (gameboard[i][j] != -1) {
                coordToIndex[{i, j}] = index;
                indexToCoord.push_back({i, j});
                index++;
            }
        }
    }

    staticMappingInitialized = true;
}



/** 
 * --------------------------------------------------------------------------------
 * Helper Methods
 * --------------------------------------------------------------------------------
 */ 

const bool Board::validPosition(const int letterIndex, const int numberIndex) const
{
    return ((letterIndex >= 0 && letterIndex < ROWS) 
        && (numberIndex >= 0 && numberIndex < COLS))
        && (gameboard[letterIndex][numberIndex] >= 0);
}


/** 
 * --------------------------------------------------------------------------------
 * Hashing
 * --------------------------------------------------------------------------------
 */ 

bool Board::zobristInitialized = false;
std::array<std::array<std::array<uint64_t, 3>, COLS>, ROWS> Board::zobristTable;

void Board::initZobrist() {
    std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<uint64_t> dist;

    for (int i = 0; i < ROWS; ++i)
        for (int j = 0; j < COLS; ++j)
            for (int p = 0; p < 3; ++p)
                zobristTable[i][j][p] = dist(rng);

    zobristInitialized = true;
}

uint64_t Board::getZobristHash() const {
    if (!zobristInitialized) initZobrist();

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
            } else if (state == 'w') {
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

    std::cout << finalOss.str() << ": " << numPlayerOnePieces << ", " << numPlayerTwoPieces<< std::endl;
}

/**
 * Prints matrix representation of gameboard.
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
 * Prints accurate representation of gameboard.
 */
void Board::printBoard() const {
    int middleIndex = (ROWS + 1) / 2;
    for (int i = 0; i < ROWS; ++i) {
        // Calculate leading spaces for centering
        int leadingSpaces = (std::abs((i + 1) - middleIndex)) % middleIndex;

        // Print leading spaces
        std::cout << std::string(leadingSpaces, ' ');

        // Print row elements
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
 * Creates a list of all game pieces from a string of all pieces on the gameboard.
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