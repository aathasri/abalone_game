#include "game.h"
#include "move_generator.h"
#include "board_generator.h"
#include "minimax.h"
#include <iostream>
#include <random>

Game::Game(const GameSettings& settings)
    : settings(settings),
      board(Board(generateStandardBoard())),
      turnCount(0),
      moveCountP1(0),
      moveCountP2(0),
      ai(3)
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

    bool gameRunning = true;

    while (!isGameOver()) {
        turnCount++;
        std::cout << "\nTurn " << turnCount << ": Player " << currentPlayer << " ("
                  << (settings.getPlayerColourMap().at(PlayerColour::BLACK) == currentPlayer ? "Black" : "White")
                  << ")\n";

        MoveGenerator moveGen;
        moveGen.generateMoves(currentPlayer, board);
        const std::set<Move>& validMoves = moveGen.getGeneratedMoves();

        if (validMoves.empty()) {
            std::cout << "Player " << currentPlayer << " has no valid moves.\n";
            break;
        }

        Move chosenMove;

        if (currentPlayer == 1) {
            std::cout << "Available moves: " << validMoves.size() << ". Enter index: \n";
            int i = 0;
            std::vector<Move> moveList(validMoves.begin(), validMoves.end());
            for (const Move& m : moveList) {
                std::cout << i << ": ";
                m.printString();
                ++i;
            }
            int choice;
            std::cin >> choice;
            while (choice < 0 || choice >= static_cast<int>(moveList.size())) {
                std::cout << "Invalid choice. Try again: ";
                std::cin >> choice;
            }
            chosenMove = moveList[choice];
        } else {
            std::cout << "AI thinking...\n";
            chosenMove = ai.findBestMove(board, currentPlayer);
            std::cout << "AI chose: ";
            chosenMove.printString();
        }

        board.applyMove(chosenMove);
        board.printBoard();

        if (currentPlayer == 1) moveCountP1++;
        else moveCountP2++;

        switchPlayer();
    }

    announceWinner();
}

void Game::switchPlayer() {
    currentPlayer = (currentPlayer == 1) ? 2 : 1;
    turnCount++;
}

bool Game::isGameOver() const {
    return board.getNumPlayerOnePieces() == 0 || board.getNumPlayerTwoPieces() == 0;
}

void Game::announceWinner() const {
    int p1 = board.getNumPlayerOnePieces();
    int p2 = board.getNumPlayerTwoPieces();

    std::cout << "\nGame Over!\n";
    std::cout << "Player 1 (" << (settings.getPlayerColourMap().at(PlayerColour::WHITE) == 1 ? "White" : "Black") << "): " << p1 << " marbles left\n";
    std::cout << "Player 2 (" << (settings.getPlayerColourMap().at(PlayerColour::BLACK) == 2 ? "Black" : "White") << "): " << p2 << " marbles left\n";

    if (p1 > p2) {
        std::cout << "Player 1 wins!\n";
    } else if (p2 > p1) {
        std::cout << "Player 2 wins!\n";
    } else {
        std::cout << "It's a draw!\n";
    }
}