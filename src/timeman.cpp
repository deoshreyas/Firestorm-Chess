#include "timeman.h"

int GetThinkingTime(chess::Board board, int wtime, int btime, int winc, int binc, int movesToGo, int outOfBookMoves) {
    int timeRemainingMs = board.sideToMove()==chess::Color::WHITE ? wtime : btime;
    int incrementMs = board.sideToMove()==chess::Color::WHITE ? winc : binc;

    int outOfBookFactor = std::min(outOfBookMoves, 10);
    int urgencyFactor = 1;

    if (timeRemainingMs / movesToGo < 5000) {
        urgencyFactor = 2; // little time left
    } else {
        urgencyFactor = 1; // enough time left
    }

    int timeForThisMove = timeRemainingMs / movesToGo * outOfBookFactor / urgencyFactor;

    // Add increment to the time for this move
    timeForThisMove += incrementMs;

    // If time for this move is less than 0
    // Use 0.1 seconds to at least get some move 
    if (timeForThisMove < 0) {
        timeForThisMove = 100;
    }

    return timeForThisMove;
}
