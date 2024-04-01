//misc.cpp

#include "defs.h"

#include <cstdio>

#ifdef WIN32
#include "sysinfoapi.h"
#else
#include "time.h"
#endif

int GetTimeMs() {
    #ifdef WIN32
        return GetTickCount();
/*
    #else
        struct timeval t;
        gettimeofday(&t, NULL);
        return t.tv_sec*1000 + t.tv_usec/1000;
*/
    #endif
}