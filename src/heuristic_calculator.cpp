#include "heuristic_calculator.h"

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
        std::cout << currHeuristic << std::endl;
    }

    return bestBoard;
}

int HeuristicCalculator::calculateHeuristic(const Board& b) {
    // auto start = std::chrono::high_resolution_clock::now();  // ⏱️ Start timing

    const auto& board = b.getBoard();
    const auto& adj = b.getAdjacencyMatrix();
    const auto& coords = b.getIndexToCoord();
    const auto& coordToIndex = b.getCoordToIndex();

    if (coords.size() != adj.size()) {
        std::cerr << "[ERROR] coords and adj size mismatch\n";
        return 0;
    }

    int cohesion[3] = {};         
    int pushPotential[3] = {};
    int vulnerability[3] = {};
    int isolated[3] = {};
    int proximity[3] = {};

    int centerX = ROWS / 2;
    int centerY = COLS / 2;

    for (int i = 0; i < coords.size(); ++i) {
        if (i >= adj.size()) continue;
        auto [x, y] = coords[i];
        int player = board[x][y];
        if (player != 1 && player != 2) continue;

        int dist = std::abs(x - centerX) + std::abs(y - centerY);
        proximity[player] += (10 - dist);

        int allies = 0, enemies = 0;

        for (int j = 0; j < adj[i].size(); ++j) {
            if (!adj[i][j] || j >= coords.size()) continue;

            auto [nx, ny] = coords[j];
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
    score += 15 * (lineAlignment(p1, b) - lineAlignment(p2, b));
    score += 25 * marbleDifference(p1, b);

    // auto end = std::chrono::high_resolution_clock::now();  // ⏱️ End timing
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
    const auto& adj = b.getAdjacencyMatrix();
    const auto& board = b.getBoard();
    const auto& coords = b.getIndexToCoord();

    if (coords.size() != adj.size()) {
        std::cerr << "[ERROR] coords and adj size mismatch in groupingAdvantage\n";
        return 0;
    }

    std::vector<bool> visited(coords.size(), false);
    int groupsWithMajority = 0;

    for (int i = 0; i < coords.size(); ++i) {
        if (i >= adj.size() || visited[i]) continue;
        auto [x, y] = coords[i];
        if (board[x][y] <= 0) continue;

        std::queue<int> q;
        q.push(i);
        visited[i] = true;
        int playerCount = 0, oppCount = 0;

        while (!q.empty()) {
            int curr = q.front(); q.pop();
            if (curr >= coords.size() || curr >= adj.size()) continue;
            auto [cx, cy] = coords[curr];
            int color = board[cx][cy];
            if (color == player) playerCount++;
            else if (color == 3 - player) oppCount++;

            for (int j = 0; j < adj[curr].size(); ++j) {
                if (!visited[j] && adj[curr][j]) {
                    visited[j] = true;
                    q.push(j);
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
    const auto& coordToIndex = b.getCoordToIndex();

    std::set<std::string> countedGroups;
    int aligned = 0;

    for (int i = 0; i < coords.size(); ++i) {
        auto [x1, y1] = coords[i];
        if (board[x1][y1] != player) continue;

        for (const auto& [dx, dy] : std::vector<std::pair<int, int>>{
            {-1, 0}, {-1, 1}, {0, 1}, {1, 0}, {1, -1}, {0, -1}}) {

            int x2 = x1 + dx, y2 = y1 + dy;
            int x3 = x2 + dx, y3 = y2 + dy;

            if (coordToIndex.count({x2, y2}) && coordToIndex.count({x3, y3}) &&
                board[x2][y2] == player && board[x3][y3] == player) {
                std::ostringstream oss;
                oss << x1 << "," << y1 << "-" << x2 << "," << y2 << "-" << x3 << "," << y3;
                std::string key = oss.str();
                if (countedGroups.insert(key).second) aligned++;
            }
        }
    }

    return aligned;
}