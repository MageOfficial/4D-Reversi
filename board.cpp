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

  int score[4];

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
    score[0] = 0;
    score[1] = 0;
    score[2] = 0;
    score[3] = 0;
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
  fullboard->score[0] = 0;
  fullboard->score[1] = 0;
  fullboard->score[2] = 0;
  fullboard->score[3] = 0;
  for (int i = 0; i < fullboard->w_size; i++) {
    for (int j = 0; j < fullboard->z_size; j++) {
      Board curBoard = fullboard->w_axis[i].z_axis[j];
      fullboard->score[0] += Bitcount(curBoard.board[0]);
      fullboard->score[1] += Bitcount(curBoard.board[1]);
      fullboard->score[2] += Bitcount(curBoard.board[2]);
      fullboard->score[3] += Bitcount(~curBoard.board[2]);
    }
  }
}

template<int dir> void makeMoveRecursive(FullBoard* fullboard, int color, const int w, const int z, U64 candidates[9], bool bools[9]) {
  int newW, newZ;
  if constexpr (dir == 0) { newW = w + 1; newZ = z; }
  if constexpr (dir == 1) { newW = w + 1; newZ = z + 1; }
  if constexpr (dir == 2) { newW = w; newZ = z + 1; }
  if constexpr (dir == 3) { newW = w - 1; newZ = z + 1; }
  if constexpr (dir == 4) { newW = w - 1; newZ = z; }
  if constexpr (dir == 5) { newW = w - 1; newZ = z - 1; }
  if constexpr (dir == 6) { newW = w; newZ = z - 1; }
  if constexpr (dir == 7) { newW = w + 1; newZ = z - 1; }

  if (newW < 0 || newW >= fullboard->w_size || newZ < 0 || newZ >= fullboard->z_size) {
    return;
  }
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

  U64 currentCandidates[9] = {
    curBoard->board[1 - color] & tempCandidates[0],
    curBoard->board[1 - color] & tempCandidates[1],
    curBoard->board[1 - color] & tempCandidates[2],
    curBoard->board[1 - color] & tempCandidates[3],
    curBoard->board[1 - color] & tempCandidates[4],
    curBoard->board[1 - color] & tempCandidates[5],
    curBoard->board[1 - color] & tempCandidates[6],
    curBoard->board[1 - color] & tempCandidates[7],
    curBoard->board[1 - color] & tempCandidates[8]
  };

  bool newBoolArray[9];
  for (int i = 0; i < 9;i++) {
    bools[i] = (bools[i] || ((curBoard->board[color] & tempCandidates[i]) != 0));
    newBoolArray[i] = bools[i];
  }



  makeMoveRecursive<dir>(fullboard, color, newW, newZ, currentCandidates, bools);

  for (int i = 0; i < 9; i++) {
    if ((!newBoolArray[i]) && (bools[i])) {
      curBoard->board[color] ^= currentCandidates[i];
      curBoard->board[1 - color] ^= currentCandidates[i];
    }
  }
  return;
}

void makeMove(FullBoard* fullboard, Move move, int color) {
  Board* curBoard = &(fullboard->w_axis[move.w].z_axis[move.z]);
  curBoard->board[color] |= move.move;
  curBoard->board[2] |= move.move;

  {//2D
    U64 toFlip = 0;
    {
      U64 candidates = curBoard->board[1 - color] & (move.move >> 1) & fullboard->notRight;
      U64 possibleFlips = candidates;
      while (candidates != 0) {
        candidates = (candidates >> 1) & fullboard->notRight;
        if ((curBoard->board[color] & candidates) != 0) {
          toFlip |= possibleFlips;
          break;
        }
        possibleFlips |= curBoard->board[1 - color] & candidates;
        candidates = curBoard->board[1 - color] & candidates;
      }
    }
    {
      U64 candidates = curBoard->board[1 - color] & (move.move >> 7) & fullboard->notLeft & fullboard->boardArea;
      U64 possibleFlips = candidates;
      while (candidates != 0) {
        candidates = (candidates >> 7) & fullboard->notLeft & fullboard->boardArea;
        if ((curBoard->board[color] & candidates) != 0) {
          toFlip |= possibleFlips;
          break;
        }
        possibleFlips |= curBoard->board[1 - color] & candidates;
        candidates = curBoard->board[1 - color] & candidates;
      }
    }
    {
      U64 candidates = curBoard->board[1 - color] & (move.move >> 8) & fullboard->boardArea;
      U64 possibleFlips = candidates;
      while (candidates != 0) {
        candidates = (candidates >> 8) & fullboard->boardArea;
        if ((curBoard->board[color] & candidates) != 0) {
          toFlip |= possibleFlips;
          break;
        }
        possibleFlips |= curBoard->board[1 - color] & candidates;
        candidates = curBoard->board[1 - color] & candidates;
      }
    }
    {
      U64 candidates = curBoard->board[1 - color] & (move.move >> 9) & fullboard->notRight;
      U64 possibleFlips = candidates;
      while (candidates != 0) {
        candidates = (candidates >> 9) & fullboard->notRight;
        if ((curBoard->board[color] & candidates) != 0) {
          toFlip |= possibleFlips;
          break;
        }
        possibleFlips |= curBoard->board[1 - color] & candidates;
        candidates = curBoard->board[1 - color] & candidates;
      }
    }
    {
      U64 candidates = curBoard->board[1 - color] & (move.move << 1) & fullboard->notLeft & fullboard->boardArea;
      U64 possibleFlips = candidates;
      while (candidates != 0) {
        candidates = (candidates << 1) & fullboard->notLeft & fullboard->boardArea;
        if ((curBoard->board[color] & candidates) != 0) {
          toFlip |= possibleFlips;
          break;
        }
        possibleFlips |= curBoard->board[1 - color] & candidates;
        candidates = curBoard->board[1 - color] & candidates;
      }
    }
    {
      U64 candidates = curBoard->board[1 - color] & (move.move << 7) & fullboard->notRight;
      U64 possibleFlips = candidates;
      while (candidates != 0) {
        candidates = (candidates << 7) & fullboard->notRight;
        if ((curBoard->board[color] & candidates) != 0) {
          toFlip |= possibleFlips;
          break;
        }
        possibleFlips |= curBoard->board[1 - color] & candidates;
        candidates = curBoard->board[1 - color] & candidates;
      }
    }
    {
      U64 candidates = curBoard->board[1 - color] & (move.move << 8) & fullboard->boardArea;
      U64 possibleFlips = candidates;
      while (candidates != 0) {
        candidates = (candidates << 8) & fullboard->boardArea;
        if ((curBoard->board[color] & candidates) != 0) {
          toFlip |= possibleFlips;
          break;
        }
        possibleFlips |= curBoard->board[1 - color] & candidates;
        candidates = curBoard->board[1 - color] & candidates;
      }
    }
    {
      U64 candidates = curBoard->board[1 - color] & (move.move << 9) & fullboard->notLeft & fullboard->boardArea;
      U64 possibleFlips = candidates;
      while (candidates != 0) {
        candidates = (candidates << 9) & fullboard->notLeft & fullboard->boardArea;
        if ((curBoard->board[color] & candidates) != 0) {
          toFlip |= possibleFlips;
          break;
        }
        possibleFlips |= curBoard->board[1 - color] & candidates;
        candidates = curBoard->board[1 - color] & candidates;
      }
    }
    curBoard->board[color] ^= toFlip;
    curBoard->board[1 - color] ^= toFlip;
  }

  {//3D&4D
    U64 candidatesArray[9] = {
      move.move,
      move.move,
      move.move,
      move.move,
      move.move,
      move.move,
      move.move,
      move.move,
      move.move
    };
    bool boolArray0[9] = { false,false,false,false,false,false,false,false,false };
    bool boolArray1[9] = { false,false,false,false,false,false,false,false,false };
    bool boolArray2[9] = { false,false,false,false,false,false,false,false,false };
    bool boolArray3[9] = { false,false,false,false,false,false,false,false,false };
    bool boolArray4[9] = { false,false,false,false,false,false,false,false,false };
    bool boolArray5[9] = { false,false,false,false,false,false,false,false,false };
    bool boolArray6[9] = { false,false,false,false,false,false,false,false,false };
    bool boolArray7[9] = { false,false,false,false,false,false,false,false,false };

    makeMoveRecursive<0>(fullboard, color, move.w, move.z, candidatesArray, boolArray0);
    makeMoveRecursive<1>(fullboard, color, move.w, move.z, candidatesArray, boolArray1);
    makeMoveRecursive<2>(fullboard, color, move.w, move.z, candidatesArray, boolArray2);
    makeMoveRecursive<3>(fullboard, color, move.w, move.z, candidatesArray, boolArray3);
    makeMoveRecursive<4>(fullboard, color, move.w, move.z, candidatesArray, boolArray4);
    makeMoveRecursive<5>(fullboard, color, move.w, move.z, candidatesArray, boolArray5);
    makeMoveRecursive<6>(fullboard, color, move.w, move.z, candidatesArray, boolArray6);
    makeMoveRecursive<7>(fullboard, color, move.w, move.z, candidatesArray, boolArray7);
  }

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

std::vector<Move> movegen(FullBoard fullboard, int color) {
  //Spatial Moves
  std::vector<Move> moveList;
  {
    for (int i = 0; i < fullboard.w_size; i++) {
      for (int j = 0; j < fullboard.z_size; j++) {
        Board curBoard = fullboard.w_axis[i].z_axis[j];
        U64 playerPieces = curBoard.board[color];
        U64 enemyPieces = curBoard.board[1 - color];
        U64 unoccupied = ~curBoard.board[2];
        //2D
        {
          U64 moves = 0;
          //right shift
          {
            U64 candidates = enemyPieces & (playerPieces >> 1) & fullboard.notRight;
            while (candidates != 0) {
              candidates = (candidates >> 1) & fullboard.notRight;
              moves |= unoccupied & candidates;
              candidates = enemyPieces & candidates;
            }
          }
          {
            U64 candidates = enemyPieces & (playerPieces >> 7) & fullboard.notLeft & fullboard.boardArea;
            while (candidates != 0) {
              candidates = (candidates >> 7) & fullboard.notLeft & fullboard.boardArea;
              moves |= unoccupied & candidates;
              candidates = enemyPieces & candidates;
            }
          }
          {
            U64 candidates = enemyPieces & (playerPieces >> 8) & fullboard.boardArea;
            while (candidates != 0) {
              candidates = (candidates >> 8) & fullboard.boardArea;
              moves |= unoccupied & candidates;
              candidates = enemyPieces & candidates;
            }
          }
          {
            U64 candidates = enemyPieces & (playerPieces >> 9) & fullboard.notRight;
            while (candidates != 0) {
              candidates = (candidates >> 9) & fullboard.notRight;
              moves |= unoccupied & candidates;
              candidates = enemyPieces & candidates;
            }
          }
          //left shift
          {
            U64 candidates = enemyPieces & (playerPieces << 1) & fullboard.notLeft & fullboard.boardArea;
            while (candidates != 0) {
              candidates = (candidates << 1) & fullboard.notLeft & fullboard.boardArea;
              moves |= unoccupied & candidates;
              candidates = enemyPieces & candidates;
            }
          }
          {
            U64 candidates = enemyPieces & (playerPieces << 7) & fullboard.notRight;
            while (candidates != 0) {
              candidates = (candidates << 7) & fullboard.notRight;
              moves |= unoccupied & candidates;
              candidates = enemyPieces & candidates;
            }
          }
          {
            U64 candidates = enemyPieces & (playerPieces << 8) & fullboard.boardArea;
            while (candidates != 0) {
              candidates = (candidates << 8) & fullboard.boardArea;
              moves |= unoccupied & candidates;
              candidates = enemyPieces & candidates;
            }
          }
          {
            U64 candidates = enemyPieces & (playerPieces << 9) & fullboard.notLeft & fullboard.boardArea;
            while (candidates != 0) {
              candidates = (candidates << 9) & fullboard.notLeft & fullboard.boardArea;
              moves |= unoccupied & candidates;
              candidates = enemyPieces & candidates;
            }
          }

          //MoveStorage
          while (moves) {
            moveList.push_back(Move(i, j, PopBit(moves)));
          }
        }

        //3D&4D
        {
          Board nextBoard = Board(0, 0);
          int iChange, jChange;
          for (int dir = 0;dir < 8;dir++) {
            if (dir == 0) {
              iChange = i + 1;
              jChange = j;
            }
            else if (dir == 1) {
              iChange = i + 1;
              jChange = j + 1;
            }
            else if (dir == 2) {
              iChange = i;
              jChange = j + 1;
            }
            else if (dir == 3) {
              iChange = i - 1;
              jChange = j + 1;
            }
            else if (dir == 4) {
              iChange = i - 1;
              jChange = j;
            }
            else if (dir == 5) {
              iChange = i - 1;
              jChange = j - 1;
            }
            else if (dir == 6) {
              iChange = i;
              jChange = j - 1;
            }
            else if (dir == 7) {
              iChange = i + 1;
              jChange = j - 1;
            }


            if (iChange >= 0 && iChange < fullboard.w_size && jChange >= 0 && jChange < fullboard.z_size) {
              nextBoard = fullboard.w_axis[iChange].z_axis[jChange];
              U64 candidates[9] = {
              nextBoard.board[1 - color] & (playerPieces),
              nextBoard.board[1 - color] & (playerPieces << 1) & fullboard.notLeft & fullboard.boardArea,
              nextBoard.board[1 - color] & (playerPieces << 9) & fullboard.notLeft & fullboard.boardArea,
              nextBoard.board[1 - color] & (playerPieces << 8) & fullboard.notLeft & fullboard.boardArea,
              nextBoard.board[1 - color] & (playerPieces << 7) & fullboard.notRight,
              nextBoard.board[1 - color] & (playerPieces >> 1) & fullboard.notRight,
              nextBoard.board[1 - color] & (playerPieces >> 9) & fullboard.notRight,
              nextBoard.board[1 - color] & (playerPieces >> 8) & fullboard.notRight,
              nextBoard.board[1 - color] & (playerPieces >> 7) & fullboard.notLeft & fullboard.boardArea
              };

              int increment = 2;
              if (dir == 0) {
                iChange = i + increment;
                jChange = j;
              }
              else if (dir == 1) {
                iChange = i + increment;
                jChange = j + increment;
              }
              else if (dir == 2) {
                iChange = i;
                jChange = j + increment;
              }
              else if (dir == 3) {
                iChange = i - increment;
                jChange = j + increment;
              }
              else if (dir == 4) {
                iChange = i - increment;
                jChange = j;
              }
              else if (dir == 5) {
                iChange = i - increment;
                jChange = j - increment;
              }
              else if (dir == 6) {
                iChange = i;
                jChange = j - increment;
              }
              else if (dir == 7) {
                iChange = i + increment;
                jChange = j - increment;
              }

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
                  candidates[d] = nextBoard.board[1 - color] & candidates[d];
                }

                //std::cout<<"Move: 0x"<<std::hex<<moves<<" z:"<<iChange<<" w:"<<jChange<<std::endl;
                while (moves) {
                  moveList.push_back(Move(iChange, jChange, PopBit(moves))); //****U64 for each board which has moves, then pop out and make movelist ****
                }

                increment++;
                if (dir == 0) {
                  iChange = i + increment;
                  jChange = j;
                }
                else if (dir == 1) {
                  iChange = i + increment;
                  jChange = j + increment;
                }
                else if (dir == 2) {
                  iChange = i;
                  jChange = j + increment;
                }
                else if (dir == 3) {
                  iChange = i - increment;
                  jChange = j + increment;
                }
                else if (dir == 4) {
                  iChange = i - increment;
                  jChange = j;
                }
                else if (dir == 5) {
                  iChange = i - increment;
                  jChange = j - increment;
                }
                else if (dir == 6) {
                  iChange = i;
                  jChange = j - increment;
                }
                else if (dir == 7) {
                  iChange = i + increment;
                  jChange = j - increment;
                }
              }
            }
          }
        }
      }
    }
    return moveList;
  }
};

bool gameOver(FullBoard fullboard, int color) {
  return movegen(fullboard, color).empty() && movegen(fullboard, 1 - color).empty();
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
