#include "board.hpp"


Game::Game(int bs, int ws, int zs, std::vector<std::vector<Board>>& grid)
    : board_size(bs),
      w_size(ws),
      z_size(zs),
      board_grid(grid),
      boardArea(boardBits[bs - 1]),
      notLeft(0xfefefefefefefefeULL),
      notRight(bs == 8 ? 0x7f7f7f7f7f7f7f7fULL : boardBits[bs - 1]) {
    updateScore();
}

void Game::updateScore() {
    std::fill(std::begin(score), std::end(score), 0);
    for (int i = 0; i < w_size; i++) {
        for (int j = 0; j < z_size; j++) {
            auto& curBoard = board_grid[i][j];
            score[Black] += Bitcount(curBoard.board[Black]);
            score[White] += Bitcount(curBoard.board[White]);
            score[Occupied] += Bitcount(curBoard.board[Occupied]);
            score[Empty] += Bitcount(~curBoard.board[Occupied]);
        }
    }
}

void Game::processDirection(U64& toFlip, U64 move, int shift, U64 boundary, const Board* curBoard, bool color) {
    U64 candidates = curBoard->board[!color] & shiftBits(move, shift) & boundary;
    U64 possibleFlips = candidates;
    while (candidates != 0) {
        candidates = shiftBits(candidates, shift) & boundary;
        if (curBoard->board[color] & candidates) {
            toFlip |= possibleFlips;
            break;
        }
        possibleFlips |= curBoard->board[!color] & candidates;
        candidates = curBoard->board[!color] & candidates;
    }
}

void Game::makeMove(const Move& move) {
    if (move.pass) {
        color = !color;
        return;
    }

    Board* curBoard = &(board_grid[move.w][move.z]);
    curBoard->board[color] |= move.move;
    curBoard->board[Occupied] |= move.move;

    //2D
    U64 toFlip = 0;
    processDirection(toFlip, move.move, -1, notRight, curBoard, color);
    processDirection(toFlip, move.move, -7, notLeft & boardArea, curBoard, color);
    processDirection(toFlip, move.move, -8, boardArea, curBoard, color);
    processDirection(toFlip, move.move, -9, notRight, curBoard, color);
    processDirection(toFlip, move.move, 1, notLeft & boardArea, curBoard, color);
    processDirection(toFlip, move.move, 7, notRight, curBoard, color);
    processDirection(toFlip, move.move, 8, boardArea, curBoard, color);
    processDirection(toFlip, move.move, 9, notLeft & boardArea, curBoard, color);

    curBoard->board[color] ^= toFlip;
    curBoard->board[!color] ^= toFlip; 

    {//3D & 4D
        U64 candidatesArray[9];
        std::fill(std::begin(candidatesArray), std::end(candidatesArray), move.move);
        bool boolArrays[8][9] = {};
        for (int i = 0; i < 8; i++) 
        std::fill(std::begin(boolArrays[i]), std::end(boolArrays[i]), false);

        static_for<8>([&](auto dir) {
            constexpr std::size_t i = dir.value;
            makeMoveRecursive<i>(move.w, move.z, candidatesArray, boolArrays[i]);
        });

    }

    color = !color;
    updateScore();
}

U64 Game::calculateMoves(U64 playerPieces, U64 enemyPieces, U64 unoccupied, U64 gameBounds, int shift) {
    U64 moves = 0;
    U64 candidates;

    candidates = enemyPieces & shiftBits(playerPieces, shift) & gameBounds;
    while (candidates != 0) {
        candidates = shiftBits(candidates, shift)  & gameBounds;

        moves |= unoccupied & candidates;
        candidates = enemyPieces & candidates;
    }

    return moves;
}

vector<Move> Game::movegen() {
    vector<Move> moveList;
    for (int i = 0; i < w_size; i++) {
        for (int j = 0; j < z_size; j++) {
            Board curBoard = board_grid[i][j];
            U64 playerPieces = curBoard.board[color];
            U64 enemyPieces = curBoard.board[!color];
            U64 unoccupied = ~curBoard.board[Occupied];
            //2D
            {
                U64 moves = 0;
                moves |= calculateMoves(playerPieces, enemyPieces, unoccupied, notRight, -1);
                moves |= calculateMoves(playerPieces, enemyPieces, unoccupied, notLeft & boardArea, -7);
                moves |= calculateMoves(playerPieces, enemyPieces, unoccupied, boardArea, -8);
                moves |= calculateMoves(playerPieces, enemyPieces, unoccupied, notRight, -9);

                moves |= calculateMoves(playerPieces, enemyPieces, unoccupied, notLeft & boardArea, 1);
                moves |= calculateMoves(playerPieces, enemyPieces, unoccupied, notRight, 7);
                moves |= calculateMoves(playerPieces, enemyPieces, unoccupied, boardArea, 8);
                moves |= calculateMoves(playerPieces, enemyPieces, unoccupied, notLeft & boardArea, 9);

                while (moves) {
                    moveList.push_back(Move(i, j, PopBit(moves)));
                }
            }
            //3D & 4D Movement
            {
                Board nextBoard = Board(0, 0);
                int directionOffsets[8][2] = {
                    { 1,  0},  // Direction 0: right
                    { 1,  1},  // Direction 1: top-right
                    { 0,  1},  // Direction 2: up
                    {-1,  1},  // Direction 3: top-left
                    {-1,  0},  // Direction 4: left
                    {-1, -1},  // Direction 5: bottom-left
                    { 0, -1},  // Direction 6: down
                    { 1, -1}   // Direction 7: bottom-right
                };

                for (int dir = 0;dir < 8;dir++) {
                    int iNew = i + directionOffsets[dir][0];
                    int jNew = j + directionOffsets[dir][1];
                    if (iNew >= 0 && iNew < w_size && jNew >= 0 && jNew < z_size) {
                        nextBoard = board_grid[iNew][jNew];
                        U64 nextEnemyPieces = nextBoard.board[!color];
                        U64 candidates[9] = {
                            nextEnemyPieces & (playerPieces),
                            nextEnemyPieces & (playerPieces << 1) & notLeft & boardArea,
                            nextEnemyPieces & (playerPieces << 9) & notLeft & boardArea,
                            nextEnemyPieces & (playerPieces << 8) & notLeft & boardArea,
                            nextEnemyPieces & (playerPieces << 7) & notRight,
                            nextEnemyPieces & (playerPieces >> 1) & notRight,
                            nextEnemyPieces & (playerPieces >> 9) & notRight,
                            nextEnemyPieces & (playerPieces >> 8) & notRight,
                            nextEnemyPieces & (playerPieces >> 7) & notLeft & boardArea
                        };

                        int increment = 2;
                        
                        iNew = i + increment*directionOffsets[dir][0];
                        jNew = j + increment*directionOffsets[dir][1];


                        while (iNew >= 0 && iNew < w_size && jNew >= 0 && jNew < z_size) {

                            nextBoard = board_grid[iNew][jNew];

                            candidates[1] = (candidates[1] << 1) & notLeft & boardArea;
                            candidates[2] = (candidates[2] << 9) & notLeft & boardArea;
                            candidates[3] = (candidates[3] << 8) & notLeft & boardArea;
                            candidates[4] = (candidates[4] << 7) & notRight;
                            candidates[5] = (candidates[5] >> 1) & notRight;
                            candidates[6] = (candidates[6] >> 9) & notRight;
                            candidates[7] = (candidates[7] >> 8) & notRight;
                            candidates[8] = (candidates[8] >> 7) & notLeft & boardArea;

                            U64 moves = 0;
                            for (int d = 0; d < 9;d++) {
                                moves |= ~nextBoard.board[Occupied] & candidates[d];
                                candidates[d] = nextBoard.board[!color] & candidates[d];
                            }

                            while (moves) {
                                moveList.push_back(Move(iNew, jNew, PopBit(moves))); //****U64 for each board which has moves, then pop out and make movelist ****
                            }

                            increment++;
                            iNew = i + increment*directionOffsets[dir][0];
                            jNew = j + increment*directionOffsets[dir][1];
                        }
                    }
                }
            }
        }
    }
    return moveList;
}

bool Game::gameOver() {
    bool playerNoMoves = movegen().empty();
    color = !color;
    bool enemyNoMoves = movegen().empty();
    color = !color;
    return playerNoMoves && enemyNoMoves;
}

std::string Game::toString() const {
    std::string output;

    for (int l = 0; l < w_size; l++) {
        output += "═";
        for (int i = 0; i < board_size; i++) {
            if (i == board_size / 2)
                output += std::to_string(l + 1) + "═";
            else
                output += "══";
        }
    }
    output += "═\n";

    for (int k = z_size - 1; k >= 0; k--) {
        for (int j = 0; j < board_size; j++) {
            if (j == board_size / 2)
                output += std::to_string(k + 1);
            else
                output += "║";

            for (int l = 0; l < w_size; l++) {
                for (int i = 0; i < board_size; i++) {
                    const Board& curBoard = board_grid[l][k];
                    U64 sq = 1ull << ((board_size - 1) * 8 - (j * 8) + i);

                    if (curBoard.board[0] & sq)
                        output += "⚫";
                    else if (curBoard.board[1] & sq)
                        output += "⚪";
                    else
                        output += " \u20E3 ";
                }
                output += "║";
            }
            output += "\n";
        }

        for (int l = 0; l < w_size; l++) {
            output += "═";
            for (int i = 0; i < board_size; i++)
                output += "══";
        }
        output += "═\n";
    }

    return output;
}

std::ostream& operator<<(std::ostream& os, const Game& game) {
    return os << game.toString();
}
