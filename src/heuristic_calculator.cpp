#include "heuristic_calculator.h"
#include <cmath>    // std::abs
#include <iostream> // std::cerr
#include <set>      // for std::set
#include <queue>    // for std::queue

Board HeuristicCalculator::selectBoard(std::vector<Board> generatedBoards) const
{
    Board bestBoard = generatedBoards[0];
    int bestHeuristic = 0;

    // Because 'selectBoard' is const, we must call a const method 'calculateHeuristic'
    for (Board& b : generatedBoards) {
        int currHeuristic = calculateHeuristic(b);
        if (currHeuristic > bestHeuristic) {
            bestHeuristic = currHeuristic;
            bestBoard = b;
        }
        b.printPieces();
        std::cout << "Heuristic: " << currHeuristic << std::endl;
    }

    return bestBoard;
}

// IMPORTANT: No trailing semicolon here! 
// This is the definition of the function, not a declaration.
int HeuristicCalculator::calculateHeuristic(const Board& b) const
{
    // The "full" approach, same as your original or referencing adjacency LIST
    // We'll do something minimal here:

    const auto& board   = b.getBoard();
    const auto& adjList = b.getAdjacencyList();
    const auto& coords  = b.getIndexToCoord();

    if (coords.size() != adjList.size()) {
        std::cerr << "[ERROR] mismatch in calculateHeuristic\n";
        return 0;
    }

    // Basic counters
    int cohesion[3]      = {};
    int pushPotential[3] = {};
    int vulnerability[3] = {};
    int isolated[3]      = {};
    int proximity[3]     = {};

    // Board center for a 9x9
    int centerX = ROWS / 2;
    int centerY = COLS / 2;

    // Tally #marbles
    int p1count = b.getNumPlayerOnePieces();
    int p2count = b.getNumPlayerTwoPieces();

    for (int i = 0; i < static_cast<int>(coords.size()); ++i) {
        auto [x, y] = coords[i];
        int player  = board[x][y];
        if (player != 1 && player != 2) {
            continue;
        }

        // proximity
        int dist = std::abs(x - centerX) + std::abs(y - centerY);
        proximity[player] += (10 - dist);

        int allies = 0, enemies = 0;
        for (int neighborIdx : adjList[i]) {
            auto [nx, ny] = coords[neighborIdx];
            int neighbor  = board[nx][ny];
            if (neighbor == player) {
                cohesion[player]++;
                allies++;
            } else if (neighbor == 3 - player) {
                pushPotential[player]++;
                enemies++;
            }
        }

        if (allies == 0) {
            isolated[player]++;
        }
        if (enemies >= 2) {
            vulnerability[player]++;
        }
    }

    // Weighted sum
    int p1 = 1, p2 = 2;
    int score = 0;
    score += 10 * (cohesion[p1] - cohesion[p2]);
    score += 15 * (pushPotential[p1] - pushPotential[p2]);
    score += 12 * (vulnerability[p2] - vulnerability[p1]);
    score += 10 * (isolated[p2]      - isolated[p1]);
    score +=  8 * (proximity[p1]     - proximity[p2]);
    // For brevity, let's just add marbleDifference:
    score += 25 * marbleDifference(p1, b);

    return score;
}

int HeuristicCalculator::marbleDifference(int player, const Board& b) const
{
    int p1Count = b.getNumPlayerOnePieces();
    int p2Count = b.getNumPlayerTwoPieces();
    return (player == 1) ? (p1Count - p2Count) : (p2Count - p1Count);
}

/**
 * -------------------------------------------------------
 *   INCREMENTAL METHODS
 * -------------------------------------------------------
 */

HeuristicCache HeuristicCalculator::initHeuristicCache(const Board& b) const
{
    HeuristicCache cache;
    const auto& coords     = b.getIndexToCoord();
    const auto& adjList    = b.getAdjacencyList();
    const auto& boardArr   = b.getBoard();

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

    // Fill occupant info
    for (int i = 0; i < static_cast<int>(coords.size()); ++i) {
        auto [x, y]   = coords[i];
        cache.occupant[i] = boardArr[x][y];  // 0,1,2, or -1
    }

    // Now compute local adjacency-based data & partial heuristic
    int p1 = 1, p2 = 2;
    int cohesion[3]      = {};
    int pushPotential[3] = {};
    int vulnerability[3] = {};
    int isolated[3]      = {};
    int proximityArr[3]  = {};

    for (int i = 0; i < static_cast<int>(coords.size()); ++i) {
        recalcCellMetrics(i, b, cache, centerX, centerY);

        int player = cache.occupant[i];
        if (player != 1 && player != 2) {
            continue;
        }

        // We'll accumulate partial results for final scoring
        proximityArr[player] += cache.proximity[i];

        int allies  = cache.allyCount[i];
        int enemies = cache.enemyCount[i];
        if (allies == 0) {
            isolated[player]++;
        }
        if (enemies >= 2) {
            vulnerability[player]++;
        }
        cohesion[player]      += allies;
        pushPotential[player] += enemies;
    }

    // Combine them in a "totalHeuristic"
    int score = 0;
    score += 10 * (cohesion[p1]      - cohesion[p2]);
    score += 15 * (pushPotential[p1] - pushPotential[p2]);
    score += 12 * (vulnerability[p2] - vulnerability[p1]);
    score += 10 * (isolated[p2]      - isolated[p1]);
    score +=  8 * (proximityArr[p1]  - proximityArr[p2]);
    // For demonstration, add marbleDifference:
    score += 25 * (static_cast<int>(cache.marbleCountP1)
                 - static_cast<int>(cache.marbleCountP2));

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
    const auto& boardArr= childBoard.getBoard();
    if (coords.size() != adjList.size()) {
        std::cerr << "[ERROR] mismatch in updateHeuristicCache\n";
        return newCache; // partial
    }

    // Re-check marble counts after the move
    newCache.marbleCountP1 = childBoard.getNumPlayerOnePieces();
    newCache.marbleCountP2 = childBoard.getNumPlayerTwoPieces();

    // Figure out which cells to re-check
    std::set<int> cellsToUpdate;

    // The Move class presumably has getSize() and getDirection(),
    // plus 'positions' vector. 
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

    // Also check neighbors
    std::set<int> extended;
    for (int cIdx : cellsToUpdate) {
        extended.insert(cIdx);
        for (int nIdx : adjList[cIdx]) {
            extended.insert(nIdx);
        }
    }

    // occupant might have changed from parent
    for (int i : extended) {
        auto [x, y] = coords[i];
        newCache.occupant[i] = boardArr[x][y];
    }

    // Reset adjacency tallies for changed cells
    for (int i : extended) {
        newCache.allyCount[i]  = 0;
        newCache.enemyCount[i] = 0;
        newCache.proximity[i]  = 0;
    }

    // Recalc each changed cell
    int centerX = ROWS / 2;
    int centerY = COLS / 2;
    for (int i : extended) {
        recalcCellMetrics(i, childBoard, newCache, centerX, centerY);
    }

    // For now, do a simpler approach: recalc the entire heuristic from scratch 
    // to set newCache.totalHeuristic
    newCache.totalHeuristic = calculateHeuristic(childBoard);

    return newCache;
}

int HeuristicCalculator::getCachedHeuristic(const HeuristicCache& cache) const
{
    return cache.totalHeuristic;
}

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
    const auto& boardArr= b.getBoard();

    auto [x, y] = coords[i];
    int player  = boardArr[x][y];

    cache.allyCount[i]  = 0;
    cache.enemyCount[i] = 0;
    cache.proximity[i]  = 0;

    if (player != 1 && player != 2) {
        cache.occupant[i] = player; // 0 or -1
        return; // empty or invalid
    }

    // occupant
    cache.occupant[i] = player;

    // proximity
    int dist = std::abs(x - centerX) + std::abs(y - centerY);
    cache.proximity[i] = (10 - dist);

    // adjacency
    for (int neighborIdx : adjList[i]) {
        auto [nx, ny] = coords[neighborIdx];
        int neighbor  = boardArr[nx][ny];
        if (neighbor == player) {
            cache.allyCount[i]++;
        } else if (neighbor == 3 - player) {
            cache.enemyCount[i]++;
        }
    }
}