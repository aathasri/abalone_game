// move.cpp
#include "move.h"
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <set>

const std::unordered_map<std::string, std::pair<int, int>> directionDeltas = {
    {"NE", {-1,  1}}, {"NW", {-1, 0}},
    {"E",  { 0,  1}}, {"W",  { 0, -1}},
    {"SE", { 1,  0}}, {"SW", { 1, -1}}
};

bool isLine(const std::vector<int>& group) {
    if (group.size() < 2) return false;
    for (int i = 1; i < group.size(); ++i) {
        if (std::find(neighborIndices[group[i]].begin(), neighborIndices[group[i]].end(), group[i - 1]) == neighborIndices[group[i]].end())
            return false;
    }
    return true;
}

// Helper to get inline direction from two marble indices
std::string getDirection(int from, int to) {
    for (const auto& [dir, delta] : directionDeltas) {
        if (std::find(neighborIndices[from].begin(), neighborIndices[from].end(), to) != neighborIndices[from].end()) {
            return dir;
        }
    }
    return "";
}

std::vector<Move> generateLegalMoves(const BoardArray& board, CellState player) {
    std::vector<Move> moves;
    std::set<std::string> seen;
    CellState opponent = (player == BLACK) ? WHITE : BLACK;

    for (int i = 0; i < 61; ++i) {
        if (board[i] != player) continue;

        // Single marble sidesteps
        for (int n : neighborIndices[i]) {
            if (board[n] == EMPTY) {
                std::string dir = getDirection(i, n);
                moves.push_back({ {i}, dir, SIDESTEP });
            }
        }

        for (int j : neighborIndices[i]) {
            if (board[j] != player) continue;
            std::vector<int> group2 = {i, j};
            std::sort(group2.begin(), group2.end());
            if (!isLine(group2)) continue;
            std::string dir = getDirection(i, j);

            int front = j;
            int next = -1;
            for (int n : neighborIndices[front]) {
                if (getDirection(front, n) == dir) {
                    next = n;
                    break;
                }
            }
            if (next == -1) continue;

            if (board[next] == EMPTY || board[next] == opponent) {
                if (board[next] == EMPTY) {
                    moves.push_back({ group2, dir, INLINE });
                } else {
                    // Sumito: 2v1
                    int pushDest = -1;
                    for (int n2 : neighborIndices[next]) {
                        if (getDirection(next, n2) == dir) {
                            pushDest = n2;
                            break;
                        }
                    }
                    if (pushDest == -1 || board[pushDest] == EMPTY) {
                        moves.push_back({ group2, dir, INLINE });
                    }
                }
            }

            for (int k : neighborIndices[j]) {
                if (board[k] != player || k == i) continue;
                std::vector<int> group3 = {i, j, k};
                std::sort(group3.begin(), group3.end());
                if (!isLine(group3)) continue;
                std::string dir3 = getDirection(j, k);
                if (dir3 != dir) continue;

                int front3 = k;
                int next3 = -1;
                for (int n : neighborIndices[front3]) {
                    if (getDirection(front3, n) == dir3) {
                        next3 = n;
                        break;
                    }
                }
                if (next3 == -1) continue;

                if (board[next3] == EMPTY || board[next3] == opponent) {
                    if (board[next3] == EMPTY) {
                        moves.push_back({ group3, dir3, INLINE });
                    } else {
                        int oppCount = 1;
                        int push1 = next3;
                        int push2 = -1;

                        for (int n2 : neighborIndices[push1]) {
                            if (getDirection(push1, n2) == dir3) {
                                push2 = n2;
                                break;
                            }
                        }

                        if (push2 != -1 && board[push2] == opponent) {
                            oppCount = 2;
                        }

                        if ((int)group3.size() > oppCount) {
                            int beyond = -1;
                            for (int n : neighborIndices[push2 != -1 ? push2 : push1]) {
                                if (getDirection(push2 != -1 ? push2 : push1, n) == dir3) {
                                    beyond = n;
                                    break;
                                }
                            }

                            if (beyond == -1 || board[beyond] == EMPTY) {
                                moves.push_back({ group3, dir3, INLINE });
                            }
                        }
                    }
                }
            }
        }
    }

    return moves;
}

BoardArray applyMove(const BoardArray& board, const Move& move) {
    BoardArray newBoard = board;
    if (move.type == SIDESTEP && move.marbles.size() == 1) {
        int from = move.marbles[0];
        int to = -1;
        for (int neighbor : neighborIndices[from]) {
            if (board[neighbor] == EMPTY) {
                to = neighbor;
                break;
            }
        }
        if (to != -1) {
            newBoard[to] = newBoard[from];
            newBoard[from] = EMPTY;
        }
    }

    // TODO: Expand apply logic for inline and Sumito moves
    return newBoard;
}

std::string moveToString(const Move& move) {
    std::string s = "{";
    for (size_t i = 0; i < move.marbles.size(); ++i) {
        s += idxToPos[move.marbles[i]];
        if (i != move.marbles.size() - 1) s += ", ";
    }
    s += " -> " + move.direction + (move.type == INLINE ? ", inline" : ", sidestep") + "}";
    return s;
}
