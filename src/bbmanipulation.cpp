#include "bbmanipulation.h"

// returns the pawn attacks for a bitboard of pawns and color
chess::Bitboard pawnAttacksBB(chess::Color C, chess::Bitboard pawns) {
    return (C == chess::Color::WHITE 
    ? chess::attacks::shift<chess::Direction::NORTH_WEST>(pawns) | chess::attacks::shift<chess::Direction::NORTH_EAST>(pawns) 
    : chess::attacks::shift<chess::Direction::SOUTH_EAST>(pawns) | chess::attacks::shift<chess::Direction::SOUTH_WEST>(pawns));
}

// returns the front span of a pawn 
chess::Bitboard pawnFrontSpan(chess::Board board, chess::Square sq, chess::Color c) {
    chess::Bitboard span = 0;
    int rank = sq.rank();
    if (c == chess::Color::WHITE) {
        span = ~0ULL << ((rank + 1) * 8);
    } else {
        span = ~0ULL >> ((8 - rank) * 8);
    }
    return span & (chess::Bitboard)sq.file();
}