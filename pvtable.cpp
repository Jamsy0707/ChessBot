//pvtable.cpp

#include "defs.h"

#include <cstdio>
#include <iostream>

const int PvSize = 0x100000 * 2;  //2MB


/*
    Name:    ClearPvTable
    Vars:    S_PVTABLE *t - A pointer to the table storing the list of known positions.
    Purpose: Clear the pvTable of keys and moves -- both to 0.
*/
static void ClearPvTable(S_PVTABLE *t) {
    S_PVENTRY *pvEntry;
    //Loop through all entries in the table; set the key and move to 0.
    for (pvEntry = t->pTable; pvEntry < t->pTable + t->numEntries; pvEntry++) {
        pvEntry->posKey = 0ULL;
        pvEntry->move = NOMOVE;
    }
}


/*
    Name:    InitPvTable
    Vars:    S_PVTABLE *t - A pointer to the table storing the list of known positions.
    Purpose: Initialize a new pvTable by allocating memory and calling ClearPvTable
*/
void InitPvTable(S_PVTABLE *t) {
    t->numEntries = PvSize / sizeof(S_PVENTRY);  //Size of table / size of entries gives the number of entries
    t->numEntries -= 2;  //Make sure program doesn't try to access entries outside the table
    t->pTable = (S_PVENTRY *)malloc(t->numEntries * sizeof(S_PVENTRY));  //Allocate enough memory for the table
    free(t->pTable);  //Free the memory *t points to
    ClearPvTable(t);
    std::cout << "PVTABLE init complete with " << t->numEntries << " entries.\n";

    //Make PvTable a vector of pairs?
}