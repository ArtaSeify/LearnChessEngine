#pragma once

#include "position.h"
#include "Piece.h"

#include <array>

class Board
{
public:
    Board(const std::string& fenString, bool analyzeBoard = true);

    size_t numLegalMovesOfPiece(Stockfish::Square square) const;
    size_t numCapturesPossibleFromPiece(Stockfish::Square square) const;
    bool isPiecePinned(Stockfish::Square square) const;
    bool isPieceHanging(Stockfish::Square square) const;
    size_t numLegalMovesForPiecesThePieceCanCapture(Stockfish::Square capturingPieceSquare) const;

    // NOTE: only the first 5 moves are ordered properly. Please see _analyzeBoard() for the reason and a potential better solution
    std::vector<Move> getBestMoves(size_t numMoves) const;

    // per Levy Rozman, you should always look for checks then captures first!
    std::vector<Move> getAllCheckMoves() const;
    std::vector<Move> getAllCaptures() const;

    bool moveCapturesHangingPiece(Stockfish::Square from, Stockfish::Square to) const;
    bool isWinningCaptureStaticExchangeEvaluation(Stockfish::Square from, Stockfish::Square to) const;

private:
    void _analyzeBoard();
    void _initializePieces();
    void _initializePinnedPieces();
    void _initializeHangingPieces();
    void _generateLegalMoves();

    std::vector<Move> _allPossibleMoves() const;
    int _squareToIndex(Stockfish::Square square) const;

    // [ from, to ]
    std::vector<std::pair<Stockfish::Square, Stockfish::Square>> _getAttackersFromAndToSquare(Stockfish::Bitboard& attackingPieces, Stockfish::Square attackSquare, Stockfish::Color attackingColour) const;
    bool _checkIfOnePieceCanDefendSquare(Stockfish::Bitboard& defendingPieces, Stockfish::Square attackingFromSquare, Stockfish::Square defendingSquare, Stockfish::Color defensiveColour) const;

    Stockfish::Position mRawBoard;
    Stockfish::StateListPtr mStateList;
    std::array<std::optional<Piece>, 64> mBoard;
    std::vector<Move> mOrderedMoves;
};

