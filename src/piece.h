#pragma once
#include <cstdint>

class Piece {
public:
    uint8_t piece;
    constexpr Piece(uint8_t piece = 12) : piece(piece) {}
    operator uint8_t() const { return piece; }

    Piece type() { return piece >= 6 ? piece - 6 : piece; }
    const Piece type() const { return piece >= 6 ? piece - 6 : piece; }

    Piece operator + (const Piece &other) const { return piece + other.piece; }
    Piece operator - (const Piece &other) const { return piece - other.piece; }
    Piece operator + (const int &other) const { return piece + other; }
    Piece operator - (const int &other) const { return piece - other; }

    Piece& operator += (const Piece &other) { piece += other.piece; return *this; }
    Piece& operator -= (const Piece &other) { piece -= other.piece; return *this; }
    Piece& operator ++ () { piece++; return *this; }
    Piece operator ++ (int) { Piece temp = *this; piece++; return temp; }
};

namespace PieceTypes {
constexpr Piece PAWN   = Piece(0);
constexpr Piece KNIGHT = Piece(1);
constexpr Piece BISHOP = Piece(2);
constexpr Piece ROOK   = Piece(3);
constexpr Piece QUEEN  = Piece(4);
constexpr Piece KING   = Piece(5);
};

namespace Pieces {
constexpr Piece BlackPawn   = Piece(0);
constexpr Piece BlackKnight = Piece(1);
constexpr Piece BlackBishop = Piece(2);
constexpr Piece BlackRook   = Piece(3);
constexpr Piece BlackQueen  = Piece(4);
constexpr Piece BlackKing   = Piece(5);
constexpr Piece WhitePawn   = Piece(6);
constexpr Piece WhiteKnight = Piece(7);
constexpr Piece WhiteBishop = Piece(8);
constexpr Piece WhiteRook   = Piece(9);
constexpr Piece WhiteQueen  = Piece(10);
constexpr Piece WhiteKing   = Piece(11);
};

constexpr Piece NO_PIECE    = Piece(12);