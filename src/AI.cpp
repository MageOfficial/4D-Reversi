#include "board.hpp"
#include <optional>

#define CORNER 0x9000009ULL
#define EDGE 0xf09090fULL

using std::string;
using std::vector;

bool isCornerMove(const Move& move, const Game& game) {
    return (move.move & CORNER) && (
        (move.z == 0 || move.z == game.z_size-1) && 
        (move.w == 0 || move.w == game.w_size-1)
    );
}

// Basic greedy evaluation function that returns the best move based on immediate score gain
Move greedyEval(Game& game) {
    vector<Move> moveList = game.movegen();
    if (moveList.empty()) {
        return Move(true, 0);
    }
    int max = std::numeric_limits<int>::min();
    Move bestMove = moveList[0];
    for (Move& move : moveList) {
        Game tempBoard = game;
        tempBoard.makeMove(move);
        int scoreGain = tempBoard.score[game.color] - game.score[game.color];
        if (max < scoreGain) {
            max = scoreGain;
            bestMove = move;
        }
    }
    return bestMove;
}

//Static Evaluation Function
//Incentivizes corner and edge control, more score, and more move options
int eval(Game& game) {
    //Initial greedy evaluation
    int eval = game.score[game.color] - game.score[!game.color];

    //Encourage more moves for the player and less for the opponent
    eval += game.movegen().size();
    game.color = !game.color;
    eval -= game.movegen().size();
    game.color = !game.color;

    //Corner and edge bonuses
    for (int w = 0; w < game.w_size; w++) {
        for (int z = 0; z < game.z_size; z++) {
            Board curBoard = game.board_grid[w][z];
            int mult = 1;
            if ((w == 0 && z == 0) || (w == 0 && z == game.z_size-1) ||
                (w == game.w_size-1 && z == 0) || (w == game.w_size-1 && z == game.z_size-1)) {
                eval += Bitcount(curBoard.board[game.color]);
                eval -= Bitcount(curBoard.board[!game.color]);
                mult = 4;
            }
            eval += 4 * Bitcount(curBoard.board[game.color] & CORNER) * mult;
            eval += 2 * Bitcount(curBoard.board[game.color] & EDGE) * mult;
            eval -= 4 * Bitcount(curBoard.board[!game.color] & CORNER) * mult;
            eval -= 2 * Bitcount(curBoard.board[!game.color] & EDGE) * mult;
        }
    }
    return eval;
}

Move advancedGreedyEval(Game& game) {
    vector<Move> moveList = game.movegen();
    if (moveList.empty()) {
        return Move(true, 0);
    }
    int max = std::numeric_limits<int>::min();
    Move bestMove = moveList[0];
    for (Move& move : moveList) {
        Game tempBoard = game;
        tempBoard.makeMove(move);
        int scoreGain = eval(tempBoard);
        if (max < scoreGain) {
            max = scoreGain;
            bestMove = move;
        }
    }
    return bestMove;
}

Move depthSearch(Game& game, int depth, int alpha = std::numeric_limits<int>::min(), int beta = std::numeric_limits<int>::max()) {
    if (depth == 0) {
        return Move(true, eval(game));
    }

    vector<Move> moveList = game.movegen();
    if (moveList.empty()) {
        game.color = !game.color;
        return Move(true, -(depthSearch(game, depth - 1, -beta, -alpha).value));
    }

    int mult = 1;
    for (Move& move : moveList) {
        if (isCornerMove(move, game)) {
            move.value += 1;
            mult = 4;
        }
        move.value += 4 * Bitcount(move.move & CORNER) * mult;
        move.value += 2 * Bitcount(move.move & EDGE) * mult;
    }
    std::sort(moveList.begin(), moveList.end());

    Move bestMove = moveList[0];
    int score = -INFINITY;
    for (Move& move : moveList) {
        Game tempBoard = game;
        tempBoard.makeMove(move);

        Move res = depthSearch(tempBoard, depth - 1, -beta, -alpha);
        score = std::max(score, -res.value);

        if (score > alpha) {
            bestMove = move;
            alpha = score;
        }
        if (alpha >= beta) {
            break;
        }
    }
    bestMove.value = score;
    return bestMove;
}