#ifndef SEARCH_H
#define SEARCH_H

#include "chess.hpp"
#include "evaluation.h"
#include "ordering.h"
#include "transposition.h"
#include "reader.hpp"

const int MATE_VALUE = 25000000;

struct SearchInfo {
    int depth;
    int nodes;
    int duration;
    bool infinite;
    bool stopped;
    bool usingNullMoves;
    bool useOwnBook;
    
    SearchInfo() {
        depth = 1000;
        nodes = 0;
        duration = -1;
        infinite = false;
        stopped = true;
        usingNullMoves = true;
        useOwnBook = true;
    }
};

extern SearchInfo info;

chess::Move Search(chess::Board board, int depth);

#endif