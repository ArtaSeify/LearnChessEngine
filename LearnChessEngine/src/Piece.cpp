#include "Piece.h"

#include "types.h"

Piece::Piece(Stockfish::Color colour, Stockfish::PieceType type)
    : mColour(colour)
    , mType(type)
{
    
}

void Piece::addLegalMove(Move&& move)
{
    mLegalMoves.push_back(std::move(move));
}

const std::vector<Move>& Piece::getLegalMoves() const 
{
    return mLegalMoves;
}

Stockfish::PieceType Piece::pieceType() const
{
    return mType;
}

Stockfish::Color Piece::colour() const
{
    return mColour;
}

bool Piece::isPinned() const
{
    return mIsPinned;
}

bool Piece::isHanging() const
{
    return mIsHanging;
}

bool Piece::canMoveToSquare(const Stockfish::Square square) const
{
    return std::find_if(mLegalMoves.begin(), mLegalMoves.end(), [square](const Move& move) 
        {
            return square == move.mToSquare;
        }) != mLegalMoves.end();
}

void Piece::setIsPinned()
{
    mIsPinned = true;
}

void Piece::setIsHanging()
{
    mIsHanging = true;
}
