//movegen.cpp

/*
    1. Make move
    2. Get from, to, cap
    3. Store current pos in history array
    4. move the current piece from -> to
    5. if a capture was made, remove captured piece from piece list
    6. Update 50 move rule
    7. Handle promotions
    8. Enpas captures
    9. Set enpas squarel
    10. Update all positoin counters and lists for pieces added/removed/moved
    11. Maintain poskey
    12. Castle permissions
    13. Change sides, increment ply and hisPly
*/

#include "defs.h"

#include <cstdlib>
#include <cstdio>
#include <iostream>

#define MOVE(f,t,ca,pro,fl) ((f) | ((t) << 7) | ((ca) << 14) | ((pro) << 20) | (fl))  //Gives a move integer given the from_square, to_square, capture_piece, promotion_piece, and flag
#define SQOFFBOARD(sq) (FilesBrd[(sq)] == OFFBOARD)  //Gives bool true if the square is off the board

static void AddCaptureMove (const S_BOARD *pos, int move, S_MOVELIST *list);  //These declarations exist so I can keep the functions in alphabetical order
static void AddQuietMove (const S_BOARD *pos, int move, S_MOVELIST *list);

//Array used for generating moves for non-sliding pieces. The array will loop through until a 0 is found which allows for white and black pieces to be stored together.
const int LoopNonSlidePce[6] = {wN, wK, 0, bN, bK, 0};
const int LoopNonSlideIndex[2] = {0, 3};  //Indexes for white and black starts.

//Array used for generating moves for sliding pieces.
const int LoopSlidePce[8] = {wB, wR, wQ, 0, bB, bR, bQ, 0};
const int LoopSlideIndex[2] = {0, 4};

//Array used for determining the number of meaningful directions in each row of PceDir
const int NumDir[13] = {0, 0, 8, 4, 4, 8, 8, 0, 8, 4, 4, 8, 8};

//Array used for looping through piece directions. Pawns are calculated separately and are blank in this array.
const int PceDir[13][8] = {
    {0, 0, 0, 0, 0, 0, 0},              //Empty
    {0, 0, 0, 0, 0, 0, 0},              //Pawn
    {-8, -19, -21, -12, 8, 19, 21, 12}, //Knight
    {-9, -11, 11, 9, 0, 0, 0, 0},       //Bishop
    {-1, -10, 1, 10, 0, 0, 0, 0},       //Rook
    {-1, -10, 1, 10, -9, -11, 11, 9},   //Queen
    {-1, -10, 1, 10, -9, -11, 11, 9},   //King
    {0, 0, 0, 0, 0, 0, 0},
    {-8, -19, -21, -12, 8, 19, 21, 12},
    {-9, -11, 11, 9, 0, 0, 0, 0},
    {-1, -10, 1, 10, 0, 0, 0, 0},
    {-1, -10, 1, 10, -9, -11, 11, 9},
    {-1, -10, 1, 10, -9, -11, 11, 9}
};


/*
    Name:    AddBlackPawnCapMove
    Vars:    S_BOARD *pos - Pointer to a position.
             int from     - The square is pawn is first on.
             int to       - The square the pawn is moving to.
             int cap      - The piece being captured.
             S_MOVELIST   - The list of all possible moves to be expanded by this function.
    Purpose: Generate the possible next positions for when a black pawn captures a piece.
*/
static void AddBlackPawnCapMove(const S_BOARD *pos, const int from, const int to, const int cap, S_MOVELIST *list) {
    //Check that the from, to, and cap variables passed in are valid.
    ASSERT(PieceValidEmpty(cap));
    ASSERT(SqOnBoard(from));
    ASSERT(SqOnBoard(to));
    
    if (RanksBrd[from] == RANK_2) {  //If the pawn was on a rank 2 square, it will promote by capturing.
        AddCaptureMove(pos, MOVE(from, to, cap, bQ, 0), list);
        AddCaptureMove(pos, MOVE(from, to, cap, bR, 0), list);
        AddCaptureMove(pos, MOVE(from, to, cap, bB, 0), list);
        AddCaptureMove(pos, MOVE(from, to, cap, bN, 0), list);
    } 
    else  //Else it's not on rank 2 and will not promote.
        AddCaptureMove(pos, MOVE(from, to, cap, EMPTY, 0), list);
} 

/*
    Name:    AddBlackPawnMove
    Vars:    S_BOARD *pos - Pointer to a position.
             int from     - The square is pawn is first on.
             int to       - The square the pawn is moving to.
             S_MOVELIST   - The list of all possible moves to be expanded by this function.
    Purpose: Generate the possible next positions for when a black pawn moves without capturing.
*/
static void AddBlackPawnMove(const S_BOARD *pos, const int from, const int to, S_MOVELIST *list) {
    //Check that the from, to, and cap variables passed in are valid.
    ASSERT(SqOnBoard(from));
    ASSERT(SqOnBoard(to));
    
    if (RanksBrd[from] == RANK_2) {  //If the pawn was on a rank 7 square, it will promote by capturing.
        AddQuietMove(pos, MOVE(from, to, EMPTY, bQ, 0), list);
        AddQuietMove(pos, MOVE(from, to, EMPTY, bR, 0), list);
        AddQuietMove(pos, MOVE(from, to, EMPTY, bB, 0), list);
        AddQuietMove(pos, MOVE(from, to, EMPTY, bN, 0), list);
    } 
    else  //Else it's not on rank 2 and will not promote.
        AddQuietMove(pos, MOVE(from, to, EMPTY, EMPTY, 0), list);
} 

/*
    Name:    AddCaptureMove
    Vars:    S_BOARD *pos     - Pointer to a position.
             int move         - The move to be added to the list of possible next moves.
             S_MOVELIST *list - Pointer to the move list to add moves to.
    Purpose: For a given position, add a capture move to the list of possible next moves.
*/
static void AddCaptureMove (const S_BOARD *pos, int move, S_MOVELIST *list) {
    list->moves[list->count].move = move;  //Store the move
    list->moves[list->count].score = 0;    //Store the score for the move. This is used for finding the best move.
    list->count++;                         //Increment the number of moves in the list.
}

/*
    Name:    AddEnPassantMove
    Vars:    S_BOARD *pos     - Pointer to a position.
             int move         - The move to be added to the list of possible next moves.
             S_MOVELIST *list - Pointer to the move list to add moves to.
    Purpose: For a given position, add an en passant move to the list of possible next moves.
*/
static void AddEnPassantMove (const S_BOARD *pos, int move, S_MOVELIST *list) {
    list->moves[list->count].move = move;  //Store the move
    list->moves[list->count].score = 0;    //Store the score for the move. This is used for finding the best move.
    list->count++;                         //Increment the number of moves in the list.
}

/*
    Name:    AddQuietMove
    Vars:    S_BOARD *pos     - Pointer to a position.
             int move         - The move to be added to the list of possible next moves.
             S_MOVELIST *list - Pointer to the move list to add moves to.
    Purpose: For a given position, add a quiet move (not a capture or en passant) to the list of possible next moves.
*/
static void AddQuietMove (const S_BOARD *pos, int move, S_MOVELIST *list) {
    ASSERT(SqOnBoard(FROMSQ(move)));
	ASSERT(SqOnBoard(TOSQ(move)));
	ASSERT(CheckBoard(pos));
    
    list->moves[list->count].move = move;  //Store the move
    list->moves[list->count].score = 0;    //Store the score for the move. This is used for finding the best move.
    list->count++;                         //Increment the number of moves in the list.
}


/*
    Name:    AddWhitePawnCapMove
    Vars:    S_BOARD *pos - Pointer to a position.
             int from     - The square is pawn is first on.
             int to       - The square the pawn is moving to.
             int cap      - The piece being captured.
             S_MOVELIST   - The list of all possible moves to be expanded by this function.
    Purpose: Generate the possible next positions for when a white pawn captures a piece.
*/
static void AddWhitePawnCapMove(const S_BOARD *pos, const int from, const int to, const int cap, S_MOVELIST *list) {
    //Check that the from, to, and cap variables passed in are valid.
    ASSERT(PieceValidEmpty(cap));
    ASSERT(SqOnBoard(from));
    ASSERT(SqOnBoard(to));
    ASSERT(CheckBoard(pos));
    
    
    if (RanksBrd[from] == RANK_7) {  //If the pawn was on a rank 7 square, it will promote by capturing.
        AddCaptureMove(pos, MOVE(from, to, cap, wQ, 0), list);
        AddCaptureMove(pos, MOVE(from, to, cap, wR, 0), list);
        AddCaptureMove(pos, MOVE(from, to, cap, wB, 0), list);
        AddCaptureMove(pos, MOVE(from, to, cap, wN, 0), list);
    } 
    else  //Else it's not on rank 7 and will not promote.
        AddCaptureMove(pos, MOVE(from, to, cap, EMPTY, 0), list);
} 


/*
    Name:    AddWhitePawnMove
    Vars:    S_BOARD *pos - Pointer to a position.
             int from     - The square is pawn is first on.
             int to       - The square the pawn is moving to.
             S_MOVELIST   - The list of all possible moves to be expanded by this function.
    Purpose: Generate the possible next positions for when a white pawn moves without capturing.
*/
static void AddWhitePawnMove(const S_BOARD *pos, const int from, const int to, S_MOVELIST *list) {
    //Check that the from, to, and cap variables passed in are valid.
    ASSERT(SqOnBoard(from));
    ASSERT(SqOnBoard(to));
    
    if (RanksBrd[from] == RANK_7) {  //If the pawn was on a rank 7 square, it will promote by capturing.
        AddQuietMove(pos, MOVE(from, to, EMPTY, wQ, 0), list);
        AddQuietMove(pos, MOVE(from, to, EMPTY, wR, 0), list);
        AddQuietMove(pos, MOVE(from, to, EMPTY, wB, 0), list);
        AddQuietMove(pos, MOVE(from, to, EMPTY, wN, 0), list);
    } 
    else  //Else it's not on rank 7 and will not promote.
        AddQuietMove(pos, MOVE(from, to, EMPTY, EMPTY, 0), list);
} 


/*
    Name:    GenerateAllMoves
    Vars:    S_BOARD *pos     - Pointer to a position.
             S_MOVELIST *list - Pointer to the move list to add moves to.
    Purpose: Generate all possible moves from a given position.
*/
void GenerateAllMoves (const S_BOARD *pos, S_MOVELIST *list) {
    ASSERT(CheckBoard(pos));  //Assert that the position is valid

    list->count = 0;
    int dir=0, index=0, pce=EMPTY, pceIndex=0, pceNum=0, side=pos->side, sq=0, t_sq=0;


    if (side == WHITE) {
        for (pceNum = 0; pceNum < pos->pceNum[wP]; ++pceNum) {                    //Loop through every white pawn on the board
            sq = pos->pList[wP][pceNum];

            ASSERT(SqOnBoard(sq));                                                //Assert that the square is on the board

            if (pos->pieces[sq+10] == EMPTY) {                                    //If the pawn can move staight ahead
                AddWhitePawnMove(pos, sq, sq+10, list);                           //Add the move to the list

                if (RanksBrd[sq] == RANK_2 && pos->pieces[sq+20] == EMPTY)        //If the pawn is on the start square, it may move 2 squares
                    AddQuietMove(pos, MOVE(sq, (sq+20), EMPTY, EMPTY, MFLAGPS), list);  //Add the move with the pawn start flag
            }

            if (!SQOFFBOARD(sq+9) && PieceCol[pos->pieces[sq+9]] == BLACK)        //Add moves for when a white pawn can capture a black piece without promotion
                AddWhitePawnCapMove(pos, sq, sq+9, pos->pieces[sq+9], list);
            if (!SQOFFBOARD(sq+11) && PieceCol[pos->pieces[sq+11]] == BLACK)
                AddWhitePawnCapMove(pos, sq, sq+11, pos->pieces[sq+11], list);

            if (sq+9 == pos->enPas)                                               //Add en passant moves with en passant flag
                AddCaptureMove(pos, MOVE(sq, sq+9, EMPTY, EMPTY, MFLAGEP), list);
            else if (sq+11 == pos->enPas)
                AddCaptureMove(pos, MOVE(sq, sq+11, EMPTY, EMPTY, MFLAGEP), list);
        }

        //Generate White side castling moves
        if (pos->castlePerm & WKCA &&                                   //If White King castle perms are set,
            pos->pieces[F1] == EMPTY && pos->pieces[G1] == EMPTY &&     //and there's a clear path from the king to the rook,
            !SqAttacked(E1, BLACK, pos) && !SqAttacked(F1, BLACK, pos)) //and both the king and f1 are not under attack. G1 is already checked in MakeMove.
            AddQuietMove(pos, MOVE(E1, G1, EMPTY, EMPTY, MFLAGCA), list);
        
        if (pos->castlePerm & WQCA &&                                   //If White Queen castle perms are set
            pos->pieces[D1] == EMPTY && pos->pieces[C1] == EMPTY && pos->pieces[B1] == EMPTY &&
            !SqAttacked(E1, BLACK, pos) && !SqAttacked(D1, BLACK, pos)) 
            AddQuietMove(pos, MOVE(E1, C1, EMPTY, EMPTY, MFLAGCA), list);

    } else {  //Black pawns
        for (pceNum = 0; pceNum < pos->pceNum[bP]; ++pceNum) {  //Loop through every black pawn on the board
            sq = pos->pList[bP][pceNum];

            ASSERT(SqOnBoard(sq));  //Assert that the square is on the board

            if (pos->pieces[sq-10] == EMPTY) {           //If the pawn can move staight ahead
                AddBlackPawnMove(pos, sq, sq-10, list);  //Add the move to the list

                if (RanksBrd[sq] == RANK_7 && pos->pieces[sq-20] == EMPTY)              //If the pawn is on the start square, it may move 2 squares
                    AddQuietMove(pos, MOVE(sq, (sq-20), EMPTY, EMPTY, MFLAGPS), list);  //Add the move with the pawn start flag
            }

            if (!SQOFFBOARD(sq-9) && PieceCol[pos->pieces[sq-9]] == WHITE)  //Add moves for when a black pawn can capture a white piece without promotion
                AddBlackPawnCapMove(pos, sq, sq-9, pos->pieces[sq-9], list);
            if (!SQOFFBOARD(sq-11) && PieceCol[pos->pieces[sq-11]] == WHITE)
                AddBlackPawnCapMove(pos, sq, sq-11, pos->pieces[sq-11], list);

            if (sq-9 == pos->enPas)  //Add en passant moves with en passant flag
                AddCaptureMove(pos, MOVE(sq, sq-9, EMPTY, EMPTY, MFLAGEP), list);
            else if (sq-11 == pos->enPas)
                AddCaptureMove(pos, MOVE(sq, sq-11, EMPTY, EMPTY, MFLAGEP), list);
        }

        //Generate Black side castling moves
        if (pos->castlePerm & BKCA &&                                   //If White King castle perms are set
            pos->pieces[F8] == EMPTY && pos->pieces[G8] == EMPTY &&     //and there's a clear path from the king to the rook
            !SqAttacked(E8, WHITE, pos) && !SqAttacked(F8, WHITE, pos)) //and both the king and f1 are not under attack
            AddQuietMove(pos, MOVE(E8, G8, EMPTY, EMPTY, MFLAGCA), list);
        
        if (pos->castlePerm & BQCA &&                                   //If White Queen castle perms are set
            pos->pieces[D8] == EMPTY && pos->pieces[C8] == EMPTY && pos->pieces[B8] == EMPTY &&
            !SqAttacked(E8, WHITE, pos) && !SqAttacked(D8, WHITE, pos)) 
            AddQuietMove(pos, MOVE(E8, C8, EMPTY, EMPTY, MFLAGCA), list);
    }


    //sliding pieces
    pceIndex = LoopSlideIndex[side];  //Will be 0 for white and 4 for black initially
    pce = LoopSlidePce[pceIndex++];

    while (pce != 0) {  //Loop through all sliding pieces of the given colour. 0 in the piece array indicates the end of that colour's pieces.
        ASSERT(PieceValid(pce));

        //Loop through pieces of the same type and print the square they're on
        for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
            sq = pos->pList[pce][pceNum];
            ASSERT(SqOnBoard(sq));

            //Loop through all directions that the piece can go
            for (index = 0; index < NumDir[pce]; ++index) {
                dir = PceDir[pce][index];
                t_sq = sq + dir;

                //Continue in the same direction since the piece can slide across the board
                while (!SQOFFBOARD(t_sq)) {
                    if (pos->pieces[t_sq] != EMPTY) {                   //If there exists a piece already on the square
                        if (PieceCol[pos->pieces[t_sq]] == side ^ 1)    //Determine if the piece is of the same colour or opposite. Ex. BLACK ^ 1 == WHITE
                            AddCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);  //If it's opposite coloured, moving to that square would result in a capture
                        break;                                          //The square is occupied so the direction has been fully explored
                    }

                    AddQuietMove(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), list);  //If the new square was onboard and empty, moving to it is a normal move
                    t_sq += dir;                                        //Continue in the same direction
                }
            }
        }
        pce = LoopSlidePce[pceIndex++];
    }

    //non-sliding pieces
    pceIndex = LoopNonSlideIndex[side];  //Will be 0 for white and 3 for black initially
    pce = LoopNonSlidePce[pceIndex++];

    //Loop through all non-sliding pieces of the given colour. 0 in the piece array indicates the end of that colour's pieces.
    //For each piece type (ex. rooks), loop and print the squares they are on
    //Then, for each piece and position, loop through the possible squares they may go to next.
    //Determine if moving to that new square results in a capture or not
    while (pce != 0) {
        ASSERT(PieceValid(pce));

        //Loop through pieces of the same type and print the square they're on
        for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
            sq = pos->pList[pce][pceNum];
            ASSERT(SqOnBoard(sq));

            //Loop through all directions that the piece can go
            for (index = 0; index < NumDir[pce]; ++index) {
                dir = PceDir[pce][index];
                t_sq = sq + dir;

                if (SQOFFBOARD(t_sq))  //If the new square is offboard, continue to the next possible square
                    continue;

                if (pos->pieces[t_sq] != EMPTY) {                   //If there exists a piece already on the square
                    if (PieceCol[pos->pieces[t_sq]] == side ^ 1)    //Determine if the piece is of the same colour or opposite. Ex. BLACK ^ 1 == WHITE
                        AddCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list) ;  //If it's opposite coloured, moving to that square would result in a capture
                    continue;                                       //Continue in the while loop to the next square
                }

                AddQuietMove(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), list);  //If the new square was onboard and empty, moving to it is a normal move
            }
        }
        pce = LoopNonSlidePce[pceIndex++];
    }
}