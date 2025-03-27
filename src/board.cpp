#include "board.h"

// Constructor implementation
Board::Board(const std::array<std::array<int, COLS>, ROWS>& matrix) : gameboard(matrix) {}

std::array<std::array<int, COLS>, ROWS>& Board::getBoard()
{
    return gameboard;
}

/**
 * Places pieces from a list of pieces onto the board.
 */
void Board::placePieces(const std::vector<std::string> &pieces)
{
    for (const std::string& p : pieces) {
        int letterAxis = 'I' - p[0];
        int numberAxis = p[1] - '1';
        char colour = p[2];
        if (Board::validPosition(letterAxis, numberAxis)) {
            gameboard[letterAxis][numberAxis] = playerCharMap.at(colour);
        } else {
            std::cerr << "Error: Pieces cannot be placed on gameboard" << std::endl;
        }
    }
}


bool Board::validPosition(const int letterIndex, const int numberIndex)
{
    return ((letterIndex >= 0 && letterIndex < ROWS) 
        && (numberIndex >= 0 && numberIndex < COLS))
        && (gameboard[letterIndex][numberIndex] >= 0);
}

// void Board::sortMovePieces(std::vector<std::string> &pieces)
// {
//     if (pieces.size() < 2) return;
//     std::string temp;
//     if (pieces[0] > pieces[1]) {
//         temp = pieces[0]; 
//         pieces[0] = pieces[1]; 
//         pieces[1] = temp;
//     }
//     if (pieces.size() == 3) {
//         if (pieces[1] > pieces[2]) {
//             temp = pieces[1]; 
//             pieces[1] = pieces[2]; 
//             pieces[2] = temp;
//         }
//         if (pieces[0] > pieces[1]) {
//             temp = pieces[0]; 
//             pieces[0] = pieces[1]; 
//             pieces[1] = temp;
//         }
//     }
// }


/**
 * Prints matrix representation of gameboard.
 */
void Board::printMatrix() const {
    for (const auto& row : gameboard) {
        for (int cell : row) {
            char colour = printMap.at(cell);
            std::cout << colour << " ";
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
 * Creates a string of all pieces on the gameboard.
 */
std::string Board::toString() const
{
    std::string result;
    result.reserve(14 * 4 * 2); // reserve max space for all pieces on board
    std::string whitePieces;
    whitePieces.reserve(14 * 4); // 14 white pieces max, reprented with 3 characters + commas
    
    for (int i = COLS - 1; i > 0; --i) {
        for (int j = 0; j < ROWS; j++) {
            char letterPosition = static_cast<char>('I' - i);
            std::string position = std::string(1, letterPosition) + std::to_string(j + 1);
            if(gameboard[i][j] == 'b') {
                result += position + "b,";
            } else if (gameboard[i][j] == 'w') {
                whitePieces += position + "w,";
            }
        }
    }
    // Removes trailing comma
    result += whitePieces;
    if (!result.empty()) {
        result.pop_back();
    }
    return result;
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

