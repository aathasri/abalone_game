#pragma once
#include "ai.h"
#include "settings.h"

// Loads the initial board based on the selected starting position
BoardArray loadStartingBoard(const std::string& startingPosition);

// Debug/Utility
void printBoard(const BoardArray& board);