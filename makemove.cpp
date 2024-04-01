//makemove.cpp

#include "defs.h"

#include <cstdio>
#include <iostream>

//Macros to update the position key after changes to piecekeys, castlekeys, or sidekey
#define HASH_PCE(pce,sq) (pos->posKey ^= (PieceKeys[(pce)][(sq)]))
#define HASH_CA          (pos->posKey ^= (CastleKeys[(pos->castlePerm)]))
#define HASH_SIDE        (pos->posKey ^= (SideKey))
#define HASH_EP          (pos->posKey ^= (PieceKeys[EMPTY][(pos->enPas)]))

//ca_perm &= CastlePerm[from]
//1111 == 15
//ex. ca_perm &= 3 gives 0011 so black loses both castling perms
const int CastlePerm[120] = {               //A1=13, E1=12, H1=14, A8=7, E8=3, G8=11
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 13, 15, 15, 15, 12, 15, 15, 14, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15,  7, 15, 15, 15,  3, 15, 15, 11, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15
};


/*
    Name:    AddPiece
    Vars:    int sq       - The square containing the piece to be cleared.
             S_BOARD *pos - Pointer to a position.
             int pce      - The piece to be added.
    Purpose: Add a piece from the board and all relevant piece lists.
*/
static void AddPiece(const int sq, S_BOARD *pos, const int pce) {
    ASSERT(PieceValid(pce));  //Assert the piece and square is valid
    ASSERT(SqOnBoard(sq));

    int col = PieceCol[pce];

    HASH_PCE(pce, sq);        //Hash the piece into the position key

    pos->pieces[sq] = pce;
    pos->material[col] += PieceVal[pce];

    if (PieceBig[pce]) {
        pos->bigPce[col]++;                   //Increase bigPce count and eith majPce or minPce
        (PieceMaj[pce])? pos->majPce[col]++
                       : pos->minPce[col]++;
    } else {
        SETBIT(pos->pawns[col], SQ64(sq));    //Or if the piece is a pawn, set the bits for the correct colour and for both
        SETBIT(pos->pawns[BOTH], SQ64(sq));
    }

    pos->pList[pce][pos->pceNum[pce]] = sq;   //Increment piece num and add the square to pList
    pos->pceNum[pce]++; 
}


/*
    Name:    ClearPiece
    Vars:    int sq       - The square containing the piece to be cleared.
             S_BOARD *pos - Pointer to a position.
    Purpose: Clear a piece from the board and all relevant piece lists.
*/
static void ClearPiece(const int sq, S_BOARD *pos) {
    ASSERT(SqOnBoard(sq));    //Check that the square is valid

    int pce = pos->pieces[sq];
    ASSERT(PieceValid(pce));  //Check that the piece is valid

    int col = PieceCol[pce];
    int i = 0;
    int t_pceNum = -1;

    HASH_PCE(pce, sq);        //XOR the piece on that square. I.e. hashing it out of the position key

    pos->pieces[sq] = EMPTY;  //Clear the square
    pos->material[col] -= PieceVal[pce]; //Remove the piece value from the material count

    if (PieceBig[pce]) {      //Decrement the bigPce count and either the maj or min counts
        pos->bigPce[col]--;
        (PieceMaj[pce])? pos->majPce[col]-- 
                       : pos->minPce[col]--;
    } else {                  //Or if the piece is a pawn, clear it from the bitboards
        CLRBIT(pos->pawns[col], SQ64(sq));
        CLRBIT(pos->pawns[BOTH], SQ64(sq));
    }

    //Remove the piece from the piece list 
    for (i = 0; i < pos->pceNum[pce] && t_pceNum == -1; ++i)  //pceNum[pce] gives the number of type 'pce' pieces on the board
        if (pos->pList[pce][i] == sq)         //Find the index of the piece being cleared
            t_pceNum = i;                     //Store the index in t_pceNum

    ASSERT(t_pceNum != -1);   //Double check that the index was set

    pos->pceNum[pce]--;       //Decrement the number of 'pce' pieces
    pos->pList[pce][t_pceNum] = pos->pList[pce][pos->pceNum[pce]]; //Set index t_pceNum in the pList to the last square. This effectively removes the square from pList
}


/*
    Name:    MovePiece
    Vars:    int from     - The old square being moved from.
             int to       - The new square being moved to.
             S_BOARD *pos - Pointer to a position.
    Purpose: Move a piece from one square to another. 
             This is implemented seperately from AddPiece & ClearPiece for efficiency since counters won't change 
*/
static void MovePiece(const int from, const int to, S_BOARD *pos) {
    ASSERT(SqOnBoard(from));  //Check that the from and to squares are valid
    ASSERT(SqOnBoard(to));

    int i = 0;
    int pce = pos->pieces[from];  //Extract the piece to be moved
    int col = PieceCol[pce];      //Get the colour of the piece being moved

    HASH_PCE(pce, from);          //Hash out the 'from' square from the key
    pos->pieces[from] = EMPTY;    //Empty the 'from' square

    HASH_PCE(pce, to);            //Hash in the 'to' square to the key
    pos->pieces[to] = pce;        //Fill the 'to' square

    if (!PieceBig[pce]) {         //If the piece is a pawn, clear the old square and add the new one to the bitboard
        CLRBIT(pos->pawns[col],  SQ64(from));
        CLRBIT(pos->pawns[BOTH], SQ64(from));
        SETBIT(pos->pawns[col],  SQ64(to));
        SETBIT(pos->pawns[BOTH], SQ64(to));
    }

#ifdef DEBUG
    int t_PieceNum = FALSE;       //Used for debugging to check that the correct square has been found 
#endif

    //Increment through pList to find the 'from' square and set it to the 'to' square
    for (i = 0; i < pos->pceNum[pce]; ++i) {
        if (pos->pList[pce][i] == from) {
            pos->pList[pce][i] = to;

#ifdef DEBUG
            t_PieceNum = TRUE;    //Used in debugging to show the correct square has been found
#endif
            break;
        }
    }

    ASSERT(t_PieceNum == TRUE);   //Check that the 'from' square was found in the for loop
}


/*
    Name:    MakeMove
    Vars:    S_BOARD *pos - Pointer to a position.
             int move     - An integer in the form 0000 0000 0000 0000 0000 0000 0000 storing all information about the move
    Purpose: Set the board up for the move to be made by resetting enpassant squares, clearing captured pieces, saving history, rehashing the key, and finally calling MovePiece
*/
int MakeMove(S_BOARD *pos, int move) {
    ASSERT(CheckBoard(pos));

    int from = FROMSQ(move);
    int to   = TOSQ(move);
    int side = pos->side;

    ASSERT(SqOnBoard(from));      //Check that all variables are valid
    ASSERT(SqOnBoard(to));
    ASSERT(SideValid(side));
    ASSERT(PieceValid(pos->pieces[from]));

    pos->history[pos->hisPly].posKey = pos->posKey;  //Save the current key in history so the move can be undone

    if (move & MFLAGEP) {         //If the move is enpassant
        (side == WHITE)? ClearPiece(to-10, pos) : ClearPiece(to+10, pos);
    }
    else if (move & MFLAGCA) {    //If the move is castling, determine the square the rook is going to and call MovePiece
        switch (to) {
            case C1: MovePiece(A1, D1, pos); break;  //White castling long
            case C8: MovePiece(A8, D8, pos); break;  //Black castling long
            case G1: MovePiece(H1, F1, pos); break;  //White castling short
            case G8: MovePiece(H8, F8, pos); break;  //Black castling short
            default: ASSERT(FALSE); break;           //Otherwise some error has occurred
        }
    }

    if (pos->enPas != NO_SQ) HASH_EP;  //Rehash enpassant when an enpassant square exists
    HASH_CA;  //Remove castling permissions from the hash key

    //Save the state of the board in history
    pos->history[pos->hisPly].move = move;
    pos->history[pos->hisPly].fiftyMove = pos->fiftyMove;
    pos->history[pos->hisPly].enPas = pos->enPas;
    pos->history[pos->hisPly].castlePerm = pos->castlePerm;

    //Reset castling permissions
    pos->castlePerm &= CastlePerm[from];
    pos->castlePerm &= CastlePerm[to];

    //Reset enpassant permissions since capturing enpassant has to be done immediately
    pos->enPas = NO_SQ;

    HASH_CA;  //Replace new castling permissions back into the hash key

    int captured = CAPTURED(move); //Determine if the move was a capture. 0 - 15
    pos->fiftyMove++;              //Increase fifty 50 move counter

    //For a capture, clear the captured piece and reset 50 moves
    if (captured != EMPTY) {
        ASSERT(PieceValid(captured));  //Check that the piece being captured is valid
        ClearPiece(to, pos);       //Clear the piece being captured from the board
        pos->fiftyMove = 0;        //Reset 50 move counter since a capture was made
    }

    pos->hisPly++;                 //Increase the size of move history
    pos->ply++;                    //Increase the number of total moves made

    //Set a new enpassant square if applicable and rehash the key
    if (PiecePawn[pos->pieces[from]]) {
        pos->fiftyMove = 0;        //Chess rules state the 50 move rule resets when a pawn is pushed
        if (move & MFLAGPS) {
            if (side == WHITE) {
                pos->enPas = from + 10;
                ASSERT(RanksBrd[pos->enPas] == RANK_3);
            } else {
                pos->enPas = from - 10;
                ASSERT(RanksBrd[pos->enPas] == RANK_6);
            }
            HASH_EP;
        }
    }

    MovePiece(from, to, pos);      //The board has been prepped so the move can finally be made

    int prPce = PROMOTED(move);    //Get the promoted piece if there is one
    if (prPce != EMPTY) {
        ASSERT(PieceValid(prPce) && !PiecePawn[prPce]);  //A new piece from a promotion must be valid and cannot be a pawn
        ClearPiece(to, pos);       //Clear the pawn
        AddPiece(to, pos, prPce);  //Add the new piece
    }

    //Set the king square if the king was moved
    if (PieceKing[pos->pieces[to]])
        pos->KingSq[pos->side] = to;

    //Switch sides
    pos->side ^= 1;
    HASH_SIDE;

    ASSERT(CheckBoard(pos));       //Double check that the board is still ok

    //If the king is attacked by the new side to move, the last move was illegal so undo it and return false. I.e. a king in check must move
    if (SqAttacked(pos->KingSq[side], pos->side, pos)) {
        TakeMove(pos);
        return FALSE;
    }

    return TRUE;
}


/*
    Name:    TakeMove
    Vars:    S_BOARD *pos - Pointer to a position.
    Purpose: Undo a move. This may occur if the move was illegal or while reviewing a game.
*/
void TakeMove(S_BOARD *pos) {
    ASSERT(CheckBoard(pos));

    pos->hisPly--;  //Decrement move numbers
    pos->ply--;

    int move = pos->history[pos->hisPly].move;  //Reload the last move
    int from = FROMSQ(move);
    int to = TOSQ(move);

    ASSERT(SqOnBoard(from));
    ASSERT(SqOnBoard(to));

    //Remove current enpassant and castling perms from posKey so they can be reset
    if (pos->enPas != NO_SQ) HASH_EP;
    HASH_CA;

    //Reset all board state variables
    pos->castlePerm = pos->history[pos->hisPly].castlePerm;
    pos->fiftyMove = pos->history[pos->hisPly].fiftyMove;
    pos->enPas = pos->history[pos->hisPly].enPas;

    //Reset enpassant and castling perms to posKey
    if (pos->enPas != NO_SQ) HASH_EP;
    HASH_CA;

    //Switch sides
    pos->side ^= 1;
    HASH_SIDE;

    //Undo enpassant or castling
    if (MFLAGEP & move)
        (pos->side == WHITE)? AddPiece(to-10, pos, bP) : AddPiece(to+10, pos, wP);
    else if (MFLAGCA & move) {
        switch (to) {
            case C1: MovePiece(D1, A1, pos); break;  //White castling long
            case C8: MovePiece(D8, A8, pos); break;  //Black castling long
            case G1: MovePiece(F1, H1, pos); break;  //White castling short
            case G8: MovePiece(F8, H8, pos); break;  //Black castling short
            default: ASSERT(FALSE); break;           //Otherwise some error has occurred
        }
    }

    MovePiece(to, from, pos);  //Move the piece away before adding the captured piece back to the 'to' square

    if (PieceKing[pos->pieces[from]])
        pos->KingSq[pos->side] = from;

    //If there was a captured piece, add it back to the square it was captured from
    int captured = CAPTURED(move);
    if (captured != EMPTY) {
        ASSERT(PieceValid(captured));
        AddPiece(to, pos, captured);
    }

    //If there was a promotion, remove the piece and add a pawn back to the board
    if (PROMOTED(move) != EMPTY) {
        ASSERT(PieceValid(PROMOTED(move)) && !PiecePawn[PROMOTED(move)]);
        ClearPiece(from, pos);
        AddPiece(from, pos, ((PieceCol[PROMOTED(move)] == WHITE)? wP : bP));
    }

    ASSERT(CheckBoard(pos));
}