#pragma once

#include "board.hpp"
#include <vector>

#define CORNER 0x9000009ULL
#define EDGE 0xf09090fULL

bool isCornerMove(const Move& move, const Game& game);

Move greedyEval(Game& game);
int eval(Game& game);
Move advancedGreedyEval(Game& game);
Move depthSearch(Game& game, int depth, int alpha = std::numeric_limits<int>::min(), int beta = std::numeric_limits<int>::max());
