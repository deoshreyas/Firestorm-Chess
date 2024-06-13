#ifndef ORDERING_H
#define ORDERING_H

#include "chess.hpp"
#include "transposition.h"

void OrderMoves(chess::Board board, chess::Movelist& moves, int depth); 

#endif