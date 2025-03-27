#pragma once
#include <string>

enum CellState { EMPTY = 0, BLACK, WHITE };

struct Settings {
    CellState playerColor = BLACK;
    std::string startingPosition = "standard";
    double agentTimeLimit = 5.0;
    double opponentTimeLimit = 5.0;
    int maxMoves = 200;
    double agentMaxAggregateTime = 300.0;
    double opponentMaxAggregateTime = 300.0;
};

struct Settings loadSettings(const std::string& filename);
