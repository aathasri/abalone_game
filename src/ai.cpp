#include "ai.h"
#include <cmath>
#include <unordered_map>
#include <chrono>

// Position index mapping (61 valid cells in linear format)
const std::unordered_map<std::string, int> posToIdx = {
    // row A
    {"A1", 0}, {"A2", 1}, {"A3", 2}, {"A4", 3}, {"A5", 4},
    // row B
    {"B1", 5}, {"B2", 6}, {"B3", 7}, {"B4", 8}, {"B5", 9}, {"B6", 10},
    // row C
    {"C1", 11}, {"C2", 12}, {"C3", 13}, {"C4", 14}, {"C5", 15}, {"C6", 16}, {"C7", 17},
    // row D
    {"D1", 18}, {"D2", 19}, {"D3", 20}, {"D4", 21}, {"D5", 22}, {"D6", 23}, {"D7", 24}, {"D8", 25},
    // row E
    {"E1", 26}, {"E2", 27}, {"E3", 28}, {"E4", 29}, {"E5", 30}, {"E6", 31}, {"E7", 32}, {"E8", 33}, {"E9", 34},
    // row F
    {"F2", 35}, {"F3", 36}, {"F4", 37}, {"F5", 38}, {"F6", 39}, {"F7", 40}, {"F8", 41}, {"F9", 42},
    // row G
    {"G3", 43}, {"G4", 44}, {"G5", 45}, {"G6", 46}, {"G7", 47}, {"G8", 48}, {"G9", 49},
    // row H
    {"H4", 50}, {"H5", 51}, {"H6", 52}, {"H7", 53}, {"H8", 54}, {"H9", 55},
    // row I
    {"I5", 56}, {"I6", 57}, {"I7", 58}, {"I8", 59}, {"I9", 60},
};

const std::array<std::string, 61> idxToPos = {
    "A1","A2","A3","A4","A5","B1","B2","B3","B4","B5","B6","C1","C2","C3","C4","C5","C6","C7",
    "D1","D2","D3","D4","D5","D6","D7","D8","E1","E2","E3","E4","E5","E6","E7","E8","E9",
    "F2","F3","F4","F5","F6","F7","F8","F9","G3","G4","G5","G6","G7","G8","G9",
    "H4","H5","H6","H7","H8","H9","I5","I6","I7","I8","I9"
};

// Neighbor indices for each position (6 directions in hex grid)
const std::array<std::vector<int>, 61> neighborIndices = {
    /* A1 */ {1, 6},                       /* A2 */ {0, 2, 6, 7},
    /* A3 */ {1, 3, 7, 8},                 /* A4 */ {2, 4, 8, 9},
    /* A5 */ {3, 9, 10},                   /* B1 */ {0, 6, 11},
    /* B2 */ {1, 5, 7, 11, 12},            /* B3 */ {2, 6, 8, 12, 13},
    /* B4 */ {3, 7, 9, 13, 14},            /* B5 */ {4, 8, 10, 14, 15},
    /* B6 */ {9, 15, 16},                  /* C1 */ {5, 12, 18},
    /* C2 */ {6, 11, 13, 18, 19},          /* C3 */ {7, 12, 14, 19, 20},
    /* C4 */ {8, 13, 15, 20, 21},          /* C5 */ {9, 14, 16, 21, 22},
    /* C6 */ {10, 15, 17, 22, 23},         /* C7 */ {16, 23, 24},
    /* D1 */ {11, 19, 26},                 /* D2 */ {12, 18, 20, 26, 27},
    /* D3 */ {13, 19, 21, 27, 28},         /* D4 */ {14, 20, 22, 28, 29},
    /* D5 */ {15, 21, 23, 29, 30},         /* D6 */ {16, 22, 24, 30, 31},
    /* D7 */ {17, 23, 25, 31, 32},         /* D8 */ {24, 32, 33},
    /* E1 */ {18, 27, 35},                 /* E2 */ {19, 26, 28, 35, 36},
    /* E3 */ {20, 27, 29, 36, 37},         /* E4 */ {21, 28, 30, 37, 38},
    /* E5 */ {22, 29, 31, 38, 39},         /* E6 */ {23, 30, 32, 39, 40},
    /* E7 */ {24, 31, 33, 40, 41},         /* E8 */ {25, 32, 34, 41, 42},
    /* E9 */ {33, 42},                     /* F2 */ {26, 36, 43},
    /* F3 */ {27, 35, 37, 43, 44},         /* F4 */ {28, 36, 38, 44, 45},
    /* F5 */ {29, 37, 39, 45, 46},         /* F6 */ {30, 38, 40, 46, 47},
    /* F7 */ {31, 39, 41, 47, 48},         /* F8 */ {32, 40, 42, 48, 49},
    /* F9 */ {34, 41, 49},                 /* G3 */ {35, 44, 50},
    /* G4 */ {36, 43, 45, 50, 51},         /* G5 */ {37, 44, 46, 51, 52},
    /* G6 */ {38, 45, 47, 52, 53},         /* G7 */ {39, 46, 48, 53, 54},
    /* G8 */ {40, 47, 49, 54, 55},         /* G9 */ {42, 48, 55},
    /* H4 */ {43, 51, 56},                 /* H5 */ {44, 50, 52, 56, 57},
    /* H6 */ {45, 51, 53, 57, 58},         /* H7 */ {46, 52, 54, 58, 59},
    /* H8 */ {47, 53, 55, 59, 60},         /* H9 */ {48, 54, 60},
    /* I5 */ {50, 57},                     /* I6 */ {51, 56, 58},
    /* I7 */ {52, 57, 59},                 /* I8 */ {53, 58, 60},
    /* I9 */ {54, 59},
};

// Precomputed distances from center (E5 = idx 30)
const std::array<int, 61> centerDistances = {
    8,7,6,5,4, 7,6,5,4,3,2, 6,5,4,3,2,1,0,
    6,5,4,3,2,1,0,1, 5,4,3,2,1,0,1,2,3,
    6,5,4,3,2,1,2,3, 5,4,3,2,3,4,5,
    6,5,4,5,6,7, 8,7,6,7,8
};

const std::unordered_map<int, bool> edgeMap = {
    {0,true}, {1,true}, {2,true}, {3,true}, {4,true},
    {5,true}, {10,true}, {11,true}, {17,true},
    {18,true}, {25,true}, {26,true}, {34,true},
    {35,true}, {42,true}, {43,true}, {49,true},
    {50,true}, {55,true}, {56,true}, {57,true},
    {58,true}, {59,true}, {60,true},
};

bool isEdge(int idx) {
    return edgeMap.count(idx);
}

int evaluateBoard(const BoardArray& board, CellState player) {
    CellState opponent = (player == BLACK) ? WHITE : BLACK;

    int playerCount = 0;
    int opponentCount = 0;
    int centerControl = 0;
    int cohesion = 0;
    int edgePenalty = 0;

    for (int i = 0; i < 61; ++i) {
        if (board[i] == player) {
            playerCount++;
            centerControl += (10 - centerDistances[i]);

            for (int neighbor : neighborIndices[i]) {
                if (board[neighbor] == player) cohesion++;
            }

            if (isEdge(i)) edgePenalty++;

        } else if (board[i] == opponent) {
            opponentCount++;
        }
    }

    int score = 0;
    score += (playerCount - opponentCount) * 100;
    score += centerControl * 5;
    score += cohesion * 3;
    score -= edgePenalty * 4;

    return score;
}
