#ifndef GAME_H
#define GAME_H

#include "board.h"
#include "settings.h"
#include "minimax.h"
#include "move.h"
#include <set>

class Game {
public:
    Game(const GameSettings& settings);

    void play();

private:
    Board generateStandardBoard();
    Board generateGermanBoard();
    Board generateBelgianBoard();
    Board initializeBoard();

    void switchPlayer();
    bool isGameOver() const;
    void announceWinner() const;

    GameSettings settings;
    Board board;
    int turnCount;
    int moveCountP1;
    int moveCountP2;
    int currentPlayer;
    Minimax ai;  // your minimax AI

};

#endif