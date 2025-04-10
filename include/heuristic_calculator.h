#ifndef HEURISTIC_CALCULATOR_H
#define HEURISTIC_CALCULATOR_H

#include "board.h"
#include "move.h"
#include <vector>
#include <bitset>

struct HeuristicCache {
    std::vector<int> occupant;
    std::vector<int> enemyCount;
    int marbleCountP1;
    int marbleCountP2;
    int totalHeuristic;

    HeuristicCache() : marbleCountP1(0), marbleCountP2(0), totalHeuristic(0) {}
};

class HeuristicCalculator {
public:
    HeuristicCalculator() = default;

    int calculateHeuristic(const Board& b) const;
    Board selectBoard(std::vector<Board> generatedBoards) const;

    HeuristicCache initHeuristicCache(const Board& b) const;
    HeuristicCache updateHeuristicCache(
        const Board& parentBoard,
        const HeuristicCache& parentCache,
        Board& childBoard,
        const Move& m
    ) const;

    int getCachedHeuristic(const HeuristicCache& cache) const;

    inline void recalcCellMetrics(
        int i,
        const Board& b,
        HeuristicCache& cache
    ) const;

    int marbleDifference(int player, const Board& b) const;

    // Heuristic weights
    static int W_PUSH_AI;
    static int W_PUSH_OPP;
    static int W_VULN_AI;
    static int W_VULN_OPP;
    static int W_MDIFF;
};

#endif // HEURISTIC_CALCULATOR_H