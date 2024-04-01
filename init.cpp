//init.cpp

#include "defs.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>

using namespace std;

//Generate a random 64 bit number
//rand() generates a 15 bit num so we generate multiple numbers and add them to a 64 bit number
#define RAND_64 ( (U64)rand() | \
                  (U64)rand() << 15 | \
                  (U64)rand() << 30 | \
                  (U64)rand() << 45 | \
                  ((U64)rand() & 0xf) << 60 )

int Sq120ToSq64[BRD_SQ_NUM]; //120 int bitboard
int Sq64ToSq120[64];         //64 int bitboard

U64 SetMask[64];
U64 ClearMask[64];

U64 PieceKeys[13][120];
U64 SideKey;
U64 CastleKeys[16];

int FilesBrd[BRD_SQ_NUM];
int RanksBrd[BRD_SQ_NUM];

/*
    Name:    InitFilesRanksBrd
    Purpose: Setup the filesbrd and ranksbrd arrays with value OFFBOARD
*/
void InitFilesRanksBrd() {
    int i = 0;
    int file  = FILE_A;
    int rank  = RANK_1;
    int sq    = A1;
    int sq64  = 0;

    for (i = 0; i < BRD_SQ_NUM; ++i) {
        FilesBrd[i] = OFFBOARD;
        RanksBrd[i] = OFFBOARD;
    }

    for (rank = RANK_1; rank <= RANK_8; ++rank) {
        for (file = FILE_A; file <= FILE_H; ++file) {
            sq = FR2SQ(file,rank);
            FilesBrd[sq] = file;
            RanksBrd[sq] = rank;
        }
    }
}

/*
    Name:    InitHashKeys
    Purpose: Setup the piecekeys and castlekeys arrays with random 64 bit integers
*/
void InitHashKeys() {
    int i=0, i2=0;
    for (i = 0; i < 13; ++i) 
        for (i2 = 0; i2 < 120; ++i2)
            PieceKeys[i][i2] = RAND_64;

    SideKey = RAND_64;
    for (i = 0; i < 16; ++i)
        CastleKeys[i] = RAND_64;
}

/*
    Name:    InitBitMasks
    Purpose: Setup the setmask and clearmask arrays
*/
void InitBitMasks() {
    int i = 0;
    for (i = 0; i < PLAY_SQ_NUM; ++i) {
        SetMask[i] = 0ULL;
        ClearMask[i] = 0ULL;
    }

    for (i = 0; i < PLAY_SQ_NUM; ++i) {
        SetMask[i] |= (1ULL << i);
        ClearMask[i] = ~SetMask[i];  //bitwise compliment 
    }
}

/*
    Name:    InitSq120To64
    Purpose: Convert the indexes of a 120 int board to a traditional 64 int board
*/
void InitSq120To64() {
    int i = 0;
    int file  = FILE_A;
    int rank  = RANK_1;
    int sq    = A1;
    int sq64  = 0;

    for (i = 0; i < BRD_SQ_NUM; ++i) {
        Sq120ToSq64[i] = 65;
    }

    for (i = 0; i < 64; ++i) {
        Sq64ToSq120[i] = 120;
    }

    for (rank = RANK_1; rank <= RANK_8; ++rank) {
        for (file = FILE_A; file <= FILE_H; ++file) {
            sq = FR2SQ(file,rank);
            Sq64ToSq120[sq64] = sq;
            Sq120ToSq64[sq] = sq64;
            sq64++;
        }
    }
}

/*
    Name:    AllInit
    Purpose: Initialize all functions
*/
void AllInit() {
    InitSq120To64();
    InitBitMasks();
    InitHashKeys();
    InitFilesRanksBrd();
}