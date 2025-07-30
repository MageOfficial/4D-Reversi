#include "AI.hpp"
#include <limits>

bool isCornerMove(const Move& move, const Game& game) {
    return (move.move & CORNER) && (
        (move.z == 0 || move.z == game.z_size-1) && 
        (move.w == 0 || move.w == game.w_size-1)
    );
}

inline Move PASS{true, 0};

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

int posChecked = 0;

//Static Evaluation Function
//Incentivizes corner and edge control, more score, and more move options
int eval(Game& game) {
    posChecked++;
    //Initial greedy evaluation
    int eval = game.score[game.color] - game.score[!game.color];

    //Options Incentive
    int playerMoves=game.movegen().size();
    eval += playerMoves;
    game.color = !game.color;
    int enemyMoves=game.movegen().size();
    eval -= enemyMoves;
    game.color = !game.color;

    //Game Over
    if(playerMoves == 0 && enemyMoves == 0){
        if(game.score[game.color] > game.score[!game.color]) {
            return std::numeric_limits<int>::max(); // Player wins
        } else if(game.score[game.color] < game.score[!game.color]) {
            return std::numeric_limits<int>::min(); // Player loses
        } else {
            return 0; // Draw
        }
    }

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

void sortMoves( Game& game, vector<Move> &moveList) {
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
}


Move depthSearch(Game& game, int depth, int alpha, int beta) {
    if (depth == 0) {
        return Move(true, eval(game));
    }

    vector<Move> moveList = game.movegen();
    if (moveList.empty()) {
        game.makeMove(PASS);
        if(game.movegen().empty()) return Move(true, eval(game)); //Game Over

        return Move(true, -(depthSearch(game, depth - 1, -beta, -alpha).value));
    }

    sortMoves(game, moveList);

    Move bestMove = moveList[0];
    int score = std::numeric_limits<int>::min();
    for (Move& move : moveList) {
        Game tempBoard = game;
        tempBoard.makeMove(move);

        Move res = depthSearch(tempBoard, depth - 1, -beta, -alpha);

        if(-res.value>score) {
            score = -res.value;
            bestMove = move;
        }

        if (score > alpha) {
            alpha = score;
        }
        if (alpha >= beta) {
            break;
        }
    }
    bestMove.value = score;
    return bestMove;
}