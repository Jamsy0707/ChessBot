//search.cpp

#include "defs.h"

#include <cstdio>
#include <iostream>


/*
    Name:    IsRepetition
    Vars:    S_BOARD *pos  - A pointer to the board.
    Purpose: Find if the board position has already been seen to improve performance when calculating moves.
    Returns: TRUE if the position has been seen.
             FALSE otherwise.
*/
int IsRepetition (const S_BOARD *pos) {
    //Loop from the last time the 50 move rule was reset to search for identical board positions
    for (int i = pos->hisPly - pos->fiftyMove; i < pos->hisPly-1; ++i) {
        ASSERT(i >= 0 && i < MAXGAMEMOVES);
        if (pos->posKey == pos->history[i].posKey) 
            return TRUE; 
    }
    return FALSE;
}


/*
    Name:    SearchPosition
    Vars:    S_BOARD *pos  - A pointer to the board.
    Purpose: 
*/
void SearchPosition(S_BOARD *pos) {

}