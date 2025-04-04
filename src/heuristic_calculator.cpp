#include "heuristic_calculator.h"
#include <queue>     // needed for BFS in groupingAdvantage
#include <iostream>  // for std::cerr
#include <cmath>     // for std::abs

Board HeuristicCalculator::selectBoard(std::vector<Board> generatedBoards) const
{
    Board bestBoard = generatedBoards[0];
    int bestHeuristic = 0;

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

int HeuristicCalculator::calculateHeuristic(const Board& b) {
    // auto start = std::chrono::high_resolution_clock::now();  // (Optional) Start timing

    // Retrieve board data
    const auto& board      = b.getBoard();
    const auto& adjList    = b.getAdjacencyList();  // <-- Use adjacency LIST
    const auto& coords     = b.getIndexToCoord();
    const auto& coordToIdx = b.getCoordToIndex();

    // Sanity check
    if (coords.size() != adjList.size()) {
        std::cerr << "[ERROR] coords.size() and adjList.size() mismatch in calculateHeuristic\n";
        return 0;
    }

    // We'll store various heuristic factors indexed by [player], so [0] is unused
    int cohesion[3]      = {};
    int pushPotential[3] = {};
    int vulnerability[3] = {};
    int isolated[3]      = {};
    int proximity[3]     = {};

    // "Center" of a 9x9 might be row=4, col=4 in zero-based
    int centerX = ROWS / 2; 
    int centerY = COLS / 2; 

    // Loop over all valid positions
    for (int i = 0; i < static_cast<int>(coords.size()); ++i) {
        auto [x, y] = coords[i];
        int player  = board[x][y];
        if (player != 1 && player != 2) {
            // Skip empty or invalid
            continue;
        }

        // Proximity to center: simple Manhattan distance
        int dist = std::abs(x - centerX) + std::abs(y - centerY);
        proximity[player] += (10 - dist);

        // We'll count how many allies & enemies around this marble
        int allies  = 0;
        int enemies = 0;

        // Go through each neighbor index in the adjacency list
        for (int neighborIndex : adjList[i]) {
            auto [nx, ny] = coords[neighborIndex];
            int neighbor  = board[nx][ny];

            if (neighbor == player) {
                cohesion[player]++;
                allies++;
            } else if (neighbor == 3 - player) {
                pushPotential[player]++;
                enemies++;
            }
        }

        // If no allies around, it's isolated
        if (allies == 0) {
            isolated[player]++;
        }
        // If 2 or more enemies around, it might be vulnerable
        if (enemies >= 2) {
            vulnerability[player]++;
        }
    }

    // Compose final score from your weighting
    int p1 = 1, p2 = 2;
    int score = 0;
    score += 10 * (cohesion[p1]       - cohesion[p2]);
    score += 15 * (pushPotential[p1] - pushPotential[p2]);
    score += 12 * (vulnerability[p2] - vulnerability[p1]);
    score += 10 * (isolated[p2]      - isolated[p1]);
    score +=  8 * (proximity[p1]     - proximity[p2]);
    // Additional heuristics
    score += 20 * (groupingAdvantage(p1, b) - groupingAdvantage(p2, b));
    score += 15 * (lineAlignment(p1, b)     - lineAlignment(p2, b));
    score += 25 *  marbleDifference(p1, b);

    // auto end = std::chrono::high_resolution_clock::now();   // (Optional) End timing
    // auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    // std::cout << "Heuristic took " << duration.count() << " microseconds\n";

    return score;
}

int HeuristicCalculator::marbleDifference(int player, const Board& b) {
    int p1Count = b.getNumPlayerOnePieces();
    int p2Count = b.getNumPlayerTwoPieces();
    return (player == 1) ? (p1Count - p2Count) : (p2Count - p1Count);
}

int HeuristicCalculator::groupingAdvantage(int player, const Board& b) {
    // We'll do BFS for connected components using adjacency list
    const auto& adjList = b.getAdjacencyList();
    const auto& board   = b.getBoard();
    const auto& coords  = b.getIndexToCoord();

    if (coords.size() != adjList.size()) {
        std::cerr << "[ERROR] coords and adjList size mismatch in groupingAdvantage\n";
        return 0;
    }

    std::vector<bool> visited(coords.size(), false);
    int groupsWithMajority = 0;

    for (int i = 0; i < static_cast<int>(coords.size()); ++i) {
        if (visited[i]) {
            continue;
        }
        auto [x, y] = coords[i];
        int color   = board[x][y];
        if (color <= 0) {
            // Empty or invalid cell
            continue;
        }

        // We'll BFS from i to find all connected cells
        std::queue<int> q;
        q.push(i);
        visited[i] = true;

        int playerCount = 0;
        int oppCount    = 0;

        while (!q.empty()) {
            int curr = q.front();
            q.pop();

            auto [cx, cy] = coords[curr];
            int currColor = board[cx][cy];
            // Tally marbles
            if (currColor == player) {
                playerCount++;
            } else if (currColor == 3 - player) {
                oppCount++;
            }

            // Explore neighbors
            for (int neighborIndex : adjList[curr]) {
                if (!visited[neighborIndex]) {
                    visited[neighborIndex] = true;
                    q.push(neighborIndex);
                }
            }
        }

        // If this connected component has majority of 'player' marbles:
        if (playerCount > oppCount) {
            groupsWithMajority++;
        }
    }

    return groupsWithMajority;
}

int HeuristicCalculator::lineAlignment(int player, const Board& b) {
    // This function doesn't use adjacency at all; it checks 3 in a row 
    // directly via offsets.
    const auto& board       = b.getBoard();
    const auto& coords      = b.getIndexToCoord();
    const auto& coordToIdx  = b.getCoordToIndex();

    std::set<std::string> countedGroups;
    int aligned = 0;

    // For each cell in coords, if it's the player's marble, 
    // check 6 directions for a 3-in-a-line.
    for (int i = 0; i < static_cast<int>(coords.size()); ++i) {
        auto [x1, y1] = coords[i];
        if (board[x1][y1] != player) {
            continue;
        }

        for (auto [dx, dy] : std::vector<std::pair<int,int>>{
             {-1, 0}, {-1, 1}, {0, 1}, {1, 0}, {1, -1}, {0, -1}}) 
        {
            int x2 = x1 + dx, y2 = y1 + dy;
            int x3 = x2 + dx, y3 = y2 + dy;

            // If both next spots exist and hold the player's marble...
            if (coordToIdx.count({x2, y2}) && coordToIdx.count({x3, y3}) &&
                board[x2][y2] == player && board[x3][y3] == player) 
            {
                // Build a string key to ensure we don't double-count
                std::ostringstream oss;
                oss << x1 << "," << y1 << "-" 
                    << x2 << "," << y2 << "-" 
                    << x3 << "," << y3;

                if (countedGroups.insert(oss.str()).second) {
                    aligned++;
                }
            }
        }
    }

    return aligned;
}