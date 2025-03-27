#include "settings.h"
#include "board.h"
#include "ai.h"
#include "timer.h"
#include "move.h"
#include "json.hpp"
#include <iostream>

int main() {
    Settings settings = loadSettings("settings.json");
    BoardArray board = loadStartingBoard(settings.startingPosition);
    printBoard(board);

    AggregateTimer agentClock;

    for (int moveNumber = 1; moveNumber <= settings.maxMoves; ++moveNumber) {
        std::cout << "Move #" << moveNumber << " (Agent as " << (settings.playerColor == BLACK ? "Black" : "White") << ")" << std::endl;

        Timer turnTimer;

        // Iterative deepening loop
        int bestScore = -1e9;
        int bestDepthReached = 0;
        Move bestMove; // You'll define this structure later
        double timeUsed = 0.0;

        for (int depth = 1; ; ++depth) {
            double remainingTime = settings.agentTimeLimit - turnTimer.elapsedSeconds();
            if (remainingTime <= 0.05) break;

            // Placeholder: replace with real search later
            std::cout << "[Info] Searching at depth: " << depth << std::endl;
            bestDepthReached = depth;
            bestScore = 0; // temporary placeholder score
            // bestMove = ...
        }

        timeUsed = turnTimer.elapsedSeconds();
        agentClock.addDuration(timeUsed);

        std::cout << "[Info] Move selected at depth " << bestDepthReached
                  << " in " << timeUsed << " seconds."
                  << " Aggregate time: " << agentClock.totalElapsed() << " s\n";

        if (agentClock.totalElapsed() >= settings.agentMaxAggregateTime) {
            std::cout << "[Warning] Agent time exhausted. Game over.\n";
            break;
        }

        // TODO: Apply bestMove to board here
        // board = applyMove(board, bestMove);
        printBoard(board);
    }

    std::cout << "Game loop finished.\n";
    return 0;
}
