#include "Tests.h"

#include "position.h"
#include "thread.h"

#include "Board.h"

namespace 
{
    Board _setupBoard(const std::string& fenString, const bool analyzeBoard = false)
    {
        return Board(fenString, analyzeBoard);
    }
}

void _legalMovesOfPawn()
{
    // pawn hasnt moved white
    {
        const Board board = _setupBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

        const size_t numMoves = board.numLegalMovesOfPiece(Stockfish::Square::SQ_E2);

        assert(numMoves == 2);
    }

    // pawn hasnt moved black
    {
        const Board board = _setupBoard("rnbqkbnr/pppppppp/8/8/8/4P3/PPPP1PPP/RNBQKBNR b KQkq - 0 1");

        const size_t numMoves = board.numLegalMovesOfPiece(Stockfish::Square::SQ_E7);

        assert(numMoves == 2);
    }

    // pawn has moved white
    {
        const Board board = _setupBoard("rnbqkbnr/pppp1ppp/4p3/8/8/4P3/PPPP1PPP/RNBQKBNR w KQkq - 0 1");

        const size_t numMoves = board.numLegalMovesOfPiece(Stockfish::Square::SQ_E3);

        assert(numMoves == 1);
    }

    // pawn has moved black
    {
        const Board board = _setupBoard("rnbqkbnr/pppp1ppp/4p3/8/8/4PP2/PPPP2PP/RNBQKBNR b KQkq - 0 1");

        const size_t numMoves = board.numLegalMovesOfPiece(Stockfish::Square::SQ_E6);

        assert(numMoves == 1);
    }
}

void _legalMovesPinned()
{
    // pawn pinned by queen
    {
        const Board board = _setupBoard("rnb1kbnr/pp1ppppp/2p5/q7/8/4P3/PPPP1PPP/RNBQKBNR w KQkq - 0 1");

        const size_t numMoves = board.numLegalMovesOfPiece(Stockfish::Square::SQ_D2);
        const bool isPiecePinned = board.isPiecePinned(Stockfish::Square::SQ_D2);

        assert(numMoves == 0);
        assert(isPiecePinned);
    }

    // knight pinned by queen diagonally
    {
        const Board board = _setupBoard("rnb1kbnr/pp1ppppp/2p5/q7/3P4/2N5/PPP1PPPP/R1BQKBNR w KQkq - 0 1");

        const size_t numMoves = board.numLegalMovesOfPiece(Stockfish::Square::SQ_C3);
        const bool isPiecePinned = board.isPiecePinned(Stockfish::Square::SQ_C3);

        assert(numMoves == 0);
        assert(isPiecePinned);
    }

    // bishop pinned by queen
    {
        const Board board = _setupBoard("rnb1kbnr/pp1ppppp/2p5/q7/3P4/8/PPPBPPPP/RN1QKBNR w KQkq - 0 1");

        const size_t numMoves = board.numLegalMovesOfPiece(Stockfish::Square::SQ_D2);
        const bool isPiecePinned = board.isPiecePinned(Stockfish::Square::SQ_D2);

        assert(numMoves == 3);
        assert(isPiecePinned);
    }

    // queen pinned by queen
    {
        const Board board = _setupBoard("rnb1kbnr/pp1ppppp/2p5/q7/3P4/8/PPPQPPPP/RNB1KBNR w KQkq - 0 1");

        const size_t numMoves = board.numLegalMovesOfPiece(Stockfish::Square::SQ_D2);
        const bool isPiecePinned = board.isPiecePinned(Stockfish::Square::SQ_D2);

        assert(numMoves == 3);
        assert(isPiecePinned);
    }

    // knight pinned by queen straight
    {
        const Board board = _setupBoard("rnb1kb1r/ppppqppp/8/2n1N3/8/5P1P/PPPP2P1/RNBQKB1R w KQkq - 0 1");

        const size_t numMoves = board.numLegalMovesOfPiece(Stockfish::Square::SQ_E5);
        const bool isPiecePinned = board.isPiecePinned(Stockfish::Square::SQ_E5);

        assert(numMoves == 0);
        assert(isPiecePinned);
    }

    // knight pinned by bishop. Bishop can only take knight that is pinned
    {
        const Board board = _setupBoard("rnbqk1nr/ppp2ppp/4p3/3p4/1b1PP3/2N5/PPP2PPP/R1BQKBNR w KQkq - 0 1");

        const size_t numMoves = board.numLegalMovesOfPiece(Stockfish::Square::SQ_C3);
        const bool isPiecePinned = board.isPiecePinned(Stockfish::Square::SQ_C3);
        const size_t numMovesOfTakenPiece = board.numLegalMovesForPiecesThePieceCanCapture(Stockfish::Square::SQ_B4); 

        assert(numMoves == 0);
        assert(isPiecePinned);
        assert(numMovesOfTakenPiece == 0);
    }
}

void _countLegalMoves()
{
    _legalMovesOfPawn();
    _legalMovesPinned();
}

void _captures()
{
    // en passant
    {
        const Board board = _setupBoard("rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 1");

        const size_t numMoves = board.numCapturesPossibleFromPiece(Stockfish::Square::SQ_E5);

        assert(numMoves == 1);
    }

    // contrived example where a knight can take 8 different pawns
    {
        const Board board = _setupBoard("4k3/8/2p1p3/1p3p2/3N4/1p3p2/2p1p3/4K3 w - - 0 1");

        const size_t numMoves = board.numCapturesPossibleFromPiece(Stockfish::Square::SQ_D4);

        assert(numMoves == 8);
    }

    // king is the only protection, but there are 2 attackers
    {
        const Board board = _setupBoard("rnbqkb1r/ppp2ppp/5n2/3pp3/4P3/4K1P1/PPPP1P1P/RNBQ1BNR b - - 0 1");

        const bool moveCapturesHangingPiece = board.moveCapturesHangingPiece(Stockfish::Square::SQ_D5, Stockfish::Square::SQ_E4);

        assert(moveCapturesHangingPiece);
    }
}

void _hanging()
{
    // Scandinavian opening, pawn is not hanging
    {
        const Board board = _setupBoard("rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 1");

        const bool isHanging = board.isPieceHanging(Stockfish::Square::SQ_D5);
        const bool moveCapturesHangingPiece = board.moveCapturesHangingPiece(Stockfish::Square::SQ_E4, Stockfish::Square::SQ_D5);

        assert(!isHanging);
        assert(!moveCapturesHangingPiece);
    }

    // simple pawn takes pawn
    {
        const Board board = _setupBoard("rnbqkbnr/ppp1pppp/8/3p4/4P3/7P/PPPP1PP1/RNBQKBNR b KQkq - 0 1");

        const bool isHanging = board.isPieceHanging(Stockfish::Square::SQ_E4);
        const bool moveCapturesHangingPiece = board.moveCapturesHangingPiece(Stockfish::Square::SQ_D5, Stockfish::Square::SQ_E4);

        assert(isHanging);
        assert(moveCapturesHangingPiece);
    }

    // pinned piece is the only protection
    {
        const Board board = _setupBoard("rnbqk1nr/ppp2ppp/4p3/3p4/1b1PP3/2N5/PPP2PPP/R1BQKBNR w KQkq - 0 1");

        const bool isHanging = board.isPieceHanging(Stockfish::Square::SQ_E4);

        assert(isHanging);
    }

    // king is the only protection, and there is only 1 attacker
    {
        const Board board = _setupBoard("rnbqkbnr/ppp2ppp/8/3pp3/4P3/4K3/PPPP1PPP/RNBQ1BNR b - - 0 1");

        const bool isHanging = board.isPieceHanging(Stockfish::Square::SQ_E4);
        const bool moveCapturesHangingPiece = board.moveCapturesHangingPiece(Stockfish::Square::SQ_D5, Stockfish::Square::SQ_E4);

        assert(!isHanging);
        assert(!moveCapturesHangingPiece);
    }

    // king is the only protection, but there are 2 attackers
    {
        const Board board = _setupBoard("rnbqkb1r/ppp2ppp/5n2/3pp3/4P3/4K1P1/PPPP1P1P/RNBQ1BNR b - - 0 1");

        const bool isHanging = board.isPieceHanging(Stockfish::Square::SQ_E4);
        const bool moveCapturesHangingPiece = board.moveCapturesHangingPiece(Stockfish::Square::SQ_D5, Stockfish::Square::SQ_E4);

        assert(isHanging);
        assert(moveCapturesHangingPiece);
    }
}

void _getBestMove()
{
    // 2 pieces can take a hanging queen, with no available checks. The knight should be preferred because it doesn't 
    // double up the but they both should be the top moves
    const Board board = _setupBoard("rnbqkb1r/pppp1p1p/5np1/4p2Q/8/4PP1P/PPPP2P1/RNB1KBNR b KQkq - 0 1", true);

    const bool isHanging = board.isPieceHanging(Stockfish::Square::SQ_H5);
    const std::vector<Move> bestMoves = board.getBestMoves(5);

    assert(isHanging);
    assert(bestMoves.size() == 5);
    assert(bestMoves[0].mFromSquare == Stockfish::Square::SQ_F6);
    assert(bestMoves[1].mFromSquare == Stockfish::Square::SQ_G6);
}

void _checksAndCaptures()
{
    const Board board = _setupBoard("rn1qkbnr/1ppbppp1/p6p/3pN3/4P1Q1/8/PPPP1PPP/RNB1KB1R w KQkq - 0 1");

    const std::vector<Move> checks = board.getAllCheckMoves();
    const std::vector<Move> captures = board.getAllCaptures();

    assert(checks.size() == 1);
    assert(captures.size() == 6);
    assert(checks[0].mFromSquare == Stockfish::Square::SQ_G4);
    assert(checks[0].mToSquare == Stockfish::Square::SQ_D7);
    assert(captures[0].mFromSquare == Stockfish::Square::SQ_F1);
    assert(captures[0].mToSquare == Stockfish::Square::SQ_A6);
    assert(captures[1].mFromSquare == Stockfish::Square::SQ_E4);
    assert(captures[1].mToSquare == Stockfish::Square::SQ_D5);
    assert(captures[2].mFromSquare == Stockfish::Square::SQ_G4);
    assert(captures[2].mToSquare == Stockfish::Square::SQ_D7);
    assert(captures[3].mFromSquare == Stockfish::Square::SQ_G4);
    assert(captures[3].mToSquare == Stockfish::Square::SQ_G7);
    assert(captures[4].mFromSquare == Stockfish::Square::SQ_E5);
    assert(captures[4].mToSquare == Stockfish::Square::SQ_D7);
    assert(captures[5].mFromSquare == Stockfish::Square::SQ_E5);
    assert(captures[5].mToSquare == Stockfish::Square::SQ_F7);
}

void Tests::RunTests()
{
    _countLegalMoves();
    _captures();
    _hanging();
    _getBestMove();
    _checksAndCaptures();
}
