#pragma once
#include "types.h"

struct Move
{
    Move(const Stockfish::Move& move)
        : mStockfishMove(move)
        , mFromSquare(Stockfish::from_sq(move))
        , mToSquare(Stockfish::to_sq(move))
        , mTypeOfMove(Stockfish::type_of(move))
        , mPromotionType(Stockfish::promotion_type(move))
    {            

    }

    Stockfish::Move mStockfishMove;
    Stockfish::Square mFromSquare;
    Stockfish::Square mToSquare;
    Stockfish::MoveType mTypeOfMove;
    Stockfish::PieceType mPromotionType;
};
