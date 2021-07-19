#pragma once
#include "CommonData.h"

#include "types.h"

class Piece
{
public:
    Piece(Stockfish::Color colour, Stockfish::PieceType type);

    void addLegalMove(Move&& move);
    const std::vector<Move>& getLegalMoves() const;
    Stockfish::PieceType pieceType() const;
    Stockfish::Color colour() const;

    bool isPinned() const;
    bool isHanging() const;
    bool canMoveToSquare(Stockfish::Square square) const;

    void setIsPinned();
    void setIsHanging();

private:
    Stockfish::Color mColour = Stockfish::Color::COLOR_NB;
    Stockfish::PieceType mType = Stockfish::PieceType::NO_PIECE_TYPE;
    bool mIsPinned = false;
    bool mIsHanging = false;

    std::vector<Move> mLegalMoves;
};

