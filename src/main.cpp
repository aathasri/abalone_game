#include <iostream>
#include <cstdlib>  // For system() function

int main(int argc, char* argv[]) {
    std::string movegenCmd;
    std::string inputFile;
    if (argc == 1) {
        movegenCmd = "movegen Test1.input";
        inputFile = "Test1.input";
    } else if (argc == 2) {
        inputFile = argv[1];
        movegenCmd = "movegen " + inputFile;
    } else { // Check for correct number of arguments
        movegenCmd = "movegen " + inputFile;
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    // Execute movegen
    int movegenStatus = system(movegenCmd.c_str());
    if (movegenStatus != 0) {
        std::cerr << "Error executing movegen." << std::endl;
        return 1;
    }

    // Generate moves file name by replacing ".input" with ".moves"
    std::string movesFile = inputFile.substr(0, inputFile.find_last_of(".")) + ".moves";

    // Execute boardgen
    std::string boardgenCmd = "boardgen " + inputFile + " " + movesFile;
    int boardgenStatus = system(boardgenCmd.c_str());
    if (boardgenStatus != 0) {
        std::cerr << "Error executing boardgen." << std::endl;
        return 1;
    }

    return 0;
}
