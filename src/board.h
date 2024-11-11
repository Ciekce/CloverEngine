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
#include "defs.h"
#include "attacks.h"
#include "cuckoo.h"

class HistoricalState {
public:
    Square enPas;
    uint8_t castleRights;
    Piece captured;
    uint16_t halfMoves, moveIndex;
    Bitboard checkers, pinnedPieces;
    uint64_t key, pawn_key, mat_key[2];
};

class Board {
public:
    bool turn, chess960;

    HistoricalState state;
    std::array<HistoricalState, STACK_SIZE> history; /// fuck it

    std::array<Piece, 64> board;
    MultiArray<Square, 2, 2> rookSq;    
    MultiArray<uint8_t, 2, 64> castleRightsDelta;

    uint16_t ply, game_ply;

    std::array<Bitboard, 12> bb;
    std::array<Bitboard, 2> pieces;

    constexpr Board() = default;

    void clear() { ply = 0; }

    void print() {
        for (int i = 7; i >= 0; i--) {
            for (Square sq = Square(i, 0); sq < Square(i + 1, 0); sq++)
                std::cerr << piece_char[board[sq]] << " ";
            std::cerr << "\n";
        }
    }

    uint64_t& key() { return state.key; }
    uint64_t& pawn_key() { return state.pawn_key; }
    uint64_t& mat_key(const bool color) { return state.mat_key[color]; }
    Bitboard& checkers() { return state.checkers; }
    Bitboard& pinned_pieces() { return state.pinnedPieces; }
    Square& enpas() { return state.enPas; }
    uint16_t& half_moves() { return state.halfMoves; }
    uint16_t& move_index() { return state.moveIndex; }
    uint8_t& castle_rights() { return state.castleRights; }
    Piece& captured() { return state.captured; }

    Bitboard get_bb_piece(const Piece piece, const bool color) const { return bb[Piece(piece, color)]; }
    Bitboard get_bb_color(const bool color) const { return pieces[color]; }
    Bitboard get_bb_piece_type(const Piece piece_type) const {  return get_bb_piece(piece_type, WHITE) | get_bb_piece(piece_type, BLACK); }

    Bitboard diagonal_sliders(const bool color) { return get_bb_piece(PieceTypes::BISHOP, color) | get_bb_piece(PieceTypes::QUEEN, color); }
    Bitboard orthogonal_sliders(const bool color) { return get_bb_piece(PieceTypes::ROOK, color) | get_bb_piece(PieceTypes::QUEEN, color); }

    Bitboard get_attackers(const bool color, const Bitboard blockers, const Square sq) {
        return (attacks::genAttacksPawn(1 ^ color, sq) & get_bb_piece(PieceTypes::PAWN, color)) |
            (attacks::genAttacksKnight(sq) & get_bb_piece(PieceTypes::KNIGHT, color)) | 
            (attacks::genAttacksBishop(blockers, sq) & diagonal_sliders(color)) |
            (attacks::genAttacksRook(blockers, sq) & orthogonal_sliders(color)) | 
            (attacks::genAttacksKing(sq) & get_bb_piece(PieceTypes::KING, color));
    }

    Bitboard get_pinned_pieces() {
        const bool enemy = turn ^ 1;
        const Square king = get_king(turn);
        Bitboard us = pieces[turn], them = pieces[enemy];
        Bitboard pinned(0ull); /// squares attacked by enemy / pinned pieces
        Bitboard mask = (attacks::genAttacksRook(them, king) & orthogonal_sliders(enemy)) | 
                        (attacks::genAttacksBishop(them, king) & diagonal_sliders(enemy));

        while (mask) {
            Bitboard b2 = us & between_mask[mask.get_square_pop()][king];
            if (b2.count() == 1) pinned ^= b2;
        }

        return pinned;
    }

    Bitboard get_pawn_attacks(const bool color) {
        const Bitboard b = get_bb_piece(PieceTypes::PAWN, color);
        const int fileA = color == WHITE ? 0 : 7, fileH = 7 - fileA;
        return shift_mask<NORTHWEST>(color, b & ~file_mask[fileA]) | shift_mask<NORTHEAST>(color, b & ~file_mask[fileH]);
    }

    Piece piece_at(const Square sq) { return board[sq]; }
    Piece piece_type_at(const Square sq) { return piece_at(sq).type(); }
    Piece get_captured_type(const Move move) { return move.get_type() == MoveTypes::ENPASSANT ? PieceTypes::PAWN : piece_type_at(move.get_to()); }

    Square get_king(const bool color) const { return get_bb_piece(PieceTypes::KING, color).get_lsb_square(); }

    bool is_capture(const Move move) { return move.get_type() != MoveTypes::CASTLE && piece_at(move.get_to()) != NO_PIECE; }
    bool is_noisy_move(const Move move) { 
        return (move.get_type() && move.get_type() != MoveTypes::CASTLE) || is_capture(move); 
    }

    bool is_attacked_by(const bool color, const Square sq) { 
        return get_attackers(color, get_bb_color(WHITE) | get_bb_color(BLACK), sq); 
    }
    
    void make_move(const Move move);
    void undo_move(const Move move);
    void make_null_move();
    void undo_null_move();

    int gen_legal_moves(MoveList &moves);
    int gen_legal_noisy_moves(MoveList &moves);
    int gen_legal_quiet_moves(MoveList &moves);

    bool has_non_pawn_material(const bool color) {
        return (get_bb_piece(PieceTypes::KING, color) ^ get_bb_piece(PieceTypes::PAWN, color)) != get_bb_color(color);
    }


    NetInput to_netinput() {
        NetInput ans;
        for (auto color : {WHITE, BLACK}) {
            for (Piece i = Pieces::BlackPawn; i <= Pieces::WhiteKing; i++) {
                Bitboard b = bb[i];
                while (b) {
                    ans.ind[color].push_back(net_index(i, b.get_lsb_square(), get_king(color), color));
                    b ^= b.lsb();
                }
            }
        }

        return ans;
    }

    void place_piece_at_sq(Piece piece, Square sq) {
        board[sq] = piece;
        key() ^= hashKey[piece][sq];
        if (piece.type() == PieceTypes::PAWN) pawn_key() ^= hashKey[piece][sq];
        else mat_key(piece.color()) ^= hashKey[piece][sq];

        pieces[piece.color()] |= (1ULL << sq);
        bb[piece] |= (1ULL << sq);
    }

    void set_fen(const std::string fen);
    void set_frc_side(bool color, int idx);
    void set_dfrc(int idx);

    std::string fen();

    uint64_t speculative_next_key(const Move move) {
        const int from = move.get_from(), to = move.get_to();
        const Piece piece = piece_at(from);
        return key() ^ hashKey[piece][from] ^ hashKey[piece][to] ^ (piece_at(to) != NO_PIECE ? hashKey[piece_at(to)][to] : 0) ^ 1;
    }

    bool is_material_draw() {
        /// KvK, KBvK, KNvK, KNNvK
        const int num = (get_bb_color(WHITE) | get_bb_color(BLACK)).count();
        return num == 2 || 
            (num == 3 && (get_bb_piece_type(PieceTypes::BISHOP) || get_bb_piece_type(PieceTypes::KNIGHT))) ||
            (num == 4 && (bb[Pieces::WhiteKnight].count() == 2 || bb[Pieces::BlackKnight].count() == 2));
    }

    bool is_repetition(const int ply) {
        int cnt = 1;
        for (int i = 2; i <= game_ply && i <= half_moves(); i += 2) {
            if (history[game_ply - i].key == key()) {
                cnt++;
                if (ply > i || cnt == 3) return true;
            }
        }
        return false;
    }

    bool is_draw(const int ply) {
        if (half_moves() < 100 || !checkers()) return is_material_draw() || is_repetition(ply) || half_moves() >= 100;
        MoveList moves;
        return gen_legal_moves(moves) > 0;
    }

    bool has_upcoming_repetition(const int ply) {
        const Bitboard all_pieces = get_bb_color(WHITE) | get_bb_color(BLACK);
        uint64_t b = ~(key() ^ history[game_ply - 1].key);
        for (int i = 3; i <= half_moves() && i <= game_ply; i += 2) {
            b ^= ~(history[game_ply - i].key ^ history[game_ply - i + 1].key);
            if (b) continue;
            const uint64_t key_delta = key() ^ history[game_ply - i].key;
            int cuckoo_ind = cuckoo::hash1(key_delta);

            if (cuckoo::cuckoo[cuckoo_ind] != key_delta) cuckoo_ind = cuckoo::hash2(key_delta);
            if (cuckoo::cuckoo[cuckoo_ind] != key_delta) continue;

            const Move move = cuckoo::cuckoo_move[cuckoo_ind];
            const Square from = move.get_from(), to = move.get_to();
            if ((between_mask[from][to] ^ Bitboard(to)) & all_pieces) continue;
            if (ply > i) return true;
            const Piece piece = piece_at(from) != NO_PIECE ? piece_at(from) : piece_at(to);
            return piece != NO_PIECE && piece.color() == turn; 
        }
        return false;
    }
};