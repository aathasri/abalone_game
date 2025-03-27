#pragma once
#include <array>
#include <vector>
#include <unordered_map>
#include <string>

enum CellState { EMPTY = 0, BLACK, WHITE };
using BoardArray = std::array<CellState, 61>; // 61 valid positions on the Abalone board

int evaluateBoard(const BoardArray& board, CellState player);

extern const std::unordered_map<std::string, int> posToIdx;
extern const std::array<std::string, 61> idxToPos;
extern const std::array<std::vector<int>, 61> neighborIndices;
extern const std::array<int, 61> centerDistances;
