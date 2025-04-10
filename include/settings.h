#ifndef GAMESETTINGS_H
#define GAMESETTINGS_H

#include <array>
#include <map>
#include <string>
#include "board.h"

enum class PlayerColour {
    BLACK,
    WHITE
};

enum class BoardLayout {
    STANDARD,
    GERMAN_DAISY,
    BELGIAN_DAISY
};

enum class GameMode {
    PLAYER_VS_PLAYER,
    PLAYER_VS_COMPUTER
};

class GameSettings {
private:
    BoardLayout layout;
    PlayerColour player1Color;
    PlayerColour player2Color;
    std::map<PlayerColour, int> playerColourMap;
    GameMode gameMode;
    int moveLimit;
    bool sameTimeLimit;
    int moveTimeLimitPlayer1;
    int moveTimeLimitPlayer2;

public:
    GameSettings();

    // Setters
    void setBoardLayout(BoardLayout newLayout);
    void setPlayer1Color(PlayerColour color);
    void setGameMode(GameMode mode);
    void setMoveLimit(int limit);
    void setTimeLimits(bool sameLimit, int limitP1, int limitP2);

    // Getters
    BoardLayout getBoardLayout() const {return layout;}
    const std::map<PlayerColour, int>& getPlayerColourMap() const {return playerColourMap;}
    GameMode getGameMode() const {return gameMode;}
    int getMoveLimit() const {return moveLimit;}
    int getMoveTimeLimit(int playerNum) const;
};

#endif // GAMESETTINGS_H