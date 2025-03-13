#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>

/*
 * Generate legal moves given a file containing a player's colour and a valid board
 */

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

    // change a cell's state
    void setCellState(const std::string& pos, CellState state) {
        if (board.find(pos) != board.end()) {
            board[pos].state = state;
        }
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
            // if (col < 'I' && row < 9) {
                adjPos = std::string(1, col + 1) + std::to_string(row + 1);
            // }
        } else if (dir == "E") {
            // if (row < 9) {
                adjPos = std::string(1, col) + std::to_string(row + 1);
            // }
        } else if (dir == "NW") {
            // if (col < 'I' && row > 1) {
                adjPos = std::string(1, col + 1) + std::to_string(row);
            // }
        } else if (dir == "SE") {
            // if (col > 'A' && row < 9) {
                adjPos = std::string(1, col - 1) + std::to_string(row);
            // }
        } else if (dir == "W") {
            // if (row > 1) {
                adjPos = std::string(1, col) + std::to_string(row - 1);
            // }
        } else if (dir == "SW") {
            // if (col > 'A' && row > 1) {
                adjPos = std::string(1, col - 1) + std::to_string(row - 1);
            // }
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

// Parse the file and initialize the board
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

int main() {
    AbaloneBoard board;
    CellState playerToMove;

    // parseFile(R"(Test1.input)", board, playerToMove);
    parseFile(R"(C:\Users\16046\CLionProjects\untitled9\test4.input)", board, playerToMove);


    // Generate all legal moves for the player to move
    std::vector<std::string> legalMoves = board.generateLegalMoves(playerToMove);

    std::ofstream outFile(R"(test4.moves)");
    if (!outFile) {
        std::cerr << "Error writing to file." << std::endl;
        return 1;
    }

    for (const auto& move : legalMoves) {
        outFile << move << std::endl;
    }

    outFile.close();
    std::cout << "Legal moves written to file" << std::endl;
    return 0;
}