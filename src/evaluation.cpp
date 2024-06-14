#include "evaluation.h"

const int *PST[6] = {
    PAWN_PST,
    KNIGHT_PST,
    BISHOP_PST,
    ROOK_PST,
    QUEEN_PST,
    KING_PST
};

const int ISOLANI_WEIGHT = 12;
const int DOUBLED_WEIGHT = 18;
const int WEAK_WEIGHT = 15;
const int PASSED_PAWN_WEIGHT = 5;

// Returns the endgame weight of a given position 
// Endgame weight is indirectly proportional to the number of 
// pieces left on the board
int endgameWeight(chess::Board board) {
    int midgameLimit = 15258;
    int endgameLimit  = 3915;
    int nonPawnMaterial = 0;
    nonPawnMaterial += board.pieces(chess::PieceType::PAWN).count();
    nonPawnMaterial += board.pieces(chess::PieceType::KNIGHT).count();
    nonPawnMaterial += board.pieces(chess::PieceType::BISHOP).count();
    nonPawnMaterial += board.pieces(chess::PieceType::ROOK).count() * 2;
    nonPawnMaterial += board.pieces(chess::PieceType::QUEEN).count() * 4;
    nonPawnMaterial = std::max(endgameLimit, std::min(nonPawnMaterial, midgameLimit));
    return 128 - ((((nonPawnMaterial - endgameLimit) * 128) / (midgameLimit - endgameLimit)) << 0);
}

// Evaluates a position relative to a certain side based on the 
// material count and the Piece-Square Tables
int material_count(chess::Board board, chess::Color c, bool endgame) {
    int eval = 0;
    for (int p=(int)chess::PieceType::PAWN; p<=(int)chess::PieceType::KING; p++) {
        chess::Bitboard pieces = board.pieces(PIECETYPES[p], c);
        while (pieces) {
            chess::Square square = pieces.pop();
            eval += PIECE_VALUES[p];
            if (c == chess::Color::WHITE) {
                if (p==KING && endgame) {
                    eval += KING_EG_PST[FLIP[square.index()]];
                } else if (p==QUEEN && endgame) {
                    eval += PST[p][FLIP[square.index()]];
                } else if (p!=QUEEN) {
                    eval += PST[p][FLIP[square.index()]];
                }
            } else {
                if (p==KING && endgame) {
                    eval += KING_EG_PST[square.index()];
                } else if (p==QUEEN && endgame) {
                    eval += PST[p][square.index()];
                } else if (p!=QUEEN) {
                    eval += PST[p][square.index()];
                }
            }
        }
    }
    return eval;
}

// Returns the number of isolated pawns of a given color in a position 
/* A pawn is considered to be an isolani when: 
    - There are no friendly pawns on the adjacent files
*/
int get_isolanis(chess::Board board, chess::Color c) {
    int count = 0;
    chess::Bitboard pawns = board.pieces(chess::PieceType::PAWN, c);
    while (pawns) {
        int square = pawns.pop();
        int file = square & 7;
        if (file != 0) {
            if (board.pieces(chess::PieceType::PAWN, c) & (0x0101010101010101 << (file - 1))) {
                continue; 
            }
        }
        if (file != 7) {
            if (board.pieces(chess::PieceType::PAWN, c) & (0x0101010101010101 << (file + 1))) {
                continue; 
            }
        }
        count++;
    }
    return count;
}

// Returns the number of doubled pawn groups of a given color in a position
/* A group of pawns are said to be doubled when:
    - There are two or more of them in the same file 
*/
int get_doubled(chess::Board board, chess::Color c) {
    int count = 0;
    chess::Bitboard pawns = board.pieces(chess::PieceType::PAWN, c);
    for (int i=0; i<8; i++) {
        chess::Bitboard file = (0x0101010101010101 << i);
        int onFile = (file & pawns).count();
        if (onFile >= 2) {
            count += onFile - 1;
        }
    }
    return count;
}

// Returns the number of weak pawns relative to given color
/* A pawn is considered weak when:
    - It is not defended by another pawn
    - It cannot be pushed to a defended square
    - It cannot be double-pushed to a defended square
    - A pawn cannot be pushed to defend it
    - A pawn cannot be double-pushed to defend it
*/
int get_weak(chess::Board board, chess::Color c) {
    chess::Color other = ~c;
    // Calculate Weak Pawns 
    chess::Bitboard weakPawns = board.pieces(chess::PieceType::PAWN, c);
    chess::Bitboard pawnAttacks = pawnAttacksBB(c, weakPawns);
    chess::Bitboard allPawns = board.pieces(chess::PieceType::PAWN);
    weakPawns &= ~pawnAttacks;
    weakPawns &= ~((pawnAttacks & ~allPawns) >> 8);
    weakPawns &= ~(((pawnAttacks & ~allPawns & ~(allPawns << 8)) >> 16) & (chess::Bitboard)chess::Rank::RANK_2);
    chess::Bitboard pawnStep1 = (board.pieces(chess::PieceType::PAWN, c) << 8) & ~allPawns;
    chess::Bitboard pawnStep2 = (pawnStep1 << 8) & ~allPawns & (chess::Bitboard)chess::Rank::RANK_4;
    weakPawns &=  ~pawnAttacksBB(c, pawnStep1 | pawnStep2);
    return weakPawns.count();
}

// Checks if a pawn on a given square is a passed pawn
/* A pawn is considered a passer when:
    - Its front span is not attacked by any enemy pawns
    - There are no pawns of either color in the squares in front of it
*/
bool passer(chess::Board board, chess::Square sq) {
    chess::Color c = board.at(sq).color();
    chess::Color other = ~c;
    chess::Bitboard otherPawns = board.pieces(chess::PieceType::PAWN, other);
    chess::Bitboard otherPawnsAttacks = pawnAttacksBB(other, otherPawns);
    chess::Bitboard allPawns = board.pieces(chess::PieceType::PAWN);
    chess::Bitboard pFrontSpan = pawnFrontSpan(board, sq, c);
    return (!(otherPawnsAttacks & pFrontSpan) && !(allPawns & pFrontSpan));
}

// Returns the number of passed pawns of a given color in a position
int get_passed(chess::Board board, chess::Color c) {
    int count = 0;
    chess::Bitboard pawns = board.pieces(chess::PieceType::PAWN, c);
    while (pawns) {
        int square = pawns.pop();
        if (passer(board, chess::Square(square))) {
            count++;
        }
    }
    return count;
}

// Returns the mobility score for a given color in a position
// Value decreases in significance in the endgame
int mobility(chess::Board board, chess::Color c) {
    int count = 0;
    for (int sq=0; sq<64; sq++) {
        chess::Square square = chess::Square(sq);
        chess::Piece piece = board.at(square);
        if (piece.color() == c) {
            chess::PieceType p = piece.type();
            chess::Bitboard attacks;
            if (p == chess::PieceType::KNIGHT) {
                attacks = chess::attacks::knight(sq);
            } else if (p == chess::PieceType::BISHOP) {
                attacks = chess::attacks::bishop(sq, board.occ());
            } else if (p == chess::PieceType::ROOK) {
                attacks = chess::attacks::rook(sq, board.occ());
            } else if (p == chess::PieceType::QUEEN) {
                attacks = chess::attacks::queen(sq, board.occ());
            } else {
                continue;
            }
            attacks &= ~(board.pieces(chess::PieceType::KING, c) | board.pieces(chess::PieceType::PAWN, c));
            chess::Bitboard otherPawns = board.pieces(chess::PieceType::PAWN, ~c);
            if (c == chess::Color::WHITE) {
                attacks &= ~(chess::attacks::shift<chess::Direction::SOUTH_EAST>(otherPawns));
                attacks &= ~(chess::attacks::shift<chess::Direction::SOUTH_WEST>(otherPawns));
            } else if (c == chess::Color::BLACK) {
                attacks &= ~(chess::attacks::shift<chess::Direction::NORTH_EAST>(otherPawns));
                attacks &= ~(chess::attacks::shift<chess::Direction::NORTH_WEST>(otherPawns));
            }

            if (p == chess::PieceType::KNIGHT) {
                count += knightMob[attacks.count()];
            } else if (p == chess::PieceType::BISHOP) {
                count += bishopMob[attacks.count()];
            } else if (p == chess::PieceType::ROOK) {
                count += rookMob[attacks.count()];
            } else { // queen
                count += queenMob[attacks.count()];
            }
        }
    }
    return count;
}

// Returns the king safety score for a given color in a position
// Value decreases in significance in the endgame
int king_safety(chess::Board board, chess::Color c) {
    int count = 0;
    chess::Color opp = ~c;
    int up = (c == chess::Color::WHITE ? 8 : -8);
    chess::Square kingSquare = board.kingSq(c);
    chess::Bitboard kingZone = chess::attacks::king(kingSquare) | (chess::attacks::king(kingSquare) << up);
    while (kingZone) {
        chess::Square square = kingZone.pop();
        chess::Bitboard attackers = chess::attacks::attackers(board, opp, square);
        while (attackers) {
            chess::Square attackSq = attackers.pop();
            chess::PieceType p = board.at<chess::PieceType>(attackSq);
            if (p==chess::PieceType::KNIGHT || p==chess::PieceType::BISHOP) {
                count += 2;
            } else if (p==chess::PieceType::ROOK) {
                count += 3;
            } else if (p==chess::PieceType::QUEEN) {
                count += 5;
            }
        }
    }
    return -safetyTable[count];
}

// Special king endgame evaluation to force opponent kings to corner
// This makes it easy to later deliver checkmate, as without it
// The computer hopelessly shuffles pieces around
int king_endgame_eval(chess::Board board, chess::Color c, int endgameWeight) {
    int eval = 0;
    chess::Color oppColor = ~c;
    chess::Square kingSq = board.kingSq(c);
    chess::Square oppKingSq = board.kingSq(oppColor);

    // Favour positions where opponent king is far away from centre
    int oppKingRank = oppKingSq.rank();
    int oppKingFile = oppKingSq.file();
    int oppKingDistFromCentreFile = std::max(3 - oppKingFile, oppKingFile - 4);
    int oppKingDistFromCentreRank = std::max(3 - oppKingRank, oppKingRank - 4);
    int oppKingDistFromCentre = oppKingDistFromCentreFile + oppKingDistFromCentreRank;
    eval += oppKingDistFromCentre;

    // Favour positions where the kings are closer to each other
    int friendlyKingRank = kingSq.rank();
    int friendlyKingFile = kingSq.file();
    int distBwFiles = std::abs(friendlyKingFile - oppKingFile);
    int distBwRanks = std::abs(friendlyKingRank - oppKingRank);
    int distBwKings = distBwFiles + distBwRanks;
    eval += 14 - distBwKings;

    return eval*endgameWeight/10;
}

// Returns the evaluation for a given position
int evaluate(chess::Board board) {
    int endgameScore = material_count(board, chess::Color::WHITE, true) - material_count(board, chess::Color::BLACK, true);
    int middlegameScore = material_count(board, chess::Color::WHITE, false) - material_count(board, chess::Color::BLACK, false);
    int egWeight = endgameWeight(board);
    int eval = ((middlegameScore * (128 - egWeight)) + (endgameScore * egWeight)) / 128;
    eval -= ISOLANI_WEIGHT * (get_isolanis(board, chess::Color::WHITE) - get_isolanis(board, chess::Color::BLACK));
    eval -= WEAK_WEIGHT * (get_weak(board, chess::Color::WHITE) - get_weak(board, chess::Color::BLACK));
    eval -= DOUBLED_WEIGHT * (get_doubled(board, chess::Color::WHITE) - get_doubled(board, chess::Color::BLACK));
    eval += PASSED_PAWN_WEIGHT * (get_passed(board, chess::Color::WHITE) - get_passed(board, chess::Color::BLACK));
    if (!egWeight>115) { // if we are in the endgame, mobility and safety scores don't matter much
        eval += (mobility(board, chess::Color::WHITE) - mobility(board, chess::Color::BLACK));
        eval += (king_safety(board, chess::Color::WHITE) - king_safety(board, chess::Color::BLACK));
    } else {
        egWeight *= 2;
    }
    eval += king_endgame_eval(board, chess::Color::WHITE, egWeight) - king_endgame_eval(board, chess::Color::BLACK, egWeight);
    return (board.sideToMove() == chess::Color::WHITE ? eval : -eval);
}
