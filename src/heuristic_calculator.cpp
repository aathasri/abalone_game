#include "heuristic_calculator.h"
#include <cmath>
#include <iostream>
#include <set>
#include <queue>
#include <limits>
#include <algorithm>

// --------------------------------------------------------------------------
// STATIC WEIGHTS - Updated; cohesion, prox, and isolation removed
// --------------------------------------------------------------------------
int HeuristicCalculator::W_PUSH_AI   = 40; 
int HeuristicCalculator::W_PUSH_OPP  = 30;
int HeuristicCalculator::W_VULN_AI   = 50;
int HeuristicCalculator::W_VULN_OPP  = 75;
int HeuristicCalculator::W_MDIFF     = 850;

// --------------------------------------------------------------------------
// selectBoard(...) - Picks the board with the best heuristic from a list
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
// calculateHeuristic(...) - Main Heuristic (AI perspective):
//   occupant code 1 = Opponent, occupant code 2 = AI (Computer)
// --------------------------------------------------------------------------
int HeuristicCalculator::calculateHeuristic(const Board& b) const
{
    const int AIuser   = 2;
    const int Opponent = 1;

    const auto& board   = b.getBoard();
    const auto& adjList = b.getAdjacencyList();
    const auto& coords  = b.getIndexToCoord();

    if (coords.size() != adjList.size()) {
        std::cerr << "[ERROR] mismatch in calculateHeuristic\n";
        return 0;
    }

    // Initialize metrics only for push potential and vulnerability.
    int pushPotential[3] = {0, 0, 0};
    int vulnerability[3] = {0, 0, 0};

    // Get marble counts.
    int oppMarbles = b.getNumPlayerOnePieces();  // Opponent: occupant code 1
    int aiMarbles  = b.getNumPlayerTwoPieces();    // AI: occupant code 2

    // Tally stats for each cell.
    for (int i = 0; i < static_cast<int>(coords.size()); ++i) {
        auto [x, y] = coords[i];
        int occupant = board[x][y];
        if (occupant != AIuser && occupant != Opponent) {
            continue;
        }

        int enemies = 0;
        // Check all neighboring cells.
        for (int neighborIdx : adjList[i]) {
            auto [nx, ny] = coords[neighborIdx];
            int neighbor = board[nx][ny];
            if (neighbor == (3 - occupant)) {
                pushPotential[occupant]++;
                enemies++;
            }
        }
        if (enemies >= 2) {
            vulnerability[occupant]++;
        }
    }

    // Compute weighted sum from the AI's perspective.
    int score = 0;
    score += (W_PUSH_AI  * pushPotential[AIuser])    - (W_PUSH_OPP * pushPotential[Opponent]);
    score -= (W_VULN_AI  * vulnerability[AIuser]);
    score += (W_VULN_OPP * vulnerability[Opponent]);

    int mDiff = (aiMarbles - oppMarbles);
    score += (W_MDIFF * mDiff);

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
// initHeuristicCache(...) - For incremental caching (adjusted)
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
    cache.enemyCount.resize(coords.size(), 0);
    // Removed allyCount and proximity since they are no longer used.

    cache.marbleCountP1 = b.getNumPlayerOnePieces();
    cache.marbleCountP2 = b.getNumPlayerTwoPieces();

    for (int i = 0; i < static_cast<int>(coords.size()); ++i) {
        auto [x, y] = coords[i];
        cache.occupant[i] = boardArr[x][y];
    }

    // Only accumulate metrics for push potential and vulnerability.
    int pushPotential[3] = {0, 0, 0};
    int vulnerability[3] = {0, 0, 0};

    int centerX = ROWS / 2;
    int centerY = COLS / 2;
    for (int i = 0; i < static_cast<int>(coords.size()); ++i) {
        recalcCellMetrics(i, b, cache, centerX, centerY);
        int occupant = cache.occupant[i];
        if (occupant != 1 && occupant != 2) {
            continue;
        }
        int enemies = cache.enemyCount[i];
        pushPotential[occupant] += enemies;
        if (enemies >= 2)
            vulnerability[occupant]++;
    }

    int score = 0;
    // Note: AI is code 2 and Opponent is code 1.
    score += (W_PUSH_AI      * pushPotential[2]) - (W_PUSH_OPP * pushPotential[1]);
    score -= (W_VULN_AI      * vulnerability[2]);
    score += (W_VULN_OPP     * vulnerability[1]);
    int p1Count = b.getNumPlayerOnePieces();
    int p2Count = b.getNumPlayerTwoPieces();
    score += (W_MDIFF * (p2Count - p1Count));
    
    cache.totalHeuristic = score;
    return cache;
}

// --------------------------------------------------------------------------
// updateHeuristicCache(...) - Adjusted for removed metrics
// --------------------------------------------------------------------------
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

    // Only enemyCount is needed.
    for (int i : extended) {
        newCache.enemyCount[i] = 0;
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
// recalcCellMetrics(...) - Recompute enemy count for cell i only
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
    cache.occupant[i] = occupant;

    // Reset enemy count.
    cache.enemyCount[i] = 0;

    // Compute only the enemy count (used in push potential and vulnerability).
    for (int neighborIdx : adjList[i]) {
        auto [nx, ny] = coords[neighborIdx];
        int neighVal = boardArr[nx][ny];
        if (neighVal == 3 - occupant) {
            cache.enemyCount[i]++;
        }
    }
}