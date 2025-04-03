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

int HeuristicCalculator::calculateHeuristic(const Board& b) {
    auto start = std::chrono::high_resolution_clock::now();

    const auto& board = b.getBoard();
    const auto& adj = b.getAdjacencyMatrix();
    const auto& coords = b.getIndexToCoord();

    int cohesion[3] = {};         
    int pushPotential[3] = {};
    int vulnerability[3] = {};
    int isolated[3] = {};
    int proximity[3] = {};

    int centerX = ROWS / 2;
    int centerY = COLS / 2;

    for (int i = 0; i < coords.size(); ++i) {
        auto [x, y] = coords[i];
        if (x < 0 || x >= ROWS || y < 0 || y >= COLS) continue;

        int player = board[x][y];
        if (player != 1 && player != 2) continue;

        int dist = std::abs(x - centerX) + std::abs(y - centerY);
        proximity[player] += (10 - dist);

        int allies = 0, enemies = 0;

        for (int j = 0; j < adj[i].size(); ++j) {
            if (!adj[i][j]) continue;
            if (j >= coords.size()) continue;

            auto [nx, ny] = coords[j];
            if (nx < 0 || nx >= ROWS || ny < 0 || ny >= COLS) continue;

            int neighbor = board[nx][ny];

            if (neighbor == player) {
                cohesion[player]++;
                allies++;
            } else if (neighbor == 3 - player) {
                pushPotential[player]++;
                enemies++;
            }
        }

        if (allies == 0) isolated[player]++;
        if (enemies >= 2) vulnerability[player]++;
    }

    int p1 = 1, p2 = 2;
    int score = 0;
    score += 10 * (cohesion[p1] - cohesion[p2]);
    score += 15 * (pushPotential[p1] - pushPotential[p2]);
    score += 12 * (vulnerability[p2] - vulnerability[p1]);
    score += 10 * (isolated[p2] - isolated[p1]);
    score += 8  * (proximity[p1] - proximity[p2]);
    score += 20 * (groupingAdvantage(p1, b) - groupingAdvantage(p2, b));
    // score += 15 * (lineAlignment(p1, b) - lineAlignment(p2, b));
    score += 25 * marbleDifference(p1, b);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Heuristic took " << duration.count() << " microseconds\n";

    return score;
}

int HeuristicCalculator::marbleDifference(int player, const Board& b) {
    int p1Count = b.getNumPlayerOnePieces();
    int p2Count = b.getNumPlayerTwoPieces();
    return (player == 1) ? (p1Count - p2Count) : (p2Count - p1Count);
}

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
        if (x < 0 || x >= ROWS || y < 0 || y >= COLS) continue;
        if (board[x][y] <= 0) continue;

        std::queue<int> q;
        q.push(i);
        visited[i] = true;
        int playerCount = 0, oppCount = 0;

        while (!q.empty()) {
            int curr = q.front(); q.pop();
            auto [cx, cy] = coords[curr];
            if (cx < 0 || cx >= ROWS || cy < 0 || cy >= COLS) continue;

            int color = board[cx][cy];
            if (color == player) playerCount++;
            else if (color == 3 - player) oppCount++;

            for (int j = 0; j < n; ++j) {
                if (!visited[j] && adj[curr][j]) {
                    auto [nx, ny] = coords[j];
                    if (nx < 0 || nx >= ROWS || ny < 0 || ny >= COLS) continue;
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

int HeuristicCalculator::lineAlignment(int player, const Board& b) {
    const auto& board = b.getBoard();
    const auto& coords = b.getIndexToCoord();
    const auto& adj = b.getAdjacencyMatrix();
    int aligned = 0;

    std::set<std::set<int>> countedGroups;

    for (int i = 0; i < adj.size(); ++i) {
        auto [x1, y1] = coords[i];
        if (x1 < 0 || x1 >= ROWS || y1 < 0 || y1 >= COLS) continue;
        if (board[x1][y1] != player) continue;

        for (int j = 0; j < adj[i].size(); ++j) {
            if (!adj[i][j]) continue;
            auto [x2, y2] = coords[j];
            if (x2 < 0 || x2 >= ROWS || y2 < 0 || y2 >= COLS) continue;
            if (board[x2][y2] != player) continue;

            int dx = x2 - x1, dy = y2 - y1;
            int tx = x2 + dx, ty = y2 + dy;

            if (tx >= 0 && tx < ROWS && ty >= 0 && ty < COLS) {
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
