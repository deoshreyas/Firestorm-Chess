#include "transposition.h"

int TABLE_SIZE;
HashEntry *TTable;

void InitTranspositionTable(int sizeMb) {
    TABLE_SIZE = sizeMb * 1024 * 1024 / sizeof(HashEntry);
    TTable = new HashEntry[TABLE_SIZE];
    return;
}

void ClearTranspositionTable() {
    delete[] TTable;
    TTable = new HashEntry[TABLE_SIZE];
    return;
}

int ProbeHash(chess::Board board, int depth, int alpha, int beta) {
    HashEntry *entry = &TTable[board.zobrist() % TABLE_SIZE];
    if (entry->key == board.zobrist()) {
        if (entry->depth >= depth) {
            if (entry->flag == EXACT) {
                return entry->value;
            }
            if ((entry->flag == ALPHA) && (entry->value <= alpha)) {
                return alpha;
            }
            if ((entry->flag == BETA) && (entry->value >= beta)) {
                return beta;
            }
        }
    }
    return VALUEUNKNOWN;
}

void RecordHash(chess::Board board, int depth, int val, int flag, chess::Move best, bool cancelled) {
    if (cancelled) {
        return; // don't record anything if search was cancelled
    }
    HashEntry *entry = &TTable[board.zobrist() % TABLE_SIZE];
    entry->key = board.zobrist();
    entry->value = val;
    entry->flag = flag;
    entry->depth = depth;
    entry->best = best;
}

chess::Move GetStoredMove(chess::Board board, int depth, int alpha, int beta) {
    HashEntry entry = TTable[board.zobrist() % TABLE_SIZE];
    chess::Move move;
    if (entry.flag!=VALUEUNKNOWN && entry.depth>=depth) { // valid node
        if (entry.key == board.zobrist()) {
            if (entry.flag == EXACT) {
                move = entry.best;
                move.setScore(entry.value);
                return move;
            } else if (entry.key == ALPHA) {
                alpha = std::max(alpha, entry.value);
            } else {
                beta = std::max(beta, entry.value);
            }
        }
        if (alpha > beta) {
            move = entry.best;
            move.setScore(entry.value);
            return move;
        }
    }
    return chess::Move::NULL_MOVE;
}

chess::Move TryGetStoredMove(chess::Board board) {
    HashEntry entry = TTable[board.zobrist() % TABLE_SIZE];
    return entry.best;
}