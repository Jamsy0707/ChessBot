//hashkeys.cpp

#include "defs.h"

#include <cstdlib>
#include <iostream>

/*
    Name:    GeneratePosKey
    Vars:    S_BOARD *pos - A pointer to the position in which a key should be generated.
    Purpose: Create a unique key for a given position.
    Returns: A 64 bit key.
*/
U64 GeneratePosKey(const S_BOARD *pos) {
    int sq = 0;
    U64 finalKey = 0;   //Final key to be returned
    int piece = EMPTY;

    //Generate a unique key based on the pieces on the board
    for (sq = 0; sq < BRD_SQ_NUM; ++sq) {
        piece = pos->pieces[sq];                //Extract the piece from a square on the given board
        if (piece != NO_SQ && piece != EMPTY && piece != OFFBOARD) { //If the square is on the playing board and has a piece on it
            ASSERT(piece >= wP && piece <= bK); //Assert that the piece is valid (its value is between a white pawn and black king)
            finalKey ^= PieceKeys[piece][sq];   //XOR the final key with the new key for the piece and update the final key
        }
    }

    if (pos->side == WHITE)  //Update the final key if the side to play is white. Black remains upchanged.
        finalKey ^= SideKey;

    if (pos->enPas != NO_SQ) { //If the square is an en passant square, XOR it to the key.
        ASSERT(pos->enPas>=0 && pos->enPas<BRD_SQ_NUM);
        finalKey ^= PieceKeys[EMPTY][pos->enPas];
    }

    ASSERT(pos->castlePerm>=0 && pos->castlePerm<=15); //Check that the castle permission value is valid.

    finalKey ^= CastleKeys[pos->castlePerm]; //XOR the castle permission value to the key

    return finalKey;
}