//validate.cpp

#include "defs.h"

//Check that a file/rank number is valid
int FileRankValid(const int fr) {
    return (fr >= 0 && fr <= 7) ? 1 : 0;
}

//Check that an integer is a valid piece
int PieceValid(const int pce) {
    return (pce >= wP && pce <= bK) ? 1 : 0;
}

//Check that an integer is a valid piece or empty
int PieceValidEmpty(const int pce) {
    return (pce >= EMPTY && pce <= bK) ? 1 : 0;
}

//Check that the side is either White or Black
int SideValid(const int side) {
    return (side == WHITE || side == BLACK) ? 1 : 0;
}

//Check that the square is on the board.
int SqOnBoard(const int sq) {
    return FilesBrd[sq]==OFFBOARD ? 0 : 1;
}