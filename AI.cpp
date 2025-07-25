#include "board.cpp"
#include <optional>

#define CORNER 0x9000009ULL
#define EDGE 0xf09090fULL

using std::string;
using std::vector;

bool isCornerMove(const Move& move, const FullBoard& fullboard) {
    return (move.move & CORNER) && (
        (move.z == 0 || move.z == fullboard.z_size-1) && 
        (move.w == 0 ||move.w == fullboard.w_size-1)
        );
}

// Basic greedy evaluation function that returns the best move based on immediate score gain
Move greedyEval(FullBoard& fullboard) {
    vector<Move> moveList = movegen(fullboard);
    if (moveList.empty()) {
        return Move(true, 0);
    }
    int max = std::numeric_limits<int>::min();;
    Move& bestMove = moveList[0];
    for (Move& move : moveList) {
        FullBoard tempBoard = fullboard;
        makeMove(tempBoard, move);
        int scoreGain = tempBoard.score[fullboard.color] - fullboard.score[fullboard.color];
        if (max < scoreGain) {
            max = scoreGain;
            bestMove = move;
        }
    }
    return bestMove;
}

//Static Evaluation Function
//Incentivizes corner and edge control, more score, and more move options
int eval(FullBoard& fullboard) {
    //Initial greedy evaluation
    int eval = fullboard.score[fullboard.color]-fullboard.score[!fullboard.color];

    //Encourage more moves for the player and less for the opponent
    eval += movegen(fullboard).size();
    fullboard.color = !fullboard.color;
    eval -= movegen(fullboard).size();
    fullboard.color = !fullboard.color;

    //Corner and edge bonuses
    for (int w = 0; w < fullboard.w_size; w++) {
        for (int z = 0; z < fullboard.z_size; z++) {
            Board curBoard =fullboard.w_axis[w].z_axis[z];
            int mult=1;
            if((w==0&&z==0)||(w==0&&z==fullboard.z_size-1)||(w==fullboard.w_size-1&&z==0)||(w==fullboard.w_size-1&&z==fullboard.z_size-1)){
                eval+=Bitcount(curBoard.board[fullboard.color]);
                eval-=Bitcount(curBoard.board[!fullboard.color]);
                mult=4;
            }
            eval+=4*Bitcount(curBoard.board[fullboard.color]&CORNER)*mult;
            eval+=2*Bitcount(curBoard.board[fullboard.color]&EDGE)*mult;
            eval-=4*Bitcount(curBoard.board[!fullboard.color]&CORNER)*mult;
            eval-=2*Bitcount(curBoard.board[!fullboard.color]&EDGE)*mult;
        }
    }
    return eval;
}

Move advancedGreedyEval(FullBoard& fullboard) {
    vector<Move> moveList = movegen(fullboard);
    if (moveList.empty()) {
        return Move(true, 0);
    }
    int max = std::numeric_limits<int>::min();;
    Move& bestMove = moveList[0];
    for (Move& move : moveList) {
        FullBoard tempBoard = fullboard;
        makeMove(tempBoard, move);
        int scoreGain = eval(tempBoard);
        if (max < scoreGain) {
            max = scoreGain;
            bestMove = move;
        }
    }
    return bestMove;
}

Move depthSearch(FullBoard& fullboard, int depth, int alpha = -INFINITY, int beta = INFINITY) {
    if (depth == 0) {
        
        return  Move(true, eval(fullboard));
    }

    vector<Move> moveList = movegen(fullboard);
    if (moveList.empty()) {
        fullboard.color = !fullboard.color;
        return Move(true, -(depthSearch(fullboard, depth - 1, -beta, -alpha).value));
    }

    int mult=1;
    for (Move& move : moveList) {
        if(isCornerMove(move, fullboard)) {
            move.value+=1;
            mult=4;
        }
        move.value+=4*Bitcount(move.move&CORNER)*mult;
        move.value+=2*Bitcount(move.move&EDGE)*mult;
    }
    std::sort(moveList.begin(), moveList.end());

    Move bestMove = moveList[0];
    int score = -INFINITY;
    for (Move& move : moveList) {
        FullBoard tempBoard = fullboard;
        makeMove(tempBoard, move);

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