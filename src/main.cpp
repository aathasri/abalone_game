#include <iostream>
#include "game.h"
#include "settings.h"

int main() {
    std::cout << "Welcome to Abalone!\n";

    // Create and configure game settings
    GameSettings settings;
    settings.setBoardLayout(BoardLayout::STANDARD);              // Choose STANDARD layout
    settings.setPlayer1Color(PlayerColour::BLACK);               // Player 1 is WHITE, Player 2 auto becomes BLACK
    settings.setGameMode(GameMode::PLAYER_VS_COMPUTER);          // Set to Human vs AI
    settings.setMoveLimit(200);                                  // Optional: move limit
    settings.setTimeLimits(true, 5, 5);                        // Both players have 60 seconds per move

    // Start the game
    Game game(settings);
    game.play();

    return 0;
}