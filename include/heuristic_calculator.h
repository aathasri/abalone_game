#ifndef HEURISTIC_CALCULATOR_H
#define HEURISTIC_CALCULATOR_H

#include "board.h"
#include "move.h"
#include <vector>
#include <set>

// Optional: a structure for storing partial heuristic data
struct HeuristicCache
{
    // occupant[i] for each cell (0.. coords.size()-1)
    std::vector<int> occupant;

    // adjacency-based tallies
    std::vector<int> allyCount;
    std::vector<int> enemyCount;
    std::vector<int> proximity;

    // total marbles for occupant 1 and 2
    int marbleCountP1;
    int marbleCountP2;

    // final heuristic after partial or full calculation
    int totalHeuristic = 0;
};

class HeuristicCalculator {
public:
    // --------------------------------------------------------------------------
    // Selects the board with the best heuristic from a list
    // --------------------------------------------------------------------------
    Board selectBoard(std::vector<Board> generatedBoards) const;

    // --------------------------------------------------------------------------
    // Main heuristic that returns a score from the AI user's perspective
    // --------------------------------------------------------------------------
    int calculateHeuristic(const Board& b) const;

    // --------------------------------------------------------------------------
    // Incremental methods (only if you still need them)
    // --------------------------------------------------------------------------
    HeuristicCache initHeuristicCache(const Board& b) const;
    HeuristicCache updateHeuristicCache(const Board& parentBoard,
                                        const HeuristicCache& parentCache,
                                        Board& childBoard,
                                        const Move& m) const;
    int getCachedHeuristic(const HeuristicCache& cache) const;

    // --------------------------------------------------------------------------
    // Static Weights for each sub-heuristic
    // We separate AI user vs. Opponent for each factor:
    // --------------------------------------------------------------------------
    static int W_COHESION_AI;     // AI user's cohesion
    static int W_COHESION_OPP;    // Opponent's cohesion

    static int W_PUSH_AI;         // AI user's pushPotential
    static int W_PUSH_OPP;        // Opponent's pushPotential

    static int W_EDGE_PUSH_AI;    // AI user's edge pushPotential
    static int W_EDGE_PUSH_OPP;   // Opponent's edge pushPotential


    static int W_VULN_AI;         // AI user's vulnerability
    static int W_VULN_OPP;        // Opponent's vulnerability

    static int W_ISOLATED_AI;     // AI user's isolation
    static int W_ISOLATED_OPP;    // Opponent's isolation

    static int W_PROX_AI;         // AI user's proximity to center
    static int W_PROX_OPP;        // Opponent's proximity to center

    static int W_MDIFF;           // Marble difference (AI's marbles vs. Opp's)

private:
    // Helper for partial BFS or difference-based usage if needed
    int marbleDifference(int player, const Board& b) const;

    // If you do partial re-check in caches
    void recalcCellMetrics(int i, const Board& b,
                           HeuristicCache& cache,
                           int centerX, int centerY) const;
};

#endif // HEURISTIC_CALCULATOR_H