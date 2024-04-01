//attack.cpp

#include "defs.h"

#include <iostream>


//Values define possible moves or move directions and are relative to the 120 int bit board
const int KnDir[8] = {-8, -19, -21, -12, 8, 19, 21, 12};
const int RkDir[4] = {-1, -10, 1, 10};
const int BiDir[4] = {-9, -11, 11, 9};
const int KiDir[8] = {-1, -10, 1, 10, -9, -11, 11, 9};

/*
    Name:    SqAttacked
    Vars:    int sq       - The square being looked at. Ex. Is D2 attacked?
             int side     - The side attacking.
             S_BOARD *pos - A pointer to the current position.
    Purpose: Determine if a given square is being attacked.
    Returns: TRUE (1) if it is attacked, FALSE(0) otherwise.
*/
int SqAttacked(const int sq, const int side, const S_BOARD *pos) {
    int pce, i, t_sq, dir;

    //First, ensure that the square is on the board, the side is valid, and the position is valid
    ASSERT(SqOnBoard(sq));
    ASSERT(SideValid(side));
    ASSERT(CheckBoard(pos));

    //Check if a pawn is attacking
    //If the side attacking is white and there's a white pawn on 1 of 2 possible attacking squares, then the sq is being attacked by a wP. Same idea for black.
    if (side == WHITE) {
        if (pos->pieces[sq-11] == wP || pos->pieces[sq-9] == wP)
            return TRUE;
    } else {
        if (pos->pieces[sq+11] == bP || pos->pieces[sq+9] == bP)
            return TRUE;
    }

    //Check if a knight is attacking
    //Loop through all possible squares a knight may be attacking from. If the square holds a knight of the attacking colour, return true.
    for (i = 0; i < 8; ++i) {
        dir = KnDir[i];
        t_sq = sq + dir;
        pce = pos->pieces[t_sq];
        if(pce != OFFBOARD && pce != EMPTY && IsKn(pce) && PieceCol[pce] == side) //Check that pce is not offboard or empty first since IsKn array only has 13 elements.
            return TRUE;
    }

    //Check if a bishop/queen is attacking
    for (i = 0; i < 4; ++i) {       //Can attack from 4 diagonals
        dir = BiDir[i];
        t_sq = sq + dir;
        pce = pos->pieces[t_sq];
        while (pce != OFFBOARD) {   //Loop through until a piece is found or the end of the board
            if (pce != EMPTY) {     //Break once a piece has been found since bishops cannot jump pieces 
                if (IsBQ(pce) && PieceCol[pce] == side)  //Return true if there's a rook in the attacking square 
                    return TRUE;
                break;
            }
            t_sq += dir;            //Increment down the row/column
            pce = pos->pieces[t_sq];
        }
    }

    //Check if a rook/queen is attacking
    for (i = 0; i < 4; ++i) {   //Can attack from up, down, left, and right
        dir = RkDir[i];
        t_sq = sq + dir;
        pce = pos->pieces[t_sq];
        while (pce != OFFBOARD) {   //Loop through the columns/rows since rooks can slide across the entire board
            if (pce != EMPTY) {     //Since rooks cannot jump pieces, if there's a piece other than a rook, break
                if (IsRQ(pce) && PieceCol[pce] == side)  //Return true if there's a rook in the attacking square 
                    return TRUE;
                break;
            }
            t_sq += dir;    //Increment down the row/column
            pce = pos->pieces[t_sq];
        }
    }

    //Check if a king is attacking
    for (i = 0; i < 8; ++i) {   //A king can attack from 8 directions
        dir = KiDir[i];
        t_sq = sq + dir;
        pce = pos->pieces[t_sq];
        if (pce != OFFBOARD && pce != EMPTY && IsKi(pce) && PieceCol[pce] == side) //Check that pce is not offboard or empty first since IsKi array only has 13 elements.
            return TRUE;
    }
    
    return FALSE; //Finally, return false if no attacking pieces are found
}