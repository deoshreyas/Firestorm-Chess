#include "search.h"

SearchInfo info;

// Null Move Pruning Reduction Constant
/* For a depth of 10, we only search it to depth 8 when null-moving.
    R=2 is commonly accepted as a good reduction to search a null-move. 
    R=1 is usually too small, making the search long. And R=3 is too 
    large, rendering the search ineffective. */
const int R = 2;

// Quiscence Search to avoid the horizon effect
// Special type of search where only the capture moves are analyzed
int QuiescenceSearch(chess::Board board, int alpha, int beta) {
    // Search cancelled 
    if (info.stopped) {
        return 0;
    }

    info.nodes++;
    int stand_pat = evaluate(board);
    if (stand_pat >= beta) {
        return beta;
    }
    if (alpha < stand_pat) {
        alpha = stand_pat;
    }
    chess::Movelist moves;
    chess::movegen::legalmoves<chess::movegen::MoveGenType::CAPTURE>(moves, board);
    OrderMoves(board, moves, -1);
    for (chess::Move move : moves) {
        // Search cancelled 
        if (info.stopped) {
            return 0;
        }

        board.makeMove(move);
        int score = -QuiescenceSearch(board, -beta, -alpha);
        board.unmakeMove(move);
        if (score >= beta) {
            return beta;
        }
        if (score > alpha) {
            alpha = score;
        }
    }
    return alpha;
}

// NegaMax Search with Alpha-Beta Pruning
int NegaMax(chess::Board board, int depth, int alpha, int beta, bool allowNull) {
    // Search cancelled 
    if (info.stopped) {
        return 0;
    }

    info.nodes++;

    int HashFlag = ALPHA;
    chess::Move curr_best;

    int ttValue = ProbeHash(board, depth, alpha, beta);
    if (ttValue != VALUEUNKNOWN) {
        return ttValue;
    }

    if (board.isRepetition(1)) {
        return 0; // draw
    } 
    if (board.isHalfMoveDraw()) {
        if (board.getHalfMoveDrawType().first == chess::GameResultReason::FIFTY_MOVE_RULE) {
            return 0; // draw
        }
    }

    if (depth == 0) {
        int evaluation = QuiescenceSearch(board, alpha, beta);
        RecordHash(board, depth, evaluation, EXACT, curr_best, info.stopped);
        return evaluation;
    }

    int ply = info.depth - depth;

    chess::Movelist movelist;
    chess::movegen::legalmoves(movelist, board);

    if (movelist.size() == 0) {
        if (board.inCheck()) {
            return -(MATE_VALUE-ply); // checkmate
        } else {
            return 0; // draw
        }
    }

    OrderMoves(board, movelist, depth);

    // Null Move Pruning
    if (allowNull && depth>R && !board.inCheck()) {
        // Only do null-move pruning in positions with more material. 
        // This is to prevent zugswang.
        if (material_count(board, chess::Color::WHITE, false) + material_count(board, chess::Color::BLACK, false) > 1800) {
            board.makeNullMove(); // Making the null-move
            int eval = -NegaMax(board, depth-R-1, -beta, -beta+1, false);
            board.unmakeNullMove(); // Unmaking the null-move
            if (eval >= beta) {
                return eval; // Cutoff
            }
        }
    }

    for (chess::Move move : movelist) {
        // Search cancelled 
        if (info.stopped) {
            return 0;
        }

        board.makeMove(move);
        int score = -NegaMax(board, depth-1, -beta, -alpha, info.usingNullMoves);
        board.unmakeMove(move);
        if (score >= beta) {
            RecordHash(board, depth, beta, BETA, curr_best, info.stopped);
            return beta;
        }
        if (score > alpha) {
            alpha = score;
            HashFlag = EXACT;
            curr_best = move;
        }
        if (alpha >= beta) {
            break;
        }
    }

    RecordHash(board, depth, alpha, HashFlag, curr_best, info.stopped); 
    return alpha;
}

// Root call for NegaMax
chess::Move Search(chess::Board board, int depth) {
    // Look for the best move in the transposition table
    chess::Move best_move = GetStoredMove(board, depth, -INT_MAX, INT_MAX);
    if (best_move != chess::Move::NULL_MOVE) {
        return best_move;
    }
    // Call NegaMax for finding best move
    chess::Movelist movelist;
    chess::movegen::legalmoves(movelist, board);
    OrderMoves(board, movelist, -1);
    best_move = movelist[0];
    int maxScore = -INT_MAX;
    for (chess::Move move : movelist) {
        // Search cancelled 
        if (info.stopped) {
            return best_move;
        }

        board.makeMove(move);
        int score = -NegaMax(board, depth - 1, -INT_MAX, INT_MAX, info.usingNullMoves);
        board.unmakeMove(move);
        best_move.setScore(score);
        if (score > maxScore && !info.stopped) {
            maxScore = score;
            best_move = move;
        }
    }
    return best_move;
}