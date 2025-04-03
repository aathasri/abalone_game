#ifndef GAME_H
#define GAME_H

#include "board.h"
#include "minimax.h"
#include "move_generator.h"
#include "settings.h"
#include <memory>

class Game {
private:
    GameSettings settings;
    Board board;
    int currentPlayer;
    int turnCount;
    int moveCountP1;
    int moveCountP2;
    Minimax ai;

public:
    Game(const GameSettings& settings);

    void play();

private:

    std::array<std::array<int, COLS>, ROWS> generateStandardBoard();
    std::array<std::array<int, COLS>, ROWS> generateGermanBoard();
    std::array<std::array<int, COLS>, ROWS> generateBelgianBoard();
    Board initializeBoard();
    void switchPlayer();
    bool isGameOver() const;
    void announceWinner() const;
};

#endif // GAME_H