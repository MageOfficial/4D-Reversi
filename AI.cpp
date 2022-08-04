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
                int mult=1;
                if((w==0&&z==0)||(w==0&&z==fullboard.z_size-1)||(w==fullboard.w_size-1&&z==0)||(w==fullboard.w_size-1&&z==fullboard.z_size-1)){
                    eval+=Bitcount(curBoard.board[color]);
                    eval-=Bitcount(curBoard.board[1-color]);
                    mult=4;
                }
                eval+=4*Bitcount(curBoard.board[color]&0x9000009ULL)*mult;
                eval+=2*Bitcount(curBoard.board[color]&0xf09090fULL)*mult;
                eval-=4*Bitcount(curBoard.board[1-color]&0x9000009ULL)*mult;
                eval-=2*Bitcount(curBoard.board[1-color]&0xf09090fUL)*mult;
            }
        }
        return eval;
    }

    std::vector<Move> moveList = movegen(fullboard, color);
    if (moveList.empty()) {
        return -depthSearch(fullboard, depth - 1, 1 - color, -beta, -alpha);
    }

    int mult=1;
    for (Move& move : moveList) {
        if((move.w==0&&move.z==0)||(move.w==0&&move.z==fullboard.z_size-1)||(move.w==fullboard.w_size-1&&move.z==0)||(move.w==fullboard.w_size-1&&move.z==fullboard.z_size-1)){
            move.sortVal+=1;
            mult=4;
        }
        move.sortVal+=4*Bitcount(move.move&0x9000009ULL)*mult;
        move.sortVal+=2*Bitcount(move.move&0xf09090fULL)*mult;
    }
    std::sort(moveList.begin(), moveList.end());

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

    int mult=1;
    for (Move& move : moveList) {
        move.sortVal=0;
        if((move.w==0&&move.z==0)||(move.w==0&&move.z==fullboard->z_size-1)||(move.w==fullboard->w_size-1&&move.z==0)||(move.w==fullboard->w_size-1&&move.z==fullboard->z_size-1)){
            move.sortVal+=1;
            mult=4;
        }
        move.sortVal+=4*Bitcount(move.move&0x9000009ULL)*mult;
        move.sortVal+=2*Bitcount(move.move&0xf09090fULL)*mult;
    }
    std::sort(moveList.begin(), moveList.end());

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

