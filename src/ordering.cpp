#include "ordering.h"

const int PIECE_VALUES[6] = {
    100,  // Pawn
    320,  // Knight
    330,  // Bishop
    500,  // Rook
    900,  // Queen
    20000 // King
};

struct compare {
    bool operator()(chess::Move const &a, chess::Move const &b) { 
            return a.score() > b.score();
    }
};

void OrderMoves(chess::Board board, chess::Movelist& moves, int depth) {
    int moveScoreGuess;
    chess::PieceType movePieceType;
    chess::PieceType capturePieceType;

    for (chess::Move move : moves) {
        moveScoreGuess = 0;
        movePieceType = board.at<chess::PieceType>(move.from());
        capturePieceType = board.at<chess::PieceType>(move.to());
        chess::Color oppColor = ~board.sideToMove();

        // Order moves based on lines previously searched at lower depths 
        if (TryGetStoredMove(board) == move) {
            moveScoreGuess += 1000;
        }

        // Order moves based on the MVV-LVA Heuristic (Most Valuable Victim - Least Valuable Aggressor)
        // We prioritize capturing high value pieces using low value pieces
        if (board.isCapture(move)) {
            moveScoreGuess += PIECE_VALUES[(int)capturePieceType] - PIECE_VALUES[(int)movePieceType];
        }

        // Priotize promotions 
        if (move.typeOf()==chess::Move::PROMOTION) {
            moveScoreGuess += PIECE_VALUES[(int)move.promotionType()];
        }

        move.setScore(move.score());
    }
    std::sort(moves.begin(), moves.end(), compare());
}