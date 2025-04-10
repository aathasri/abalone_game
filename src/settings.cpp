#include "settings.h"

GameSettings::GameSettings() {}

void GameSettings::setBoardLayout(BoardLayout newLayout) {
    layout = newLayout;
}

void GameSettings::setPlayer1Color(PlayerColour color) {
    player1Color = color;
    player2Color = (color == PlayerColour::BLACK) ? PlayerColour::WHITE : PlayerColour::BLACK;

    playerColourMap.clear();
    playerColourMap[player1Color] = 1;
    playerColourMap[player2Color] = 2;
}

void GameSettings::setGameMode(GameMode mode) {
    gameMode = mode;
}

void GameSettings::setMoveLimit(int limit) {
    moveLimit = limit;
}

void GameSettings::setTimeLimits(bool sameLimit, int limitP1, int limitP2) {
    sameTimeLimit = sameLimit;
    moveTimeLimitPlayer1 = limitP1;
    moveTimeLimitPlayer2 = sameLimit ? limitP1 : limitP2;
}

int GameSettings::getMoveTimeLimit(int playerNum) const {
    if (playerNum == 1) return moveTimeLimitPlayer1;
    if (playerNum == 2) return moveTimeLimitPlayer2;
    throw std::out_of_range("Invalid player number for time limit.");
}