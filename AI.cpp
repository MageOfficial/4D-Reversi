#include "board.cpp"

void greedyMoveMake(FullBoard* fullboard, int color) {
    std::vector<Move> moveList = movegen(*fullboard, color);
    if (moveList.empty()) {
        return;
    }
    int max = 0;
    Move* maxMove;
    int scoreGain = 0;
    for (Move& move : moveList) {
        if ((move.move & 0x9000009ULL) && (
            move.z == 0 && move.w == 0 ||
            move.z == 0 && move.w == fullboard->w_size ||
            move.z == fullboard->z_size && move.w == 0 ||
            move.z == fullboard->z_size && move.w == fullboard->w_size
            )) {
            scoreGain += 10;
        }
        FullBoard tempBoard = *fullboard;
        makeMove(&tempBoard, move, color);
        scoreGain += tempBoard.score[color] - fullboard->score[color];
        if (max < scoreGain) {
            max = scoreGain;
            maxMove = &move;
        }
    }
    makeMove(fullboard, *maxMove, color);
}

int depthSearch(FullBoard fullboard, int depth, int color, int alpha, int beta) {

    if (depth == 0) {

        int eval = fullboard.score[color]-fullboard.score[1-color] + movegen(fullboard, color).size()-movegen(fullboard, 1-color).size();
    
        for (int w = 0; w < fullboard.w_size; w++) {
            for (int z = 0; z < fullboard.z_size; z++) {
                Board curBoard =fullboard.w_axis[w].z_axis[z];
                eval+=2*Bitcount(curBoard.board[color]&0x9000009ULL);
                eval-=2*Bitcount(curBoard.board[1-color]&0x9000009ULL);
            }
        }
        return eval;
    }

    std::vector<Move> moveList = movegen(fullboard, color);
    if (moveList.empty()) {
        return -depthSearch(fullboard, depth - 1, 1 - color, -beta, -alpha);
    }
    Move bestMove = moveList[0];
    int score = -INFINITY;
    for (Move& move : moveList) {
        FullBoard tempBoard = fullboard;
        makeMove(&tempBoard, move, color);
        score = std::max(score, -depthSearch(tempBoard, depth - 1, 1 - color, -beta, -alpha));

        if (score > alpha) {
            alpha = score;
        }
        if (alpha >= beta) {
            break;
        }
    }
    return score;
}

void depthMakeMove(FullBoard* fullboard, int depth, int color, int alpha = -INFINITY, int beta = INFINITY) {

    std::vector<Move> moveList = movegen(*fullboard, color);
    if (moveList.empty()) {
        return;
    }
    Move bestMove = moveList[0];
    int score = -INFINITY;
    for (Move& move : moveList) {

        FullBoard tempBoard = *fullboard;
        makeMove(&tempBoard, move, color);

        score = std::max(score, -depthSearch(tempBoard, depth - 1, 1 - color, -beta, -alpha));

        if (score > alpha) {
            alpha = score;
            bestMove = move;
        }
        if (alpha >= beta) {
            break;
        }
    }
    makeMove(fullboard, bestMove, color);
}

