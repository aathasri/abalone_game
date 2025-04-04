#ifndef HEURISTIC_CALCULATOR_H
#define HEURISTIC_CALCULATOR_H

#include "board.h"
#include "move.h"
#include <vector>
#include <queue>
#include <set>

struct HeuristicCache {
    // For each valid cell 'i', we store:
    std::vector<int> occupant;     // occupant[i]: 0 or 1 or 2 (or -1) for the cell i
    std::vector<int> allyCount;    // how many allies are adjacent
    std::vector<int> enemyCount;   // how many enemies are adjacent
    std::vector<int> proximity;    // proximity points for occupant

    int marbleCountP1 = 0;  // store #marbles for player1
    int marbleCountP2 = 0;  // store #marbles for player2

    // The final heuristic after weighting everything
    int totalHeuristic = 0;
};

// Example heuristic calculator
class HeuristicCalculator {
    public:
        Board selectBoard(std::vector<Board> generatedBoards) const;
        int calculateHeuristic(const Board& b) const;
    
        HeuristicCache initHeuristicCache(const Board& b) const;
        HeuristicCache updateHeuristicCache(const Board& parentBoard,
                                            const HeuristicCache& parentCache,
                                            Board& childBoard,
                                            const Move& m) const;
        int getCachedHeuristic(const HeuristicCache& cache) const;
    
        int marbleDifference(int player, const Board& b) const;
    
    private:
        void recalcCellMetrics(int i, const Board& b,
                               HeuristicCache& cache,
                               int centerX, int centerY) const;
    };

#endif