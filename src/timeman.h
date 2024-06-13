#ifndef TIMEMAN_H
#define TIMEMAN_H

#include "chess.hpp"
#include <cmath>

int GetThinkingTime(chess::Board board, int wtime, int btime, int winc, int binc, int movesToGo, int outOfBookMoves);

#endif