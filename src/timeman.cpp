#include "timeman.h"

int GetThinkingTime(chess::Board board, int wtime, int btime, int winc, int binc, int movesToGo, int outOfBookMoves) {
    int timeRemainingMs = board.sideToMove()==chess::Color::WHITE ? wtime : btime;
    int incrementMs = board.sideToMove()==chess::Color::WHITE ? winc : binc;
    
    int timeForThisMove;
    if (outOfBookMoves <= 40) {
        timeForThisMove = timeRemainingMs / (71 - outOfBookMoves); // spend more time in the middle games
    } else {
        timeForThisMove = timeRemainingMs / 30; // endgames should be faster
    }

    // Add increment to the time for this move
    timeForThisMove += incrementMs / (2 * movesToGo);

    // If time for this move is less than 0
    // Use 0.1 seconds to at least get some move 
    if (timeForThisMove < 0) {
        timeForThisMove = 100;
    }

    return timeForThisMove;
}
