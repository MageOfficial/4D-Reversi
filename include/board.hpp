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
#define _ForceInline __forceinline
#define Bitcount(X) __popcnt64(X)
#elif defined(__clang__)
#define _ASSUME(cond) ((cond) ? static_cast<void>(0) : __builtin_unreachable())
#define _Compiletime __attribute__((always_inline)) static constexpr
#define _NoInline __attribute__((noinline))
#define _ForceInline __attribute__((always_inline))
#define Bitcount(X) static_cast<uint64_t>(__builtin_popcountll(X))
#elif defined(__GNUC__)
#define _ASSUME(cond) ((cond) ? static_cast<void>(0) : __builtin_unreachable())
#define _Compiletime __attribute__((always_inline)) static constexpr
#define _NoInline __attribute__((noinline))
#define _ForceInline __attribute__((always_inline)) inline
#define Bitcount(X) static_cast<uint64_t>(__builtin_popcountll(X))
#else
#define _ASSUME(cond) static_cast<void>(!!(cond))
#define _Compiletime static constexpr
#endif

#define SquareOf(X) _tzcnt_u64(X)
#define Bitloop(X) for (; X; X = _blsr_u64(X))

typedef uint64_t U64;

using std::string;
using std::vector;

inline U64 PopBit(U64& val) {
    U64 lsb = _blsi_u64(val);
    val ^= lsb;
    return lsb;
}

inline U64 shiftBits(U64 bits, int shift) {
    return (shift > 0) ? bits << shift : bits >> -shift;
}

template <typename F, std::size_t... Is>
constexpr void static_for_impl(F&& f, std::index_sequence<Is...>) {
    (f(std::integral_constant<std::size_t, Is>{}), ...);
}

template <std::size_t N, typename F>
constexpr void static_for(F&& f) {
    static_for_impl(std::forward<F>(f), std::make_index_sequence<N>{});
}

constexpr U64 boardBits[8] = {0x1ULL, 0x303ULL, 0x70707ULL, 0xf0f0f0fULL, 0x1f1f1f1f1fULL, 0x3f3f3f3f3f3fULL, 0x7f7f7f7f7f7f7fULL, 0xffffffffffffffffULL};

enum Color {
    Black = 0,
    White = 1,
    Occupied = 2,
    Empty = 3
};

struct Board {
  U64 board[3];
  Board(U64 w, U64 b) {
    board[Black] = b;
    board[White] = w;
    board[Occupied] = w | b;
  }
};

struct Move {
  bool pass = false;
  int w = 0;
  int z = 0;
  int value = 0;
  U64 move = 0;

  Move(int w, int z, U64 move) : w(w), z(z), move(move) {}
  Move(int w, int z, U64 move, int value) : w(w), z(z), move(move), value(value) {}
  Move(bool pass, int value) : pass(pass), value(value) {}

  bool operator<(const Move& other) const {
    return value > other.value;
  }
};


class Game
{
public:
    Game(int bs, int ws, int zs, vector<vector<Board>>& grid);

    void updateScore();
    void makeMove(const Move &move);
    vector<Move> movegen();
    bool gameOver();

    friend std::ostream &operator<<(std::ostream &os, const Game &game);
    
    // Constants
    const int board_size;
    const int w_size;
    const int z_size;

    // Game state
    int score[4] = {};
    bool color = Black;
    vector<vector<Board>> board_grid; // [w][z]

    // Bitboard Masks
    const U64 boardArea;
    const U64 notLeft;
    const U64 notRight;

private:
    //Internal helper functions
    //Recursively makes moves in 3D and 4D
    template<int dir> void makeMoveRecursive(const int w, const int z, U64 candidates[9], bool bools[9]) {
        static constexpr int dW[] = {1, 1, 0, -1, -1, -1, 0, 1};
        static constexpr int dZ[] = {0, 1, 1, 1, 0, -1, -1, -1};
        
        int newW = w + dW[dir], newZ = z + dZ[dir];
        if (newW < 0 || newW >= w_size || newZ < 0 || newZ >= z_size) return;
        
        Board* curBoard = &(board_grid[newW][newZ]);
        
        U64 tempCandidates[9] = {
            candidates[0],
            (candidates[1] << 1) & notLeft & boardArea,
            (candidates[2] << 9) & notLeft & boardArea,
            (candidates[3] << 8) & notLeft & boardArea,
            (candidates[4] << 7) & notRight,
            (candidates[5] >> 1) & notRight,
            (candidates[6] >> 9) & notRight,
            (candidates[7] >> 8) & notRight,
            (candidates[8] >> 7) & notLeft & boardArea
        };

        U64 currentCandidates[9];
        for (int i = 0; i < 9; i++) 
            currentCandidates[i] = curBoard->board[!color] & tempCandidates[i];

        bool newBoolArray[9];
        for (int i = 0; i < 9;i++) {
            bools[i] = (bools[i] || ((curBoard->board[color] & tempCandidates[i]) != 0));
            newBoolArray[i] = bools[i];
        }

        makeMoveRecursive<dir>(newW, newZ, currentCandidates, bools);

        for (int i = 0; i < 9; i++) {
            if ((!newBoolArray[i]) && (bools[i])) {
            curBoard->board[color] ^= currentCandidates[i];
            curBoard->board[!color] ^= currentCandidates[i];
            }
        }
        return;
    }

    U64 calculateMoves(U64 playerPieces, U64 enemyPieces, U64 unoccupied, U64 gameBounds, int shift);
    void processDirection(U64 &toFlip, U64 move, int shift, U64 boundary, const Board *curBoard, bool color);
    std::string toString() const;
};
