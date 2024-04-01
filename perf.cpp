//perf.cpp

#include "defs.h"

#include <cstdio>
#include <iostream>

using namespace std;

long leafNodes;

//Generate all moves for a position up to a specified depth for purposes of finding the best move sequence
void Perft(int depth, S_BOARD *pos) {
    ASSERT(CheckBoard(pos));

    //If depth is 0 we can't proceed any deeper so increment leaf count and return
    if (depth == 0) {
        leafNodes++;
        return;
    }

    //Make a movelist and generate all moves for this position
    S_MOVELIST list[1];
    GenerateAllMoves(pos, list);

    //Loop through all possible moves and call perft again using those moves
    //This way, all possibilities are considered up to a specified depth that can be changed at any time
    int MoveNum = 0;
    for (MoveNum = 0; MoveNum < list->count; ++MoveNum) {
        if (!MakeMove(pos, list->moves[MoveNum].move))
            continue;
        Perft(depth-1, pos);
        TakeMove(pos);
    }

    return;
}

//Similar to Perft function but print results for visually checking program values
void PerftTest(int depth, S_BOARD *pos) {
    ASSERT(CheckBoard(pos));

    PrintBoard(pos);

    cout << "\nStarting Test To Depth: " << depth << endl;

    leafNodes = 0;
    int start = GetTimeMs();

    S_MOVELIST list[1];
    GenerateAllMoves(pos, list);

    int move;
    int MoveNum = 0;
    for (MoveNum = 0; MoveNum < list->count; ++MoveNum) {
        move = list->moves[MoveNum].move;
        if (!MakeMove(pos, move))
            continue;
        long cumnodes = leafNodes;
        Perft(depth-1, pos);
        TakeMove(pos);
        long oldnodes = leafNodes - cumnodes;
        cout << "Move " << MoveNum+1 << " : " << PrMove(move) << " : " << oldnodes << endl;
    }

    cout << "\nTest Complete : " << leafNodes << " leaf nodes visited in " << GetTimeMs()-start << "ms." << endl;

    return;
}

