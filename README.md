# Firestorm-Chess

<img alt="Firestorm-Logo" src="https://github.com/deoshreyas/Firestorm-Chess/blob/main/Firestorm-Logo.png" height="500px">

## :zap: About
Firestorm is a Chess Engine written in C++ that follows the UCI (Universal Chess Interface) Protocol. This is my first _real_ attempt at creating atleast a basic chess program. It supports a wide variety of commands and has its own book to play in the openings. You can use it as it is (as an executable) or pair it with a Chess GUI.

## :books: Supported Commands
- Basic processing and identification commands
- The `go` command supports time controls using `wtime`, `winc`, `btime`, `binc`, `movestogo` or `movetime`
- The `go` command also supports `infinite` search (use `stop` when needed)
- The `go` command also supports `depth` only searches (use `stop` when needed)
- Following options are supported using `setoption`:
  - `OwnBook`: Engine can use its own opening book (default = true)
  - `NullMove`: Engine can use Null Move pruning (default = true)
  - `Clear Hash`: Clear the Transposition Table
  - `Hash`: Set the size of the Transposition Table in mb (default = 64)
- The `d` command can be used to print a representation of the current board state whenever needed

## :star: Features
- A robust and efficient evaluation function that recognizes game phases, pawn structures, piece-square tables, etc.
- Supports time management
- Uses the `komodo.bin` opening book to play well (although randomly (to make games interesting)) in the opening
- The NegaMax algorithm for searching along with Alpha-Beta pruning
- A simple transposition table with an "always-replace" scheme
- Basic Null Move pruning
- Basic Move Orderig that recognizes TT Moves, the MVV-LVA heuristic and promotions

## :sparkles: Gratitude
- @Disservin for creating the [C++ Chess library](https://github.com/disservin/chess-library/) which helped me focus on the algorithms and heuristics instead of worrying (too much) about things like move generation, zobrist hashing, etcetera
- The [Chess Programming Wiki](https://www.chessprogramming.org/Main_Page) for being such a brilliant source of beginner, intermediate and advanced Chess Engine concepts
- [Sebastian Lague](https://www.youtube.com/c/SebastianLague) on YouTube for inspiring me to attempt to create my own Chess Engine
- The creators of the Komodo chess engine for the `komodo.bin` Polyglot Chess Opening Book which Firestorm currently uses
