//main.cpp

#include "defs.h"

#include <iostream>

using namespace std;

char PAWNMOVESW[] = {"rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"};
char PAWNMOVESB[] = {"rnbqkbnr/p1p1p3/3p3p/1p1p4/2P1Pp2/8/PP1P1PpP/RNBQKBNR b KQkq e3 0 1"};
char KNIGHTSKINGS[] = {"5k2/1n6/4n3/6N1/8/3N4/8/5K2 b - - 0 1"};
char ROOKS[] = {"6k1/8/5r2/8/1nR5/5N2/8/6K1 w - - 0 1"};
char QUEENS[] = {"6k1/8/4nq2/8/1nQ5/5N2/1N6/6K1 w - - 0 1"};
char BISHOPS[] = {"6k1/1b6/4n3/8/1n4B1/1B3N2/1N6/2b3K1 b - - 0 1"};
char CASTLE[] = {"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"};
char START_FEN[] = {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"};

char PERFFEN[] = {"n1n5/PPPk4/8/8/8/8/4Kppp/5N1N w - - 0 1"};


/*
    Name:    main
    Purpose: Driver function
*/
int main () {
    AllInit();

    S_BOARD board[1];
    S_MOVELIST list[1];

    ParseFen(START_FEN, board);
    //PerftTest(3, board);

    char input[6];
    int Move = NOMOVE;

    while (TRUE) {
        PrintBoard(board);
        cout << "Enter a move: ";
        cin >> input;

        if (input[0] == 'q') {
            break;
        } else if (input[0] == 't') {
            TakeMove(board);
            continue;
        } else if (input[0] == 'p') {
            PerftTest(4, board);
        } else {
            Move = ParseMove(input, board);
            if (Move != NOMOVE) 
                MakeMove(board, Move);
            else
                cout << "Move not parsed. " << input << endl;
        }
    }


    return 0;
} 