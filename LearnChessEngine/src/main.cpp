#include "bitboard.h"
#include "endgame.h"
#include "position.h"
#include "psqt.h"
#include "search.h"
#include "syzygy/tbprobe.h"
#include "thread.h"
#include "tt.h"
#include "uci.h"

#include "Board.h"
#include "Tests.h"

int main(int argc, char* argv[]) {
    Stockfish::CommandLine::init(argc, argv);
    Stockfish::UCI::init(Stockfish::Options);
    Stockfish::Tune::init();
    Stockfish::PSQT::init();
    Stockfish::Bitboards::init();
    Stockfish::Position::init();
    Stockfish::Bitbases::init();
    Stockfish::Endgames::init();
    Stockfish::Threads.set(size_t(12));
    Stockfish::Search::clear(); // After threads are up

    Tests::RunTests();

    Stockfish::Threads.set(0);
    return 0;
}