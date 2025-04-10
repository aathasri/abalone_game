#include "heuristic_calculator.h"
#include <iostream>
#include <limits>
#include <set>
#include <algorithm>

// --------------------------------------------------------------------------
// STATIC WEIGHTS - Streamlined
// --------------------------------------------------------------------------
int HeuristicCalculator::W_PUSH_AI   = 4; 
int HeuristicCalculator::W_PUSH_OPP  = 3;
int HeuristicCalculator::W_VULN_AI   = 5;
int HeuristicCalculator::W_VULN_OPP  = 7;
int HeuristicCalculator::W_MDIFF     = 850;

// --------------------------------------------------------------------------
// calculateHeuristic - Core Heuristic
// --------------------------------------------------------------------------
int HeuristicCalculator::calculateHeuristic(const Board& b) const
{
    const auto& board   = b.getBoard();
    const auto& adjList = b.getAdjacencyList();
    const auto& coords  = b.getIndexToCoord();

    int pushPotential[3] = {0, 0, 0};
    int vulnerability[3] = {0, 0, 0};

    for (int i = 0; i < coords.size(); ++i) {
        auto [x, y] = coords[i];
        int occupant = board[x][y];
        if (occupant < 1 || occupant > 2) continue;

        int enemies = 0;
        for (int neighborIdx : adjList[i]) {
            auto [nx, ny] = coords[neighborIdx];
            int neighbor = board[nx][ny];
            if (neighbor == 3 - occupant) {
                pushPotential[occupant]++;
                enemies++;
            }
        }
        if (enemies >= 2) vulnerability[occupant]++;
    }

    int aiMarbles  = b.getNumPlayerTwoPieces();
    int oppMarbles = b.getNumPlayerOnePieces();

    return W_PUSH_AI * pushPotential[2] 
         - W_PUSH_OPP * pushPotential[1]
         - W_VULN_AI * vulnerability[2]
         + W_VULN_OPP * vulnerability[1]
         + W_MDIFF * (aiMarbles - oppMarbles);
}

// --------------------------------------------------------------------------
// HeuristicCache Management
// --------------------------------------------------------------------------
HeuristicCache HeuristicCalculator::initHeuristicCache(const Board& b) const
{
    HeuristicCache cache;
    const auto& coords  = b.getIndexToCoord();
    const auto& boardArr = b.getBoard();

    cache.occupant.resize(coords.size());
    cache.enemyCount.resize(coords.size());

    for (int i = 0; i < coords.size(); ++i) {
        auto [x, y] = coords[i];
        cache.occupant[i] = boardArr[x][y];
    }

    cache.marbleCountP1 = b.getNumPlayerOnePieces();
    cache.marbleCountP2 = b.getNumPlayerTwoPieces();
    cache.totalHeuristic = calculateHeuristic(b);

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

    newCache.marbleCountP1 = childBoard.getNumPlayerOnePieces();
    newCache.marbleCountP2 = childBoard.getNumPlayerTwoPieces();

    std::set<int> cellsToUpdate;

    for (int i = 0; i < m.getSize(); ++i) {
        auto [oldX, oldY] = m.getPosition(i);
        auto oldIt = childBoard.getCoordToIndex().find({oldX, oldY});
        if (oldIt != childBoard.getCoordToIndex().end())
            cellsToUpdate.insert(oldIt->second);

        auto [dx, dy] = DirectionHelper::getDelta(m.getDirection());
        int newX = oldX + dx, newY = oldY + dy;
        auto newIt = childBoard.getCoordToIndex().find({newX, newY});
        if (newIt != childBoard.getCoordToIndex().end())
            cellsToUpdate.insert(newIt->second);
    }

    std::set<int> extended;
    for (int idx : cellsToUpdate) {
        extended.insert(idx);
        extended.insert(adjList[idx].begin(), adjList[idx].end());
    }

    for (int idx : extended) {
        auto [x, y] = coords[idx];
        newCache.occupant[idx] = boardArr[x][y];

        newCache.enemyCount[idx] = 0;
        for (int neighborIdx : adjList[idx]) {
            auto [nx, ny] = coords[neighborIdx];
            if (boardArr[nx][ny] == 3 - boardArr[x][y])
                newCache.enemyCount[idx]++;
        }
    }

    newCache.totalHeuristic = calculateHeuristic(childBoard);

    return newCache;
}

int HeuristicCalculator::getCachedHeuristic(const HeuristicCache& cache) const
{
    return cache.totalHeuristic;
}