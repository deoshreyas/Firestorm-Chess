#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H

#include "chess.hpp"

const int VALUEUNKNOWN = -99999999;

enum Flag {
    EXACT,
    ALPHA,
    BETA
};

struct HashEntry {
    uint64_t key;
    int depth; 
    int flag;
    int value;
    chess::Move best = chess::Move::NULL_MOVE;
};

void InitTranspositionTable(int sizeMb);

void ClearTranspositionTable();

int ProbeHash(chess::Board board, int depth, int alpha, int beta);

void RecordHash(chess::Board board, int depth, int val, int flag, chess::Move best, bool cancelled);

chess::Move GetStoredMove(chess::Board board, int depth, int alpha, int beta);

chess::Move TryGetStoredMove(chess::Board board);

#endif