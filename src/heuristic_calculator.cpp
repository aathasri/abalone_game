#include "heuristic_calculator.h"

Board HeuristicCalculator::selectBoard(std::vector<Board> generatedBoards) const
{
    Board& bestBoard = generatedBoards[0];
    int bestHeuristic = 0;

    for (Board& b : generatedBoards) {
        int currHeuristic = calculateHeuristic(b);
        if (currHeuristic > bestHeuristic) {
            bestHeuristic = currHeuristic;
            bestBoard = b;
        }
        b.printPieces();
        std::cout << currHeuristic << std::endl;
    }

    return bestBoard;
}

int HeuristicCalculator::calculateHeuristic(Board b)
{
    auto start = std::chrono::high_resolution_clock::now();

    int p1 = 1;
    int p2 = 2;

    int score = 0;
    score += 10 * (groupCohesion(p1, b) - groupCohesion(p2, b));
    score += 15 * (pushPotential(p1, b) - pushPotential(p2, b));
    score += 12 * (pushVulnerability(p2, b) - pushVulnerability(p1, b));
    score += 20 * (groupingAdvantage(p1, b) - groupingAdvantage(p2, b));
    score += 10 * (isolationPenalty(p2, b) - isolationPenalty(p1, b));
    score += 15 * (lineAlignment(p1, b) - lineAlignment(p2, b));
    score += 25 * marbleDifference(p1, b);
    score += 8  * (centerProximity(p1, b) - centerProximity(p2, b));

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    // std::cout << "Heuristic took " << duration.count() << " microseconds\n";

    return score;
}


/** 
 * --------------------------------------------------------------------------------
 * Heuristic Calculators
 * --------------------------------------------------------------------------------
 */ 

// Returns the difference in marble count for the player (player - opponent)
int HeuristicCalculator::marbleDifference(int player, const Board& b) {
    int p1Count = b.getNumPlayerOnePieces();
    int p2Count = b.getNumPlayerTwoPieces();
    return (player == 1) ? (p1Count - p2Count) : (p2Count - p1Count);
}

// Sums the inverse of distance from center for each player marble (closer = higher score)
int HeuristicCalculator::centerProximity(int player, const Board& b) {
    const auto& board = b.getBoard();
    int centerX = ROWS / 2;
    int centerY = COLS / 2;

    int proximity = 0;
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            if (board[i][j] == player) {
                int dist = std::abs(i - centerX) + std::abs(j - centerY);
                proximity += (10 - dist); // the closer, the more positive
            }
        }
    }
    return proximity;
}

// Counts adjacent same-colored marbles (used to measure cohesion)
int HeuristicCalculator::groupCohesion(int player, const Board& b) {
    const auto& adj = b.getAdjacencyMatrix();
    const auto& board = b.getBoard();
    const auto& coords = b.getIndexToCoord();
    int cohesion = 0;

    for (int i = 0; i < adj.size(); ++i) {
        auto [x1, y1] = coords[i];
        if (board[x1][y1] != player) continue;

        for (int j = i + 1; j < adj[i].size(); ++j) {
            if (adj[i][j]) {
                auto [x2, y2] = coords[j];
                if (board[x2][y2] == player) {
                    cohesion++;
                }
            }
        }
    }
    return cohesion;
}

// Counts how many opponent marbles are adjacent to player marbles (potential to push)
int HeuristicCalculator::pushPotential(int player, const Board& b) {
    const auto& adj = b.getAdjacencyMatrix();
    const auto& board = b.getBoard();
    const auto& coords = b.getIndexToCoord();
    int potential = 0;

    for (int i = 0; i < adj.size(); ++i) {
        auto [x, y] = coords[i];
        if (board[x][y] != player) continue;

        for (int j = 0; j < adj[i].size(); ++j) {
            if (!adj[i][j]) continue;
            auto [nx, ny] = coords[j];
            if (board[nx][ny] == 3 - player) {
                potential++;
            }
        }
    }
    return potential;
}

// Counts how many player marbles are adjacent to 2+ opponent marbles (vulnerable to push)
int HeuristicCalculator::pushVulnerability(int player, const Board& b) {
    const auto& adj = b.getAdjacencyMatrix();
    const auto& board = b.getBoard();
    const auto& coords = b.getIndexToCoord();
    int vulnerable = 0;

    for (int i = 0; i < adj.size(); ++i) {
        auto [x, y] = coords[i];
        if (board[x][y] != player) continue;

        int oppAdj = 0;
        for (int j = 0; j < adj[i].size(); ++j) {
            if (!adj[i][j]) continue;
            auto [nx, ny] = coords[j];
            if (board[nx][ny] == 3 - player) {
                oppAdj++;
            }
        }
        if (oppAdj >= 2) vulnerable++;
    }
    return vulnerable;
}

// For each group of connected marbles, counts only if player has majority over opponent
int HeuristicCalculator::groupingAdvantage(int player, const Board& b) {
    const auto& adj = b.getAdjacencyMatrix();
    const auto& board = b.getBoard();
    const auto& coords = b.getIndexToCoord();
    int n = adj.size();

    std::vector<bool> visited(n, false);
    int groupsWithMajority = 0;

    for (int i = 0; i < n; ++i) {
        if (visited[i]) continue;
        auto [x, y] = coords[i];
        if (board[x][y] <= 0) continue;

        std::queue<int> q;
        q.push(i);
        visited[i] = true;
        int playerCount = 0, oppCount = 0;

        while (!q.empty()) {
            int curr = q.front(); q.pop();
            auto [cx, cy] = coords[curr];
            int color = board[cx][cy];
            if (color == player) playerCount++;
            else if (color == 3 - player) oppCount++;

            for (int j = 0; j < n; ++j) {
                if (!visited[j] && adj[curr][j]) {
                    auto [nx, ny] = coords[j];
                    if (board[nx][ny] > 0) {
                        visited[j] = true;
                        q.push(j);
                    }
                }
            }
        }
        if (playerCount > oppCount) groupsWithMajority++;
    }
    return groupsWithMajority;
}

// Counts how many player marbles are not adjacent to any same-colored piece
int HeuristicCalculator::isolationPenalty(int player, const Board& b) {
    const auto& adj = b.getAdjacencyMatrix();
    const auto& board = b.getBoard();
    const auto& coords = b.getIndexToCoord();
    int isolated = 0;

    for (int i = 0; i < adj.size(); ++i) {
        auto [x, y] = coords[i];
        if (board[x][y] != player) continue;

        bool hasAlly = false;
        for (int j = 0; j < adj[i].size(); ++j) {
            if (adj[i][j]) {
                auto [nx, ny] = coords[j];
                if (board[nx][ny] == player) {
                    hasAlly = true;
                    break;
                }
            }
        }
        if (!hasAlly) isolated++;
    }
    return isolated;
}

// Counts how many sets of 3 player marbles are aligned in a straight line
int HeuristicCalculator::lineAlignment(int player, const Board& b) {
    const auto& board = b.getBoard();
    const auto& coords = b.getIndexToCoord();
    const auto& adj = b.getAdjacencyMatrix();
    int aligned = 0;

    std::set<std::set<int>> countedGroups;

    for (int i = 0; i < adj.size(); ++i) {
        auto [x1, y1] = coords[i];
        if (board[x1][y1] != player) continue;

        for (int j = 0; j < adj[i].size(); ++j) {
            if (!adj[i][j]) continue;
            auto [x2, y2] = coords[j];
            if (board[x2][y2] != player) continue;

            int dx = x2 - x1, dy = y2 - y1;
            for (int k = 1; k <= 1; ++k) {
                int tx = x2 + dx, ty = y2 + dy;
                for (int l = 0; l < coords.size(); ++l) {
                    if (coords[l] == std::make_pair(tx, ty) && board[tx][ty] == player) {
                        std::set<int> group = { i, j, l };
                        if (countedGroups.insert(group).second) {
                            aligned++;
                        }
                    }
                }
            }
        }
    }
    return aligned;
}
