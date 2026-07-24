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

	struct Piece {
		enum pieces currPiece;
		enum colors currColor;
	};


public:

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

	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	void changePos() {

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