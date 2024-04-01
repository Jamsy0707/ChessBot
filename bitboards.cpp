//bitboards.cpp

#include "defs.h"

#include <cstdlib>
#include <iostream>

using namespace std;

//A bitboard with values. The value order was given by Tord Romstad
const int BitTable[PLAY_SQ_NUM] = {
    63, 30, 3,  32, 25, 41, 22, 33, 
    15, 50, 42, 13, 11, 53, 19, 34, 
    61, 29, 2,  51, 21, 43, 45, 10, 
    18, 47, 1,  54, 9,  57, 0,  35, 
    62, 31, 40, 4,  49, 5,  52, 26, 
    60, 6,  23, 44, 46, 27, 56, 16, 
    7,  39, 48, 24, 59, 14, 12, 55, 
    38, 28, 58, 20, 37, 17, 36, 8
};

/*
    Name:    PopBit
    Vars:    U64 *bb - Pointer to a bitboard.
    Purpose: Takes the first 1 bit (least significant) and converts it to 0.
    Returns: The index of the changed bit.
*/
int PopBit(U64 *bb) {
    U64 b = *bb ^ (*bb - 1);
    unsigned int fold = (unsigned) ((b & 0xffffffff) ^ (b >> 32));
    *bb &= (*bb - 1);
    return BitTable[(fold * 0x783a9b23) >> 26];
}

/*
    Name:    CountBits
    Vars:    U64 b - A bitboard.
    Purpose: Counts the total number of bits that are 1 in the bitboard.
    Returns: The total count.
*/
int CountBits(U64 b) {
    int r;
    for (r = 0; b; r++, b &= b - 1);
    return r;
} 

/*
    Name:    PrintBitBoard
    Vars:    U64 bb - A bitboard.
    Purpose: Function to print the working pawn bitboard for visualizing the game.
*/
void PrintBitBoard(U64 bb) {
    U64 shiftMe = 1ULL; //Unsigned long long

    int rank = 0;
    int file = 0;
    int sq   = 0;
    int sq64 = 0;

    cout << "\n";
    for (rank = RANK_8; rank >= RANK_1; --rank) {
        for (file = FILE_A; file <= FILE_H; ++file) {
            sq = FR2SQ(file,rank); //120 based square
            sq64 = SQ64(sq);       //64 based square

            if ((shiftMe << sq64) & bb) //if shifting 1ULL and a piece exists on that square
                cout << "X ";
            else
                cout << "- ";
        }
        cout << "\n";
    }
    cout << "\n\n";
}