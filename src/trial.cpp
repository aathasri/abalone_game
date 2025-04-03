#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <unordered_map>
#include <cstdlib>
#include <ctime>
#include <set>
#include <cmath>
#include <climits>

const int MAX_MOVES = 40;
const int DEPTH = 3;

// represents the states of a cell: black, empty, or white
enum class CellState { EMPTY, BLACK, WHITE };

// the 6 directions a marble can move in
const std::vector<std::string> directions = {"NE", "NW", "E", "W", "SE", "SW"};

// represents a cell in the game board
struct Cell {
    CellState state = CellState::EMPTY;
};

// abalone game board consisting of cells
class AbaloneBoard {
    std::map<std::string, Cell> board;

public:
    AbaloneBoard() {
        // create board with no marbles - all cells set to empty initially
        std::string positions[] = {
            "I5", "I6", "I7", "I8", "I9",
            "H4", "H5", "H6", "H7", "H8", "H9",
            "G3", "G4", "G5", "G6", "G7", "G8", "G9",
            "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9",
            "E1", "E2", "E3", "E4", "E5", "E6", "E7", "E8", "E9",
            "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8",
            "C1", "C2", "C3", "C4", "C5", "C6", "C7",
            "B1", "B2", "B3", "B4", "B5", "B6",
            "A1", "A2", "A3", "A4", "A5"
        };
        for (const auto& pos : positions) {
            board[pos] = Cell();
        }
    }
    // Getter for the board
    [[nodiscard]] const std::map<std::string, Cell>& getBoard() const {
        return board;
    }

    // change a cell's state
    void setCellState(const std::string& pos, CellState state) {
        if (board.find(pos) != board.end()) {
            board[pos].state = state;
        }
    }


    // Generate a string representing the current state of the board
    std::string boardToString() const {
        std::string result;
        for (const auto& entry : board) {
            const std::string& position = entry.first;
            const Cell& cell = entry.second;

            char color = (cell.state == CellState::BLACK) ? 'b' :
                         (cell.state == CellState::WHITE) ? 'w' : ' ';

            if (color != ' ') {
                result += position + color + ",";
            }
        }
        // Remove the trailing comma, if there is one
        if (!result.empty()) {
            result.pop_back();
        }
        return result;
    }


    // access a cell's state
    [[nodiscard]] CellState getCellState(const std::string& pos) const {
        if (board.find(pos) != board.end()) {
            return board.at(pos).state;
        }
        return CellState::EMPTY;
    }

    // check if a position is valid (i.e., is on the board)
    [[nodiscard]] static bool isValidPosition(const std::string& pos) {
        char col = pos[0];
        int row = pos[1] - '0';

        // Ensure the position is within the valid row-column bounds
        return (col >= 'A' && col <= 'I') &&
               ((col == 'A' && row >= 1 && row <= 5) ||
                (col == 'B' && row >= 1 && row <= 6) ||
                (col == 'C' && row >= 1 && row <= 7) ||
                (col == 'D' && row >= 1 && row <= 8) ||
                (col == 'E' && row >= 1 && row <= 9) ||
                (col == 'F' && row >= 2 && row <= 9) ||
                (col == 'G' && row >= 3 && row <= 9) ||
                (col == 'H' && row >= 4 && row <= 9) ||
                (col == 'I' && row >= 5 && row <= 9));
    }

    // Generate all legal moves for a player
    [[nodiscard]] std::vector<std::string> generateLegalMoves(CellState player) const {
        std::vector<std::string> legalMoves;

        // Single marble moves
        generateSingleMarbleMoves(player, legalMoves);

        // Inline moves for 2 marbles
        generateDoubleInlineMoves(player, legalMoves);

        // Inline moves for 3 marbles
        generateTripleInlineMoves(player, legalMoves);

        // Sidestep moves for 2 marbles
        generateDoubleSidestepMoves(player, legalMoves);

        // Sidestep moves for 3 marbles
        generateTripleSidestepMoves(player, legalMoves);

        return legalMoves;
    }




    // Helper function to get the adjacent position in a given direction
    static std::string getAdjacentPosition(const std::string& pos, const std::string& dir) {
        const char col = pos[0];
        const int row = pos[1] - '0';

        // Not sure if these are necessary since we validate position elsewhere
        // Keep them for now just in case

        // For each direction, check the movement and ensure the new position is within bounds
        std::string adjPos;

        if (dir == "NE") {

            adjPos = std::string(1, col + 1) + std::to_string(row + 1);

        } else if (dir == "E") {
                adjPos = std::string(1, col) + std::to_string(row + 1);
        } else if (dir == "NW") {
                adjPos = std::string(1, col + 1) + std::to_string(row);
        } else if (dir == "SE") {
                adjPos = std::string(1, col - 1) + std::to_string(row);
        } else if (dir == "W") {
                adjPos = std::string(1, col) + std::to_string(row - 1);
        } else if (dir == "SW") {
                adjPos = std::string(1, col - 1) + std::to_string(row - 1);
        }

        // If the adjacent position is valid, return it, otherwise return an empty string
        return adjPos.empty() || !isValidPosition(adjPos) ? "fortnite" : adjPos;
    }

    // Generate legal single marble moves
    void generateSingleMarbleMoves(const CellState player, std::vector<std::string>& legalMoves) const {
        for (const auto& [pos, cell] : board) {
            if (cell.state == player) {
                // for each direction, check if the resulting position contains a marble or is on the board
                // if so, log the move
                for (const auto& dir : directions) {
                    if (std::string targetPos = getAdjacentPosition(pos, dir); isValidPosition(targetPos)
                        && getCellState(targetPos) == CellState::EMPTY) {
                        std::string move = "i" + pos + dir;
                        legalMoves.push_back(move);
                    }
                }
            }
        }
    }

    // Generate inline moves for 2 marbles
    void generateDoubleInlineMoves(const CellState player, std::vector<std::string>& legalMoves) const {
        for (const auto& [pos, cell] : board) {
            if (cell.state == player) {
                for (const auto& dir : directions) {
                    std::string nextPos = getAdjacentPosition(pos, dir);
                    std::string nextNextPos = getAdjacentPosition(nextPos, dir);
                    std::string nextNextNextPos = getAdjacentPosition(nextNextPos, dir);


                    // Validate all positions: They must be valid and not empty (if they are the destination)
                    if (!isValidPosition(nextPos) || !isValidPosition(nextNextPos)) {
                        continue; // Skip invalid positions
                    }
                        const CellState nextState = getCellState(nextPos);
                        const CellState nextNextState = getCellState(nextNextPos);
                        const CellState nextNextNextState = getCellState(nextNextNextPos);

                        // Case 1: Empty space after two marbles (Double Inline Move)
                        if (nextState == player && nextNextState == CellState::EMPTY) {
                            legalMoves.push_back("i" + pos + dir);
                        }
                        // Case 2: Pushing an opponent's marble (Double Inline Push)
                        else if (nextState == player && nextNextState != player && nextNextNextState == CellState::EMPTY) {
                            legalMoves.push_back("i" + pos + dir);
                        }
                    }
                }
            }
        }


    // Generate inline moves for 3 marbles
    void generateTripleInlineMoves(const CellState player, std::vector<std::string>& legalMoves) const {
        for (const auto& [pos, cell] : board) {
            if (cell.state == player) {
                for (const auto& dir : directions) {
                    std::string nextPos = getAdjacentPosition(pos, dir);
                    std::string nextNextPos = getAdjacentPosition(nextPos, dir);
                    std::string nextNextNextPos = getAdjacentPosition(nextNextPos, dir);
                    std::string nextNextNextNextPos = getAdjacentPosition(nextNextNextPos, dir);
                    std::string nextNextNextNextNextPos = getAdjacentPosition(nextNextNextNextPos, dir);

                    if (!isValidPosition(nextPos) || !isValidPosition(nextNextPos) || !isValidPosition(nextNextNextPos)) {
                        continue; // Skip invalid positions
                    }

                    std::string pushPos1 = getAdjacentPosition(nextNextNextPos, dir);
                    std::string pushPos2 = getAdjacentPosition(pushPos1, dir);

                    const CellState nextState = getCellState(nextPos);
                    const CellState nextNextState = getCellState(nextNextPos);

                    // skip if either of the two marbles in front belong to other player
                    if (nextState != player || nextNextState != player) {
                        continue;
                    }


                    const CellState nextNextNextState = getCellState(nextNextNextPos);
                    const CellState nextNextNextNextState = getCellState(nextNextNextNextPos);
                    const CellState nextNextNextNextNextState = getCellState(nextNextNextNextNextPos);

                    // skip if we would push our own marble
                    if (nextNextNextState == player) {
                        continue;
                    }

                    // skip if we are moving off the board OR if one of our marbles is blocking
                    if (nextNextNextState != CellState::EMPTY) {
                        if (nextNextNextNextState == player) {
                            continue;
                        }
                        if (nextNextNextNextState != CellState::EMPTY) {
                            if (nextNextNextNextNextState == player) {
                                continue;
                            }
                        }
                    }
                    legalMoves.push_back("i" + pos + dir);
                    }
                }
            }
        }

    // Generate sidestep moves for 2 marbles
    void generateDoubleSidestepMoves(const CellState player, std::vector<std::string>& legalMoves) const {
        for (const auto& [pos, cell] : board) {
            if (cell.state == player) {
                // check all 6 directions
                for (const auto& dir : directions) {
                    if (std::string adjacentPos = getAdjacentPosition(pos, dir); isValidPosition(adjacentPos)
                        && getCellState(adjacentPos) == player) {

                        // Prevent duplicate moves by checking marbles in order
                        if (pos > adjacentPos) continue;

                        // Determine valid sidestep directions
                        std::vector<std::string> sidestepDirs;
                        if (dir == "E" || dir == "W") {
                            sidestepDirs = {"NE", "SE", "NW", "SW"};
                        } else if (dir == "NE" || dir == "SW") {
                            sidestepDirs = {"E", "W", "NW", "SE"};
                        } else if (dir == "NW" || dir == "SE") {
                            sidestepDirs = {"E", "W", "NE", "SW"};
                        }

                        // Try sidesteps
                        for (const auto& sidestepDir : sidestepDirs) {
                            std::string target1 = getAdjacentPosition(pos, sidestepDir);

                            if (std::string target2 = getAdjacentPosition(adjacentPos, sidestepDir); isValidPosition(target1)
                                && isValidPosition(target2) &&
                              getCellState(target1) == CellState::EMPTY &&
                              getCellState(target2) == CellState::EMPTY) {
                                legalMoves.push_back("s" + pos + adjacentPos + sidestepDir);
                                }
                        }
                    }
                }
            }
        }
    }


    // Generate sidestep moves for 3 marbles
    void generateTripleSidestepMoves(const CellState player, std::vector<std::string>& legalMoves) const {
        for (const auto& [pos, cell] : board) {
            if (cell.state == player) {
                // Check for two more adjacent marbles in one of the 6 directions
                for (const auto& dir : directions) {
                    std::string pos2 = getAdjacentPosition(pos, dir);

                    if (std::string pos3 = getAdjacentPosition(pos2, dir); isValidPosition(pos2) && isValidPosition(pos3) &&
                                                                           getCellState(pos2) == player && getCellState(pos3) == player) {
                        // skip duplicates
                        if (pos > pos3) continue;
                        // Determine possible sidestep directions
                        for (const auto& sideDir : directions) {
                            if (sideDir != dir && sideDir != getAdjacentPosition(dir, dir)) { // Ensure not inline
                                std::string target1 = getAdjacentPosition(pos, sideDir);
                                std::string target2 = getAdjacentPosition(pos2, sideDir);
                                if (std::string target3 = getAdjacentPosition(pos3, sideDir); isValidPosition(target1)
                                    && isValidPosition(target2) && isValidPosition(target3) &&
                                    getCellState(target1) == CellState::EMPTY &&
                                    getCellState(target2) == CellState::EMPTY &&
                                    getCellState(target3) == CellState::EMPTY) {
                                    legalMoves.push_back("s" + pos + pos3 + sideDir);
                                    }
                            }
                        }
                        }
                }
            }
        }
    }
};

void parseFile(const std::string& filename, AbaloneBoard& board, CellState& playerToMove) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }

    std::string line;
    std::getline(file, line); // Read the first line to determine the player to move
    playerToMove = (line[0] == 'b') ? CellState::BLACK : CellState::WHITE;

    std::getline(file, line); // Read the second line to get the marble positions
    std::istringstream iss(line);
    std::string marble;
    while (std::getline(iss, marble, ',')) {
        std::string pos = marble.substr(0, 2);
        CellState state = (marble.back() == 'b') ? CellState::BLACK : CellState::WHITE;
        board.setCellState(pos, state);
    }
    file.close();
}

// determine if a position is valid
static bool isValidPosition(const std::string& pos) {
    char col = pos[0];
    int row = std::stoi(pos.substr(1));

    // ensure the position is within bounds
    return (col >= 'A' && col <= 'I') &&
           ((col == 'A' && row >= 1 && row <= 5) ||
            (col == 'B' && row >= 1 && row <= 6) ||
            (col == 'C' && row >= 1 && row <= 7) ||
            (col == 'D' && row >= 1 && row <= 8) ||
            (col == 'E' && row >= 1 && row <= 9) ||
            (col == 'F' && row >= 2 && row <= 9) ||
            (col == 'G' && row >= 3 && row <= 9) ||
            (col == 'H' && row >= 4 && row <= 9) ||
            (col == 'I' && row >= 5 && row <= 9));
}

// helper function to clean the board strings
std::string removeSingleCharValues(const std::string& input) {
    std::stringstream ss(input);
    std::string token;
    std::vector<std::string> validTokens;

    // split by commas and filter out single character values
    while (std::getline(ss, token, ',')) {
        if (token.length() > 1) {
            validTokens.push_back(token);
        }
    }

    // join the remaining tokens back with commas
    std::string result;
    for (size_t i = 0; i < validTokens.size(); ++i) {
        result += validTokens[i];
        if (i != validTokens.size() - 1) {
            result += ",";
        }
    }
    return result;
}


char getMiddleLetter(char a, char b) {
    // ie. C,A -> B
    return static_cast<char>((a + b) / 2);
}

char getMiddleDigit(char a, char b) {
    return static_cast<char>((a + b) / 2);
}

// find the middle position i.e. find the middle marble in a triple sidestep
std::string generateNewPos(const std::string& str1, const std::string& str2) {
    // extract the row and col
    char letter1 = str1[0];
    char digit1 = str1[1];
    char letter2 = str2[0];
    char digit2 = str2[1];

    // find new row
    char newLetter;
    if (letter1 == letter2) {
        newLetter = letter1;
    } else {
        newLetter = getMiddleLetter(letter1, letter2);
    }

    // find new col
    char newDigit;
    if (digit1 == digit2) {
        newDigit = digit1;
    } else {
        newDigit = getMiddleDigit(digit1, digit2);
    }

    std::string result;
    result += newLetter;
    result += newDigit;

    return result;
}


// calculate new positions based on the direction
std::string movePosition(std::unordered_map<std::string, char>& boardState, const std::string& pos, const std::string& direction) {
    char col = pos[0];
    int row = std::stoi(pos.substr(1));

    // move the position based on the direction
    if (direction == "NE") {
        col++;
        row++;
    } else if (direction == "E") {
        row++;
    } else if (direction == "SE") {
        col--;
    } else if (direction == "SW") {
        col--;
        row--;
    } else if (direction == "W") {
        row--;
    } else if (direction == "NW") {
        col++;
    }
    std:: string newPos = std::string(1, col) + std::to_string(row);

    // return if valid, delete from board if not
    if (isValidPosition(newPos)) {
        return newPos;
    }
    boardState.erase(newPos);
    return "";
}

bool arePositionsNotOneMoveAway(std::unordered_map<std::string, char>& boardState, const std::string& pos1, const std::string& pos2) {
    // Try all possible move directions
    const std::vector<std::string> directions = {"NE", "E", "SE", "SW", "W", "NW"};

    for (const std::string& direction : directions) {
        // Calculate the position from pos1 based on the current direction
        std::string newPos = movePosition(boardState, pos1, direction);

        // If the new position is the same as pos2, then they are within one move of each other
        if (newPos == pos2) {
            return false;
        }
    }

    // if none of the directions lead to pos2, they are not within one move
    return true;
}



void applyMove(std::unordered_map<std::string, char>&boardState, const std::string& move) {
    // Apply move logic

    if (char moveType = move[0]; moveType == 'i') {
        std::string position = move.substr(1, 2);
        std::string direction = move.substr(3);
        char color = boardState[position];

        // calculate the new position based on the move direction
        std::string newPos = movePosition(boardState, position, direction);

        // if new position is empty, simple move
        if (boardState.find(newPos) == boardState.end()) {
            boardState.erase(position);
            boardState[newPos] = color;
            return;
        }

        // Position is occupied - attempt to push
        std::vector<std::pair<std::string, char>> toMove;
        std::string currentPos = position;
        std::string nextPos = newPos;

        // Collect all occupied positions in the push chain
        while (boardState.find(nextPos) != boardState.end()) {
            toMove.emplace_back(currentPos, boardState[currentPos]);
            currentPos = nextPos;
            nextPos = movePosition(boardState, currentPos, direction);
        }

        // Add the final empty position to the chain
        toMove.emplace_back(currentPos, boardState[currentPos]);
        std::string finalEmptyPos = nextPos;  // Where the last marble will go

        // Sift all marbles forward
        for (size_t i = toMove.size() - 1; i > 0; --i) {
            boardState[toMove[i].first] = toMove[i - 1].second;
        }
        // Place the last marble in the empty pos
        boardState[finalEmptyPos] = toMove[toMove.size() - 1].second;
        // Place the original moving marble
        boardState[toMove[0].first] = color;

        // Erase original position if we actually pushed
        if (toMove.size() > 1) {
            boardState.erase(position);
        }
    }
    else if (moveType == 's') {
            // sidestep move: extract positions and direction
            std::string position1 = move.substr(1, 2);
            std::string position2 = move.substr(3, 2);
            std::string direction = move.substr(5);
            // std::cout << "Move Type: " << moveType << ", Position 1: " << position1 << ", Position 2: " << position2 << ", Direction: " << direction << std::endl;
            char color1 = boardState[position1];
            char color2 = boardState[position2];

            // calculate new positions based on direction for both positions
            std::string newPos1 = movePosition(boardState, position1, direction);  // Move position1
            std::string newPos2 = movePosition(boardState, position2, direction);  // Move position2


            // remove old positions from the board
            boardState.erase(position1);
            boardState.erase(position2);

            // add the new positions with their colors
            boardState[newPos1] = color1;
            boardState[newPos2] = color2;

            if (arePositionsNotOneMoveAway(boardState, position1, position2)) {
                std::string middlePos = generateNewPos(position1, position2);
                char color3 = boardState[middlePos];
                std::string newPos3 = movePosition(boardState, middlePos, direction);
                boardState.erase(middlePos);
                boardState[newPos3] = color3;
            }
        }
}







// helper function to parse the board state from the second line of the file
std::unordered_map<std::string, char> parseBoard(const std::string& boardFile) {
    std::unordered_map<std::string, char> boardState;
    std::ifstream infile(boardFile);

    // Skip the first line
    std::string line;
    std::getline(infile, line);

    // Read the second line (the actual board)
    if (std::getline(infile, line)) {
        std::stringstream ss(line);
        std::string position;
        while (std::getline(ss, position, ',')) {
            char color = position.back();
            std::string pos = position.substr(0, position.size() - 1); // Remove color from position
            boardState[pos] = color;
        }
    }
    return boardState;
}

std::unordered_map<std::string, char> parseBoardFromString(const std::string& boardString) {
    std::unordered_map<std::string, char> boardState;
    std::stringstream ss(boardString);
    std::string position;

    while (std::getline(ss, position, ',')) {
        // Get the color (last character)
        char color = position.back();
        // Remove the color from the position
        std::string pos = position.substr(0, position.size() - 1);
        // Insert the position and color into the boardState map
        boardState[pos] = color;
    }

    return boardState;  // Return by value
}




// helper function to convert the board state to string format for file output
std::string boardToString(const std::unordered_map<std::string, char>& boardState) {
    std::stringstream ss;
    for (const auto& [pos, color] : boardState) {
        ss << pos << color << ",";
    }
    std::string result = ss.str();
    if (!result.empty()) result.pop_back();
    return result;
}

std::vector<std::string> generateBoardStates(const AbaloneBoard& initialBoard, const std::vector<std::string>& moves) {
    std::vector<std::string> boardStates;
    auto initialBoardState = parseBoardFromString(initialBoard.boardToString());

    for (const std::string& move : moves) {
        // Create a fresh copy of the initial board state for each move
        auto boardState = initialBoardState;

        // Apply the move
        applyMove(boardState, move);

        // Convert to string and clean it
        std::string boardStr = boardToString(boardState);
        boardStr = removeSingleCharValues(boardStr);
        boardStates.push_back(boardStr);
    }

    return boardStates;
}


int marblesOnBoard(const std::string& boardState, CellState player) {
    int playerCount = 0;
    int opponentCount = 0;
    char a;


    char playerChar = (player == CellState::BLACK) ? 'b' : 'w';  // Player's marble character
    char opponentChar = (player == CellState::BLACK) ? 'w' : 'b';  // Opponent's marble character

    // Loop through the board state (assuming two characters per marble, e.g., C5b)
    for (size_t i = 0; i < boardState.size(); i += 4) {
        a = boardState[i+2];
        if (boardState[i + 2] == playerChar) {
            playerCount++;
        } else //if
        // (boardState[i + 1] == opponentChar)
            {
            opponentCount++;
        }
    }
    // std::cout << playerCount << "\n";
    // std::cout << opponentCount << "\n";
    return playerCount - opponentCount;  // Return the difference: positive if player's marbles are more
}



int calculateDistance(const std::string& pos1, const std::string& pos2) {
    char col1 = pos1[0];
    int row1 = pos1[1] - '0';  // Convert the character to an integer

    char col2 = pos2[0];
    int row2 = pos2[1] - '0';  // Convert the character to an integer

    int col1_num = col1 - 'A' + 1;
    int col2_num = col2 - 'A' + 1;

    // Calculate the Manhattan distance
    return std::max(std::abs(col1_num - col2_num), std::abs(row1 - row2));
}

int centerProximity(const std::string& boardState, CellState player) {std::string center = "E5";  // Center of the board
    int totalDistance = 0;
    int count = 0;
    char playerChar = (player == CellState::BLACK) ? 'b' : 'w';  // Set player piece ('b' or 'w')
    char a;
    for (size_t i = 0; i < boardState.size(); i += 4) {
         a = boardState[i+2]; // Assuming each position in the board is represented by 2 characters
        if (a == playerChar) {  // Check if the second character matches the player's piece
            std::string pos = boardState.substr(i, 2);  // Get the position and piece
            totalDistance += calculateDistance(pos, center);
            count++;
        }
    }

    if (count == 0) return 0;
    return totalDistance / count; // Average distance from center
}


int cohesion(const std::string& boardState, CellState player) {
    char playerChar = (player == CellState::BLACK) ? 'b' : 'w';
    std::vector<std::string> positions;

    // Extract all positions of the player's marbles
    for (size_t i = 0; i < boardState.size(); i += 4) {
        if (boardState[i+2] == playerChar) {
            positions.push_back(boardState.substr(i, 2));
        }
    }

    // Calculate average distance between all marbles
    int totalDistance = 0;
    for (size_t i = 0; i < positions.size(); ++i) {
        for (size_t j = i+1; j < positions.size(); ++j) {
            totalDistance += calculateDistance(positions[i], positions[j]);
        }
    }

    return (positions.empty()) ? 0 : totalDistance / positions.size();
}


int opponentMarblesPushed(const std::string& boardState, CellState player) {
    char opponentChar = (player == CellState::BLACK) ? 'w' : 'b';
    int opponentMarbles = std::count(boardState.begin(), boardState.end(), opponentChar);
    return 14 - opponentMarbles;  // Starts at 14 (max marbles), decreases as opponent loses marbles
}


int evaluateBoard(const std::string& boardState, CellState player) {
    char playerChar = (player == CellState::BLACK) ? 'b' : 'w';

    int h1 = centerProximity(boardState, player); // h1: Center Proximity
    int h2 = cohesion(boardState, player);       // h2: Cohesion
    int h3 = marblesOnBoard(boardState, player);  // h3: Marbles on Board
    int h4 = opponentMarblesPushed(boardState, player);
    int h5 = -(14 - std::count(boardState.begin(), boardState.end(), playerChar));


    // std::cout << "h1 h2 h3: " << h1 << " " << h2 << " " << h3 << "\n" <<std::endl;
    int w1 = 1;
    int w2 = 10;
    int w3 = 50;
    int w4 = 200;
    int w5 = 150;  // Strong penalty for losing marbles
    // std::cout << "eval: " << w1*h1 + w2*h2 + w3*h3 << "\n";
    return w1*h1 + w2*h2 + w3*h3 + w4*h4 + w5*h5;
    // std::cout << "h3: " << h3 << "\n";
    // return h3;
}


struct TTEntry {
    int score;
    int depth;
    std::string move;
};

// Global transposition table (declare outside any function)
std::unordered_map<std::string, TTEntry> transpositionTable;

std::pair<int, std::string> minimax(AbaloneBoard& board, int depth, int alpha, int beta, CellState currentPlayer) {
    // Base case: depth 0 or terminal state
    if (depth == 0) {
        return {evaluateBoard(board.boardToString(), currentPlayer), ""};
    }

    std::vector<std::string> legalMoves = board.generateLegalMoves(currentPlayer);
    if (legalMoves.empty()) {
        return {evaluateBoard(board.boardToString(), currentPlayer), ""};
    }

    // Check transposition table
    std::string boardKey = board.boardToString();
    auto it = transpositionTable.find(boardKey);
    if (it != transpositionTable.end() && it->second.depth >= depth) {
        return {it->second.score, it->second.move}; // Reuse cached result if depth is sufficient
    }

    std::string bestMove = "";
    int bestEval;
    char playerChar = (currentPlayer == CellState::BLACK) ? 'b' : 'w';

    if (playerChar == 'b') { // Maximizing (Black)
        bestEval = INT_MIN;
        for (const std::string& move : legalMoves) {
            // Apply move directly to a copy of the board
            AbaloneBoard newBoard = board; // Copy once
            auto parsedBoard = parseBoardFromString(newBoard.boardToString());
            applyMove(parsedBoard, move);
            newBoard = AbaloneBoard(); // Reset
            for (const auto& [pos, color] : parsedBoard) {
                newBoard.setCellState(pos, color == 'b' ? CellState::BLACK : CellState::WHITE);
            }

            auto [eval, _] = minimax(newBoard, depth - 1, alpha, beta, CellState::WHITE);
            if (eval > bestEval) {
                bestEval = eval;
                bestMove = move;
            }
            alpha = std::max(alpha, bestEval);
            if (beta <= alpha) break; // Alpha-beta pruning
        }
    } else { // Minimizing (White)
        bestEval = INT_MAX;
        for (const std::string& move : legalMoves) {
            AbaloneBoard newBoard = board; // Copy once
            auto parsedBoard = parseBoardFromString(newBoard.boardToString());
            applyMove(parsedBoard, move);
            newBoard = AbaloneBoard(); // Reset
            for (const auto& [pos, color] : parsedBoard) {
                newBoard.setCellState(pos, color == 'b' ? CellState::BLACK : CellState::WHITE);
            }

            auto [eval, _] = minimax(newBoard, depth - 1, alpha, beta, CellState::BLACK);
            if (eval < bestEval) {
                bestEval = eval;
                bestMove = move;
            }
            beta = std::min(beta, bestEval);
            if (beta <= alpha) break; // Alpha-beta pruning
        }
    }

    // Store result in transposition table
    transpositionTable[boardKey] = {bestEval, depth, bestMove};
    return {bestEval, bestMove};
}



int main() {
    srand(time(nullptr));

    AbaloneBoard board;
    CellState playerToMove;
    const std::string inputFileName = R"(C:\Users\16046\CLionProjects\AI project\input1.input)";
    // User selects color
    std::string colorChoice;
    std::cout << "Choose your color (b for Black, w for White): ";
    std::cin >> colorChoice;
    CellState minimaxPlayer = (colorChoice == "b") ? CellState::BLACK : CellState::WHITE;
    CellState userPlayer = (colorChoice == "b") ? CellState::WHITE : CellState::BLACK;

    // User selects board layout
    int layoutChoice;
    std::cout << "Choose board layout (1 for Default, 2 for German, 3 for Belgian): ";
    std::cin >> layoutChoice;

    // Write to file based on choices
    std::ofstream inputFile(inputFileName);
    inputFile << colorChoice << std::endl;
    if (layoutChoice == 1) {
        inputFile << "I5w,I6w,I7w,I8w,I9w,H4w,H5w,H6w,H7w,H8w,H9w,G5w,G6w,G7w,A1b,A2b,A3b,A4b,A5b,B1b,B2b,B3b,B4b,B5b,B6b,C3b,C4b,C5b" << std::endl; // Placeholder for Default
    } else if (layoutChoice == 2) {
        inputFile << "H4w,H5w,G3w,G4w,G5w,F3w,F4w,H8b,H9b,G7b,G8b,G9b,F7b,F8b,D2b,D3b,C1b,C2b,C3b,B1b,B2b,D6w,D7w,C5w,C6w,C7w,B5w,B6w" << std::endl; // Placeholder for German
    } else if (layoutChoice == 3) {
        inputFile << "I5w,I6w,H4w,H5w,H6w,G4w,G5w,I8b,I9b,H7b,H8b,H9b,G7b,G8b,C2b,C3b,B1b,B2b,B3b,A1b,A2b,C5w,C6w,B4w,B5w,B6w,A4w,A5w" << std::endl; // Placeholder for Daisy
    }
    inputFile.close();

    // Initial parse of the board
    parseFile(inputFileName, board, playerToMove);

    for (int i = 0; i < MAX_MOVES; i++) {
        auto start = std::chrono::high_resolution_clock::now();

        // Generate all legal moves for the current state
        std::vector<std::string> legalMoves = board.generateLegalMoves(playerToMove);

        if (legalMoves.empty()) {
            std::cout << "No valid moves left. Game over!" << std::endl;
            break;
        }

        // Simulate moves directly in memory
        std::vector<std::string> possibleBoards = generateBoardStates(board, legalMoves);

        // Sort the board states alphabetically (replacing sortStringsInEachLine)
        std::sort(possibleBoards.begin(), possibleBoards.end());

        if (possibleBoards.empty()) {
            std::cout << "No valid moves available!" << std::endl;
            break;
        }

        std::string selectedMove;
        std::string selectedBoard;
        std::string before = board.boardToString();
        // std::cout << "Before: " << before << std::endl;
        if (playerToMove == minimaxPlayer) {
            // Use Minimax for the selected player's turn
            auto [eval, bestMove] = minimax(board, DEPTH, INT_MIN, INT_MAX, minimaxPlayer);
            selectedMove = bestMove;
            // Apply the move
            auto boardState = parseBoardFromString(board.boardToString());
            applyMove(boardState, selectedMove);
            selectedBoard = boardToString(boardState);
            board = AbaloneBoard();
            for (const auto& [pos, color] : boardState) {
                board.setCellState(pos, color == 'b' ? CellState::BLACK : CellState::WHITE);
            }
            std::cout << "AI chose move: " << selectedMove << std::endl;
        } else {
            // User input for the other player
            std::cout << "Your turn: ";
            std::cout << "Your turn! Legal moves: ";
            for (const auto& move : legalMoves) {
                std::cout << move << " ";
            }
            std::cout << "\nEnter your move: ";
            std::cin >> selectedMove;
            // Validate and apply the move
            if (std::find(legalMoves.begin(), legalMoves.end(), selectedMove) != legalMoves.end()) {
                auto boardState = parseBoardFromString(board.boardToString());
                applyMove(boardState, selectedMove);
                selectedBoard = boardToString(boardState);
                board = AbaloneBoard();
                for (const auto& [pos, color] : boardState) {
                    board.setCellState(pos, color == 'b' ? CellState::BLACK : CellState::WHITE);
                }
            } else {
                std::cout << "Invalid move! Game ends." << std::endl;
                return 1;
            }
        }




        // if (playerToMove == CellState::BLACK) {
        //     // Use Minimax for Black's turn
        //     auto [eval, bestMove] = minimax(board, 4, INT_MIN, INT_MAX, CellState::BLACK);
        //     selectedMove = bestMove;
        //
        //     // Apply the move to the board
        //     auto boardState = parseBoardFromString(board.boardToString());
        //     applyMove(boardState, selectedMove);
        //     selectedBoard = boardToString(boardState);
        //
        //     // Update the actual board object
        //     board = AbaloneBoard();  // Reset board
        //     for (const auto& [pos, color] : boardState) {
        //         board.setCellState(pos, color == 'b' ? CellState::BLACK : CellState::WHITE);
        //     }
        // } else {
        //     // Random move for White
        //     int randomIndex = rand() % possibleBoards.size();
        //     selectedBoard = possibleBoards[randomIndex];
        //     selectedMove = (randomIndex < legalMoves.size()) ? legalMoves[randomIndex] : "Unknown";
        //
        //     // Update the board with the selected state
        //     board = AbaloneBoard();
        //     auto boardState = parseBoardFromString(selectedBoard);
        //     for (const auto& [pos, color] : boardState) {
        //         board.setCellState(pos, color == 'b' ? CellState::BLACK : CellState::WHITE);
        //     }
        // }

        // std::cout << "board chosen by algo: " << selectedBoard << std::endl;
        // std::cout << "move chosen by algo: " << selectedMove << std::endl;
        // std::cout << "Turn " << i + 1 << ": Selected move: " << selectedMove << std::endl;

        // Count marbles
        int blackCount = std::count(selectedBoard.begin(), selectedBoard.end(), 'b');
        int whiteCount = std::count(selectedBoard.begin(), selectedBoard.end(), 'w');
        std::cout << "black count " << blackCount << "\n";
        std::cout << "white count " << whiteCount << "\n";

        // Check win conditions
        if (blackCount < 9) {
            std::cout << "White wins" << std::endl;
            return 1;
        }
        if (whiteCount < 9) {
            std::cout << "Black wins" << std::endl;
            return 2;
        }

        // Update input file with new state and switch player
        std::ofstream inputFile(inputFileName);
        playerToMove = (playerToMove == CellState::BLACK) ? CellState::WHITE : CellState::BLACK;
        inputFile << (playerToMove == CellState::BLACK ? "b" : "w") << std::endl;
        inputFile << selectedBoard << std::endl;
        inputFile.close();

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
        std::cout << "Move took " << duration.count() << " seconds)" << std::endl;
    }

    std::cout << "Max number of moves reached" << std::endl;
    return 0;
}
