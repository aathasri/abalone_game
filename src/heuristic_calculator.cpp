#include "heuristic_calculator.h"
#include <cmath>
#include <iostream>
#include <set>
#include <queue>
#include <limits>
#include <algorithm>

// --------------------------------------------------------------------------
// STATIC WEIGHTS - Already updated as per suggestion 1
// --------------------------------------------------------------------------
int HeuristicCalculator::W_COHESION_AI     = 0;
int HeuristicCalculator::W_COHESION_OPP    = 0;
int HeuristicCalculator::W_PUSH_AI         = 40; 
int HeuristicCalculator::W_PUSH_OPP        = 0;
int HeuristicCalculator::W_VULN_AI         = 0;
int HeuristicCalculator::W_VULN_OPP        = 65;
int HeuristicCalculator::W_ISOLATED_AI     = 40;
int HeuristicCalculator::W_ISOLATED_OPP    = 0;
int HeuristicCalculator::W_PROX_AI         = 50;
int HeuristicCalculator::W_PROX_OPP        = 0;
int HeuristicCalculator::W_MDIFF           = 850;

// --------------------------------------------------------------------------
// selectBoard(...) - picks the board with the best heuristic from a list
// --------------------------------------------------------------------------
Board HeuristicCalculator::selectBoard(std::vector<Board> generatedBoards) const
{
    Board bestBoard = generatedBoards[0];
    int bestHeuristic = std::numeric_limits<int>::min();

    for (Board& candidate : generatedBoards) {
        int currHeuristic = calculateHeuristic(candidate);
        if (currHeuristic > bestHeuristic) {
            bestHeuristic = currHeuristic;
            bestBoard = candidate;
        }
        candidate.printPieces();
        std::cout << "Heuristic: " << currHeuristic << std::endl;
    }
    return bestBoard;
}

// --------------------------------------------------------------------------
// The main Heuristic (AI perspective):
// occupant code 1 = Opponent, occupant code 2 = AI (Computer)
// --------------------------------------------------------------------------
int HeuristicCalculator::calculateHeuristic(const Board& b) const
{
    // Fix the role assignments: Player 2 (occupant 2) is AI, Player 1 (occupant 1) is Opponent.
    const int AIuser   = 2;
    const int Opponent = 1;

    const auto& board   = b.getBoard();
    const auto& adjList = b.getAdjacencyList();
    const auto& coords  = b.getIndexToCoord();

    if (coords.size() != adjList.size()) {
        std::cerr << "[ERROR] mismatch in calculateHeuristic\n";
        return 0;
    }

    // Initialize metrics for both occupant codes. We use indices 1 and 2.
    int cohesion[3]      = {0, 0, 0};
    int pushPotential[3] = {0, 0, 0};
    int vulnerability[3] = {0, 0, 0};
    int isolated[3]      = {0, 0, 0};
    int proximity[3]     = {0, 0, 0};

    // Define flags for whether to compute each heuristic metric.
    bool computeCohesion      = (W_COHESION_AI != 0 || W_COHESION_OPP != 0);
    bool computePushPotential = (W_PUSH_AI != 0 || W_PUSH_OPP != 0);
    bool computeVulnerability = (W_VULN_AI != 0 || W_VULN_OPP != 0);
    bool computeIsolated      = (W_ISOLATED_AI != 0 || W_ISOLATED_OPP != 0);
    bool computeProximity     = (W_PROX_AI != 0 || W_PROX_OPP != 0);

    int centerX = ROWS / 2;
    int centerY = COLS / 2;

    // Number of marbles (opponent pieces are on board positions for occupant code 1;
    // AI pieces are on positions for occupant code 2).
    int oppMarbles = b.getNumPlayerOnePieces();  // Opponent: occupant code 1
    int aiMarbles  = b.getNumPlayerTwoPieces();  // AI: occupant code 2

    // Tally stats (compute only those metrics with non-zero weights)
    for (int i = 0; i < static_cast<int>(coords.size()); ++i) {
        auto [x, y] = coords[i];
        int occupant = board[x][y];
        if (occupant != AIuser && occupant != Opponent) {
            continue;
        }

        // Calculate proximity (using Manhattan distance from center)
        if (computeProximity) {
            int dist = std::abs(x - centerX) + std::abs(y - centerY);
            proximity[occupant] += std::max(0, 10 - dist);
        }

        if (computeCohesion || computePushPotential || computeVulnerability || computeIsolated) {
            int allies = 0, enemies = 0;
            // Look at all neighboring cells (using the adjacency list)
            for (int neighborIdx : adjList[i]) {
                auto [nx, ny] = coords[neighborIdx];
                int neighbor = board[nx][ny];
                if (neighbor == occupant) {
                    if (computeCohesion) {
                        cohesion[occupant]++;
                    }
                    allies++;
                } else if (neighbor == (3 - occupant)) {
                    if (computePushPotential) {
                        // Remove edge-based extra incentive; just count enemy once.
                        pushPotential[occupant]++;
                    }
                    enemies++;
                }
            }
            if (computeIsolated && allies == 0) {
                isolated[occupant]++;
            }
            if (computeVulnerability && enemies >= 2) {
                vulnerability[occupant]++;
            }
        }
    }

    // Compute weighted sum from the AI's perspective
    int score = 0;
    score += (W_COHESION_AI  * cohesion[AIuser])    - (W_COHESION_OPP * cohesion[Opponent]);
    score += (W_PUSH_AI      * pushPotential[AIuser]) - (W_PUSH_OPP    * pushPotential[Opponent]);
    score -= (W_VULN_AI      * vulnerability[AIuser]);
    score += (W_VULN_OPP     * vulnerability[Opponent]);
    score -= (W_ISOLATED_AI  * isolated[AIuser]);
    score += (W_ISOLATED_OPP * isolated[Opponent]);
    score += (W_PROX_AI      * proximity[AIuser])     - (W_PROX_OPP    * proximity[Opponent]);

    // Marble difference contribution (favors AI if it has more pieces than the opponent)
    int mDiff = (aiMarbles - oppMarbles);
    score += (W_MDIFF * mDiff);

    // Removed the edge bonus block entirely.

    return score;
}

// --------------------------------------------------------------------------
// marbleDifference(...) for sub-calculations
// --------------------------------------------------------------------------
int HeuristicCalculator::marbleDifference(int player, const Board& b) const
{
    int p1Count = b.getNumPlayerOnePieces(); // Opponent (occupant code 1)
    int p2Count = b.getNumPlayerTwoPieces(); // AI (occupant code 2)
    return (player == 1) ? (p1Count - p2Count) : (p2Count - p1Count);
}

// --------------------------------------------------------------------------
// (Optional) For incremental caching, these remain mostly the same
// --------------------------------------------------------------------------
HeuristicCache HeuristicCalculator::initHeuristicCache(const Board& b) const
{
    HeuristicCache cache;
    const auto& coords   = b.getIndexToCoord();
    const auto& adjList  = b.getAdjacencyList();
    const auto& boardArr = b.getBoard();

    if (coords.size() != adjList.size()) {
        std::cerr << "[ERROR] mismatch in initHeuristicCache\n";
        return cache; // empty
    }

    cache.occupant.resize(coords.size(), 0);
    cache.allyCount.resize(coords.size(), 0);
    cache.enemyCount.resize(coords.size(), 0);
    cache.proximity.resize(coords.size(), 0);

    cache.marbleCountP1 = b.getNumPlayerOnePieces();
    cache.marbleCountP2 = b.getNumPlayerTwoPieces();

    int centerX = ROWS / 2;
    int centerY = COLS / 2;

    for (int i = 0; i < static_cast<int>(coords.size()); ++i) {
        auto [x, y] = coords[i];
        cache.occupant[i] = boardArr[x][y];
    }

    int cohesion[3]      = {0, 0, 0};
    int pushPotential[3] = {0, 0, 0};
    int vulnerability[3] = {0, 0, 0};
    int isolated[3]      = {0, 0, 0};
    int proximityArr[3]  = {0, 0, 0};

    for (int i = 0; i < static_cast<int>(coords.size()); ++i) {
        recalcCellMetrics(i, b, cache, centerX, centerY);

        int occupant = cache.occupant[i];
        if (occupant != 1 && occupant != 2) {
            continue;
        }

        proximityArr[occupant] += cache.proximity[i];

        int allies  = cache.allyCount[i];
        int enemies = cache.enemyCount[i];
        if (allies == 0) isolated[occupant]++;
        if (enemies >= 2) vulnerability[occupant]++;
        cohesion[occupant]      += allies;
        pushPotential[occupant] += enemies;
    }

    int score = 0;
    score += (cohesion[1]      - cohesion[2]);
    score += (pushPotential[1] - pushPotential[2]);
    score += (vulnerability[2] - vulnerability[1]);
    score += (isolated[2]      - isolated[1]);
    score += (proximityArr[1]  - proximityArr[2]);
    int p1Count = b.getNumPlayerOnePieces();
    int p2Count = b.getNumPlayerTwoPieces();
    score += (p1Count - p2Count);

    cache.totalHeuristic = score;
    return cache;
}

HeuristicCache HeuristicCalculator::updateHeuristicCache(
    const Board& parentBoard,
    const HeuristicCache& parentCache,
    Board& childBoard,
    const Move& m
) const
{
    HeuristicCache newCache = parentCache;

    const auto& coords  = childBoard.getIndexToCoord();
    const auto& adjList = childBoard.getAdjacencyList();
    const auto& boardArr = childBoard.getBoard();
    if (coords.size() != adjList.size()) {
        std::cerr << "[ERROR] coords vs. adjList mismatch\n";
        return newCache; // partial update
    }

    newCache.marbleCountP1 = childBoard.getNumPlayerOnePieces();
    newCache.marbleCountP2 = childBoard.getNumPlayerTwoPieces();

    std::set<int> cellsToUpdate;
    for (int i = 0; i < m.getSize(); ++i) {
        auto [oldX, oldY] = m.getPosition(i);
        auto oldIt = childBoard.getCoordToIndex().find({oldX, oldY});
        if (oldIt != childBoard.getCoordToIndex().end()) {
            cellsToUpdate.insert(oldIt->second);
        }
        auto [dx, dy] = DirectionHelper::getDelta(m.getDirection());
        int newX = oldX + dx;
        int newY = oldY + dy;
        auto newIt = childBoard.getCoordToIndex().find({newX, newY});
        if (newIt != childBoard.getCoordToIndex().end()) {
            cellsToUpdate.insert(newIt->second);
        }
    }

    std::set<int> extended;
    for (int cIdx : cellsToUpdate) {
        extended.insert(cIdx);
        for (int nIdx : adjList[cIdx]) {
            extended.insert(nIdx);
        }
    }

    for (int i : extended) {
        auto [x, y] = coords[i];
        newCache.occupant[i] = boardArr[x][y];
    }

    for (int i : extended) {
        newCache.allyCount[i]  = 0;
        newCache.enemyCount[i] = 0;
        newCache.proximity[i]  = 0;
    }

    int centerX = ROWS / 2;
    int centerY = COLS / 2;
    for (int i : extended) {
        recalcCellMetrics(i, childBoard, newCache, centerX, centerY);
    }

    newCache.totalHeuristic = calculateHeuristic(childBoard);
    return newCache;
}

int HeuristicCalculator::getCachedHeuristic(const HeuristicCache& cache) const
{
    return cache.totalHeuristic;
}

// --------------------------------------------------------------------------
// Recompute adjacency and proximity for cell i, if needed
// --------------------------------------------------------------------------
void HeuristicCalculator::recalcCellMetrics(
    int i, 
    const Board& b,
    HeuristicCache& cache,
    int centerX, 
    int centerY
) const
{
    const auto& coords  = b.getIndexToCoord();
    const auto& adjList = b.getAdjacencyList();
    const auto& boardArr = b.getBoard();

    auto [x, y] = coords[i];
    int occupant = boardArr[x][y];

    cache.allyCount[i]  = 0;
    cache.enemyCount[i] = 0;
    cache.proximity[i]  = 0;

    if (occupant != 1 && occupant != 2) {
        cache.occupant[i] = occupant; // 0 or -1
        return;
    }

    cache.occupant[i] = occupant;

    int dist = std::abs(x - centerX) + std::abs(y - centerY);
    cache.proximity[i] = std::max(0, 10 - dist);

    for (int neighborIdx : adjList[i]) {
        auto [nx, ny] = coords[neighborIdx];
        int neighVal = boardArr[nx][ny];
        if (neighVal == occupant) {
            cache.allyCount[i]++;
        } else if (neighVal == 3 - occupant) {
            cache.enemyCount[i]++;
        }
    }
}