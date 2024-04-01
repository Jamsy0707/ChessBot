//board.cpp

#include "defs.h"

#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;


/*
    Name:    CheckBoard
    Vars:    S_BOARD *pos - Pointer to a position.
    Purpose: Check the board/position's validity and the accuracy of variables using many asserts.
             This is important for preventing compounding mistakes and unnecessary calculations on incorrect boards.
    Returns: True if all asserts pass.
*/
int CheckBoard(const S_BOARD *pos) {
    //The following t_ variables hold info mirroring the real board.
    int t_pceNum[13] = {0};
    int t_bigPce[2] = {0};
    int t_majPce[2] = {0};
    int t_minPce[2] = {0};
    int t_material[2] = {0};

    int sq64, t_piece, t_pce_num, sq120, colour, pcount;

    U64 t_pawns[3] = {0ULL};

    t_pawns[WHITE] = pos->pawns[WHITE];
    t_pawns[BLACK] = pos->pawns[BLACK];
    t_pawns[BOTH] = pos->pawns[BOTH];

    //Check piece lists
    for (t_piece = wP; t_piece <=bK; ++t_piece)
        for (t_pce_num = 0; t_pce_num < pos->pceNum[t_piece]; ++t_pce_num) {
            sq120 = pos->pList[t_piece][t_pce_num];
            ASSERT(pos->pieces[sq120] == t_piece);
        }

    //Check counters including piece counts 
    for (sq64 = 0; sq64 < 64; ++sq64) {
        t_piece = pos->pieces[SQ120(sq64)];
        ++t_pceNum[t_piece];
        colour = PieceCol[t_piece];
        if (PieceBig[t_piece])      t_bigPce[colour]++;
        if (PieceMaj[t_piece])      t_majPce[colour]++;
        else if (PieceMin[t_piece]) t_minPce[colour]++;

        t_material[colour] += PieceVal[t_piece];
    }

    for (t_piece = wP; t_piece <= bK; ++t_piece)
        ASSERT(t_pceNum[t_piece] == pos->pceNum[t_piece]);
    
    //Check pawn bitboard count
    pcount = CNT(t_pawns[WHITE]);
    ASSERT(pcount == pos->pceNum[wP]);
    pcount = CNT(t_pawns[BLACK]);
    ASSERT(pcount == pos->pceNum[bP]);
    pcount = CNT(t_pawns[BOTH]);
    ASSERT(pcount == pos->pceNum[wP] + pos->pceNum[bP]);

    //Check pawn bitboard squares
    while (t_pawns[WHITE]) {
        sq64 = POP(&t_pawns[WHITE]);
        ASSERT(pos->pieces[SQ120(sq64)] == wP);
    }

    while (t_pawns[BLACK]) {
        sq64 = POP(&t_pawns[BLACK]);
        ASSERT(pos->pieces[SQ120(sq64)] == bP);
    }

    while (t_pawns[BOTH]) {
        sq64 = POP(&t_pawns[BOTH]);
        ASSERT(pos->pieces[SQ120(sq64)] == wP || pos->pieces[SQ120(sq64)] == bP);
    }

    //
    ASSERT(t_material[WHITE] == pos->material[WHITE] && t_material[BLACK] == pos->material[BLACK]); //Material counts are the same
    ASSERT(t_minPce[WHITE] == pos->minPce[WHITE] && t_minPce[BLACK] == pos->minPce[BLACK]); //Min piece counts are the same
    ASSERT(t_majPce[WHITE] == pos->majPce[WHITE] && t_majPce[BLACK] == pos->majPce[BLACK]); //Major pieces are the same
    ASSERT(t_bigPce[WHITE] == pos->bigPce[WHITE] && t_bigPce[BLACK] == pos->bigPce[BLACK]); //Big pieces are the same
    ASSERT(pos->side == WHITE || pos->side == BLACK); //The side is valid
    ASSERT(GeneratePosKey(pos) == pos->posKey); //The position key is still the same after generating a new one
    ASSERT(pos->enPas == NO_SQ  //The en passant square is either no square or on a valid square (rank 6 for white's turn OR rank 3 for black's)
       || (RanksBrd[pos->enPas] == RANK_6 && pos->side == WHITE)
       || (RanksBrd[pos->enPas] == RANK_3 && pos->side == BLACK));
    ASSERT(pos->pieces[pos->KingSq[WHITE]] == wK); //The white king is on the expected square
    ASSERT(pos->pieces[pos->KingSq[BLACK]] == bK); //The black king is on the expected square

    return true;
}

/*
    Name:    ParseFen
    Vars:    char *fen    - Pointer to the start of the FEN.
             S_BOARD *pos - Pointer to a position.
    Purpose: Update the board based on the position defined by the FEN. Generate a position key.
    Returns: 0 if everything worked correctly, -1 otherwise
*/
int ParseFen(char *fen, S_BOARD *pos) {
    ASSERT(fen != NULL);
    ASSERT(pos != NULL);

    //Reset all board values
    int rank = RANK_8;
    int file = FILE_A;
    int piece = 0;
    int count = 0;
    int i = 0;
    int sq64 = 0;
    int sq120 = 0;

    ResetBoard(pos);

    //Parse the FEN to extract board info
    while ((rank >= RANK_1) && *fen) {
        count = 1;
        switch (*fen) {
            case 'p': piece = bP; break;    //If char is a letter, set piece.
            case 'r': piece = bR; break;
            case 'n': piece = bN; break;
            case 'b': piece = bB; break;
            case 'k': piece = bK; break;
            case 'q': piece = bQ; break;
            case 'P': piece = wP; break;
            case 'R': piece = wR; break;
            case 'N': piece = wN; break;
            case 'B': piece = wB; break;
            case 'K': piece = wK; break;
            case 'Q': piece = wQ; break;

            case '1':                       //If char is a number, it must represent blank squares
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
                piece = EMPTY;
                count = *fen - '0';         //Ex. if FEN shows 8, there are 8 blank squares in the row
                break;

            case '/':                       //If char is '/' or blank, move to the next rank(row) and continue to the next character
            case ' ':
                rank--;
                file = FILE_A;
                fen++;
                continue;

            default:
                cout << "FEN error\n";
                return -1;
        }

        //Place the piece given by the current position in the FEN on the board and update the file(column) number
        for (i = 0; i < count; ++i, ++file) {
            sq64 = rank * 8 + file;
            sq120 = SQ120(sq64);
            if (piece != EMPTY) 
                pos->pieces[sq120] = piece;
        }
        fen++; //Move to side(next player's turn)
    }

    ASSERT(*fen == 'w' || *fen == 'b');
    pos->side = (*fen == 'w')? WHITE : BLACK; //Who's turn is it?
    fen += 2; //Move to castling permissions

    //Check and update castling permissions
    for (i = 0; i < 4; ++i) {   //At most, castling perms will be 4 characters, break early if less
        if (*fen == ' ')
            break;
        
        switch (*fen) {     //OR the relevant bits from castling permissions
            case 'K': pos->castlePerm |= WKCA; break;
            case 'Q': pos->castlePerm |= WQCA; break;
            case 'k': pos->castlePerm |= BKCA; break;
            case 'q': pos->castlePerm |= BQCA; break;
            default: break;
        }
        fen++;
    }
    fen++; //Move to en passant square

    //If the char is not a dash, there's an en passant square(Letter+Number representing square)
    if (*fen != '-') {
        file = fen[0] - 'a';    
        rank = fen[1] - '1';

        ASSERT(file >= FILE_A && file <= FILE_H); //Letter should be A to H
        ASSERT(rank >= RANK_1 && rank <= RANK_8); //Number should be 1 to 8

        pos->enPas = FR2SQ(file,rank);
    }

    pos->posKey = GeneratePosKey(pos);

    UpdateListsMaterial(pos);

    return 0;
}

/*
    Name:    PrintBoard
    Vars:    S_BOARD *pos - Pointer to a position.
    Purpose: Prints the current position on the board as well as the board statuses.
*/
void PrintBoard(const S_BOARD *pos) {
    int sq, file, rank, piece;

    cout << "\nGame Board: \n\n";

    for (rank = RANK_8; rank >= RANK_1; --rank) {
        cout << rank+1 << " ";  //Print row number
        for (file = FILE_A; file <= FILE_H; ++file) {   //Print row of squares
            sq = FR2SQ(file,rank);
            piece = pos->pieces[sq];
            cout << PceChar[piece] << "  ";
        }
        cout << "\n";
    }

    //Print column names at the bottom of the board
    cout << "  ";
    for (file = FILE_A; file <= FILE_H; ++file)
        cout << char('A'+file) << "  ";

    //Print side character
    cout << "\n" << "side: " << SideChar[pos->side] << "\n";
    
    //Print enPas character
    string eP = (pos->enPas == 99)? "None" : to_string(pos->enPas);
    cout << "enPas: " << eP << "\n";

    //Print castling permissions
    string cPerm = "";
    cPerm += (pos->castlePerm & WKCA)? 'K' : '-';
    cPerm += (pos->castlePerm & WQCA)? 'Q' : '-';
    cPerm += (pos->castlePerm & BKCA)? 'k' : '-';
    cPerm += (pos->castlePerm & BQCA)? 'q' : '-';
    cout << "castle: " << cPerm << "\n";

    //Print the position key
    cout << "PosKey: " << hex << uppercase << pos->posKey << "\n\n";
    cout << dec << nouppercase;
}

/*
    Name:    ResetBoard
    Vars:    S_BOARD *pos - Pointer to the position to be cleared.
    Purpose: Clears the board of pieces and sets all values within the board to 0.
*/
void ResetBoard(S_BOARD *pos) {
    int i = 0;

    for (int i = 0; i < BRD_SQ_NUM; ++i)    //Set all squares to OFFBOARD
        pos->pieces[i] = OFFBOARD;
    
    for (i = 0; i < 64; ++i)
        pos->pieces[SQ120(i)] = EMPTY;      //Set the 64 playable board squares to EMPTY

    for (i = 0; i < 2; ++i) {
        pos->bigPce[i] = 0;
        pos->majPce[i] = 0;
        pos->minPce[i] = 0;
        pos->material[i] = 0;
    }

    for (i = 0; i < 3; ++i)
        pos->pawns[i] = 0ULL;

    for (i = 0; i < 13; ++i)
        pos->pceNum[i] = 0;

    pos->KingSq[WHITE] = pos->KingSq[BLACK] = NO_SQ;

    pos->side       = BOTH;
    pos->enPas      = NO_SQ;
    pos->castlePerm = 0;
    pos->fiftyMove  = 0;
    pos->ply        = 0;
    pos->hisPly     = 0;
    pos->posKey     = 0ULL;

    //pos->PvTable->pTable = NULL;
    //InitPvTable(pos->PvTable);
}


/*
    Name:    UpdateListsMaterial
    Vars:    S_BOARD *pos - Pointer to a position.
    Purpose: Loops through every piece on the board and 
*/
void UpdateListsMaterial(S_BOARD *pos) {
    int piece, sq, index, colour;

    for (index = 0; index < BRD_SQ_NUM; ++index) {
        sq = index;
        piece = pos->pieces[sq];

        if (piece != OFFBOARD && piece != EMPTY) {      //Update the counter for pieces on the board
            colour = PieceCol[piece];   //Colour will be 0, 1, or 2 for White, Black, or Both

            if (PieceBig[piece]) pos->bigPce[colour]++;
            if (PieceMin[piece]) pos->minPce[colour]++;
            if (PieceMaj[piece]) pos->majPce[colour]++;

            pos->material[colour] += PieceVal[piece];   //Update the material value for the side''

            //Update the piece list
            //pList[13][10] - 13 piece types and max of 10 each
            //Ex. pList[wP][0] = a1
            pos->pList[piece][pos->pceNum[piece]] = sq;
            pos->pceNum[piece]++;

            //Update the KingSq array so the current position of the kings is known
            if (piece == wK || piece == bK) pos->KingSq[colour] = sq;

            //Set the bit on the bitboard if the piece is a pawn
            if (piece == wP || piece == bP) {
                SETBIT(pos->pawns[colour], SQ64(sq));
                SETBIT(pos->pawns[BOTH], SQ64(sq));
            }
        }
    }
}