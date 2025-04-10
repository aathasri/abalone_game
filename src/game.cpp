#include "game.h"
#include "move_generator.h"
#include "minimax.h"
#include <iostream>
#include <random>
#include <chrono>

Game::Game(const GameSettings& settings)
    : settings(settings),
      board(initializeBoard()), // Use initializeBoard to create starting board.
      turnCount(0),
      moveCountP1(0),
      moveCountP2(0),
      ai(4, settings.getMoveTimeLimit(2), 1),  // Construct AI with maxDepth=4, time limit from settings, and a 1-second buffer.
      overTimeLimitCount(0)  // Initialize overtime counter.
{
    currentPlayer = settings.getPlayerColourMap().at(PlayerColour::BLACK);
}

Board Game::generateStandardBoard()
{
    std::map<PlayerColour, int> colourMap = settings.getPlayerColourMap();

    int bP = colourMap.at(PlayerColour::BLACK);
    int wP = colourMap.at(PlayerColour::WHITE);

    std::cout << wP << " , " << bP << std::endl;

    std::array<std::array<int, COLS>, ROWS> standardBoard = {{
        {-1, -1, -1, -1, wP, wP, wP, wP, wP},
        {-1, -1, -1, wP, wP, wP, wP, wP, wP},
        {-1, -1,  0,  0, wP, wP, wP,  0,  0},
        {-1,  0,  0,  0,  0,  0,  0,  0,  0},
        { 0,  0,  0,  0,  0,  0,  0,  0,  0},
        { 0,  0,  0,  0,  0,  0,  0,  0, -1},
        { 0,  0, bP, bP, bP,  0,  0, -1, -1},
        {bP, bP, bP, bP, bP, bP, -1, -1, -1},
        {bP, bP, bP, bP, bP, -1, -1, -1, -1}
    }};

    auto [adj, c2i, i2c] = Board::createAdjacencyData(standardBoard);
    return Board(standardBoard, adj, c2i, i2c);
}

Board Game::generateGermanBoard()
{
    std::map<PlayerColour, int> colourMap = settings.getPlayerColourMap();

    int bP = colourMap.at(PlayerColour::BLACK);
    int wP = colourMap.at(PlayerColour::WHITE);

    std::array<std::array<int, COLS>, ROWS> germanBoard = {{
        {-1, -1, -1, -1,  0,  0,  0,  0,  0},
        {-1, -1, -1, wP, wP,  0,  0, bP, bP},
        {-1, -1, wP, wP, wP,  0, bP, bP, bP},
        {-1,  0, wP, wP,  0,  0, bP, bP,  0},
        { 0,  0,  0,  0,  0,  0,  0,  0,  0},
        { 0, bP, bP,  0,  0, wP, wP,  0, -1},
        {bP, bP, bP,  0, wP, wP, wP, -1, -1},
        {bP, bP,  0,  0, wP, wP, -1, -1, -1},
        { 0,  0,  0,  0,  0, -1, -1, -1, -1}
    }};

    auto [adj, c2i, i2c] = Board::createAdjacencyData(germanBoard);
    return Board(germanBoard, adj, c2i, i2c);
}

Board Game::generateBelgianBoard()
{
    std::map<PlayerColour, int> colourMap = settings.getPlayerColourMap();

    int bP = colourMap.at(PlayerColour::BLACK);
    int wP = colourMap.at(PlayerColour::WHITE);

    std::array<std::array<int, COLS>, ROWS> belgianBoard = {{
        {-1, -1, -1, -1, wP, wP,  0, bP, bP},
        {-1, -1, -1, wP, wP, wP, bP, bP, bP},
        {-1, -1,  0, wP, wP,  0, bP, bP,  0},
        {-1,  0,  0,  0,  0,  0,  0,  0,  0},
        { 0,  0,  0,  0,  0,  0,  0,  0,  0},
        { 0,  0,  0,  0,  0,  0,  0,  0, -1},
        { 0, bP, bP,  0, wP, wP,  0, -1, -1},
        {bP, bP, bP, wP, wP, wP, -1, -1, -1},
        {bP, bP,  0, wP, wP, -1, -1, -1, -1}
    }};

    auto [adj, c2i, i2c] = Board::createAdjacencyData(belgianBoard);
    return Board(belgianBoard, adj, c2i, i2c);
}

Board Game::initializeBoard()
{
    if (settings.getBoardLayout() == BoardLayout::STANDARD) {
        return generateStandardBoard();
    } else if (settings.getBoardLayout() == BoardLayout::GERMAN_DAISY) {
        return generateGermanBoard();
    } else {
        return generateBelgianBoard();
    }
}

void Game::play() {
    std::cout << "Starting Game:\n\n";
    board.printBoard();

    std::cout << "Board Made :\n\n";

    static std::mt19937 rng(std::random_device{}());

    bool gameRunning = true;

    while (!isGameOver()) {
        // Print current piece counts.
        std::cout << "\nCurrent Board Counts:\n";
        std::cout << "Player 1 Pieces: " << board.getNumPlayerOnePieces() << "\n";
        std::cout << "Player 2 Pieces: " << board.getNumPlayerTwoPieces() << "\n";

        // Fix 3: Increment turnCount only here, display upcoming turn.
        std::cout << "\nTurn " << turnCount + 1 << ": Player "
                  << currentPlayer << " ("
                  << ((settings.getPlayerColourMap().at(PlayerColour::BLACK) == currentPlayer) ? "Black" : "White")
                  << ")\n";

        // Generate moves.
        MoveGenerator moveGen;
        moveGen.generateMoves(currentPlayer, board);
        const std::set<Move>& validMoves = moveGen.getGeneratedMoves();

        if (validMoves.empty()) {
            std::cout << "\nPlayer " << currentPlayer << " has no valid moves.\n";
            break;
        }

        Move chosenMove;
        std::vector<Move> moveList(validMoves.begin(), validMoves.end());

        if (currentPlayer == 1) {
            std::uniform_int_distribution<int> dist(0, static_cast<int>(moveList.size()) - 1);
            int randomIndex = dist(rng);
            chosenMove = moveList[randomIndex];
            std::cout << "\nRandomly selected move for Player 1: index " << randomIndex << "\n";
            chosenMove.printString();
        } else {
            std::cout << "\nAI thinking...\n";
            auto startTime = std::chrono::steady_clock::now();
            chosenMove = ai.findBestMove(board, currentPlayer);
            auto endTime = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
            std::cout << "AI took " << duration.count() << " ms to pick a move.\n";
            // Check if we are over the designated time limit.
            if (duration.count() > settings.getMoveTimeLimit(2) * 1000) {
                std::cout << "OVER TIME LIMIT\n";
                overTimeLimitCount++;
            }
            std::cout << "AI chose: ";
            chosenMove.printString();
        }

        board.applyMove(chosenMove);
        board.printBoard();

        if (currentPlayer == 1)
            moveCountP1++;
        else
            moveCountP2++;

        turnCount++; // Increment turn count after each move.
        switchPlayer();
    }

    announceWinner();
}

bool Game::isGameOver() const {
    const int WIN_THRESHOLD = 8; // Game over condition (8 pieces remaining)
    return board.getNumPlayerOnePieces() <= WIN_THRESHOLD
           || board.getNumPlayerTwoPieces() <= WIN_THRESHOLD;
}

void Game::announceWinner() const {
    int p1 = board.getNumPlayerOnePieces();
    int p2 = board.getNumPlayerTwoPieces();
    const int WIN_THRESHOLD = 8;

    std::cout << "\nGame Over!\n";
    std::cout << "Player 1 ("
              << ((settings.getPlayerColourMap().at(PlayerColour::WHITE) == 1) ? "White" : "Black")
              << "): " << p1 << " marbles left\n";
    std::cout << "Player 2 ("
              << ((settings.getPlayerColourMap().at(PlayerColour::BLACK) == 2) ? "Black" : "White")
              << "): " << p2 << " marbles left\n";

    if (p1 <= WIN_THRESHOLD) {
        std::cout << "Player 2 wins by pushing 6 or more of Player 1's pieces off!\n";
    } else if (p2 <= WIN_THRESHOLD) {
        std::cout << "Player 1 wins by pushing 6 or more of Player 2's pieces off!\n";
    } else {
        std::cout << "Game ended without a winner (e.g., no valid moves).\n";
    }
    // Print the overtime count.
    std::cout << "OVER TIME LIMIT occurred " << overTimeLimitCount << " times during the game.\n";
}


void Game::applyMoveWithUndo(const Move& move, MoveUndo& undo) {
    board.makeMove(move, undo);
}

