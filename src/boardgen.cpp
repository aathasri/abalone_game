#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <vector>

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




void applyMove( std::unordered_map<std::string, char>& boardState, const std::string& move) {
    // check if the move is an inline or sidestep
    char moveType = move[0];

    if (moveType == 'i') {
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

        // position is occupied - attempt to push
        std::vector<std::pair<std::string, char>> toMove;
        std::string currentPos = position;
        std::string nextPos = newPos;

        // collect all occupied positions in the push chain
        while (boardState.find(nextPos) != boardState.end()) {
            toMove.emplace_back(currentPos, boardState[currentPos]);
            currentPos = nextPos;
            nextPos = movePosition(boardState, currentPos, direction);
        }

        // add the final empty position to the chain
        toMove.emplace_back(currentPos, boardState[currentPos]);
        std::string finalEmptyPos = nextPos;  // Where the last marble will go

        // sift all marbles forward
        for (size_t i = toMove.size() - 1; i > 0; --i) {
            boardState[toMove[i].first] = toMove[i-1].second;
        }
        // place last marble in the empty pos
        boardState[finalEmptyPos] = toMove[toMove.size() - 1].second;
        // Place the original moving marble
        boardState[toMove[0].first] = color;

        // erase original position if we actually pushed
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

// simulate moves and write boards to a new file
void simulateMoves(const std::string& boardFile, const std::string& movesFile, const std::string& outputFile) {
    auto initialBoardState = parseBoard(boardFile);

    std::ofstream outFile(outputFile);
    if (!outFile) {
        std::cerr << "Error opening output file." << std::endl;
        return;
    }

    // Read the moves file
    std::ifstream moveFile(movesFile);
    std::string move;
    while (std::getline(moveFile, move)) {
        // create a copy of the initial board state for each move
        auto boardState = initialBoardState;

        // apply the current move to the new board state
        applyMove(boardState, move);

        // convert the board to string format
        std::string boardStr = boardToString(boardState);
        boardStr = removeSingleCharValues(boardStr);

        // write the updated board to the file
        outFile << boardStr << std::endl;
    }

    outFile.close();
}


int main() {
    simulateMoves(R"(rizz)", R"(mr.beast)", "newboards.txt");
    return 0;
}


