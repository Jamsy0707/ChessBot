//io.cpp

#include "defs.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>

using namespace std;


/*
    Name:    ParseMove
    Vars:    char *ptrChar - A pointer to the last entered move in the form a1b2.
             S_BOARD *pos  - A pointer to the board.
    Purpose: Given a move from one square to another, find that move in the list of possible moves.
    Returns: The move as an integer to be used by MakeMove. 
             NOMOVE if the move cannot happen.
*/
int ParseMove(char *ptrChar, S_BOARD *pos) {

    //Move must be in the form a1b2 -- lowercase letters from a-h and numbers from 1-8
    if (ptrChar[0] > 'h' || ptrChar[0] < 'a' ||
        ptrChar[1] > '8' || ptrChar[1] < '1' ||
        ptrChar[2] > 'h' || ptrChar[2] < 'a' ||
        ptrChar[3] > '8' || ptrChar[3] < '1')
        return NOMOVE;

    int from = FR2SQ(ptrChar[0] - 'a', ptrChar[1] - '1');  //Convert first inputted square to file|rank form
    int to   = FR2SQ(ptrChar[2] - 'a', ptrChar[3] - '1');  //Convert second inputted square to file|rank form

    ASSERT(SqOnBoard(from) && SqOnBoard(to));

    //Store every move from the position so we can loop through and find the one with the same 'from' and 'to' square
    S_MOVELIST list[1];
    GenerateAllMoves(pos, list);

    int MoveNum = 0;
    int Move    = 0;
    int PromPce = EMPTY;

    //Loop through moves to find the one where the 'from' and 'to' squares match. Also, match the promotion piece if there was one.
    for (MoveNum = 0; MoveNum < list->count; ++MoveNum) {
        Move = list->moves[MoveNum].move;
        if (FROMSQ(Move) == from && TOSQ(Move) == to) {
            PromPce = PROMOTED(Move);
            if (PromPce != EMPTY) {
                if      (IsRQ(PromPce) && !IsBQ(PromPce) && ptrChar[4] == 'r') return Move;
                else if (IsBQ(PromPce) && !IsRQ(PromPce) && ptrChar[4] == 'b') return Move;
                else if (IsBQ(PromPce) &&  IsRQ(PromPce) && ptrChar[4] == 'q') return Move;
                else if (IsKn(PromPce) && ptrChar[4] == 'n')                   return Move;
                continue;
            }
            return Move;
        }
    }

    return NOMOVE;  //If the function reaches the end without finding a matching move, the move entered cannot be made.
}


/*
    Name:    PrintMoveList
    Vars:    S_MOVELIST *list - A pointer to a movelist struct that holds a count and an S_MOVE(move, score) array
    Purpose: Print out a list of available moves for a position.
*/
void PrintMoveList(const S_MOVELIST *list) {
    int i=0, score=0, move=0;

    cout << "Move List: " << list->count << "\n";

    for (i = 0; i < list->count; ++i) {
        move = list->moves[i].move;
        score = list->moves[i].score;

        printf("Move: %d > %s (score: %d)\n", i+1, PrMove(move), score);
    }

    cout << "Move List Total " << list->count << " Moves:\n\n";
}


/*
    Name:    PrMove
    Vars:    int move - The move to be printed
    Purpose: Given a move, print it using algebraic notation.
    Returns: A char array of the square from, square to, and promotion piece if it exists.
*/
char *PrMove(const int move) {
    static char MvStr[6];   //a7a8q Square from, square to, promotion piece

    int ff = FilesBrd[FROMSQ(move)]; //File from
    int rf = RanksBrd[FROMSQ(move)]; //Rank from
    int ft = FilesBrd[TOSQ(move)];   //File to
    int rt = RanksBrd[TOSQ(move)];   //Rank to
    int promoted = PROMOTED(move);   //4 bits representing promotion piece, 0000 if no promotion

    //If there was a promotion, determine what new piece is on the board
    if (promoted) {
        char promChar = 'q';    //Promotion piece is queen by default since that is most common
        if (IsRQ(promoted) && !IsBQ(promoted))  //If piece is a rook/queen but not a bishop/queen it must be a rook
            promChar = 'r';

        else if (!IsRQ(promoted) && IsBQ(promoted))
            promChar = 'b';

        else if (IsKn(promoted)) 
            promChar = 'n';
        
        sprintf(MvStr, "%c%c%c%c%c", ('a'+ff), ('1'+rf), ('a'+ft), ('1'+rt), promChar);
    }
    else
        sprintf(MvStr, "%c%c%c%c", ('a'+ff), ('1'+rf), ('a'+ft), ('1'+rt));

    return MvStr;
}


/*
    Name:    PrSq
    Vars:    int sq - The square to be printed.
    Purpose: Stores the file & rank of a square as a c_string for printing.
    Returns: A char array storing the name of the file & rank.
*/
char *PrSq(const int sq) {
    static char SqStr[3];   //Array is static because it is used outside this function. 'Static' prevents it from being deleted at function end.

    int file = FilesBrd[sq]; //Get the file and rank from the given square
    int rank = RanksBrd[sq];

    sprintf(SqStr, "%c%c", ('a'+file), ('1'+rank)); //Convert file & rank to a c_string and store it in SqStr

    return SqStr;
}