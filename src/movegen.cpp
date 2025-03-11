#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>

enum class CellState { EMPTY, BLACK, WHITE };

const std::vector<std::string> directions = {"NE", "NW", "E", "W", "SE", "SW"};

struct Cell {
    CellState state = CellState::EMPTY;
};

class AbaloneBoard {
    std::map<std::string, Cell> board;

public:
    AbaloneBoard() {
        // create board with no marbles - all cells set to empty
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

    void setCellState(const std::string& pos, CellState state) {
        if (board.find(pos) != board.end()) {
            board[pos].state = state;
        }
    }

    [[nodiscard]] CellState getCellState(const std::string& pos) const {
        if (board.find(pos) != board.end()) {
            return board.at(pos).state;
        }
        return CellState::EMPTY;
    }

    // Check if a position is valid (i.e., is on the board)
    [[nodiscard]] bool isValidPosition(const std::string& pos) const {
        return board.find(pos) != board.end();
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

private:
    // Helper function to get the adjacent position in a given direction
    static std::string getAdjacentPosition(const std::string& pos, const std::string& dir) {
        const char col = pos[0];
        const int row = pos[1] - '0';

        if (dir == "NE") {
            return std::string(1, col + 1) + std::to_string(row + 1); // G6 from F5
        }
        if (dir == "E") {
            return std::string(1, col) + std::to_string(row + 1); // G5 from F5
        }
        if (dir == "NW") {
            return std::string(1, col + 1) + std::to_string(row); // F6 from F5
        }
        if (dir == "SE") {
            return std::string(1, col - 1) + std::to_string(row); // F4 from F5
        }
        if (dir == "W") {
            return std::string(1, col) + std::to_string(row - 1); // E5 from F5
        }
        if (dir == "SW") {
            return std::string(1, col - 1) + std::to_string(row - 1); // E4 from F5
        }
        return "";
    }

    // Generate legal single marble moves
    void generateSingleMarbleMoves(CellState player, std::vector<std::string>& legalMoves) const {
        for (const auto& [pos, cell] : board) {
            if (cell.state == player) {
                // for each direction, check if the resulting position contains a marble or is on the board
                // if so, log the move
                for (const auto& dir : directions) {
                    if (std::string targetPos = getAdjacentPosition(pos, dir); isValidPosition(targetPos)
                        && getCellState(targetPos) == CellState::EMPTY) {
                        legalMoves.push_back("Inline: " + pos + " to " + targetPos + ", Direction:" + dir);
                    }
                }
            }
        }
    }

    // Generate inline moves for 2 marbles
    void generateDoubleInlineMoves(CellState player, std::vector<std::string>& legalMoves) const {
       for (const auto& [pos, cell] : board) {
        if (cell.state == player) {
            for (const auto& dir : directions) {
                std::string nextPos = getAdjacentPosition(pos, dir);
                std::string nextNextPos = getAdjacentPosition(pos, dir);
                std::string pushPos = getAdjacentPosition(pos, dir);

                if(!isValidPosition(nextPos) || !isValidPosition(nextNextPos)) continue;

                CellState nextState = getCellState(nextPos);
                CellState nextNextState = getCellState(nextNextPos);

                //Case 1: Empty space after two marbles
                if (nextState == player && nextNextState == CellState::EMPTY) {
                    legalMoves.push_back("Inline Move: " + pos + " & " + nextPos + " to " + nextNextPos + ", Direction: " + dir);
                }

                //Case 2: Pushing an oppenents marble
                if (nextState == player && nextNextState != player && nextNextState != CellState::EMPTY) {
                    legalMoves.push_back("Inline Move: " + pos + " & " + nextPos + " to " + nextNextPos + ", Direction: " + dir);
                }
            }
        }
       }
    }

    // Generate inline moves for 3 marbles
    void generateTripleInlineMoves(CellState player, std::vector<std::string>& legalMoves) const {
        for (const auto& [pos, cell] : board) {
            if (cell.state == player) {
                for (const auto& dir : directions) {
                    std::string nextPos = getAdjacentPosition(pos, dir);
                    std::string nextNextPos = getAdjacentPosition(nextPos, dir);
                    std::string nextNextNextPos = getAdjacentPosition(nextNextPos, dir);
                    std::string pushPos = getAdjacentPosition(nextNextNextPos, dir);
    
                    if (!isValidPosition(nextPos) || !isValidPosition(nextNextPos) || !isValidPosition(nextNextNextPos)) continue;
    
                    CellState nextState = getCellState(nextPos);
                    CellState nextNextState = getCellState(nextNextPos);
                    CellState nextNextNextState = getCellState(nextNextNextPos);
    
                    // Case 1: Empty space after three marbles
                    if (nextState == player && nextNextState == player && nextNextNextState == CellState::EMPTY) {
                        legalMoves.push_back("Inline Move: " + pos + ", " + nextPos + " & " + nextNextPos + " to " + nextNextNextPos + ", Direction: " + dir);
                    }
    
                    // Case 2: Pushing one or two opponent marbles
                    if (nextState == player && nextNextState == player && nextNextNextState != player && nextNextNextState != CellState::EMPTY) {
                        if (isValidPosition(pushPos) && getCellState(pushPos) == CellState::EMPTY) {
                            legalMoves.push_back("Push Move: " + pos + ", " + nextPos + " & " + nextNextPos + " pushing " + nextNextNextPos + " to " + pushPos + ", Direction: " + dir);
                        }
                    }
                }
            }
        }
    }

    // Generate sidestep moves for 2 marbles
    void generateDoubleSidestepMoves(CellState player, std::vector<std::string>& legalMoves) const {
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

                                legalMoves.push_back("Sidestep: " + pos + " and " + adjacentPos +
                                                     " -> " + target1 + " and " + target2 + ", Direction:" + dir);
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

                                    legalMoves.push_back("Sidestep: " + pos + ", " + pos2 + ", " + pos3 + " to " +
                                                         target1 + ", " + target2 + ", " + target3 + ", Direction:" + dir);
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

    parseFile(R"(Test1.input)", board, playerToMove);

    // Generate all legal moves for the player to move
    std::vector<std::string> legalMoves = board.generateLegalMoves(playerToMove);

    // Write the legal moves to moves.txt
    std::ofstream outFile(R"(Test1.moves)");
    if (!outFile) {
        std::cerr << "Error writing to file." << std::endl;
        return 1;
    }

    outFile << "Legal moves for " << (playerToMove == CellState::BLACK ? "Black" : "White") << ":\n";
    for (const auto& move : legalMoves) {
        outFile << move << std::endl;
    }

    outFile.close();
    std::cout << "Legal moves written to moves.txt" << std::endl;

    // TODO: read moves from moves.txt, create resulting boards for each move

    return 0;

}
