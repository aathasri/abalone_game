#ifndef GAME_H
#define GAME_H

#include "board.h"
#include "settings.h"
#include "minimax.h"
#include "move.h"
#include <set>
#include <memory>

class Game {
public:
    Game(const GameSettings& settings);

    void play();

    // Public methods for wrapper access
    int getCurrentPlayer() const { return currentPlayer; }
    Board& getBoard() { return board; } // Non-const for modification
    const Board& getBoard() const { return board; } // Const version for safety
    Minimax& getAI() { return ai; }
    void applyMove(const Move& move) { board.applyMove(move); }
    void switchPlayer() { currentPlayer = (currentPlayer == 1) ? 2 : 1; } // Make public
    void incrementMoveCountP1() { moveCountP1++; }
    void incrementMoveCountP2() { moveCountP2++; }
    void decrementMoveCountP1() { if (moveCountP1 > 0) moveCountP1--; }
    void decrementMoveCountP2() { if (moveCountP2 > 0) moveCountP2--; }
    const GameSettings& getSettings() const { return settings; }
    int getMoveCountP1() const { return moveCountP1; }
    int getMoveCountP2() const { return moveCountP2; }
    void applyMoveWithUndo(const Move& move, MoveUndo& undo);

private:
    Board generateStandardBoard();
    Board generateGermanBoard();
    Board generateBelgianBoard();
    Board initializeBoard();
    int overTimeLimitCount = 0;

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