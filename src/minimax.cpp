#include "minimax.h"
#include "move_generator.h"
#include "heuristic_calculator.h"
#include <algorithm>
#include <limits>
#include <iostream>
#include <future>
#include <vector>
#include <set>
#include <thread>

Minimax::Minimax(int maxDepth)
    : maxDepth(maxDepth), transpositionTable(64) {}

    int Minimax::minimax(Board& board, int depth, int currentPlayer,
        bool isMaximizing, int alpha, int beta)
{
++nodeCount;
HeuristicCalculator hcalc;
uint64_t hash = board.getZobristHash();

const int startingPieces = 14;
int p1Marbles = board.getNumPlayerOnePieces();
int p2Marbles = board.getNumPlayerTwoPieces();
int p1Lost = startingPieces - p1Marbles;
int p2Lost = startingPieces - p2Marbles;

if (p2Lost >= 6) return isMaximizing ? 100000 : -100000; // Player 2 loses
if (p1Lost >= 6) return isMaximizing ? -100000 : 100000; // Player 1 loses

TTEntry entry;
if (transpositionTable.lookup(hash, entry) && entry.depth >= depth) {
if (entry.flag == BoundType::EXACT) {
return entry.score;
} else if (entry.flag == BoundType::LOWER && entry.score >= beta) {
return entry.score;
} else if (entry.flag == BoundType::UPPER && entry.score <= alpha) {
return entry.score;
}
}

if (depth == 0) {
int score = hcalc.calculateHeuristic(board);
transpositionTable.insert(hash, { score, depth, isMaximizing, BoundType::EXACT });
return score;
}

MoveGenerator moveGen;
moveGen.generateMoves(currentPlayer, board);
std::set<Move> moves = moveGen.getGeneratedMoves();
if (moves.empty()) {
int score = hcalc.calculateHeuristic(board);
transpositionTable.insert(hash, { score, depth, isMaximizing, BoundType::EXACT });
return score;
}

std::vector<MoveOrder> childOrder; // Updated to MoveOrder
childOrder.reserve(moves.size());
for (const auto& m : moves) {
MoveUndo undo;
board.makeMove(m, undo);
int h = hcalc.calculateHeuristic(board);
board.unmakeMove(undo);
childOrder.push_back({ m, h });
}
std::sort(childOrder.begin(), childOrder.end(),
 [isMaximizing](const MoveOrder& a, const MoveOrder& b) {
     return isMaximizing ? (a.heuristic > b.heuristic)
                         : (a.heuristic < b.heuristic);
 });

int bestScore = isMaximizing ? std::numeric_limits<int>::min()
                    : std::numeric_limits<int>::max();

for (auto& child : childOrder) {
MoveUndo undo;
board.makeMove(child.move, undo);
int result = minimax(board, depth - 1, 3 - currentPlayer, !isMaximizing, alpha, beta);
board.unmakeMove(undo);

if (isMaximizing) {
bestScore = std::max(bestScore, result);
alpha = std::max(alpha, bestScore);
} else {
bestScore = std::min(bestScore, result);
beta = std::min(beta, bestScore);
}
if (beta <= alpha) break;
}

BoundType flag = BoundType::EXACT;
if (bestScore <= alpha) flag = BoundType::UPPER;
else if (bestScore >= beta) flag = BoundType::LOWER;

transpositionTable.insert(hash, { bestScore, depth, isMaximizing, flag });
return bestScore;
}

Move Minimax::findBestMove(Board& board, int currentPlayer)
{
    HeuristicCalculator hcalc;
    Move finalBestMove;
    int finalBestScore = std::numeric_limits<int>::min();

    if (currentPlayer != 2) {
        std::cerr << "[ERROR] AI must be player 2!\n";
        return Move();
    }

    transpositionTable.clear();
    nodeCount = 0;

    MoveGenerator moveGen;
    moveGen.generateMoves(currentPlayer, board);
    std::set<Move> allMoves = moveGen.getGeneratedMoves();
    std::cout << "Root: Generated " << allMoves.size() << " moves\n";
    if (allMoves.empty()) {
        std::cout << "No moves available at root!\n";
        return Move();
    }

    std::vector<MoveOrder> ordering; // Updated to MoveOrder
    ordering.reserve(allMoves.size());
    for (const Move& mv : allMoves) {
        MoveUndo undo;
        board.makeMove(mv, undo);
        int quickScore = hcalc.calculateHeuristic(board);
        board.unmakeMove(undo);
        ordering.push_back({mv, quickScore});
    }
    std::sort(ordering.begin(), ordering.end(),
              [](const MoveOrder& a, const MoveOrder& b) {
                  return a.heuristic > b.heuristic;
              });

    unsigned int numCores = std::thread::hardware_concurrency();
    if (numCores == 0) numCores = 2;
    size_t totalMoves = ordering.size();
    size_t groupSize = (totalMoves + numCores - 1) / numCores;

    ThreadPool pool(numCores);
    std::vector<std::future<std::pair<int, Move>>> futures;

    for (size_t groupStart = 0; groupStart < totalMoves; groupStart += groupSize) {
        size_t groupEnd = std::min(groupStart + groupSize, totalMoves);
        futures.push_back(pool.enqueue([this, &ordering, groupStart, groupEnd, &board, currentPlayer]() -> std::pair<int, Move> {
            int groupBestScore = std::numeric_limits<int>::min();
            Move groupBestMove;
            for (size_t i = groupStart; i < groupEnd; ++i) {
                const Move& candidate = ordering[i].move;
                Board boardCopy = board;
                MoveUndo undo;
                boardCopy.makeMove(candidate, undo);
                int score = minimax(boardCopy, maxDepth - 1, 1, false,
                                    std::numeric_limits<int>::min(),
                                    std::numeric_limits<int>::max());
                if (score > groupBestScore) {
                    groupBestScore = score;
                    groupBestMove = candidate;
                }
            }
            return std::make_pair(groupBestScore, groupBestMove);
        }));
    }

    for (auto& fut : futures) {
        auto [score, move] = fut.get();
        if (score > finalBestScore) {
            finalBestScore = score;
            finalBestMove = move;
        }
    }

    std::cout << "Best score: " << finalBestScore << "\n";
    std::cout << "Total nodes searched: " << nodeCount << "\n";
    if (finalBestMove.getSize() == 0) {
        std::cout << "No valid move found!\n";
    }

    return finalBestMove;
}