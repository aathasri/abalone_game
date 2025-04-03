#ifndef HEURISTIC_CALCULATOR_H
#define HEURISTIC_CALCULATOR_H

#include "board.h"
#include <vector>
#include <queue>
#include <cmath>
#include <set>
#include <chrono>

class HeuristicCalculator {

private:

public:

    Board selectBoard(std::vector<Board> generatedBoards) const;

    static int calculateHeuristic(Board b);

private:

    static int marbleDifference(int player, const Board& b);  // Difference in marble count
    static int centerProximity(int player, const Board& b);   // Measures how close the player's marbles are to the center
    static int groupCohesion(int player, const Board& b);     // Measures how many same-colored adjacent pairs exist
    static int pushPotential(int player, const Board& b);     // Measures how many friendly marbles are next to opponent marbles
    static int pushVulnerability(int player, const Board& b); // Measures how many player marbles are vulnerable to being pushed
    static int groupingAdvantage(int player, const Board& b); // Counts how many groups have a player majority over the opponent
    static int isolationPenalty(int player, const Board& b);  // Counts how many player marbles have no same-colored neighbors
    static int lineAlignment(int player, const Board& b);   // Detects how many 3-in-a-line formations the player has

};

#endif // HEURISTIC_CALCULATOR_H