#include "settings.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

Settings loadSettings(const std::string& filename) {
    Settings settings;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "[Error] Could not open settings file: " << filename << std::endl;
        return settings;
    }

    json j;
    try {
        file >> j;
        settings.playerColor = (j["player_color"] == "black") ? CellState::BLACK : CellState::WHITE;
        settings.startingPosition = j.value("starting_position", "standard");
        settings.agentTimeLimit = j.value("agent_time_limit", 5.0);
        settings.opponentTimeLimit = j.value("opponent_time_limit", 5.0);
        settings.maxMoves = j.value("max_moves", 200);
        settings.agentMaxAggregateTime = j.value("agent_max_aggregate_time", 300.0);
        settings.opponentMaxAggregateTime = j.value("opponent_max_aggregate_time", 300.0);
    } catch (const std::exception& e) {
        std::cerr << "[Error] Failed to parse settings: " << e.what() << std::endl;
    }

    return settings;
}
