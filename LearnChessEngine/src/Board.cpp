#include "Board.h"

#include "CommonData.h"

#include "movegen.h"
#include "search.h"
#include "thread.h"
#include "uci.h"

namespace 
{
    constexpr std::array<Stockfish::Color, 2> colours = { Stockfish::Color::WHITE, Stockfish::Color::BLACK };
}

Board::Board(const std::string& fenString, const bool analyzeBoard)
    : mBoard()
    , mOrderedMoves()
{
    mStateList = std::make_unique<std::deque<Stockfish::StateInfo>>(std::deque<Stockfish::StateInfo>(1));
    mRawBoard.set(fenString, false, &mStateList->back(), Stockfish::Threads.main());

    _initializePieces();
    _generateLegalMoves();
    if (analyzeBoard)
    {
        _analyzeBoard();
    }
    _initializePinnedPieces();
    _initializeHangingPieces();
}

size_t Board::numLegalMovesOfPiece(const Stockfish::Square square) const 
{
    if (const std::optional<Piece>& piece = mBoard[_squareToIndex(square)])
    {
        return piece->getLegalMoves().size();
    }

    return 0;
}

size_t Board::numCapturesPossibleFromPiece(const Stockfish::Square square) const
{
    size_t captures = 0;
    if (const std::optional<Piece>& piece = mBoard[_squareToIndex(square)])
    {
        const std::vector<Move>& moves = piece->getLegalMoves();
        for (const Move& move : moves)
        {
            if (mRawBoard.capture(move.mStockfishMove))
            {
                ++captures;
            }
        }
    }

    return captures;
}

bool Board::isPiecePinned(const Stockfish::Square square) const 
{
    if (const std::optional<Piece>& piece = mBoard[_squareToIndex(square)])
    {
        return piece->isPinned();
    }

    return false;
}

bool Board::isPieceHanging(const Stockfish::Square square) const
{
    if (const std::optional<Piece>& piece = mBoard[_squareToIndex(square)])
    {
        return piece->isHanging();
    }

    return false;
}

size_t Board::numLegalMovesForPiecesThePieceCanCapture(const Stockfish::Square capturingPieceSquare) const
{
    size_t numLegalMoves = 0;
    if (const std::optional<Piece>& capturingPiece = mBoard[_squareToIndex(capturingPieceSquare)])
    {
        const std::vector<Move>& capturingPieceMoves = capturingPiece->getLegalMoves();
        for (const Move& move : capturingPieceMoves)
        {
            if (const std::optional<Piece>& capturedPiece = mBoard[_squareToIndex(move.mToSquare)])
            {
                numLegalMoves += capturedPiece->getLegalMoves().size();
            }
        }
    }

    return numLegalMoves;
}

std::vector<Move> Board::getBestMoves(size_t numMoves) const
{
    numMoves = std::min(numMoves, mOrderedMoves.size());
    std::vector bestMoves(mOrderedMoves.begin(), mOrderedMoves.begin() + numMoves);

    return bestMoves;
}

std::vector<Move> Board::getAllCheckMoves() const
{
    std::vector<Move> checkMoves;
    const std::vector<Move> possibleMoves = _allPossibleMoves();

    for (const Move& move : possibleMoves)
    {
        if (mRawBoard.gives_check(move.mStockfishMove))
        {
            checkMoves.emplace_back(move);
        }
    }

    return checkMoves;
}

std::vector<Move> Board::getAllCaptures() const
{
    std::vector<Move> captureMoves;
    const std::vector<Move> possibleMoves = _allPossibleMoves();

    const Stockfish::Color currentPlayer = mRawBoard.side_to_move();

    for (const Move& move : possibleMoves)
    {
        if (const std::optional<Piece>& capturedPiece = mBoard[_squareToIndex(move.mToSquare)])
        {
            assert(capturedPiece->colour() == ~currentPlayer);
            captureMoves.emplace_back(move);
        }
    }

    return captureMoves;
}

bool Board::moveCapturesHangingPiece(const Stockfish::Square from, const Stockfish::Square to) const
{
    const std::optional<Piece>& pieceAttacking = mBoard[_squareToIndex(from)];
    const std::optional<Piece>& pieceAttacked = mBoard[_squareToIndex(to)];
    if (!pieceAttacking || !pieceAttacked)
    {
        return false;
    }

    if (pieceAttacking->colour() != mRawBoard.side_to_move() || !pieceAttacking->canMoveToSquare(to))
    {
        return false;
    }

    return pieceAttacked->isHanging();
}

bool Board::isWinningCaptureStaticExchangeEvaluation(const Stockfish::Square from, const Stockfish::Square to) const
{
    const Stockfish::Move move = Stockfish::make_move(from, to);

    return mRawBoard.see_ge(move);
}

void Board::_analyzeBoard()
{
    // the search is hard coded to find principal variation of 5 moves. This could be a variable, and a new search initiated each time
    // we want more "best moves" than all previous times. For simplicity, I just left it as 5 so it's not so slow. 
    Stockfish::Search::LimitsType limits;
    limits.depth = 12; // depth 12 so it's quick to iterate
    limits.nodes = std::numeric_limits<uint64_t>::max();

    Stockfish::Threads.start_thinking(mRawBoard, mStateList, limits, false);
    Stockfish::Threads.main()->wait_for_search_finished();
    const std::vector<Stockfish::Search::RootMove>& moves = Stockfish::Threads.main()->rootMoves;

    for (const Stockfish::Search::RootMove& rootMove : moves)
    {
        assert(!rootMove.pv.empty());
        if (!rootMove.pv.empty())
        {
            mOrderedMoves.emplace_back(rootMove.pv[0]);
        }
    }
}

void Board::_initializePieces()
{
    for (const Stockfish::Color colour : colours)
    {
        Stockfish::Bitboard pieces = mRawBoard.pieces(colour);
        while (pieces)
        {
            const Stockfish::Square square = Stockfish::pop_lsb(pieces);
            const Stockfish::PieceType pieceType = Stockfish::type_of(mRawBoard.piece_on(square));
            mBoard[_squareToIndex(square)] = Piece(colour, pieceType);
        }
    }
}

void Board::_initializePinnedPieces()
{
    for (const Stockfish::Color colour : colours)
    {
        Stockfish::Bitboard pinnedPieces = mRawBoard.blockers_for_king(colour) & mRawBoard.pieces(colour);
        while (pinnedPieces)
        {
            const Stockfish::Square square = Stockfish::pop_lsb(pinnedPieces);

            std::optional<Piece>& piece = mBoard[_squareToIndex(square)];
            assert(piece);

            if (piece)
            {
                piece->setIsPinned();
            }
        }
    }
}

void Board::_initializeHangingPieces()
{
    for (const Stockfish::Color attackingColour : colours)
    {
        const Stockfish::Color defendingColour = ~attackingColour;
        Stockfish::Bitboard pieces = mRawBoard.pieces();

        while (pieces)
        {
            const Stockfish::Square square = Stockfish::pop_lsb(pieces);
            if (attackingColour == Stockfish::color_of(mRawBoard.piece_on(square)))
            {
                continue;
            }

            const Stockfish::Bitboard piecesAttackingSquare = mRawBoard.attackers_to(square);
            Stockfish::Bitboard attackingPieces = piecesAttackingSquare & mRawBoard.pieces(attackingColour);
            // en passsant special case is not handled by default..
            if (mRawBoard.ep_square() != Stockfish::SQ_NONE)
            {
                // Only consider the move when its applicable
                if ((attackingColour == Stockfish::Color::WHITE && mRawBoard.ep_square() == Stockfish::Square(square + 8))
                    || (attackingColour == Stockfish::Color::BLACK && mRawBoard.ep_square() == Stockfish::Square(square - 8)))
                {
                    attackingPieces |= mRawBoard.pieces(attackingColour) & Stockfish::pawn_attacks_bb(defendingColour, mRawBoard.ep_square());
                }
            }

            const std::vector<std::pair<Stockfish::Square, Stockfish::Square>> attackersFromToSquare = _getAttackersFromAndToSquare(attackingPieces, square, attackingColour);

            bool isBeingDefended = true;
            for (const auto& [attackerFromSquare, attackerToSquare] : attackersFromToSquare)
            {
                Stockfish::Bitboard defendingPieces = mRawBoard.attackers_to(attackerToSquare) & mRawBoard.pieces(defendingColour);
                isBeingDefended = isBeingDefended && _checkIfOnePieceCanDefendSquare(defendingPieces, attackerFromSquare, attackerToSquare, defendingColour);
            }

            if (!attackersFromToSquare.empty() && !isBeingDefended)
            {
                std::optional<Piece>& piece = mBoard[_squareToIndex(square)];
                assert(piece);

                if (piece)
                {
                    piece->setIsHanging();
                }
            }
        }
    }
}

void Board::_generateLegalMoves()
{
    constexpr int NUM_PLAYERS = 2; // need to generate moves for both sides
    for (int i = 0; i < NUM_PLAYERS; ++i)
    {
        const Stockfish::MoveList<Stockfish::GenType::LEGAL> moveList(mRawBoard);

        for (const Stockfish::ExtMove* extMove = moveList.begin(); extMove != moveList.end(); ++extMove)
        {
            const Stockfish::Move stockfishMove = extMove->move;

            Move legalMove(stockfishMove);

            std::optional<Piece>& piece = mBoard[_squareToIndex(legalMove.mFromSquare)];
            assert(piece);

            if (piece)
            {
                piece->addLegalMove(std::move(legalMove));
            }
        }

        if (i != NUM_PLAYERS - 1)
        {
            Stockfish::StateInfo state;
            mRawBoard.do_null_move(state); // we do a null move to change the players turn so we can generate the moves for the other player
        }
    }

    mRawBoard.undo_null_move(); // have to undo
}

std::vector<Move> Board::_allPossibleMoves() const
{
    std::vector<Move> possibleMoves;

    for (const std::optional<Piece>& piece : mBoard)
    {
        if (piece && piece->colour() == mRawBoard.side_to_move())
        {
            const std::vector<Move>& moves = piece->getLegalMoves();
            std::copy(moves.begin(), moves.end(), std::back_inserter(possibleMoves));
        }
    }

    return possibleMoves;
}

int Board::_squareToIndex(const Stockfish::Square square) const
{
    return static_cast<int>(square);
}

std::vector<std::pair<Stockfish::Square, Stockfish::Square>> Board::_getAttackersFromAndToSquare(Stockfish::Bitboard& attackingPieces, const Stockfish::Square attackSquare, const Stockfish::Color attackingColour) const
{
    std::vector<std::pair<Stockfish::Square, Stockfish::Square>> attackersFromToSquare;

    while (attackingPieces)
    {
        const Stockfish::Square attackingPieceSquare = Stockfish::pop_lsb(attackingPieces);

        const std::optional<Piece>& attackingPiece = mBoard[_squareToIndex(attackingPieceSquare)];
        assert(attackingPiece);

        if (attackingPiece)
        {
            Stockfish::Square movingToSquare = attackSquare;

            // handle en passant separately
            if (attackingPiece->pieceType() == Stockfish::PieceType::PAWN)
            {
                if (attackingColour == Stockfish::Color::WHITE && Stockfish::rank_of(attackingPieceSquare) == Stockfish::Rank::RANK_5)
                {
                    movingToSquare = Stockfish::Square(attackSquare + 8);
                }
                else if (attackingColour == Stockfish::Color::BLACK && Stockfish::rank_of(attackingPieceSquare) == Stockfish::Rank::RANK_3)
                {
                    movingToSquare = Stockfish::Square(attackSquare - 8);
                    
                }
            }

            if (attackingPiece->canMoveToSquare(movingToSquare))
            {
                attackersFromToSquare.emplace_back(attackingPieceSquare, movingToSquare);
            }
        }
    }

    return attackersFromToSquare;
}

bool Board::_checkIfOnePieceCanDefendSquare(Stockfish::Bitboard& defendingPieces, const Stockfish::Square attackingFromSquare, const Stockfish::Square defendingSquare, const Stockfish::Color defensiveColour) const
{
    while (defendingPieces)
    {
        const Stockfish::Square defendingPieceSquare = Stockfish::pop_lsb(defendingPieces);

        const std::optional<Piece>& defendingPiece = mBoard[_squareToIndex(defendingPieceSquare)];
        assert(defendingPiece);

        Stockfish::Bitboard kingBoard = mRawBoard.pieces(defensiveColour, Stockfish::PieceType::KING);
        const Stockfish::Square kingSquare = Stockfish::pop_lsb(kingBoard);

        if (defendingPiece)
        {
            // if piece is pinned, the target square must be aligned with the king for the piece to be able to move
            if (defendingPiece->isPinned() && !Stockfish::aligned(defendingSquare, defendingPieceSquare, kingSquare))
            {
                return false;
            }

            // if king is defending, then the piece it has to attack should not have any defenders after it has taken the piece
            if (defendingPieceSquare == kingSquare)
            {
                const Stockfish::Bitboard piecesAttackingSquareWithCurrentAttackingPieceMissing = mRawBoard.attackers_to(defendingSquare, mRawBoard.pieces(Stockfish::ALL_PIECES) & ~Stockfish::square_bb(attackingFromSquare));
                const Stockfish::Bitboard defendersOfAttackingPieceAfterItTakes = mRawBoard.pieces(~defensiveColour) & ~Stockfish::square_bb(attackingFromSquare);
                const Stockfish::Bitboard defendedBoard = piecesAttackingSquareWithCurrentAttackingPieceMissing & defendersOfAttackingPieceAfterItTakes;

                return Stockfish::popcount(defendedBoard) == 0;
            }

            // any other piece attacking this square can take it!
            return true;
        }
    }

    return false;
}
