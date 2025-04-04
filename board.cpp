#pragma once

#include <stdint.h>
#include <iostream>
#include <vector>
#include <immintrin.h>
#include <string> 
#include <math.h>
#include <algorithm>

#ifdef _MSC_VER
#define _ASSUME(cond) __assume(cond)
#define _Compiletime __forceinline static constexpr
#define _NoInline __declspec(noinline)
#define _Inline inline
#define _ForceInline __forceinline
#define Bitcount(X) __popcnt64(X)
#elif defined(__clang__)
#define _ASSUME(cond) ((cond) ? static_cast<void>(0) : __builtin_unreachable())
#define _Compiletime __attribute__((always_inline)) static constexpr
#define _NoInline __attribute__((noinline))
#define _Inline inline 
#define _ForceInline __attribute__((always_inline))
#define Bitcount(X) static_cast<uint64_t>(__builtin_popcountll(X))
#elif defined(__GNUC__)
#define _ASSUME(cond) ((cond) ? static_cast<void>(0) : __builtin_unreachable())
#define _Compiletime __attribute__((always_inline)) static constexpr
#define _NoInline __attribute__ ((noinline))
#define _Inline inline
#define _ForceInline __attribute__((always_inline)) inline
#define Bitcount(X) static_cast<uint64_t>(__builtin_popcountll(X))
#else
#define _ASSUME(cond) static_cast<void>(!!(cond))
#define _Compiletime static constexpr
#define _Inline inline 
#endif

#define SquareOf(X) _tzcnt_u64(X)
#define Bitloop(X) for(;X; X = _blsr_u64(X))

typedef uint64_t U64;

_Inline static U64 PopBit(U64& val) {
  U64 lsb = _blsi_u64(val);
  val ^= lsb;
  return lsb;
}

constexpr U64 boardBits[8] = { 0x1ULL,0x303ULL,0x70707ULL,0xf0f0f0fULL,0x1f1f1f1f1fULL,0x3f3f3f3f3f3fULL,0x7f7f7f7f7f7f7fULL,0xffffffffffffffffULL };

typedef std::string String;

struct Board {
  U64 board[3];
  Board(U64 w, U64 b) {
    board[0] = b;
    board[1] = w;
    board[2] = w | b;
  }
};

struct BoardStack {
  std::vector<Board> z_axis;
};

struct FullBoard {
  int board_size;
  int w_size;
  int z_size;
  bool color = 0; //0 for black 1 for white

  int score[4] = {};

  U64 boardArea;
  U64 notLeft;
  U64 notRight;

  std::vector<BoardStack> w_axis;

  FullBoard(int bs, int ws, int zs, std::vector<BoardStack> boardStack) {
    board_size = bs;
    w_size = ws;
    z_size = zs;

    w_axis = boardStack;

    boardArea = boardBits[bs - 1];
    notLeft = 0xfefefefefefefefe;
    if (bs == 8) {
      notRight = 0x7f7f7f7f7f7f7f7f;
    }
    else {
      notRight = boardArea;
    }

    for (int i = 0; i < w_size; i++) {
      for (int j = 0; j < z_size; j++) {
        Board curBoard = w_axis[i].z_axis[j];
        score[0] += Bitcount(curBoard.board[0]);
        score[1] += Bitcount(curBoard.board[1]);
        score[2] += Bitcount(curBoard.board[2]);
        score[3] += Bitcount(~curBoard.board[2]);
      }
    }
  }
};

struct Move {
  int w;
  int z;
  U64 move;
  int sortVal;
  Move(int w, int z, U64 move) : w(w), z(z), move(move) {}

  bool operator < (const Move& move) const
    {
        return (sortVal > move.sortVal);
    }
};

void updateScore(FullBoard* fullboard) {
  std::fill(std::begin(fullboard->score), std::end(fullboard->score), 0);
  for (int i = 0; i < fullboard->w_size; i++) {
    for (int j = 0; j < fullboard->z_size; j++) {
      auto& curBoard = fullboard->w_axis[i].z_axis[j];
      fullboard->score[0] += Bitcount(curBoard.board[0]);
      fullboard->score[1] += Bitcount(curBoard.board[1]);
      fullboard->score[2] += Bitcount(curBoard.board[2]);
      fullboard->score[3] += Bitcount(~curBoard.board[2]);
    }
  }
}

//Recursively makes moves in 3D and 4D
template<int dir> void makeMoveRecursive(FullBoard* fullboard, const int w, const int z, U64 candidates[9], bool bools[9]) {
  static constexpr int dW[] = {1, 1, 0, -1, -1, -1, 0, 1};
  static constexpr int dZ[] = {0, 1, 1, 1, 0, -1, -1, -1};
  
  int newW = w + dW[dir], newZ = z + dZ[dir];
  if (newW < 0 || newW >= fullboard->w_size || newZ < 0 || newZ >= fullboard->z_size) return;
  
  Board* curBoard = &(fullboard->w_axis[newW].z_axis[newZ]);

  U64 tempCandidates[9] = {
    candidates[0],
    (candidates[1] << 1) & fullboard->notLeft & fullboard->boardArea,
    (candidates[2] << 9) & fullboard->notLeft & fullboard->boardArea,
    (candidates[3] << 8) & fullboard->notLeft & fullboard->boardArea,
    (candidates[4] << 7) & fullboard->notRight,
    (candidates[5] >> 1) & fullboard->notRight,
    (candidates[6] >> 9) & fullboard->notRight,
    (candidates[7] >> 8) & fullboard->notRight,
    (candidates[8] >> 7) & fullboard->notLeft & fullboard->boardArea
  };

U64 currentCandidates[9];
for (int i = 0; i < 9; i++) 
    currentCandidates[i] = curBoard->board[!fullboard->color] & tempCandidates[i];

  bool newBoolArray[9];
  for (int i = 0; i < 9;i++) {
    bools[i] = (bools[i] || ((curBoard->board[fullboard->color] & tempCandidates[i]) != 0));
    newBoolArray[i] = bools[i];
  }

  makeMoveRecursive<dir>(fullboard, newW, newZ, currentCandidates, bools);

  for (int i = 0; i < 9; i++) {
    if ((!newBoolArray[i]) && (bools[i])) {
      curBoard->board[fullboard->color] ^= currentCandidates[i];
      curBoard->board[!fullboard->color] ^= currentCandidates[i];
    }
  }
  return;
}

//Clears up repeated code in makeMove
void processDirection(U64& toFlip, U64 move, int shift, U64 boundary, const Board* curBoard, int color) {
    U64 candidates = curBoard->board[1 - color] & ((shift > 0) ? (move << shift) : (move >> -shift)) & boundary;
    U64 possibleFlips = candidates;
    while (candidates != 0) {
        candidates = ((shift > 0) ? (candidates << shift) : (candidates >> -shift)) & boundary;
        if (curBoard->board[color] & candidates) {
            toFlip |= possibleFlips;
            break;
        }
        possibleFlips |= curBoard->board[1 - color] & candidates;
        candidates = curBoard->board[1 - color] & candidates;
    }
}
  
//Makes a move on the 2D board and calls makeMoveRecursive for 3D and 4D
void makeMove(FullBoard* fullboard, Move move) {
  Board* curBoard = &(fullboard->w_axis[move.w].z_axis[move.z]);
  curBoard->board[fullboard->color] |= move.move;
  curBoard->board[2] |= move.move;

  //2D
  U64 toFlip = 0;
  processDirection(toFlip, move.move, -1, fullboard->notRight, curBoard, fullboard->color);
  processDirection(toFlip, move.move, -7, fullboard->notLeft & fullboard->boardArea, curBoard, fullboard->color);
  processDirection(toFlip, move.move, -8, fullboard->boardArea, curBoard, fullboard->color);
  processDirection(toFlip, move.move, -9, fullboard->notRight, curBoard, fullboard->color);
  processDirection(toFlip, move.move, 1, fullboard->notLeft & fullboard->boardArea, curBoard, fullboard->color);
  processDirection(toFlip, move.move, 7, fullboard->notRight, curBoard, fullboard->color);
  processDirection(toFlip, move.move, 8, fullboard->boardArea, curBoard, fullboard->color);
  processDirection(toFlip, move.move, 9, fullboard->notLeft & fullboard->boardArea, curBoard, fullboard->color);

  
  curBoard->board[fullboard->color] ^= toFlip;
  curBoard->board[!fullboard->color] ^= toFlip;
  
  {//3D & 4D
      U64 candidatesArray[9];
      std::fill(std::begin(candidatesArray), std::end(candidatesArray), move.move);
      bool boolArrays[8][9] = {};
      for (int i = 0; i < 8; i++) 
        std::fill(std::begin(boolArrays[i]), std::end(boolArrays[i]), false);

      makeMoveRecursive<0>(fullboard, move.w, move.z, candidatesArray, boolArrays[0]);
      makeMoveRecursive<1>(fullboard, move.w, move.z, candidatesArray, boolArrays[1]);
      makeMoveRecursive<2>(fullboard, move.w, move.z, candidatesArray, boolArrays[2]);
      makeMoveRecursive<3>(fullboard, move.w, move.z, candidatesArray, boolArrays[3]);
      makeMoveRecursive<4>(fullboard, move.w, move.z, candidatesArray, boolArrays[4]);
      makeMoveRecursive<5>(fullboard, move.w, move.z, candidatesArray, boolArrays[5]);
      makeMoveRecursive<6>(fullboard, move.w, move.z, candidatesArray, boolArrays[6]);
      makeMoveRecursive<7>(fullboard, move.w, move.z, candidatesArray, boolArrays[7]);
  }
  fullboard->color = !fullboard->color;
  updateScore(fullboard);
}

void printFullBoard(FullBoard fullboard) {
  String output;
  for (int l = 0; l < fullboard.w_size; l++) {
    output += "═";
    for (int i = 0; i < fullboard.board_size; i++) {
      if (i == fullboard.board_size / 2) {
        output += std::to_string(l + 1) + "═";
      }
      else {
        output += "══";
      }
    }
  }
  output += "═\n";

  for (int k = fullboard.z_size - 1; k >= 0; k--) {
    for (int j = 0; j < fullboard.board_size; j++) {
      if (j == fullboard.board_size / 2) {
        output += std::to_string(k + 1);
      }
      else {
        output += "║";
      }
      for (int l = 0; l < fullboard.w_size; l++) {
        for (int i = 0; i < fullboard.board_size; i++) {

          Board curBoard = fullboard.w_axis[l].z_axis[k];
          U64 sq = 1ull << ((fullboard.board_size - 1) * 8 - (j * 8) + i);
          if ((curBoard.board[0] & sq) != 0) {
            output += "⚫";
          }
          else if ((curBoard.board[1] & sq) != 0) {
            output += "⚪";
          }
          else {
            output += " \u20E3 ";
          }
        }
        output += "║";
      }
      output += "\n";
    }
    for (int l = 0; l < fullboard.w_size; l++) {
      output += "═";
      for (int i = 0; i < fullboard.board_size; i++) {
        output += "══";
      }
    }
    output += "═\n";
  }
  std::cout << output << std::endl;
}

//Removes repeat code in movegen
U64 calculateMoves(U64 playerPieces, U64 enemyPieces, U64 unoccupied, U64 fullboardCondition, int shift) {
    U64 moves = 0;
    U64 candidates;
    
    candidates = enemyPieces & ((shift>0)?(playerPieces >> shift):(playerPieces << -shift)) & fullboardCondition;
    while (candidates != 0) {
        candidates = ((shift>0)?(candidates >> shift):(candidates << -shift)) & fullboardCondition;
        
        moves |= unoccupied & candidates;
        candidates = enemyPieces & candidates;
    }

    return moves;
}

std::vector<Move> movegen(FullBoard fullboard) {
  //Spatial Moves
  std::vector<Move> moveList;
  {
    for (int i = 0; i < fullboard.w_size; i++) {
      for (int j = 0; j < fullboard.z_size; j++) {
        Board curBoard = fullboard.w_axis[i].z_axis[j];
        U64 playerPieces = curBoard.board[fullboard.color];
        U64 enemyPieces = curBoard.board[1 - fullboard.color];
        U64 unoccupied = ~curBoard.board[2];
        //2D
        {
          U64 moves = 0;
          moves |= calculateMoves(playerPieces, enemyPieces, unoccupied, fullboard.notRight, 1);
          moves |= calculateMoves(playerPieces, enemyPieces, unoccupied, fullboard.notLeft & fullboard.boardArea, 7);
          moves |= calculateMoves(playerPieces, enemyPieces, unoccupied, fullboard.boardArea, 8);
          moves |= calculateMoves(playerPieces, enemyPieces, unoccupied, fullboard.notRight, 9);
          
          moves |= calculateMoves(playerPieces, enemyPieces, unoccupied, fullboard.notLeft & fullboard.boardArea, -1);
          moves |= calculateMoves(playerPieces, enemyPieces, unoccupied, fullboard.notRight, -7);
          moves |= calculateMoves(playerPieces, enemyPieces, unoccupied, fullboard.boardArea, -8);
          moves |= calculateMoves(playerPieces, enemyPieces, unoccupied, fullboard.notLeft & fullboard.boardArea, -9);

          //MoveStorage
          while (moves) {
            moveList.push_back(Move(i, j, PopBit(moves)));
          }
        }

        //3D&4D
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
            int iChange = i + directionOffsets[dir][0];
            int jChange = j + directionOffsets[dir][1];
            if (iChange >= 0 && iChange < fullboard.w_size && jChange >= 0 && jChange < fullboard.z_size) {
              nextBoard = fullboard.w_axis[iChange].z_axis[jChange];
              U64 candidates[9] = {
                nextBoard.board[1 - fullboard.color] & (playerPieces),
                nextBoard.board[1 - fullboard.color] & (playerPieces << 1) & fullboard.notLeft & fullboard.boardArea,
                nextBoard.board[1 - fullboard.color] & (playerPieces << 9) & fullboard.notLeft & fullboard.boardArea,
                nextBoard.board[1 - fullboard.color] & (playerPieces << 8) & fullboard.notLeft & fullboard.boardArea,
                nextBoard.board[1 - fullboard.color] & (playerPieces << 7) & fullboard.notRight,
                nextBoard.board[1 - fullboard.color] & (playerPieces >> 1) & fullboard.notRight,
                nextBoard.board[1 - fullboard.color] & (playerPieces >> 9) & fullboard.notRight,
                nextBoard.board[1 - fullboard.color] & (playerPieces >> 8) & fullboard.notRight,
                nextBoard.board[1 - fullboard.color] & (playerPieces >> 7) & fullboard.notLeft & fullboard.boardArea
              };

              int increment = 2;
              
              iChange = i + increment*directionOffsets[dir][0];
              jChange = j + increment*directionOffsets[dir][1];


              while (iChange >= 0 && iChange < fullboard.w_size && jChange >= 0 && jChange < fullboard.z_size) {

                nextBoard = fullboard.w_axis[iChange].z_axis[jChange];

                candidates[1] = (candidates[1] << 1) & fullboard.notLeft & fullboard.boardArea;
                candidates[2] = (candidates[2] << 9) & fullboard.notLeft & fullboard.boardArea;
                candidates[3] = (candidates[3] << 8) & fullboard.notLeft & fullboard.boardArea;
                candidates[4] = (candidates[4] << 7) & fullboard.notRight;
                candidates[5] = (candidates[5] >> 1) & fullboard.notRight;
                candidates[6] = (candidates[6] >> 9) & fullboard.notRight;
                candidates[7] = (candidates[7] >> 8) & fullboard.notRight;
                candidates[8] = (candidates[8] >> 7) & fullboard.notLeft & fullboard.boardArea;

                U64 moves = 0;
                for (int d = 0; d < 9;d++) {
                  moves |= ~nextBoard.board[2] & candidates[d];
                  candidates[d] = nextBoard.board[1 - fullboard.color] & candidates[d];
                }

                //std::cout<<"Move: 0x"<<std::hex<<moves<<" z:"<<iChange<<" w:"<<jChange<<std::endl;
                while (moves) {
                  moveList.push_back(Move(iChange, jChange, PopBit(moves))); //****U64 for each board which has moves, then pop out and make movelist ****
                }

                increment++;
                iChange = i + increment*directionOffsets[dir][0];
                jChange = j + increment*directionOffsets[dir][1];
              }
            }
          }
        }
      }
    }
    return moveList;
  }
};

bool gameOver(FullBoard fullboard) {
  return movegen(fullboard).empty() && movegen(fullboard).empty();
}

//int main() {
//  system("chcp 65001 > nul");
//
//  Board* board0 = new Board(0, 0);
//  Board* board1 = new Board(0x810000000ULL, 0x1008000000ULL);
//  Board* board2 = new Board(0x1008000000ULL, 0x810000000ULL);
//
//  BoardStack* stack1 = new BoardStack();
//  BoardStack* stack2 = new BoardStack();
//  BoardStack* stack3 = new BoardStack();
//  BoardStack* stack4 = new BoardStack();
//  stack1->z_axis.push_back(*board0);
//  stack1->z_axis.push_back(*board0);
//  stack1->z_axis.push_back(*board0);
//  stack1->z_axis.push_back(*board0);
//
//  stack2->z_axis.push_back(*board0);
//  stack2->z_axis.push_back(*board1);
//  stack2->z_axis.push_back(*board2);
//  stack2->z_axis.push_back(*board0);
//
//  stack3->z_axis.push_back(*board0);
//  stack3->z_axis.push_back(*board2);
//  stack3->z_axis.push_back(*board1);
//  stack3->z_axis.push_back(*board0);
//
//  stack4->z_axis.push_back(*board0);
//  stack4->z_axis.push_back(*board0);
//  stack4->z_axis.push_back(*board0);
//  stack4->z_axis.push_back(*board0);
//
//  std::vector<BoardStack> boardStack = { *stack1,*stack2,*stack3,*stack4 };
//  FullBoard* fullboard = new FullBoard(8, 4, 4, boardStack);
//  /*
//    Board* board0 = new Board(0, 0);
//    Board* board1 = new Board(0x8040000ULL, 0x4080000ULL);
//
//    BoardStack* stack1 = new BoardStack();
//    BoardStack* stack2 = new BoardStack();
//    BoardStack* stack3 = new BoardStack();
//    stack1->z_axis.push_back(*board0);
//    stack1->z_axis.push_back(*board0);
//    stack1->z_axis.push_back(*board0);
//
//    stack2->z_axis.push_back(*board0);
//    stack2->z_axis.push_back(*board1);
//    stack2->z_axis.push_back(*board0);
//
//    stack3->z_axis.push_back(*board0);
//    stack3->z_axis.push_back(*board0);
//    stack3->z_axis.push_back(*board0);
//
//    std::vector<BoardStack> boardStack={*stack1,*stack2,*stack3};
//    FullBoard* fullboard = new FullBoard(6, 3, 3, boardStack);
//  */
// /*
//  for (size_t i = 0; i < 8; i++) {
//    {
//      std::vector<Move> moveList = movegen(*fullboard, 0);
//      makeMove(fullboard, moveList[10], 0);
//      printFullBoard(*fullboard);
//    }
//    {
//      std::vector<Move> moveList = movegen(*fullboard, 1);
//      makeMove(fullboard, moveList[10], 1);
//      printFullBoard(*fullboard);
//    }
//  }
//  */
//
//  int i=0;
//  while(i<10){
//  int w;
//  int z;
//  int moveSq;
//  std::cin >> w;
//  std::cin >> z;
//  std::cin >> moveSq;
//  Move move = Move(w, z, 1ULL<<moveSq);
//
//  makeMove(fullboard, move, (i%2));
//  printFullBoard(*fullboard);
//  i++;
//  }
//  std::cout<<"works"<<std::endl;
//  return 0;
//}
