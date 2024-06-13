#include "timeman.h"

int GetThinkingTime(chess::Board board, int wtime, int btime, int winc, int binc, int movesToGo, int outOfBookMoves) {
    int nMoves = std::min(outOfBookMoves, 10);
    int timeRemainingMs = board.sideToMove()==chess::Color::WHITE ? wtime : btime;
    int incrementMs = board.sideToMove()==chess::Color::WHITE ? winc : binc;

    int factor = 2 - nMoves / 10;

    int timeForThisMove = timeRemainingMs / movesToGo;
    timeForThisMove *= factor;

    // Add increment to the time for this move
    timeForThisMove += incrementMs;

    // If the increment puts us above the total time left
    // use the timeleft - 0.5 seconds
    if (timeForThisMove >= timeRemainingMs) {
        timeForThisMove = timeRemainingMs - 500;
    }

    // If 0.5 seconds puts us below 0
    // use 0.1 seconds to atleast get some move.
    if (timeForThisMove < 0) {
        timeForThisMove = 100;
    }

    return timeForThisMove;
}