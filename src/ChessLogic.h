#include <string>
#include <vector>
#include <algorithm>

enum class pieces {
	NONE = 0,
	PAWN = 1,
	KNIGHT = 2,
	BISHOP = 3,
	ROOK = 4,
	KING = 5,
	QUEEN = 6
};
enum class colors {
	NONE = 0,
	WHITE = 1,
	BLACK = 2
};

enum class states {
	NORMAL = 0,
	CHECK = 1,
	CHECKMATE = 2,
	STALEMATE = 3
};

class Board {

public:
	struct Piece {
		pieces currPiece = pieces::NONE;
		colors currColor = colors::NONE;

		bool isEmpty() const { return currColor == colors::NONE; }
	};

	struct Move {
		int row = -1;
		int col = -1;
	};

	/* Board Layout by index */
	// ===============================
	// 0   1   2   3   4   5   6   7
	// 8   9   10  11  12  13  14  15
	// 16  17  18  19  20  21  22  23
	// 24  25  26  27  28  29  30  31
	// 32  33  34  35  36  37  38  39
	// 40  41  42  43  44  45  46  47
	// 48  49  50  51  52  53  54  55
	// 56  57  58  59  60  61  62  63
	// ==============================
	Piece board[64];

	std::vector<Piece> capturedByWhite;   // black pieces White has taken
	std::vector<Piece> capturedByBlack;   // white pieces Black has taken

	colors sideToMove = colors::WHITE;
	states currentGameState = states::NORMAL;

	bool promotionPending = false;

	bool wKingMoved = false, bKingMoved = false;
	bool wRook0Moved = false, wRook7Moved = false;
	bool bRook0Moved = false, bRook7Moved = false;


	int promoRow = -1;
	int promoCol = -1;

	int enPassantRow = -1;
	int enPassantCol = -1;

	bool gameOver = false;

	// --------------------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------------------
	// Converts a column and row to an index in the piecePos array
	static int index(int col, int row) { 
		return row * 8 + col; 
	}
	// --------------------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------------------
	// Returns a reference to the piece at the given column and row
<<<<<<< Updated upstream
	Piece& at(int col, int row) { 
		return piecePos[index(col, row)]; 
=======
	Piece& at(int col, int row) {
		return board[index(col, row)];
>>>>>>> Stashed changes
	}
	// --------------------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------------------
	// Returns a const reference to the piece at the given column and row
<<<<<<< Updated upstream
	const Piece& at(int col, int row) const { 
		return piecePos[index(col, row)]; 
	}
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	void changePos() {

=======
	const Piece& at(int col, int row) const {
		return board[index(col, row)];
>>>>>>> Stashed changes
	}
	// --------------------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------------------
	bool canPickUp(int col, int row) const {
		if (promotionPending || gameOver) return false;
		const Piece& p = at(col, row);
		return !p.isEmpty() && p.currColor == sideToMove;
	}
	// --------------------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------------------
	/* Checks if particular piece is allowed to move */
	//
	// Validates that the piece lands on the board, on opposite color, or no color.
	// Then, checks if en passant so board is updated correctly before validating move.
	// Creates a copy of the current instance of the board (Board copy = *this), and moves
	// the piece on the copy (removing en passant pawn if it was en passant), and 
	// calls the move check depending on which piece is selected.
	// returns true if move allowed, false if not.
	bool isMoveAllowed(int fromCol, int fromRow, int toCol, int toRow) const {
		if (fromCol == toCol && fromRow == toRow) return false;
		const Piece& from = at(fromCol, fromRow);
		const Piece& to = at(toCol, toRow);
		if (from.isEmpty() || from.currColor != sideToMove) return false;
		if (to.currColor == from.currColor) return false;
<<<<<<< Updated upstream
		return true;   // TODO: per-piece movement, check, castling, en passant, promotion
	}
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	bool movePiece(int fromCol, int fromRow, int toCol, int toRow) {
		if (!isMoveAllowed(fromCol, fromRow, toCol, toRow)) return false;
		at(toCol, toRow) = at(fromCol, fromRow);
		at(fromCol, fromRow) = Piece{};
		sideToMove = (sideToMove == colors::WHITE) ? colors::BLACK : colors::WHITE;
		return true;
=======

		bool enPassanted = at(fromCol, fromRow).currPiece == pieces::PAWN
			&& (std::abs(toCol - fromCol) == 1)
			&& (at(toCol, toRow).isEmpty());

		Board copy = *this;
		copy.at(toCol, toRow) = copy.at(fromCol, fromRow);
		copy.at(fromCol, fromRow) = Piece{};

		if (enPassanted) copy.at(toCol, fromRow) = Piece{};
		if (copy.isInCheck(copy.at(toCol, toRow).currColor)) return false;

		switch (from.currPiece) {
		case pieces::ROOK:
			return rookMoveCheck(fromCol, fromRow, toCol, toRow);
		case pieces::KNIGHT:
			return knightMoveCheck(fromCol, fromRow, toCol, toRow);
		case pieces::BISHOP:
			return bishopMoveCheck(fromCol, fromRow, toCol, toRow);
		case pieces::KING:
			return kingMoveCheck(fromCol, fromRow, toCol, toRow);
		case pieces::QUEEN:
			return queenMoveCheck(fromCol, fromRow, toCol, toRow);
		case pieces::PAWN:
			return pawnMoveCheck(fromCol, fromRow, toCol, toRow);
		default:
			return false;
		}
		return true;
	}
	// --------------------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------------------
	// Moves the piece/s to the selected square if (isMoveAllowed)
	// First, checks if move was en passant, if so enPassanted = true,
	// then I check if the last move was a double pawn move (enPassantPossible?)
	// if it was, I set the enPassantCol, and enPassantRow so that pawnMoveCheck
	// returns true when the current pawn is in correct position, and last pawn move
	// was a double. Then get a refrence to the square the piece is moving to, if the
	// piece is capturing another then add it to the respective captured Pieces vector,
	// and re-sort the vector. Update rook flags

	bool movePiece(int fromCol, int fromRow, int toCol, int toRow) {
		if (!isMoveAllowed(fromCol, fromRow, toCol, toRow)) return false;

		bool enPassanted = at(fromCol, fromRow).currPiece == pieces::PAWN 
						  && (std::abs(toCol - fromCol) == 1) 
						  && (at(toCol, toRow).isEmpty());

		enPassantCol = -1;
		enPassantRow = -1;


		if (at(fromCol, fromRow).currPiece == pieces::PAWN && std::abs(fromRow - toRow) == 2) {
			enPassantCol = (fromCol + toCol) / 2;
			enPassantRow = (fromRow + toRow) / 2;
		}
		const Piece& target = at(toCol, toRow);

		if (!target.isEmpty()) { // Check if piece will be captured, if so add to captured pieces
			(sideToMove == colors::WHITE ? capturedByWhite : capturedByBlack).push_back(target);
			sortCapturedPieces(capturedByWhite, capturedByBlack);
		}

		updateRookFlags(fromCol, fromRow);
		updateRookFlags(toCol, toRow);

		bool isCastle = at(fromCol, fromRow).currPiece == pieces::KING
			&& std::abs(toCol - fromCol) == 2;

		if (at(fromCol, fromRow).currPiece == pieces::KING) {
			if (at(fromCol, fromRow).currColor == colors::WHITE) wKingMoved = true;
			else                                                 bKingMoved = true;
		}

		at(toCol, toRow) = at(fromCol, fromRow); // Move piece
		at(fromCol, fromRow) = Piece{};

		if (enPassanted) {						 // Remove the pawn from square adjacent to pawn
			(sideToMove == colors::WHITE ? capturedByWhite : capturedByBlack).push_back(at(toCol, fromRow));
			at(toCol, fromRow) = Piece{};
			sortCapturedPieces(capturedByWhite, capturedByBlack);
		}

		if (isCastle) {                          // Bring the rook across
			int dir = (toCol > fromCol) ? 1 : -1;
			int rookFrom = (dir > 0) ? 7 : 0;
			int rookTo = toCol - dir;
			at(rookTo, toRow) = at(rookFrom, toRow);
			at(rookFrom, toRow) = Piece{};
		}

		sideToMove = (sideToMove == colors::WHITE) ? colors::BLACK : colors::WHITE;

		if ((at(toCol, toRow).currPiece == pieces::PAWN && at(toCol, toRow).currColor == colors::WHITE && toRow == 7)
		 || (at(toCol, toRow).currPiece == pieces::PAWN && at(toCol, toRow).currColor == colors::BLACK && toRow == 0))
		{
			promotionPending = true;
			promoCol = toCol;
			promoRow = toRow;
		}

		updateGameState();
		return true;

	}
	// --------------------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------------------
	// Checks whether or not a piece attacks a particular square given its location and enemy color
	bool isSquareAttacked(int col, int row, colors byColor) const {
		for (int r = 0; r < 8; r++) {
			for (int c = 0; c < 8; c++) {
				const Piece& p = at(c, r);
				if (p.currColor != byColor || (c == col && r == row)) continue;

				int dc = col - c;
				int dr = row - r;
				switch (p.currPiece) {
				case pieces::PAWN: {
					int forward = (byColor == colors::WHITE) ? 1 : -1;   // white moves up
					if (dr == forward && std::abs(dc) == 1) return true; // diagonal only
					break;
				}
				case pieces::KNIGHT: if (knightMoveCheck(c, r, col, row)) return true; break;
				case pieces::BISHOP: if (bishopMoveCheck(c, r, col, row)) return true; break;
				case pieces::ROOK:   if (rookMoveCheck(c, r, col, row))   return true; break;
				case pieces::QUEEN:  if (queenMoveCheck(c, r, col, row))  return true; break;
				case pieces::KING:
					if (std::abs(dc) <= 1 && std::abs(dr) <= 1) return true;
					break;
				default: break;
				}
			}
		}
		return false;
	}
	// --------------------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------------------
	bool isInCheck(colors side) const {
		int col = -1, row = -1;
		for (int i = 0; i < 64; i++) {
			if (board[i].currColor != side) continue;
			else if (board[i].currPiece != pieces::KING) continue;
			else {
				col = i % 8;
				row = i / 8;
			}
		}
		if (col == -1 || row == -1) return false;
		return (side == colors::WHITE) ? (isSquareAttacked(col, row, colors::BLACK))
			: (isSquareAttacked(col, row, colors::WHITE));
	}
	// --------------------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------------------
	const std::vector<Move> generateMoves(int col, int row) const {
		std::vector<Move> moveList;
		Move daMove;
		for (int r = 0; r < 8; r++) {
			for (int c = 0; c < 8; c++) {
				if (isMoveAllowed(col, row, c, r)) {
					daMove.col = c;
					daMove.row = r;
					moveList.push_back(daMove);
				}
			}
		}
		return moveList;
	}
	// --------------------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------------------
	bool knightMoveCheck(int fromCol, int fromRow, int toCol, int toRow) const {
		int colDiff = std::abs(toCol - fromCol);
		int rowDiff = std::abs(toRow - fromRow);
		return (colDiff == 2 && rowDiff == 1) || (colDiff == 1 && rowDiff == 2);
	}
	// --------------------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------------------
	bool rookMoveCheck(int fromCol, int fromRow, int toCol, int toRow) const {
		if (fromCol != toCol && fromRow != toRow) return false;  // not straight
		int colStep = (toCol > fromCol) ? 1 : (toCol < fromCol) ? -1 : 0;
		int rowStep = (toRow > fromRow) ? 1 : (toRow < fromRow) ? -1 : 0;
		int col = fromCol + colStep;
		int row = fromRow + rowStep;
		while (col != toCol || row != toRow) {
			if (!at(col, row).isEmpty()) return false;   // blocked
			col += colStep;
			row += rowStep;
		}
		return true;
	}
	// --------------------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------------------
	bool bishopMoveCheck(int fromCol, int fromRow, int toCol, int toRow) const {
		if (!(std::abs(toCol - fromCol) == std::abs(toRow - fromRow))) return false;
		int colStep = (toCol > fromCol) ? 1 : -1;
		int rowStep = (toRow > fromRow) ? 1 : -1;
		int col = fromCol + colStep;
		int row = fromRow + rowStep;

		while (col != toCol && row != toRow) {
			if (!at(col, row).isEmpty()) return false;   // blocked
			col += colStep;
			row += rowStep;
		}
		return true;
	}
	// --------------------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------------------
	bool kingMoveCheck(int fromCol, int fromRow, int toCol, int toRow) const {
		int colDiff = std::abs(toCol - fromCol);
		int rowDiff = std::abs(toRow - fromRow);
		if (rowDiff <= 1 && colDiff <= 1) return true;                 // normal step
		if (rowDiff == 0 && colDiff == 2) return canCastle(fromCol, fromRow, toCol);
		return false;
	}
	// --------------------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------------------
	// Returns true if only two kings are left on the board (Stalemate)
	bool justTwoKings() {
		for (int r = 0; r < 8; r++) {
			for (int c = 0; c < 8; c++) {
				if (at(c, r).currPiece != pieces::KING && at(c, r).currPiece != pieces::NONE) return false;
			}
		}
		return true;
	}
	// --------------------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------------------
	bool canCastle(int kingCol, int row, int toCol) const {
		colors us = at(kingCol, row).currColor;
		colors them = (us == colors::WHITE) ? colors::BLACK : colors::WHITE;
		int dir = (toCol > kingCol) ? 1 : -1;
		int rookCol = (dir > 0) ? 7 : 0;

		if (us == colors::WHITE ? wKingMoved : bKingMoved) return false;
		if (rookHasMoved(us, rookCol)) return false;

		const Piece& rook = at(rookCol, row);
		if (rook.currPiece != pieces::ROOK || rook.currColor != us) return false;

		// Every square between king and rook must be empty
		for (int c = kingCol + dir; c != rookCol; c += dir)
			if (!at(c, row).isEmpty()) return false;

		// King can't castle out of, through, or into check
		for (int c = kingCol; c != toCol + dir; c += dir)
			if (isSquareAttacked(c, row, them)) return false;

		return true;
	}
	// --------------------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------------------
	bool pawnMoveCheck(int fromCol, int fromRow, int toCol, int toRow) const {
		if (std::abs(fromCol - toCol) > 1 || std::abs(fromRow - toRow) > 2) return false;
		// Logic for first move
		if ((sideToMove == colors::WHITE)) {
			if (toCol == fromCol && fromRow == 1) {
				int temp = toRow - fromRow;
				if (temp != 1 && temp != 2) return false;
				for (int row = fromRow + 1; row <= toRow; row++) {
					if (!at(toCol, row).isEmpty()) return false;   // blocked
				}
				return true;
			}
			else if (toCol == fromCol) {
				return (toRow - fromRow == 1 && at(toCol, toRow).isEmpty()) ? true : false;
			}
			else if (std::abs(toCol - fromCol) == 1 && (toRow - fromRow == 1)) {
				if (!at(toCol, toRow).isEmpty() || (enPassantCol == toCol && enPassantRow == toRow)) return true;
				return false;
			}
			return false;
		}
		else if ((sideToMove == colors::BLACK)) {
			if (toCol == fromCol && fromRow == 6) {
				int temp = toRow - fromRow;
				if (temp != -1 && temp != -2) return false;
				for (int row = fromRow - 1; row >= toRow; row--) {
					if (!at(toCol, row).isEmpty()) return false;   // blocked
				}
				return true;
			}
			else if (toCol == fromCol) {
				return (toRow - fromRow == -1 && at(toCol, toRow).isEmpty()) ? true : false;
			}
			else if (std::abs(toCol - fromCol) == 1 && (toRow - fromRow == -1)) {
				if (!at(toCol, toRow).isEmpty() || (enPassantCol == toCol && enPassantRow == toRow)) return true;
				return false;
			}
			return false;
		}
		return false;
	}
	// --------------------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------------------
	bool queenMoveCheck(int fromCol, int fromRow, int toCol, int toRow) const {
		return (rookMoveCheck(fromCol, fromRow, toCol, toRow) || bishopMoveCheck(fromCol, fromRow, toCol, toRow)) ? true : false;
	}
	// --------------------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------------------
	void promote(pieces choice) {
		if (!promotionPending) return;
		at(promoCol, promoRow).currPiece = choice;
		promotionPending = false;
		updateGameState();
>>>>>>> Stashed changes
	}
	// --------------------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------------------
	// Rook flags are updated when any piece has a valid move to the square the rook is on, or away
	// from the square a rook is on. This covers rooks moving, and another piece capturing a rook.
	const void updateRookFlags(int col, int row) {
		if (row == 0 && col == 0) wRook0Moved = true;  // a1: White queenside rook
		if (row == 0 && col == 7) wRook7Moved = true;  // h1: White kingside rook
		if (row == 7 && col == 0) bRook0Moved = true;  // a8: Black queenside rook
		if (row == 7 && col == 7) bRook7Moved = true;  // h8: Black kingside rook
	}
	// --------------------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------------------
	bool rookHasMoved(colors side, int rookCol) const {
		if (side == colors::WHITE) return rookCol == 0 ? wRook0Moved : wRook7Moved;
		return rookCol == 0 ? bRook0Moved : bRook7Moved;
	}
	// --------------------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------------------
	void updateGameState() {
		int legalCount = 0; // Counts legal moves, and updates game's state accordingly
		for (int r = 0; r < 8; r++) {
			for (int c = 0; c < 8; c++) {
				if (at(c, r).currColor != sideToMove) continue;
				legalCount += generateMoves(c, r).size();
			}
		}
		if (legalCount == 0) {
			currentGameState = isInCheck(sideToMove) ? states::CHECKMATE : states::STALEMATE;
			gameOver = true;
		}
		else {
			currentGameState = (isInCheck(sideToMove)) ? states::CHECK : states::NORMAL;
			gameOver = false;
		}
		if (justTwoKings()) {
			currentGameState = states::STALEMATE;
			gameOver = true;
		}
	}
	// --------------------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------------------
	static int pieceValue(pieces p) {
		switch (p) {
		case pieces::QUEEN:  return 9;
		case pieces::ROOK:   return 5;
		case pieces::BISHOP: return 3;
		case pieces::KNIGHT: return 3;
		case pieces::PAWN:   return 1;
		default:             return 0;
		}
	}
	// --------------------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------------------
	// Sorts both capture lists from highest to lowest value
	void sortCapturedPieces(std::vector<Piece>& white, std::vector<Piece>& black) {
		auto higherFirst = [](const Piece& a, const Piece& b) {
			int va = pieceValue(a.currPiece);
			int vb = pieceValue(b.currPiece);
			if (va != vb) return va > vb;
			return a.currPiece > b.currPiece;
			};
		std::sort(white.begin(), white.end(), higherFirst);
		std::sort(black.begin(), black.end(), higherFirst);
	}
	// --------------------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------------------
	bool findMove(std::vector<Move> moves, int col, int row) {
		for (auto it : moves) {
			if (it.col != col || it.row != row) continue;
			else {
				return true;
			}
		}
		return false;
	}
	// --------------------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------------------
	int stateToIndex() const {
		if (currentGameState == states::STALEMATE) return 6;

		int side = (sideToMove == colors::BLACK) ? 1 : 0;
		int state = (currentGameState == states::CHECK) ? 2
			: (currentGameState == states::CHECKMATE) ? 4
			: 0;
		return side + state;
	}
	// --------------------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------------------
	const void FENdecoder(std::string FEN) {
		int i = 0;

		for (auto check : FEN) {
			switch (check) {

			case 'r':
				board[i].currColor = colors::BLACK;
				board[i].currPiece = pieces::ROOK;
				i++;
				break;

			case 'n':
				board[i].currColor = colors::BLACK;
				board[i].currPiece = pieces::KNIGHT;
				i++;
				break;

			case 'b':
				board[i].currColor = colors::BLACK;
				board[i].currPiece = pieces::BISHOP;
				i++;
				break;

			case 'q':
				board[i].currColor = colors::BLACK;
				board[i].currPiece = pieces::QUEEN;
				i++;
				break;

			case 'k':
				board[i].currColor = colors::BLACK;
				board[i].currPiece = pieces::KING;
				i++;
				break;

			case 'p':
				board[i].currColor = colors::BLACK;
				board[i].currPiece = pieces::PAWN;
				i++;
				break;

			case 'R':
				board[i].currColor = colors::WHITE;
				board[i].currPiece = pieces::ROOK;
				i++;
				break;

			case 'N':
				board[i].currColor = colors::WHITE;
				board[i].currPiece = pieces::KNIGHT;
				i++;
				break;

			case 'B':
				board[i].currColor = colors::WHITE;
				board[i].currPiece = pieces::BISHOP;
				i++;
				break;

			case 'Q':
				board[i].currColor = colors::WHITE;
				board[i].currPiece = pieces::QUEEN;
				i++;
				break;

			case 'K':
				board[i].currColor = colors::WHITE;
				board[i].currPiece = pieces::KING;
				i++;
				break;

			case 'P':
				board[i].currColor = colors::WHITE;
				board[i].currPiece = pieces::PAWN;
				i++;
				break;

			case '/':
				break;

			default:
				if (check >= '1' && check <= '8') {
					int emptySquares = check - '0';
					for (int n = 0; n < emptySquares; n++) {
						board[i].currColor = colors::NONE;
						board[i].currPiece = pieces::NONE;
						i++;
					}
				}
				break;
			}

		}
	}
	// --------------------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------------------
};