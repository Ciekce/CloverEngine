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
#include "incbin.h"
#include <fstream>
#include <iomanip>
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <random>

#if defined(__ARM_NEON)
#include <arm_neon.h>
#else
#include <immintrin.h>
#endif

#if defined(__AVX512F__)
#define reg_type    __m512i
#define reg_type_s  __m512i
#define reg_add16   _mm512_add_epi16
#define reg_sub16   _mm512_sub_epi16
#define reg_max16   _mm512_max_epi16
#define reg_add32   _mm512_add_epi32
#define reg_madd16  _mm512_madd_epi16
#define reg_madd16  _mm512_madd_epi16
#define reg_load    _mm512_load_si512
#define reg_save    _mm512_store_si512
#define ALIGN       64
#elif defined(__AVX2__) || defined(__AVX__)
#define reg_type    __m256i
#define reg_type_s  __m256i
#define reg_add16   _mm256_add_epi16
#define reg_sub16   _mm256_sub_epi16
#define reg_max16   _mm256_max_epi16
#define reg_add32   _mm256_add_epi32
#define reg_madd16  _mm256_madd_epi16
#define reg_load    _mm256_load_si256
#define reg_save    _mm256_store_si256
#define ALIGN       32
#elif defined(__SSE2__)
#define reg_type    __m128i
#define reg_type_s  __m128i
#define reg_add16   _mm_add_epi16
#define reg_sub16   _mm_sub_epi16
#define reg_max16   _mm_max_epi16
#define reg_add32   _mm_add_epi32
#define reg_madd16  _mm_madd_epi16
#define reg_load    _mm_load_si128
#define reg_save    _mm_store_si128
#define ALIGN       16
#elif defined(__ARM_NEON)
#define reg_type    int16x8_t
#define reg_type_s  int32x4_t
#define reg_add16   vaddq_s16
#define reg_sub16   vsubq_s16
#define reg_max16   vmaxq_s16
#define reg_add32   vaddq_s32
#define reg_madd16(a, b) (vpaddq_s32(vmull_s16(vget_low_s16(a), vget_low_s16(b)), vmull_high_s16(a, b)))
#define reg_load(a)    (*(a))
#define reg_save(a, b) (*(a)) = (b)
#define ALIGN       16
#endif

INCBIN(Net, EVALFILE);

const int INPUT_NEURONS = 3072;
const int SIDE_NEURONS = 768;
const int HIDDEN_NEURONS = 2 * SIDE_NEURONS;
const int REG_LENGTH = sizeof(reg_type) / sizeof(int16_t);
const int NUM_REGS = SIDE_NEURONS / REG_LENGTH;
const int BUCKET_UNROLL = 256;
const int UNROLL_LENGTH = BUCKET_UNROLL / REG_LENGTH;

const int Q_IN = 16;
const int Q_HIDDEN = 128;

enum {
    SUB = 0, ADD
};

struct NetInput {
    std::vector <short> ind[2];
};

struct NetHist {
    uint16_t move;
    uint8_t piece, cap;
    bool recalc;
    bool calc[2];
};

class Network {
public:

    Network() {
        histSz = 0;

        load();
    }

    void addInput(int ind) {
        add_ind[addSz++] = ind;
    }

    int32_t get_sum(reg_type_s& x) {
#if   defined(__AVX512F__)
        __m256i reg_256 = _mm256_add_epi32(_mm512_castsi512_si256(x), _mm512_extracti32x8_epi32(x, 1));
        __m128i a = _mm_add_epi32(_mm256_castsi256_si128(reg_256), _mm256_extractf128_si256(reg_256, 1));
#elif defined(__AVX2__) || defined(__AVX__)
        __m128i a = _mm_add_epi32(_mm256_castsi256_si128(x), _mm256_extractf128_si256(x, 1));
#elif defined(__SSE2__)
        __m128i a = x;
#endif

#if   defined(__ARM_NEON)
        return vaddvq_s32(x);
#else
        __m128i b = _mm_add_epi32(a, _mm_srli_si128(a, 8));
        __m128i c = _mm_add_epi32(b, _mm_srli_si128(b, 4));

        return _mm_cvtsi128_si32(c);
#endif
    }

    int32_t calc(NetInput& input, bool stm) {
        int32_t sum;
        //float sum_f;

        for (int n = 0; n < SIDE_NEURONS; n++) {
            sum = inputBiases[n];
            //sum_f = inputBiases_f[n];

            for (auto& prevN : input.ind[WHITE]) {
                sum += inputWeights[prevN * SIDE_NEURONS + n];
                //sum_f += inputWeights_f[prevN * SIDE_NEURONS + n];
            }

            assert(-32768 <= sum && sum <= 32767);

            histOutput[0][WHITE][n] = sum;
            //histOutput_f[0][WHITE][n] = sum_f;

            sum = inputBiases[n];
            //sum_f = inputBiases_f[n];

            for (auto& prevN : input.ind[BLACK]) {
                sum += inputWeights[prevN * SIDE_NEURONS + n];
                //sum_f += inputWeights_f[prevN * SIDE_NEURONS + n];
            }

            histOutput[0][BLACK][n] = sum;
            //histOutput_f[0][BLACK][n] = sum_f;

            assert(-32768 <= sum && sum <= 32767);
        }

        /*std::cout << "WHITE\n";
        for (int n = 0; n < 16; n++)
            std::cout << std::setw(10) << histOutput_f[0][WHITE][n];
        std::cout << std::setw(10) << histOutput_f[0][WHITE][SIDE_NEURONS - 1];
        std::cout << "\nBLACK\n";
        for (int n = 0; n < 16; n++)
            std::cout << std::setw(10) << histOutput_f[0][BLACK][n];
        std::cout << std::setw(10) << histOutput_f[0][BLACK][SIDE_NEURONS - 1];
        std::cout << "\n";*/

        histSz = 1;
        hist[0].calc[0] = hist[0].calc[1] = 1;

        return getOutput(stm);
    }

    int32_t getOutput(NetInput& input, bool stm) { /// feed forward
        int32_t sum;
        int16_t va[2][SIDE_NEURONS];

        for (int n = 0; n < SIDE_NEURONS; n++) {
            sum = inputBiases[n];

            for (auto& prevN : input.ind[WHITE]) {
                sum += inputWeights[prevN * SIDE_NEURONS + n];
            }

            assert(-32768 <= sum && sum <= 32767);

            va[WHITE][n] = sum;

            sum = inputBiases[n];

            for (auto& prevN : input.ind[BLACK]) {
                sum += inputWeights[prevN * SIDE_NEURONS + n];
            }

            va[BLACK][n] = sum;

            assert(-32768 <= sum && sum <= 32767);
        }

        sum = outputBias * Q_IN;

        reg_type zero{};
        reg_type_s acc{}, acc2{};

        reg_type* v = (reg_type*)outputWeights;
        reg_type* w = (reg_type*)va[stm];

        for (int j = 0; j < NUM_REGS / 2; j++) {
            acc = reg_add32(acc, reg_madd16(reg_max16(w[2 * j], zero), v[2 * j]));
            acc2 = reg_add32(acc2, reg_madd16(reg_max16(w[2 * j + 1], zero), v[2 * j + 1]));
        }

        reg_type* w2 = (reg_type*)va[1 ^ stm];

        for (int j = 0; j < NUM_REGS / 2; j++) {
            acc = reg_add32(acc, reg_madd16(reg_max16(w2[2 * j], zero), v[2 * j + NUM_REGS]));
            acc2 = reg_add32(acc2, reg_madd16(reg_max16(w2[2 * j + 1], zero), v[2 * j + 1 + NUM_REGS]));
        }

        acc = reg_add32(acc, acc2);

        sum += get_sum(acc);

        return sum / Q_IN / Q_HIDDEN;
    }

    void applyInitial(int c) { // refresh output
        reg_type regs[UNROLL_LENGTH];

        for (int b = 0; b < SIDE_NEURONS / BUCKET_UNROLL; b++) {
            const int offset = b * BUCKET_UNROLL;
            reg_type* reg_in = (reg_type*)&inputBiases[offset];
            for (int i = 0; i < UNROLL_LENGTH; i++)
                regs[i] = reg_load(&reg_in[i]);
            for (int idx = 0; idx < addSz; idx++) {
                reg_type* reg = (reg_type*)&inputWeights[add_ind[idx] * SIDE_NEURONS + offset];
                for (int i = 0; i < UNROLL_LENGTH; i++)
                    regs[i] = reg_add16(regs[i], reg[i]);
            }
            reg_type* reg_out = (reg_type*)&histOutput[histSz - 1][c][offset];
            for (int i = 0; i < UNROLL_LENGTH; i++)
                reg_save(&reg_out[i], regs[i]);
        }
    }

    void applySubAdd(int16_t* output, int16_t* input, int ind1, int ind2) {
        reg_type regs[UNROLL_LENGTH];
        const int16_t* inputWeights1 = reinterpret_cast<const int16_t*>(&inputWeights[ind1 * SIDE_NEURONS]);
        const int16_t* inputWeights2 = reinterpret_cast<const int16_t*>(&inputWeights[ind2 * SIDE_NEURONS]);

        for (int b = 0; b < SIDE_NEURONS / BUCKET_UNROLL; b++) {
            const int offset = b * BUCKET_UNROLL;
            const reg_type* reg_in = reinterpret_cast<const reg_type*>(&input[offset]);
            for (int i = 0; i < UNROLL_LENGTH; i++)
                regs[i] = reg_load(&reg_in[i]);

            const reg_type* reg1 = reinterpret_cast<const reg_type*>(&inputWeights1[offset]);
            for (int i = 0; i < UNROLL_LENGTH; i++)
                regs[i] = reg_sub16(regs[i], reg1[i]);
            const reg_type* reg2 = reinterpret_cast<const reg_type*>(&inputWeights2[offset]);
            for (int i = 0; i < UNROLL_LENGTH; i++)
                regs[i] = reg_add16(regs[i], reg2[i]);

            reg_type* reg_out = (reg_type*)&output[offset];
            for (int i = 0; i < UNROLL_LENGTH; i++)
                reg_save(&reg_out[i], regs[i]);
        }
    }

    void applySubAddSub(int16_t* output, int16_t* input, int ind1, int ind2, int ind3) {
        reg_type regs[UNROLL_LENGTH];
        const int16_t* inputWeights1 = reinterpret_cast<const int16_t*>(&inputWeights[ind1 * SIDE_NEURONS]);
        const int16_t* inputWeights2 = reinterpret_cast<const int16_t*>(&inputWeights[ind2 * SIDE_NEURONS]);
        const int16_t* inputWeights3 = reinterpret_cast<const int16_t*>(&inputWeights[ind3 * SIDE_NEURONS]);

        for (int b = 0; b < SIDE_NEURONS / BUCKET_UNROLL; b++) {
            const int offset = b * BUCKET_UNROLL;
            const reg_type* reg_in = reinterpret_cast<const reg_type*>(&input[offset]);
            for (int i = 0; i < UNROLL_LENGTH; i++)
                regs[i] = reg_load(&reg_in[i]);

            const reg_type* reg1 = reinterpret_cast<const reg_type*>(&inputWeights1[offset]);
            for (int i = 0; i < UNROLL_LENGTH; i++)
                regs[i] = reg_sub16(regs[i], reg1[i]);
            const reg_type* reg2 = reinterpret_cast<const reg_type*>(&inputWeights2[offset]);
            for (int i = 0; i < UNROLL_LENGTH; i++)
                regs[i] = reg_add16(regs[i], reg2[i]);
            const reg_type* reg3 = reinterpret_cast<const reg_type*>(&inputWeights3[offset]);
            for (int i = 0; i < UNROLL_LENGTH; i++)
                regs[i] = reg_sub16(regs[i], reg3[i]);

            reg_type* reg_out = (reg_type*)&output[offset];
            for (int i = 0; i < UNROLL_LENGTH; i++)
                reg_save(&reg_out[i], regs[i]);
        }
    }

    void applySubAddSubAdd(int16_t* output, int16_t* input, int ind1, int ind2, int ind3, int ind4) {
        reg_type regs[UNROLL_LENGTH];
        const int16_t* inputWeights1 = reinterpret_cast<const int16_t*>(&inputWeights[ind1 * SIDE_NEURONS]);
        const int16_t* inputWeights2 = reinterpret_cast<const int16_t*>(&inputWeights[ind2 * SIDE_NEURONS]);
        const int16_t* inputWeights3 = reinterpret_cast<const int16_t*>(&inputWeights[ind3 * SIDE_NEURONS]);
        const int16_t* inputWeights4 = reinterpret_cast<const int16_t*>(&inputWeights[ind4 * SIDE_NEURONS]);

        for (int b = 0; b < SIDE_NEURONS / BUCKET_UNROLL; b++) {
            const int offset = b * BUCKET_UNROLL;
            const reg_type* reg_in = reinterpret_cast<const reg_type*>(&input[offset]);
            for (int i = 0; i < UNROLL_LENGTH; i++)
                regs[i] = reg_load(&reg_in[i]);

            const reg_type* reg1 = reinterpret_cast<const reg_type*>(&inputWeights1[offset]);
            for (int i = 0; i < UNROLL_LENGTH; i++)
                regs[i] = reg_sub16(regs[i], reg1[i]);
            const reg_type* reg2 = reinterpret_cast<const reg_type*>(&inputWeights2[offset]);
            for (int i = 0; i < UNROLL_LENGTH; i++)
                regs[i] = reg_add16(regs[i], reg2[i]);
            const reg_type* reg3 = reinterpret_cast<const reg_type*>(&inputWeights3[offset]);
            for (int i = 0; i < UNROLL_LENGTH; i++)
                regs[i] = reg_sub16(regs[i], reg3[i]);
            const reg_type* reg4 = reinterpret_cast<const reg_type*>(&inputWeights4[offset]);
            for (int i = 0; i < UNROLL_LENGTH; i++)
                regs[i] = reg_add16(regs[i], reg4[i]);

            reg_type* reg_out = (reg_type*)&output[offset];
            for (int i = 0; i < UNROLL_LENGTH; i++)
                reg_save(&reg_out[i], regs[i]);
        }
    }

    void processMove(uint16_t move, int pieceFrom, int captured, int king, bool side, int16_t* a, int16_t* b) {
        int posFrom = sqFrom(move), posTo = sqTo(move);
        bool turn = color_of(pieceFrom);
        switch (type(move)) {
        case NEUT: {
            if (!captured)
                applySubAdd(a, b, netInd(pieceFrom, posFrom, king, side), netInd(pieceFrom, posTo, king, side));
            else
                applySubAddSub(a, b, netInd(pieceFrom, posFrom, king, side), netInd(pieceFrom, posTo, king, side), netInd(captured, posTo, king, side));
        }
                 break;
        case ENPASSANT: {
            int pos = sqDir(turn, SOUTH, posTo), pieceCap = getType(PAWN, 1 ^ turn);
            applySubAddSub(a, b, netInd(pieceFrom, posFrom, king, side), netInd(pieceFrom, posTo, king, side), netInd(pieceCap, pos, king, side));
        }
                      break;
        case CASTLE: {
            int rFrom, rTo, rPiece = getType(ROOK, turn);
            if (posTo == mirror(turn, C1)) {
                rFrom = mirror(turn, A1);
                rTo = mirror(turn, D1);
            }
            else {
                rFrom = mirror(turn, H1);
                rTo = mirror(turn, F1);
            }
            applySubAddSubAdd(a, b, netInd(pieceFrom, posFrom, king, side), netInd(pieceFrom, posTo, king, side), netInd(rPiece, rFrom, king, side), netInd(rPiece, rTo, king, side));
        }
                   break;
        default: {
            int promPiece = getType(promoted(move) + KNIGHT, turn);
            if (!captured)
                applySubAdd(a, b, netInd(pieceFrom, posFrom, king, side), netInd(promPiece, posTo, king, side));
            else
                applySubAddSub(a, b, netInd(pieceFrom, posFrom, king, side), netInd(promPiece, posTo, king, side), netInd(captured, posTo, king, side));
        }
               break;
        }
    }

    void addHistory(uint16_t move, uint8_t piece, uint8_t captured) {
        hist[histSz] = { move, piece, captured, (piece_type(piece) == KING && recalc(sqFrom(move), sqTo(move))), { 0, 0 } };
        histSz++;
    }

    void revertUpdates() {
        histSz--;
    }

    int getGoodParent(int c) {
        int i = histSz - 1;
        while (!hist[i].calc[c]) {
            if (color_of(hist[i].piece) == c && hist[i].recalc)
                return -1;
            i--;
        }
        return i;
    }

    int32_t getOutput(bool stm) {
        const reg_type zero{};
        reg_type_s acc0{}, acc1{}, acc2{}, acc3{};

        const reg_type* w = reinterpret_cast<const reg_type*>(histOutput[histSz - 1][stm]);
        const reg_type* w2 = reinterpret_cast<const reg_type*>(histOutput[histSz - 1][stm ^ 1]);
        const reg_type* v = reinterpret_cast<const reg_type*>(outputWeights);
        const reg_type* v2 = reinterpret_cast<const reg_type*>(&outputWeights[SIDE_NEURONS]);

        for (int j = 0; j < NUM_REGS; j += 4) {
            acc0 = reg_add32(acc0, reg_madd16(reg_max16(w[j], zero), v[j]));
            acc0 = reg_add32(acc0, reg_madd16(reg_max16(w2[j], zero), v2[j]));

            acc1 = reg_add32(acc1, reg_madd16(reg_max16(w[j + 1], zero), v[j + 1]));
            acc1 = reg_add32(acc1, reg_madd16(reg_max16(w2[j + 1], zero), v2[j + 1]));

            acc2 = reg_add32(acc2, reg_madd16(reg_max16(w[j + 2], zero), v[j + 2]));
            acc2 = reg_add32(acc2, reg_madd16(reg_max16(w2[j + 2], zero), v2[j + 2]));

            acc3 = reg_add32(acc3, reg_madd16(reg_max16(w[j + 3], zero), v[j + 3]));
            acc3 = reg_add32(acc3, reg_madd16(reg_max16(w2[j + 3], zero), v2[j + 3]));
        }

        acc0 = reg_add32(acc0, acc1);
        acc2 = reg_add32(acc2, acc3);
        acc0 = reg_add32(acc0, acc2);

        //std::cout << "Float output is " << sum_f << "\n";

        return (outputBias * Q_IN + get_sum(acc0)) / (Q_IN * Q_HIDDEN);
    }

    void load() {
        uint64_t* intData;
        float* floatData;

        uint64_t x;
        intData = (uint64_t*)gNetData;

        x = *intData;
        assert(x == 2361601);
        intData++;

        floatData = (float*)intData;

        int sz;

        sz = SIDE_NEURONS;

        int mn = 1e9, mx = -1e9;
        for (int i = 0; i < SIDE_NEURONS * INPUT_NEURONS; i++) {
            float val = *floatData;
            mn = std::min<int>(mn, round(val * Q_IN));
            mx = std::max<int>(mx, round(val * Q_IN));
            inputWeights[(i / SIDE_NEURONS) * SIDE_NEURONS + (i % SIDE_NEURONS)] = round(val * Q_IN);
            //inputWeights_f[(i / SIDE_NEURONS) * SIDE_NEURONS + (i % SIDE_NEURONS)] = val;
            floatData++;
        }
        //std::cout << "\n";
        //std::cout << mn << " " << mx << "\n";

        mn = 1e9, mx = -1e9;
        for (int j = 0; j < sz; j++) {
            float val = *floatData;
            mn = std::min<int>(mn, round(val * Q_IN));
            mx = std::max<int>(mx, round(val * Q_IN));
            inputBiases[j] = round(val * Q_IN);
            //inputBiases_f[j] = val;
            floatData++;
            //std::cout << val << "\n";
        }
        //std::cout << mn << " " << mx << "\n";

        mn = 1e9, mx = -1e9;
        for (int j = 0; j < HIDDEN_NEURONS; j++) {
            float val = *floatData;
            mn = std::min<int>(mn, round(val * Q_HIDDEN));
            mx = std::max<int>(mx, round(val * Q_HIDDEN));
            outputWeights[j] = round(val * Q_HIDDEN);
            //outputWeights_f[j] = val;
            floatData++;
        }
        //std::cout << mn << " " << mx << "\n";

        float val = *floatData;
        outputBias = round(val * Q_HIDDEN);
        //outputBias_f = val;

        //std::cout << outputBias_f << "\n";
    }

    int histSz;

    int16_t inputBiases[SIDE_NEURONS] __attribute__((aligned(ALIGN)));
    int32_t outputBias;
    int16_t histOutput[2005][2][SIDE_NEURONS] __attribute__((aligned(ALIGN)));
    int16_t inputWeights[INPUT_NEURONS * SIDE_NEURONS] __attribute__((aligned(ALIGN)));
    int16_t outputWeights[HIDDEN_NEURONS] __attribute__((aligned(ALIGN)));
    //float inputBiases_f[SIDE_NEURONS];
    //float inputWeights_f[INPUT_NEURONS * SIDE_NEURONS];
    //float histOutput_f[2005][2][SIDE_NEURONS];
    //float outputWeights_f[HIDDEN_NEURONS];
    //float outputBias_f;

    int addSz;
    int16_t add_ind[32];
    NetHist hist[2005];
};