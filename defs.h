//defs.h

#ifndef DEFS_H
#define DEFS_H

#define DEBUG
#ifndef DEBUG
#define ASSERT(n)
#else
#define ASSERT(n) \
if(!(n)) { \
printf("%s - Failed",#n); \
printf("On %s ",__DATE__); \
printf("At %s ",__TIME__); \
printf("In File %s ",__FILE__); \
printf("At Line %d\n",__LINE__); \
exit(1); }
#endif

#define NAME "James' ChessBot"  //Name of the chessbot
#define BRD_SQ_NUM 120          //Defines board size, including out-of-bounds squares
#define PLAY_SQ_NUM 64          //Defines playable board size
#define MAXGAMEMOVES 2048       //Used for storing previous piece positions. It's rare for a game to go over 150 moves so this should be more than enough.
#define MAXPOSITIONMOVES 256    //The max number of moves calculated for any given position. The current known max is 218 so 256 is more than enough and easy to represent in binary.

//FENs describe the position in a simple text notation that is easy to parse.
//The 8 rows are given with black as lowercase and white as upper. 
//This is followed by the next player to move, castling permissions, en passant squares, halfmove clock for the 50 move rule, and fullmove clock.
//char START_FEN[] = {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1"};

typedef unsigned long long U64; //Unsigned 64 bit integer


/*
    This section outlines the pieces, ranks, colours, and squares.
*/
enum {EMPTY, wP, wN, wB, wR, wQ,              //Defining all pieces as integers. Ex. 1 == wP
      wK, bP, bN, bB, bR, bQ, bK};
enum {FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, //Defining rows
      FILE_F, FILE_G, FILE_H, FILE_NONE};
enum {RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, 
      RANK_6, RANK_7, RANK_8, RANK_NONE};     //Defining columns
enum {WHITE, BLACK, BOTH};                    //Defining players
enum {FALSE, TRUE};                           //Explicitly declaring 0 to be FALSE and 1 to be TRUE
enum {WKCA=1, WQCA=2, BKCA=4, BQCA=8};        //Castling, K is short castle, Q is long. Ex. 1001 shows that white can castle King side and black Queen side
enum {                                        //Defines playable board tiles. Values 0-20 and 92-119 are out of bounds values
    A1 = 21, B1, C1, D1, E1, F1, G1, H1,
    A2 = 31, B2, C2, D2, E2, F2, G2, H2,
    A3 = 41, B3, C3, D3, E3, F3, G3, H3,
    A4 = 51, B4, C4, D4, E4, F4, G4, H4,
    A5 = 61, B5, C5, D5, E5, F5, G5, H5,
    A6 = 71, B6, C6, D6, E6, F6, G6, H6,
    A7 = 81, B7, C7, D7, E7, F7, G7, H7,
    A8 = 91, B8, C8, D8, E8, F8, G8, H8, 
    NO_SQ, OFFBOARD
};


/*
    This section outlines the board and move/undo structs.
*/

//S_MOVE defines
typedef struct {
    int move;   //Stores all info needed for the move
    int score;  //Gives a score based on
} S_MOVE;

//Store moves for a position and a count of those moves
typedef struct {
    S_MOVE moves[MAXPOSITIONMOVES];
    int count;
} S_MOVELIST;

//Store the position key that led to a move for future lookup
typedef struct {
    U64 posKey;
    int move;
} S_PVENTRY;

//Keep a table of S_PVENTRY key-move pairs
typedef struct {
    S_PVENTRY *pTable;
    int numEntries;
} S_PVTABLE;

//S_UNDO defines the structure for undoing moves
typedef struct {
    int move;       //The most recent move to undo
    int castlePerm; //The castle permision before the move was played
    int enPas;      //En passant square before the move the undo
    int fiftyMove;  //50 move rule status before the move to undo
    U64 posKey;     //The position key the move was played at
} S_UNDO;

//S_BOARD defines the structure for the playing board
typedef struct {
    int pieces[BRD_SQ_NUM];
    U64 pawns[3];   //3 arrays of pawns for white, black, and both. 64 bit int represents the board (1 means a pawn is on that square)
    int KingSq[2];  //Holds black and white king locations
    int side;       //Keeps track of whose turn it is
    int enPas;      //Keeps track of possible en passant square if there is one (otherwise it's set to NO_SQ)
    int fiftyMove;  //Watches for the 50 move rule (if 50 turns are completed without capture, the game is drawn)
    int ply;        //How many half moves have been made into current search (1 side moving a piece == 1 ply)
    int hisPly;     //History of how many moves have been made in total. Important for checking repitition.
    int castlePerm; //Castling permissions
    U64 posKey;     //Unix key generated for each position
    int pceNum[13]; //Number of pieces still on the board. Ex. Num of white knights would be value of pceNum at pos 2 as defined by earlier enum
    int bigPce[2];  //Number of non-pawn pieces on the board. An array each for white, black, and both.
    int majPce[2];  //Rooks and queens
    int minPce[2];  //Bishops and knights
    int material[2];
    S_UNDO history[MAXGAMEMOVES]; //Stores move history for the purpose of undoing moves
    int pList[13][10];  //piece list: 13 piece types with a max of 10 each in extreme cases

    S_PVTABLE PvTable[1];
} S_BOARD;



            /*  GAME MOVES  */

/* Move info is stored in one integer
    0000 0000 0000 0000 0000 0111 1111 -> From (0x7F)
    0000 0000 0000 0011 1111 1000 0000 -> To (>> 7, 0x3F)
    0000 0000 0011 1100 0000 0000 0000 -> Captured (>> 14, 0xF)
    0000 0000 0100 0000 0000 0000 0000 -> En Passant (0x40000)
    0000 0000 1000 0000 0000 0000 0000 -> Pawn Start (0x80000)
    0000 1111 0000 0000 0000 0000 0000 -> Promoted Piece (>> 20, 0xF)
    0001 0000 0000 0000 0000 0000 0000 -> Castle (0x1000000)
*/

#define FROMSQ(m)   ( (m)      & 0x7F)
#define TOSQ(m)     (((m)>>7)  & 0x7F)
#define CAPTURED(m) (((m)>>14) & 0xF)
#define PROMOTED(m) (((m)>>20) & 0xF)

#define MFLAGEP   0x40000    //En passant
#define MFLAGCAP  0x7C000    //Captured via en passant
#define MFLAGPS   0x80000    //Pawn start
#define MFLAGFROM 0xF00000   //Start square of a move
#define MFLAGCA   0x1000000  //Captured
#define NOMOVE    0


            /*  MACROS  */

#define FR2SQ(f,r) ((21+(f)) + ((r)*10)) //Converts rank number to 120 array-based-num for that square

#define SQ120(sq64) (Sq64ToSq120[(sq64)])
#define SQ64(sq120) (Sq120ToSq64[(sq120)])

#define CNT(b) CountBits(b)
#define POP(b) PopBit(b)

#define CLRBIT(bb,sq) ((bb) &= ClearMask[(sq)]) //Use bitwise operator AND to clear specified bit
#define SETBIT(bb,sq) ((bb) |= SetMask[(sq)])   //Use bitwise operator OR to set specified bit

#define IsBQ(p) (PieceBishopQueen[(p)])         //Determine what type of piece 'p' is
#define IsKi(p) (PieceKing[(p)])
#define IsKn(p) (PieceKnight[(p)])
#define IsRQ(p) (PieceRookQueen[(p)])


            /*  GLOBALS  */

extern int Sq120ToSq64[BRD_SQ_NUM]; //Convert 120 board to 64
extern int Sq64ToSq120[64];         //Convert 64 board to 120

extern U64 ClearMask[64];           //Used for clearing bits on the bitboard
extern U64 SetMask[64];             //Used for setting bits on the bitboard

extern U64 CastleKeys[16];
extern U64 PieceKeys[13][120];
extern U64 SideKey;

extern char FileChar[];             //Indexed by corresponding integer to print piece, side, rank, and file
extern char PceChar[];
extern char RankChar[];
extern char SideChar[];

extern int PieceBig[13];            //Used to ask true/false Qs: What piece is it and what's its value and colour?
extern int PieceCol[13];
extern int PieceMaj[13];
extern int PieceMin[13];
extern int PiecePawn[13];
extern int PieceVal[13];

extern int FilesBrd[BRD_SQ_NUM];    //Given a piece, what file and rank is it on?
extern int RanksBrd[BRD_SQ_NUM];

extern int PieceBishopQueen[13];    //These arrays answer the question, is the piece a knight, king, rook/queen, or a bishop/queen
extern int PieceKing[13];           //They are used by attack.cpp
extern int PieceKnight[13];
extern int PieceRookQueen[13];
extern int PieceSlides[13];


            /*  FUNCTIONS  */

//attack.cpp
extern int SqAttacked(const int sq, const int side, const S_BOARD *pos);

//bitboards.cpp
extern int  CountBits(U64 b);
extern int  PopBit(U64 *bb);
extern void PrintBitBoard(U64 bb);

//board.cpp
extern int  CheckBoard(const S_BOARD *pos);
extern int  ParseFen(char *fen, S_BOARD *pos);
extern void PrintBoard(const S_BOARD *pos);
extern void ResetBoard(S_BOARD *pos);
extern void UpdateListsMaterial(S_BOARD *pos);

//hashkeys.cpp
extern U64 GeneratePosKey(const S_BOARD *pos);

//init.cpp
extern void AllInit();

//io.cpp
extern int  ParseMove(char *ptrChar, S_BOARD *pos);
extern void PrintMoveList(const S_MOVELIST *list);
extern char *PrMove(const int move);
extern char *PrSq(const int sq);

//makemove.cpp
extern int  MakeMove(S_BOARD *pos, int move);
extern void TakeMove(S_BOARD *pos);

//misc.cpp
extern int GetTimeMs();

//movegen.cpp
extern void GenerateAllMoves (const S_BOARD *pos, S_MOVELIST *list);

//perf.cpp
extern void PerftTest(int depth, S_BOARD *pos);

//pvtable.cpp
extern void InitPvTable(S_PVTABLE *t);

//search.cpp
extern void SearchPosition(S_BOARD *pos);

//validate.cpp
extern int FileRankValid(const int fr);
extern int PieceValid(const int pce);
extern int PieceValidEmpty(const int pce);
extern int SideValid(const int side);
extern int SqOnBoard(const int sq);





#endif