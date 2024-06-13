#ifndef BBMANIPULATION_H
#define BBMANIPULATION_H

#include "chess.hpp"

// returns the pawn attacks for a bitboard of pawns and color
chess::Bitboard pawnAttacksBB(chess::Color C, chess::Bitboard pawns);

// returns the front span of a pawn 
chess::Bitboard pawnFrontSpan(chess::Board board, chess::Square sq, chess::Color c);

#endif