#include <string>
#include <iostream>


enum class pieces {
	NONE = 0,
	PAWN = 1,
	ROOK = 2,
	KING = 3,
	QUEEN = 4,
	KNIGHT = 5,
	BISHOP = 6
};
enum class colors {
	NONE = 0,
	WHITE = 1,
	BLACK = 2
};

char pieceToChar(pieces, colors);

class Pieces {
public:
	struct Piece {
		pieces currPiece = pieces::NONE;
		colors currColor = colors::NONE;
		bool isEmpty() const { return currColor == colors::NONE; }
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
	Piece piecePos[64];


	colors sideToMove = colors::WHITE;

	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	// Converts a column and row to an index in the piecePos array
	static int index(int col, int row) { 
		return row * 8 + col; 
	}
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	// Returns a reference to the piece at the given column and row
	Piece& at(int col, int row) { 
		return piecePos[index(col, row)]; 
	}
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	// Returns a const reference to the piece at the given column and row
	const Piece& at(int col, int row) const { 
		return piecePos[index(col, row)]; 
	}
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	void changePos() {

	}
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	bool canPickUp(int col, int row) const {
		const Piece& p = at(col, row);
		return !p.isEmpty() && p.currColor == sideToMove;
	}
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	bool isMoveAllowed(int fromCol, int fromRow, int toCol, int toRow) const {
		if (fromCol == toCol && fromRow == toRow) return false;
		const Piece& from = at(fromCol, fromRow);
		const Piece& to = at(toCol, toRow);
		if (from.isEmpty() || from.currColor != sideToMove) return false;
		if (to.currColor == from.currColor) return false;
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
	}
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------

	void FENdecoder(std::string FEN) {
		int i = 0;

		for (auto checker : FEN) {

			switch (checker) {

			case 'r':
				piecePos[i].currColor = colors::BLACK;
				piecePos[i].currPiece = pieces::ROOK;
				i++;
				break;

			case 'n':
				piecePos[i].currColor = colors::BLACK;
				piecePos[i].currPiece = pieces::KNIGHT;
				i++;
				break;

			case 'b':
				piecePos[i].currColor = colors::BLACK;
				piecePos[i].currPiece = pieces::BISHOP;
				i++;
				break;

			case 'q':
				piecePos[i].currColor = colors::BLACK;
				piecePos[i].currPiece = pieces::QUEEN;
				i++;
				break;

			case 'k':
				piecePos[i].currColor = colors::BLACK;
				piecePos[i].currPiece = pieces::KING;
				i++;
				break;

			case 'p':
				piecePos[i].currColor = colors::BLACK;
				piecePos[i].currPiece = pieces::PAWN;
				i++;
				break;

			case 'R':
				piecePos[i].currColor = colors::WHITE;
				piecePos[i].currPiece = pieces::ROOK;
				i++;
				break;

			case 'N':
				piecePos[i].currColor = colors::WHITE;
				piecePos[i].currPiece = pieces::KNIGHT;
				i++;
				break;

			case 'B':
				piecePos[i].currColor = colors::WHITE;
				piecePos[i].currPiece = pieces::BISHOP;
				i++;
				break;

			case 'Q':
				piecePos[i].currColor = colors::WHITE;
				piecePos[i].currPiece = pieces::QUEEN;
				i++;
				break;

			case 'K':
				piecePos[i].currColor = colors::WHITE;
				piecePos[i].currPiece = pieces::KING;
				i++;
				break;

			case 'P':
				piecePos[i].currColor = colors::WHITE;
				piecePos[i].currPiece = pieces::PAWN;
				i++;
				break;

			case '/':
				break;

			default:
				if (checker >= '1' && checker <= '8') {
					int emptySquares = checker - '0';
					for (int n = 0; n < emptySquares; n++) {
						piecePos[i].currColor = colors::NONE;
						piecePos[i].currPiece = pieces::NONE;
						i++;
					}
				}
				break;
			}

		}
	}
	// -----------------------------------------------------------------------------------
	// -----------------------------------------------------------------------------------
};