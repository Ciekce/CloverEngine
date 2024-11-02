/*
  Clover is a UCI chess playing engine authored by Luca Metehau.
  <https://github.com/lucametehau/CloverEngine>

  Clover is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Clover is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once
#include <string>
#include <chrono>
#include <ctime>
#include <ratio>
#include <random>
#include <cassert>
#include <array>
#include <chrono>
#include <map>
#include <iostream>
#include "params.h"
#include "bitboard.h"
#include "piece.h"

#if defined(__ARM_NEON)
#include <arm_neon.h>
#else
#include <immintrin.h>
#endif

inline std::mt19937_64 gen(0xBEEF);
inline std::uniform_int_distribution <uint64_t> rng;

template<typename T, std::size_t size, std::size_t... sizes>
struct MultiArray_impl {
    using type = std::array<typename MultiArray_impl<T, sizes...>::type, size>;
};

template<typename T, std::size_t size>
struct MultiArray_impl<T, size> {
    using type = std::array<T, size>;
};

template<typename T, std::size_t... sizes>
using MultiArray = typename MultiArray_impl<T, sizes...>::type;


template<typename T, std::size_t size>
void fill_multiarray(MultiArray<T, size> &array, T value) {
    array.fill(value);
}

template<typename T, std::size_t size, std::size_t... sizes>
void fill_multiarray(MultiArray<typename MultiArray_impl<T, sizes...>::type, size> &array, T value) {
    for(std::size_t i = 0; i < size; i++)
        fill_multiarray<T, sizes...>(array[i], value);
}

typedef uint16_t Move;

class Threats {
public:
    Bitboard threats_pieces[4];
    Bitboard all_threats;
    Bitboard threatened_pieces;

    Threats() { for(int i = 0; i < 4; i++) threats_pieces[i] = Bitboard(0ULL); }
};

enum {
    BLACK = 0, WHITE
};

enum {
    NO_TYPE = 0, PROMOTION, CASTLE, ENPASSANT
};

enum {
    NORTH = 8, SOUTH = -8, EAST = 1, WEST = -1, 
    NORTHEAST = 9, NORTHWEST = 7, 
    SOUTHEAST = -7, SOUTHWEST = -9
};


enum {
    NORTH_ID = 0, SOUTH_ID, WEST_ID, EAST_ID, 
    NORTHWEST_ID, NORTHEAST_ID, 
    SOUTHWEST_ID, SOUTHEAST_ID
};

enum {
    NONE = 0, UPPER, LOWER, EXACT
};

constexpr Move NULLMOVE = 0;

constexpr int HALFMOVES = 100;
constexpr int INF = 32000;
constexpr int VALUE_NONE = INF + 10;
constexpr int MATE = 31000;
constexpr int TB_WIN_SCORE = 22000;
constexpr int MAX_DEPTH = 200;

constexpr int MAX_MOVES = 256;
typedef std::array<Move, MAX_MOVES> MoveList;

inline const std::string piece_char = "pnbrqkPNBRQK.";
inline const std::string START_POS_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

inline std::map<char, Piece> cod;
inline MultiArray<uint64_t, 12, 64> hashKey;
inline MultiArray<uint64_t, 2, 2> castleKey;
inline std::array<uint64_t, 64> enPasKey;
inline std::array<uint64_t, 16> castleKeyModifier;

inline MultiArray<Bitboard, 64, 64> between_mask, line_mask;

inline MultiArray<int, 64, 64> lmr_red;

constexpr std::pair<int, int> knightDir[8] = { {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}, {1, 2}, {2, 1}, {2, -1}, {1, -2} };
constexpr std::pair<int, int> rookDir[4] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };
constexpr std::pair<int, int> bishopDir[4] = { {-1, 1}, {-1, -1}, {1, -1}, {1, 1} };
constexpr std::pair<int, int> kingDir[8] = { {-1, 0}, {0, 1}, {1, 0}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1} };
constexpr std::pair<int, int> pawnCapDirWhite[2] = { {1, -1}, {1, 1} };
constexpr std::pair<int, int> pawnCapDirBlack[2] = { {-1, -1}, {-1, 1} };

constexpr std::array<int, 64> kingIndTable = {
    0, 1, 2, 3, 3, 2, 1, 0,
    0, 1, 2, 3, 3, 2, 1, 0,
    4, 4, 5, 5, 5, 5, 4, 4,
    4, 4, 5, 5, 5, 5, 4, 4,
    6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6,
};

class MeanValue {
private:
    std::string name;
    double sum;
    int count;

public:
    MeanValue(std::string _name = "") : name(_name), sum(0.0), count(0) {}

    void upd(double value) { sum += value; count++; }

    void print_mean() { std::cout << name << " has the mean value " << sum / count << " (" << count << " calls)\n"; }
};

inline const auto t_init = std::chrono::steady_clock::now();

inline int64_t getTime() {
    auto t = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(t - t_init).count();
}

inline int16_t net_index(Piece piece, Square sq, Square kingSq, bool side) {
    return 64 * 12 * kingIndTable[kingSq ^ (56 * !side)] + 64 * (piece + side * (piece >= 6 ? -6 : +6)) + (sq ^ (56 * !side) ^ (7 * ((kingSq >> 2) & 1))); // kingSq should be ^7, if kingSq&7 >= 4
}

inline bool recalc(Square from, Square to, bool side) {
    return (from & 4) != (to & 4) || kingIndTable[from ^ (56 * !side)] != kingIndTable[to ^ (56 * !side)];
}

inline Square mirror(bool color, Square sq) { return sq ^ (56 * !color); }

template<int direction>
inline Square shift_square(bool color, Square sq) { return color == BLACK ? sq - direction : sq + direction; }

template<int8_t direction>
inline Bitboard shift_mask(int color, Bitboard bb) {
    if (color == BLACK) return direction > 0 ? bb >> direction : bb << static_cast<int8_t>(-direction);
    return direction > 0 ? bb << direction : bb >> static_cast<int8_t>(-direction);
}

inline Piece get_piece(const Piece piece_type, const bool color) { return 6 * color + piece_type; }
inline bool color_of(Piece piece) { return piece >= 6; }

inline bool inside_board(int rank, int file) {
    return rank >= 0 && file >= 0 && rank <= 7 && file <= 7;
}

inline Move get_move(Square from, Square to, Piece prom, int type) { return from | (to << 6) | (prom << 12) | (type << 14); }
inline Square sq_from(Move move) { return move & 63; }
inline Square sq_to(Move move) { return (move & 4095) >> 6; }
inline int from_to(Move move) { return move & 4095; }
inline int type(Move move) { return move >> 14; }
inline Piece promoted(Move move) { return (move & 16383) >> 12; }
inline Square special_sqto(Move move) {
    return type(move) != CASTLE ? sq_to(move) : static_cast<Square>(8 * (sq_from(move) / 8) + (sq_from(move) < sq_to(move) ? 6 : 2));
}

inline std::string move_to_string(Move move, bool chess960 = false) {
    int sq1 = sq_from(move), sq2 = !chess960 ? special_sqto(move) : sq_to(move);
    std::string ans;
    ans += char((sq1 & 7) + 'a');
    ans += char((sq1 >> 3) + '1');
    ans += char((sq2 & 7) + 'a');
    ans += char((sq2 >> 3) + '1');
    if (type(move) == PROMOTION)
        ans += piece_char[promoted(move) + Pieces::BlackKnight];
    return ans;
}

inline void init_defs() {
    cod['p'] = Pieces::BlackPawn, cod['n'] = Pieces::BlackKnight, cod['b'] = Pieces::BlackBishop;
    cod['r'] = Pieces::BlackRook, cod['q'] = Pieces::BlackQueen, cod['k'] = Pieces::BlackKing;
    cod['P'] = Pieces::WhitePawn, cod['N'] = Pieces::WhiteKnight, cod['B'] = Pieces::WhiteBishop;
    cod['R'] = Pieces::WhiteRook, cod['Q'] = Pieces::WhiteQueen, cod['K'] = Pieces::WhiteKing;

    /// zobrist keys
    for (Piece i = Pieces::BlackPawn; i <= Pieces::WhiteKing; i++) {
        for (Square j = 0; j < 64; j++)
            hashKey[i][j] = rng(gen);
    }

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++)
            castleKey[i][j] = rng(gen);
    }

    for (int i = 0; i < (1 << 4); i++) {
        for (int j = 0; j < 4; j++) {
            castleKeyModifier[i] ^= castleKey[j / 2][j % 2] * ((i >> j) & 1);
        }
    }

    for (int i = 0; i < 64; i++)
        enPasKey[i] = rng(gen);

    /// mask squares between 2 squares
    for (int file = 0; file < 8; file++) {
        for (int rank = 0; rank < 8; rank++) {
            for (int i = 0; i < 8; i++) {
                int r = rank, f = file;
                Bitboard mask;
                while (true) {
                    r += kingDir[i].first, f += kingDir[i].second;
                    if (!inside_board(r, f))
                        break;
                    between_mask[Square(rank, file)][Square(r, f)] = mask;
                    int x = r, y = f, d = (i < 4 ? (i + 2) % 4 : 11 - i);
                    Bitboard mask2;
                    while (inside_board(x, y)) {
                        mask2 |= Bitboard(Square(x, y));
                        x += kingDir[i].first, y += kingDir[i].second;
                    }
                    x = rank, y = file;
                    while (inside_board(x, y)) {
                        mask2 |= Bitboard(Square(x, y));
                        x += kingDir[d].first, y += kingDir[d].second;
                    }
                    line_mask[Square(rank, file)][Square(r, f)] = mask | mask2;

                    mask |= Bitboard(Square(r, f));
                }
            }
        }
    }

#ifndef TUNE_FLAG
    for (int i = 1; i < 64; i++) { /// depth
        for (int j = 1; j < 64; j++) { /// moves played 
            lmr_red[i][j] = LMRQuietBias + log(i) * log(j) / LMRQuietDiv;
        }
    }
#endif
}
