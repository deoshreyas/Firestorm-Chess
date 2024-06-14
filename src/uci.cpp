#include "uci.h"

const std::string ENGINE_NAME = "Firestorm";
const std::string ENGINE_VERSION = "v0.0.1";
const std::string ENGINE_AUTHOR = "Shreyas Deo";

// Transposition Table 
int TABLE_SIZE_MB = 64;

// Opening Book
const char *path = "books/komodo.bin";
Reader::Book book;

// For Time Management
int noOfMovesOutOfBook = 0;

// Get a random move from a list of possible moves 
std::string GetRandomMove(std::vector<std::string> moves) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, moves.size()-1);
    return moves[dis(gen)];
}

// Available UCI Options
std::string options = 
"\noption name Hash type spin default 64 min 1 max 33554432\n\
option name Clear Hash type button\n\
option name NullMove type check default true\n\
option name OwnBook type check default true";

// UCI Constructor
UCI::UCI() {
    wtime = 0;
}

// UCI Loop Method
void UCI::loop() {
    // Use Transposition Table by default with 64MB
    InitTranspositionTable(TABLE_SIZE_MB);
    // Init Polyglot Opening Book 
    book.Load(path);

    std::string line;
    std::string token;
    std::cout.setf(std::ios::unitbuf);
    while (std::getline(std::cin, line)) {
        std::istringstream is(line);
        token.clear();
        is >> std::skipws >> token;

        if (token == "uci") {
            std::cout << "id name " << ENGINE_NAME << std::endl;
            std::cout << "id author " << ENGINE_AUTHOR << std::endl;
            std::cout << options << std::endl;
            std::cout << "uciok" << std::endl;
        } else if (token == "setoption") {
            std::string name, value;
            is >> std::skipws >> token;
            if (token == "name") {
                is >> std::skipws >> name;
                if (name == "Hash") {
                    is >> std::skipws >> name;
                    if (name == "value") {
                        is >> std::skipws >> value;
                        TABLE_SIZE_MB = stoi(value);
                        ClearTranspositionTable();
                        InitTranspositionTable(TABLE_SIZE_MB);
                        continue;
                    }
                } else if (name == "Clear") {
                    is >> std::skipws >> name;
                    if (name == "Hash") {
                        if (TABLE_SIZE_MB != 0) {
                            ClearTranspositionTable();
                            continue;
                        }
                    }
                } else if (name == "NullMove") {
                    is >> std::skipws >> name;
                    if (name == "value") {
                        is >> std::skipws >> value;
                        if (value == "true") {
                            info.usingNullMoves = true;
                            continue;
                        } else if (value == "false") {
                            info.usingNullMoves = false;
                            continue;
                        }
                    }
                } else if (name == "OwnBook") {
                    is >> std::skipws >> name;
                    if (name == "value") {
                        is >> std::skipws >> value;
                        if (value == "true") {
                            info.useOwnBook = true;
                            continue;
                        } else if (value == "false") {
                            info.useOwnBook = false;
                            continue;
                        }
                    }
                }
            } 
            std::cout << "Unknown option." << std::endl;
        } else if (token == "isready") {
            std::cout << "readyok" << std::endl;
        } else if (token == "ucinewgame") {
            board = chess::Board(chess::constants::STARTPOS);
        } else if (token == "position") {
            is >> std::skipws >> token;
            if (token == "startpos") {
                board = chess::Board(chess::constants::STARTPOS);
            } else if (token == "fen") {
                std::string fen;
                while (is >> std::skipws >> token && token != "moves") {
                    fen += token + " ";
                }
                fen.pop_back();
                board = chess::Board(fen);
            }
            while (is >> std::skipws >> token) {
                if (token != "moves") {
                    chess::Move move = chess::uci::uciToMove(board, token);
                    board.makeMove(move);
                }
            }
        } else if (token == "go") {
            if (info.stopped) {
                int max = 1000; // default depth
                info.duration = 0;
                info.stopped = false;
                while (is >> std::skipws >> token) {
                    if (token == "depth") {
                        is >> std::skipws >> max;
                        is >> std::skipws >> info.depth;
                    } else if (token == "wtime") {
                        is >> std::skipws >> wtime;
                    } else if (token == "btime") {
                        is >> std::skipws >> btime;
                    } else if (token == "movestogo") {
                        is >> std::skipws >> movestogo;
                    } else if (token == "infinite") {
                        info.infinite = true;
                        max = 1000;
                        info.depth = 1000;
                    } else if (token == "movetime") {
                        is >> std::skipws >> info.duration;
                    }
                }
                std::thread th1(&UCI::findMove, this, max);
                th1.detach();
            }
        } else if (token == "stop") {
            info.stopped = true;
        } else if (token == "d") {
            std::cout << board << std::endl;
        } else if (token == "quit") {
            break;
        } else {
            std::cout << "Unknown command: \"" << token << "\"" << std::endl;
        }
    }
    // Close book
    book.Clear();
}

void UCI::findMove(int max) {
    // Look for the best move in the Polyglot opening book 
    if (info.useOwnBook) {
        Reader::BookMoves book_moves = book.GetBookMoves(board.zobrist());
        if (book_moves.size()>0) {
            std::string book_move = Reader::ConvertBookMoveToUci(Reader::RandomBookMove(book_moves));
            std::cout << "bestmove " << book_move << std::endl;
            info.stopped = true;
            info.nodes = 0; 
            noOfMovesOutOfBook = 0; // reset counter if found a book move
            return;
        }
    }
    // Calculate time control for this move
    int moveTime = 0;
    if (info.duration==-1 && wtime>0 && btime>0) { // if movetime is not set
        moveTime = GetThinkingTime(board, wtime, btime, winc, binc, movestogo, noOfMovesOutOfBook);
        std::thread th2(&UCI::timer, this, moveTime);
        th2.detach();
    } else if (info.duration>0) { // if movetime is set
        moveTime = info.duration; 
        std::thread th2(&UCI::timer, this, moveTime);
        th2.detach();
    }
    // Normal search
    chess::Move best_move;
    chess::Move curr_best;
    for (int i=1; i<=max; i++) {
        curr_best = Search(board, i);
        if (info.stopped) {
            break;
        }
        best_move = curr_best;
        std::cout << "info depth " << i << " nodes " << info.nodes << " score cp " << best_move.score() / 100 << " pv " << best_move << std::endl;
    }
    std::cout << "bestmove " << chess::uci::moveToUci(best_move) << std::endl;
    noOfMovesOutOfBook++;
    info.stopped = true;
    info.nodes = 0;
}

void UCI::timer(int movetime) {
    std::this_thread::sleep_for(std::chrono::milliseconds(movetime));
    info.stopped = true;
}
