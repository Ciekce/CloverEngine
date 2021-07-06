#pragma once

int psqt[13][2][64];

int mat[2][7] = {
    {0, 66, 303, 332, 486, 1094, 0},
    {0, 119, 376, 415, 718, 1265, 0},
};

int bonusTable[7][2][64] = {
    {},
    {
        {
            42, 1, 4, 5, 1, 0, 3, 1,
            146, 75, 116, 138, 132, 75, 29, 0,
            6, 14, 38, 51, 63, 80, 43, 7,
            1, 1, 0, 16, 26, 22, 6, -4,
            -3, -7, 4, 7, 15, 19, 11, -11,
            -11, -12, -7, -3, 6, 13, 15, -10,
            -6, -1, -4, 0, 0, 21, 26, 0,
            31, 17, -4, 6, 4, 1, -2, 1,

        },
        {
            -1, -3, 2, 4, -1, 1, 0, 0,
            124, 143, 114, 82, 70, 97, 118, 130,
            76, 67, 40, 7, 3, 18, 43, 55,
            37, 28, 16, -12, -12, 11, 25, 27,
            22, 23, 5, -2, -6, 10, 19, 17,
            17, 17, 14, 14, 17, 13, 11, 11,
            25, 20, 20, 21, 33, 14, 16, 18,
            15, 4, 0, 0, 2, 1, 1, 2,

        },
    },
    {
        {
            -163, -55, -49, -69, 3, -85, -54, -108,
            -5, -3, 30, 50, 20, 86, 1, -1,
            -23, 3, 15, 29, 64, 68, 31, 7,
            2, 17, 31, 35, 29, 46, 28, 36,
            -2, 10, 21, 29, 33, 26, 36, 22,
            -11, 4, 3, 11, 18, 17, 18, 9,
            -18, -5, -8, 4, 3, 3, 6, 5,
            -28, -14, -29, -6, 0, -6, -8, -17,

        },
        {
            -15, 4, 21, 24, 7, -10, -17, -60,
            3, 19, 10, 23, 26, -16, 10, -11,
            -6, 5, 41, 34, 15, 23, -1, -12,
            5, 19, 42, 52, 57, 41, 31, 4,
            4, 16, 44, 45, 44, 47, 20, 17,
            -9, 1, 12, 33, 30, 10, 9, -3,
            4, 8, -4, 7, 10, -5, 13, 23,
            2, -15, 6, 8, 5, 0, -1, -2,

        },
    },
    {
        {
            -56, -71, -38, -99, -78, -58, -41, -83,
            -19, -5, -9, -3, -4, -21, -39, -17,
            -24, -14, -1, 3, 4, 17, 0, -9,
            -29, 9, 3, 26, 12, 13, 14, -15,
            -9, 6, 9, 13, 23, 8, 12, 12,
            6, 15, 1, 13, 12, 11, 19, 19,
            19, 10, 15, 3, 3, 12, 21, 26,
            17, 22, -6, -5, -8, -4, 13, 25,

        },
        {
            7, 41, 27, 39, 33, 17, 12, 5,
            12, 9, 24, 22, 17, 26, 8, 8,
            7, 19, 4, 11, 15, 7, 21, 4,
            9, 13, 13, 22, 24, 18, 18, 17,
            -4, 4, 19, 18, 20, 20, 10, -17,
            -2, 8, 0, 15, 14, -2, 2, 9,
            1, -16, -4, 5, 7, -7, -12, -14,
            -9, -3, 2, -2, 2, 6, 4, -13,

        },
    },
    {
        {
            29, 4, 16, -1, -1, 8, 31, 44,
            2, -10, 10, 23, 13, 19, 19, 58,
            -17, 14, 7, 22, 33, 42, 71, 35,
            -10, 1, 14, 30, 25, 20, 29, 10,
            -23, -22, -17, -5, -6, -12, 13, -2,
            -31, -19, -26, -18, -15, -15, 18, -4,
            -40, -23, -17, -14, -12, -7, 10, -18,
            -19, -17, -11, -6, -5, -7, 3, -3,

        },
        {
            38, 50, 44, 48, 46, 45, 39, 39,
            39, 47, 45, 47, 49, 31, 28, 11,
            38, 29, 40, 32, 22, 28, 4, 14,
            31, 33, 33, 30, 28, 26, 16, 23,
            17, 29, 29, 21, 20, 24, 11, 7,
            -2, 5, 4, -2, -5, -5, -16, -21,
            -12, -11, -10, -15, -15, -22, -26, -15,
            -6, -5, -5, -12, -11, -5, -15, -22,

        },
    },
    {
        {
            9, 0, 14, 31, 13, 36, 28, 10,
            5, -44, -21, -14, -25, -18, -16, 44,
            -7, -1, -14, -2, -8, 29, 16, 28,
            -5, 5, -7, -2, -3, 13, 22, 11,
            7, 7, 8, 3, 5, 14, 22, 18,
            3, 20, 14, 6, 8, 13, 23, 24,
            7, 13, 20, 16, 18, 24, 29, 45,
            14, 14, 17, 26, 28, 0, 19, 15,

        },
        {
            32, 50, 58, 56, 83, 61, 60, 52,
            35, 75, 96, 91, 135, 110, 104, 65,
            30, 41, 73, 80, 112, 121, 107, 60,
            54, 60, 72, 89, 117, 100, 116, 92,
            37, 70, 52, 81, 74, 75, 68, 72,
            27, 20, 38, 28, 32, 47, 26, 15,
            9, 3, -20, 6, 3, -24, -19, -32,
            -5, -2, -1, 6, -9, 3, 20, 24,

        },
    },
    {
        {
            -52, 88, 90, 5, 8, -59, 45, -43,
            35, 61, 49, 39, -20, 27, 4, -51,
            -72, 18, 13, -48, -28, -9, 9, -45,
            -89, -26, -75, -105, -69, -71, -84, -109,
            -52, -76, -43, -81, -76, -73, -77, -123,
            -5, -23, -44, -60, -49, -36, -10, -22,
            29, -11, -24, -70, -53, -53, 8, 25,
            28, 38, 11, -44, -9, -36, 35, 40,

        },
        {
            -209, -86, -45, -16, 9, 7, -37, -130,
            -14, 33, 51, 34, 52, 52, 63, 2,
            15, 55, 58, 62, 56, 64, 61, 10,
            16, 44, 66, 74, 67, 68, 61, 17,
            -8, 33, 49, 66, 66, 56, 45, 7,
            -30, 3, 25, 43, 37, 24, -2, -26,
            -49, -16, 2, 13, 10, 7, -26, -60,
            -88, -59, -38, -33, -65, -28, -61, -113,

        },
    },
};


void initPSQT() {
  for(int p = PAWN; p <= KING; p++) {
    for(int s = MG; s <= EG; s++) {
      for(int sq = 0; sq < 64; sq++) {
        psqt[getType(p, WHITE)][s][sq] = +mat[s][p] + bonusTable[p][s][mirror(BLACK, sq)];
        psqt[getType(p, BLACK)][s][sq] = -mat[s][p] - bonusTable[p][s][mirror(WHITE, sq)];
      }
    }
  }
}

void addPiece(int16_t score[], int p, int sq) {
  score[MG] += psqt[p][MG][sq];
  score[EG] += psqt[p][EG][sq];
}

void remPiece(int16_t score[], int p, int sq) {
  score[MG] -= psqt[p][MG][sq];
  score[EG] -= psqt[p][EG][sq];
}
