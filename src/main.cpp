#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader.h"
#include "ChessLogic.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb_image.h"

#include <iostream>

const GLuint SCR_WIDTH = 1920;
const GLuint SCR_HEIGHT = 1080;

const std::string startFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";
const std::string revStart = "RNBQKBNR/PPPPPPPP/8/8/8/8/pppppppp/rnbqkbnr";

const GLint BOARD_SIZE = 8;

// Board layout constants, shared between rendering and mouse picking so
// they can never drift out of sync with one another.
const GLfloat squareSize = 0.18f;
const GLfloat boardExtent = BOARD_SIZE * squareSize;
const GLfloat boardStart = -boardExtent / 2.0f;

// Layout constants for captured piece windows
const GLfloat trayGap = 0.02f;   // space between board and tray
const GLfloat trayPad = 0.02f;   // inner margin
const GLfloat trayIcon = 0.07f;   // size of one captured piece
const GLint   trayCols = 16;       // pieces per row
const GLint   trayRowsPerSide = 2;
const GLfloat trayWidth = boardExtent;
const GLfloat trayHeight = 3.5f * trayPad;  

const GLfloat trayLeft = boardStart;
const GLfloat trayBottom = boardStart - trayHeight - trayGap;
const GLfloat trayBottomHigh = boardStart + boardExtent + trayGap;
// Promotion window constants
const GLfloat promotionTrayLeft = trayBottomHigh + 0.02f;
const GLfloat promotionTrayBHigh = trayBottomHigh - 0.49f;
const GLfloat promotionTrayBLow = trayBottomHigh - 1.38f;
const GLfloat promotionTraySize = trayIcon * 3.0f + trayGap;
const GLfloat promotionPanelPad = 0.015f;   // panel margin around the 2 x 2 piece slots

// PromotionPanel.png is a 1920 x 1080 canvas. The visible rounded panel covers only
// pixels x 398..1507, y 20..1052 (measured from the image's alpha channel; y counts down
// from the top). Update these if the image is re-exported with a different layout.



// Button Constants
const GLfloat   resetBtnGap = 0.01f;
const glm::vec2 resetBtnSize(0.26f, 0.14625f);
const glm::vec2 resetBtnCenter(boardStart + boardExtent + resetBtnGap * 0.5f + 0.155f, trayBottomHigh - 0.005f);
const GLfloat   resetBtnPressScale = 0.92f;   // size while held down
const GLfloat   resetBtnHoverScale = 1.03f;   // size while hovered
const double    resetBtnReleaseTime = 0.12;   // seconds to grow back after release
const GLfloat resetPulseAmount = 0.02f;   // ±2% size

// White/Black to move constants
const glm::vec2 whosPlayingSize(0.28f, 0.498f);
const glm::vec2 whosPlayingCenter(boardStart + boardExtent + resetBtnGap * 0.5f + 0.155f, trayBottomHigh - 0.255f);

// Promotion panel dimensions
const glm::vec2 promoPanelImgSize(1920.0f, 1080.0f);
const glm::vec2 promoPanelPxMin(398.0f, 20.0f);
const glm::vec2 promoPanelPxMax(1507.0f, 1052.0f);

// Promotion choices, in reading order across the 2 x 2 tray
const pieces promotionChoices[4] = { pieces::QUEEN, pieces::ROOK, pieces::BISHOP, pieces::KNIGHT };
const GLfloat promoPulseAmount = 0.08f;   // ±8% size
const double  promoPulseSpeed = 6.0;     // radians/sec, about one pulse per second

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------

struct DragState {
	bool active = false;        // a piece is currently lifted
	bool wasSelected = false;   // square was already selected at press (click again = deselect)
	GLint fromCol = -1;
	GLint fromRow = -1;
	Board::Piece piece;        // copy of the lifted piece, drawn at the cursor
	glm::vec2 cursor{ 0.0f };   // cursor position in world space
};

struct ButtonState {
	bool hover = false;
	bool pressed = false;        // left button went down on it and is still held
	double releaseTime = -1.0;   // glfwGetTime() at release; drives the grow-back animation
	double hoverStart = -1.0;    // NEW: glfwGetTime() when hover began; drives the pulse
};

struct GameState {
	Board board;
	DragState drag;
	GLint selectedCol = -1, selectedRow = -1;
	GLint hoverCol = -1, hoverRow = -1;
	GLint lastMoveCol = -1, lastMoveRow = -1;   // square the most recently moved piece landed on
	ButtonState resetBtn;
	ButtonState promoBtns[4];
	GLint promoHover = -1;
};

struct RenderContext {
	GameState* game = nullptr;
	const Shader* boardShader = nullptr;
	const Shader* pieceShader = nullptr;
	GLuint        VAO = 0;
	const GLuint* pieceTex = nullptr;   // textureArr
	const GLuint* statusTex = nullptr;   // whiteOrBlackArr
	GLuint        resetTex = 0;
	GLuint        moveDots = 0;
	GLuint		  promotionTray = 0;
};

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
// Function declarations
GLvoid renderFrame(GLFWwindow* window, RenderContext& ctx);

GLvoid window_refresh_callback(GLFWwindow* window);
GLuint loadTexture(const char* path);
GLvoid framebuffer_size_callback(GLFWwindow* window, int width, int height);
GLvoid mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
GLvoid cursor_pos_callback(GLFWwindow* window, double x, double y);

glm::mat4 makeProjection(GLFWwindow* window);
glm::vec2 cursorToWorld(GLFWwindow* window, double mouseX, double mouseY);
glm::vec2 promotionSlotCenter(int i, const GLfloat& tBottom);

GLboolean worldToSquare(glm::vec2 world, GLint& outCol, GLint& outRow);
GLvoid drawBoard(const Shader& shader, const GameState& gs);
GLvoid drawPieces(const Shader& shader, const GameState& gs, const GLuint textures[]);
GLvoid drawPieceAt(const Shader& shader, const Board::Piece& p, glm::vec2 center, GLfloat size, const GLuint textures[]);
GLuint textureFor(const Board::Piece& p, const GLuint textures[]);
GLvoid resetGame(GameState& gs);
GLvoid drawRect(const Shader& shader, glm::vec2 center, glm::vec2 size, glm::vec3 color);
GLvoid drawCaptureTray(const Shader& shader);
GLvoid drawCapturedPieces(const Shader& shader, const GameState& gs, const GLuint textures[]);
bool insideRect(glm::vec2 p, glm::vec2 center, glm::vec2 size);
GLfloat buttonScale(const ButtonState& b);
bool buttonAnimating(const ButtonState& b);
GLvoid drawResetButton(const Shader& shader, const ButtonState& b, const GLuint& tex);
GLvoid drawText(const Shader& shader, const GLuint tex[], GameState& gs);
GLvoid drawPromotionTray(const Shader& shader, const GameState& gs, const GLuint textures[], const GLfloat& tBottom, const GLuint promPanel);
GLvoid drawPossibleMoves(const Shader& shader, GameState& gs, const GLuint dots);
int promotionSlotAt(glm::vec2 world, const GLfloat& tBottom);
GLfloat promoScale(const ButtonState& b, const GLfloat scale);
GLfloat promotionTrayBottom(const Board& b);
// ----------------------------------------------------------------------------------------------------------------------

int main() {


	// Initialize GLFW
	glfwInit();

	// Tells GLFW what version of OpenGL I am using
	// Using OpenGL 4.4 for this project
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	// Tells GLFW that I am using the core profile
	// core profile has only modern functions within OpenGL
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);



	// Create GLFW window object of 800x800 pixels named "OpenGL Window", 
	// fullscreen = null, shareResources = null (no other window to share resources)
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "CHESS!!", nullptr, nullptr);

	// Check if window object created successfully
	// if not, terminate GLFW, and return -1
	if (!window) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	// Introduce the window object into the current context
	// (make the window interactable)
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	GameState game;
<<<<<<< Updated upstream
	game.board.FENdecoder(revStart);   // row 0 is drawn at the bottom, so white goes first in the string
	glfwSetWindowUserPointer(window, &game);   // lets callbacks reach the game without globals
=======
	RenderContext ctx;
	ctx.game = &game;
	game.board.FENdecoder(startFEN);
	glfwSetWindowUserPointer(window, &ctx);  

	glfwSetWindowRefreshCallback(window, window_refresh_callback);
>>>>>>> Stashed changes
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_pos_callback);


	// Load glad so it configures OpenGL
	if (!gladLoadGL()) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	Shader boardShader("boardShaders/bShader.vert", "boardShaders/bShader.frag");
	Shader pieceShader("pieceShaders/pShader.vert", "pieceShaders/pShader.frag");

	// Flip piece on load since piece shader renders upside-down
	stbi_set_flip_vertically_on_load(true);

	// Create black pieces texture objects 
	const GLuint bRookTex = loadTexture("ChessPieces/b-rook.png");
	const GLuint bPawnTex = loadTexture("ChessPieces/b-pawn.png");
	const GLuint bKnightTex = loadTexture("ChessPieces/b-knight.png");
	const GLuint bKingTex = loadTexture("ChessPieces/b-king.png");
	const GLuint bQueenTex = loadTexture("ChessPieces/b-queen.png");
	const GLuint bBishopTex = loadTexture("ChessPieces/b-bishop.png");

	// Create white pieces texture objects 
	const GLuint wRookTex = loadTexture("ChessPieces/w-rook.png");
	const GLuint wPawnTex = loadTexture("ChessPieces/w-pawn.png");
	const GLuint wKnightTex = loadTexture("ChessPieces/w-knight.png");
	const GLuint wKingTex = loadTexture("ChessPieces/w-king.png");
	const GLuint wQueenTex = loadTexture("ChessPieces/w-queen.png");
	const GLuint wBishopTex = loadTexture("ChessPieces/w-bishop.png");

	// Button Picture
	const GLuint resetTex = loadTexture("Icons/PlayAgainButton.png");

	// White to play & Black to play pictures
	const GLuint whitePlay = loadTexture("Icons/WhiteToPlay.png");
	const GLuint blackPlay = loadTexture("Icons/BlackToPlay.png");
	const GLuint whiteChecked = loadTexture("Icons/WhiteInCheck.png");
	const GLuint blackChecked = loadTexture("Icons/BlackInCheck.png");
	const GLuint whiteWins = loadTexture("Icons/WhiteCheckmate.png");
	const GLuint blackWins = loadTexture("Icons/BlackCheckmate.png");
	const GLuint staleMate = loadTexture("Icons/Stalemate.png");

	// Dots
	const GLuint moveDots = loadTexture("Icons/MoveDots.png");

	const GLuint promoPanel = loadTexture("Icons/PromotionPanel.png");

	// Create array of piece texture objects
	const GLuint textureArr[] = {
		bRookTex, bPawnTex, bKnightTex, bKingTex, bQueenTex, bBishopTex,
		wRookTex, wPawnTex, wKnightTex, wKingTex, wQueenTex, wBishopTex
	};
	// Create array for text display objects
	const GLuint whiteOrBlackArr[] = { 
		whitePlay, blackPlay, whiteChecked, 
		blackChecked, blackWins, whiteWins,
		staleMate
	};

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	// Vertices data for the triangles (4 positions)
	GLfloat vertices[] = {
	/*   positions			    Texture Coords */
		 0.5f,  0.5f, 0.0f,		1.0f, 1.0f, // top right,	 top right
		 0.5f, -0.5f, 0.0f,		1.0f, 0.0f, // bottom right, bottom right
		-0.5f, -0.5f, 0.0f,		0.0f, 0.0f, // bottom left,  bottom left
		-0.5f,  0.5f, 0.0f,		0.0f, 1.0f, // top left,	 top left


	};

	// Array of traingle corner indices (4 values -> 4 corners)
	// Each index maps to a position coordinate in vertices array
	// Together, both triangles form a GL_SQUARES primitive
	GLuint indices[] = {
		0, 1, 3, // top right, bottom right, top left
		1, 2, 3  // bottom right, bottom left, top left
	};


	//Specify the viewport of openGL from x = 0, y = 0, to x = 1920, y = 1080
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);


	// Define and generate Vertex Buffer Obj. (stores positions & texture coords),
	// Vertex Attribute Obj. (defines how VBO is read: 0 -> position, 1 -> Texture Coords),
	// & Element Buffer (stores indices arrays for how to draw VBO)
	GLuint VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	// bind the Vertex Attribute Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	// Bind VBO as active buffer in OpenGL
	glBindBuffer(GL_ARRAY_BUFFER, VBO);


	// Put vertices data into VBO data buffer
	// Static usage pattern indicates the buffer will be written once, but read many times
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Give VAO the Texture Coordinates Attribute @ 
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Bind Element Buffer Obj. with indices data
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Add all current rendering context (shaders, texture objects, Vertex Attribute Object) for frame rendering
	ctx.boardShader = &boardShader;
	ctx.pieceShader = &pieceShader;
	ctx.pieceTex = textureArr;
	ctx.statusTex = whiteOrBlackArr;
	ctx.resetTex = resetTex;
	ctx.moveDots = moveDots;
	ctx.VAO = VAO;
	ctx.promotionTray = promoPanel;
	
	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	// glBindVertexArray(0);

	/* RENDERING LOOP*/
	// Keeps window open until user clicks exit, 
	// or some other function tells window to close
		while (!glfwWindowShouldClose(window)) {
			renderFrame(window, ctx);

			bool animating = buttonAnimating(game.resetBtn);
			if (game.board.promotionPending) { for (const auto& b : game.promoBtns) animating |= buttonAnimating(b); }
			if (animating) glfwPollEvents(); else glfwWaitEventsTimeout(0.1f);	   // Save CPU/GPU usage by waiting for events

		}
	// Destroy the GLFW window object
	glfwDestroyWindow(window);
	// Terminate GLFW
	glfwTerminate();

	return 0;
}
// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------
GLvoid renderFrame(GLFWwindow* window, RenderContext& ctx) {
	glm::mat4 projection = makeProjection(window);
	glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindVertexArray(ctx.VAO);
	ctx.boardShader->use();
	ctx.boardShader->setMat4("projection", projection);
	drawBoard(*ctx.boardShader, *ctx.game);
	drawCaptureTray(*ctx.boardShader);

	ctx.pieceShader->use();
	ctx.pieceShader->setMat4("projection", projection);
	drawCapturedPieces(*ctx.pieceShader, *ctx.game, ctx.pieceTex);
	drawPossibleMoves(*ctx.pieceShader, *ctx.game, ctx.moveDots);
	drawPieces(*ctx.pieceShader, *ctx.game, ctx.pieceTex);
	drawResetButton(*ctx.pieceShader, ctx.game->resetBtn, ctx.resetTex);
	drawText(*ctx.pieceShader, ctx.statusTex, *ctx.game);
	drawPromotionTray(*ctx.pieceShader, *ctx.game, ctx.pieceTex, promotionTrayBottom(ctx.game->board), ctx.promotionTray); 

	glfwSwapBuffers(window);
}
// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------
// GLFW cursor position callback: updates the global hover square based on the mouse position
glm::mat4 makeProjection(GLFWwindow* window) {
	int fbWidth, fbHeight;
	glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
	if (fbWidth == 0 || fbHeight == 0) return glm::mat4(1.0f);   // minimized
	GLfloat aspect = (GLfloat)fbWidth / (GLfloat)fbHeight;
	return (aspect >= 1.0f) ? glm::ortho(-aspect, aspect, -1.0f, 1.0f) : glm::ortho(-1.0f, 1.0f, -1.0f / aspect, 1.0f / aspect);
}
// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------
// Converts mouse cursor position in screen space to world space coordinates
glm::vec2 cursorToWorld(GLFWwindow* window, double mouseX, double mouseY) {
	int winWidth, winHeight;
	glfwGetWindowSize(window, &winWidth, &winHeight);
	if (winWidth == 0 || winHeight == 0) return glm::vec2(0.0f);
	GLfloat ndcX = (2.0f * (GLfloat)mouseX) / (GLfloat)winWidth - 1.0f;
	GLfloat ndcY = 1.0f - (2.0f * (GLfloat)mouseY) / (GLfloat)winHeight;
	glm::vec4 world = glm::inverse(makeProjection(window)) * glm::vec4(ndcX, ndcY, 0.0f, 1.0f);
	return glm::vec2(world.x, world.y);
}
// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------
// Converts world space coordinates to a square on the chessboard, returning false if outside the board
GLboolean worldToSquare(glm::vec2 world, GLint& outCol, GLint& outRow) {
	GLint col = (GLint)glm::floor((world.x - boardStart) / squareSize);
	GLint row = (GLint)glm::floor((world.y - boardStart) / squareSize);
	if (col < 0 || col >= BOARD_SIZE || row < 0 || row >= BOARD_SIZE) return GL_FALSE;
	outCol = col;
	outRow = row;
	return GL_TRUE;
}
// GLFW framebuffer size callback: resizes the viewport when the window is resized
GLvoid framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}
// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------
// Loads a texture from a .png file and returns a pointer to the start of the data
GLuint loadTexture(const char* path) {
	GLuint textureID;

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLint width, height, nrChannels;
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
	if (data) {
		GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "ERROR::TEXTURE FAILED TO LOAD: " << path << std::endl;
	}
	stbi_image_free(data);
	return textureID;
}
// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------
GLvoid drawBoard(const Shader& shader, const GameState& gs) {
	for (GLint row = 0; row < BOARD_SIZE; row++) {
		for (GLint col = 0; col < BOARD_SIZE; col++) {
			GLfloat centerX = boardStart + (col + 0.5f) * squareSize;
			GLfloat centerY = boardStart + (row + 0.5f) * squareSize;
			glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(centerX, centerY, 0.0f));
			model = glm::scale(model, glm::vec3(squareSize, squareSize, 1.0f));
			shader.setMat4("model", model);

			glm::vec3 squareColor = ((col + row) % 2 == 0)
				? glm::vec3(0.412f, 0.263f, 0.0f)
				: glm::vec3(0.961f, 0.831f, 0.6f);

			if (col == gs.selectedCol && row == gs.selectedRow)
				squareColor = glm::mix(squareColor, glm::vec3(0.85f, 0.1f, 0.1f), 0.6f);
			else if (gs.drag.active && col == gs.hoverCol && row == gs.hoverRow)
				squareColor = glm::mix(squareColor, glm::vec3(1.0f), 0.35f);   // drop target
			else if (col == gs.lastMoveCol && row == gs.lastMoveRow)
				squareColor = glm::mix(squareColor, glm::vec3(1.0f, 0.867f, 0.267f), 0.30f);   // last moved piece

			shader.setVec3("squareColor", squareColor);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}
	}
}
// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------
// textures[] order: b-rook, b-pawn, b-knight, b-king, b-queen, b-bishop, then the same six for white.
GLuint textureFor(const Board::Piece& p, const GLuint textures[]) {
	GLint base = (p.currColor == colors::WHITE) ? 6 : 0;
	switch (p.currPiece) {
	case pieces::ROOK:   return textures[base + 0];
	case pieces::PAWN:   return textures[base + 1];
	case pieces::KNIGHT: return textures[base + 2];
	case pieces::KING:   return textures[base + 3];
	case pieces::QUEEN:  return textures[base + 4];
	case pieces::BISHOP: return textures[base + 5];
	default:             return 0;
	}
}
// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------
// Draws a single piece at the given center position, with the given size (in world units)
GLvoid drawPieceAt(const Shader& shader, const Board::Piece& p, glm::vec2 center,
	GLfloat size, const GLuint textures[]) {
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(center, 0.0f));
	model = glm::scale(model, glm::vec3(size, size, 1.0f));
	shader.setMat4("model", model);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureFor(p, textures));
	shader.setInt("pieceTexture", 0);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------
// Draws all pieces in the game state, skipping the currently dragged piece (which is drawn last)
GLvoid drawPieces(const Shader& shader, const GameState& gs, const GLuint textures[]) {
	for (GLint row = 0; row < BOARD_SIZE; row++) {
		for (GLint col = 0; col < BOARD_SIZE; col++) {
			const Board::Piece& p = gs.board.at(col, row);
			if (p.isEmpty()) continue;
			if (gs.drag.active && col == gs.drag.fromCol && row == gs.drag.fromRow) continue;  // lifted
			glm::vec2 center(boardStart + (col + 0.5f) * squareSize,
				boardStart + (row + 0.5f) * squareSize);
			drawPieceAt(shader, p, center, squareSize, textures);
		}
	}
	// Dragged piece last, so it is on top of every other piece, 10% larger.
	if (gs.drag.active)
		drawPieceAt(shader, gs.drag.piece, gs.drag.cursor, squareSize * 1.1f, textures);
}
// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
// Clears the selected square in the game state
static void clearSelection(GameState& gs) { gs.selectedCol = gs.selectedRow = -1; }
// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
// GLFW mouse button callback: handles piece selection, lifting, and dropping
GLvoid mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	GameState& gs = *static_cast<RenderContext*>(glfwGetWindowUserPointer(window))->game;
	DragState& d = gs.drag;

	// Right click while dragging cancels: the piece snaps home.
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS && d.active) {
		d.active = false;
		return;
	}
	if (button != GLFW_MOUSE_BUTTON_LEFT) return;

	double mx, my;
	glfwGetCursorPos(window, &mx, &my);
	glm::vec2 world = cursorToWorld(window, mx, my);

	if (gs.board.promotionPending) {
		int slot = promotionSlotAt(world, promotionTrayBottom(gs.board));
		if (action == GLFW_PRESS && slot != -1) {
			gs.promoBtns[slot].pressed = true;
		}
		else if (action == GLFW_RELEASE) {
			for (int i = 0; i < 4; i++) {
				if (!gs.promoBtns[i].pressed) continue;
				gs.promoBtns[i].pressed = false;
				gs.promoBtns[i].releaseTime = glfwGetTime();
				if (slot == i) gs.board.promote(promotionChoices[i]);
			}
		}
		return;
	}

	GLint col = -1, row = -1;
	bool onBoard = worldToSquare(world, col, row);

	ButtonState& btn = gs.resetBtn;
	bool overBtn = insideRect(world, resetBtnCenter, resetBtnSize);
	if (action == GLFW_PRESS && overBtn && !d.active) {
		btn.pressed = true;
		return;                              // don't let the board treat it as a click
	}
	if (action == GLFW_RELEASE && btn.pressed) {
		btn.pressed = false;
		btn.releaseTime = glfwGetTime();     // grow back whether or not the click counts
		if (overBtn) resetGame(gs);
		return;
	}

	if (action == GLFW_PRESS) {
		if (!onBoard) { clearSelection(gs); return; }

		bool hasSel = gs.selectedCol != -1;
		bool sameAsSel = hasSel && col == gs.selectedCol && row == gs.selectedRow;

		// Click-to-move: a piece is selected and this square is empty or an enemy.
		if (hasSel && !sameAsSel && !gs.board.canPickUp(col, row)) {
			if (gs.board.movePiece(gs.selectedCol, gs.selectedRow, col, row)) {  // illegal -> no-op
				gs.lastMoveCol = col;
				gs.lastMoveRow = row;
			}
			clearSelection(gs);
			return;
		}

		// Lift the piece. The board is NOT modified; the piece is only hidden while dragged.
		if (gs.board.canPickUp(col, row)) {
			d.active = true;
			d.wasSelected = sameAsSel;
			d.fromCol = col;
			d.fromRow = row;
			d.piece = gs.board.at(col, row);
			d.cursor = world;
			gs.hoverCol = col;
			gs.hoverRow = row;
			gs.selectedCol = col;
			gs.selectedRow = row;
		}
		else {
			clearSelection(gs);
		}
	}
	else if (action == GLFW_RELEASE && d.active) {
		d.active = false;
		bool sameSquare = onBoard && col == d.fromCol && row == d.fromRow;

		if (onBoard && !sameSquare) {
			if (gs.board.movePiece(d.fromCol, d.fromRow, col, row)) {  // illegal -> snaps back
				gs.lastMoveCol = col;
				gs.lastMoveRow = row;
			}
			clearSelection(gs);
		}
		else if (sameSquare && d.wasSelected) {
			clearSelection(gs);   // clicked an already-selected piece: toggle off
		}
		// Otherwise a plain click: the piece stays selected for click-to-move.
	}
}
// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
// GLFW cursor position callback: updates the global hover square based on the mouse position
GLvoid cursor_pos_callback(GLFWwindow* window, double x, double y) {
	GameState& gs = *static_cast<RenderContext*>(glfwGetWindowUserPointer(window))->game;
	glm::vec2 world = cursorToWorld(window, x, y);
	gs.resetBtn.hover = insideRect(world, resetBtnCenter, resetBtnSize);

	int slot = gs.board.promotionPending ? promotionSlotAt(world, promotionTrayBottom(gs.board)) : -1;
	gs.promoHover = slot;
	for (int i = 0; i < 4; i++) {
		bool now = (slot == i);
		if (now && !gs.promoBtns[i].hover) gs.promoBtns[i].hoverStart = glfwGetTime();
		gs.promoBtns[i].hover = now;
	}

	if (!gs.drag.active) return;
	gs.drag.cursor = world;
	if (!worldToSquare(gs.drag.cursor, gs.hoverCol, gs.hoverRow)) {
		gs.hoverCol = gs.hoverRow = -1;
	}
}
// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
GLvoid window_refresh_callback(GLFWwindow* window) {
	auto* ctx = static_cast<RenderContext*>(glfwGetWindowUserPointer(window));
	if (!ctx || ctx->VAO == 0) return;   // resources not ready yet
	renderFrame(window, *ctx);
}
// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
// Resets the game state to a fresh board, white to move, no selection, no drag
GLvoid resetGame(GameState& gs) {
<<<<<<< Updated upstream
	gs = GameState{};                 // fresh board, white to move, no selection, no drag
	gs.board.FENdecoder(revStart);
=======
	ButtonState btn = gs.resetBtn;   // keep the button's animation across the reset
	gs = GameState{};				 // fresh gamestate
	gs.resetBtn = btn;
	gs.board.FENdecoder(startFEN);
>>>>>>> Stashed changes
}
// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
// Draws a solid-colour rectangle with the board shader (the unit quad, scaled to any width and height)
GLvoid drawRect(const Shader& shader, glm::vec2 center, glm::vec2 size, glm::vec3 color) {
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(center, 0.0f));
	model = glm::scale(model, glm::vec3(size, 1.0f));
	shader.setMat4("model", model);
	shader.setVec3("squareColor", color);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
// Draws the empty captured-pieces tray to the right of the board
GLvoid drawCaptureTray(const Shader& shader) {
	glm::vec2 lowCenter(trayLeft + trayWidth * 0.5f, trayBottom + trayHeight * 0.5f);
	glm::vec2 highCenter(trayLeft + trayWidth * 0.5f, trayBottomHigh + trayHeight * 0.5f);
	const GLfloat border = 0.006f;

	drawRect(shader, lowCenter, glm::vec2(trayWidth + 2 * border, trayHeight + 2 * border), glm::vec3(0.25f));  // outline
	drawRect(shader, lowCenter, glm::vec2(trayWidth, trayHeight), glm::vec3(0.45f));                            // white body

	drawRect(shader, highCenter, glm::vec2(trayWidth + 2 * border, trayHeight + 2 * border), glm::vec3(0.25f));  // outline
	drawRect(shader, highCenter, glm::vec2(trayWidth, trayHeight), glm::vec3(0.45f));                            // white body
}
// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
GLvoid drawCapturedPieces(const Shader& shader, const GameState& gs, const GLuint textures[]) {
	for (int i = 0; i < gs.board.capturedByWhite.size(); i++) {
		GLint col = i % trayCols;

		GLfloat x = trayLeft + trayPad + (col + 0.5f) * trayIcon;
		GLfloat y = trayBottom + trayPad * trayIcon + 0.035f;
		drawPieceAt(shader, gs.board.capturedByWhite[i], glm::vec2(x, y), trayIcon, textures);
	}
	for (int i = 0; i < gs.board.capturedByBlack.size(); i++) {
		GLint col = i % trayCols;

		GLfloat x = trayLeft + trayPad + (col + 0.4f) * trayIcon;
		GLfloat y = trayBottomHigh + trayPad * trayIcon + 0.035f;
		drawPieceAt(shader, gs.board.capturedByBlack[i], glm::vec2(x, y), trayIcon, textures);
	}
}
// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
bool insideRect(glm::vec2 p, glm::vec2 center, glm::vec2 size) {
	return glm::abs(p.x - center.x) <= size.x * 0.5f &&
		glm::abs(p.y - center.y) <= size.y * 0.5f;
}
// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
// Current draw scale of the button: shrunk while held, eased back after release
GLfloat buttonScale(const ButtonState& b) {
	GLfloat rest = b.hover ? resetBtnHoverScale : 1.0f;
	if (b.pressed && b.hover) return resetBtnPressScale;
	if (b.releaseTime >= 0.0) {
		double t = (glfwGetTime() - b.releaseTime) / resetBtnReleaseTime;
		if (t < 1.0) {
			GLfloat e = 1.0f - (GLfloat)((1.0 - t) * (1.0 - t));   // ease-out
			return glm::mix(resetBtnPressScale, rest, e);
		}
	}
	return rest;
}
// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
bool buttonAnimating(const ButtonState& b) {
	return (b.releaseTime >= 0.0 && glfwGetTime() - b.releaseTime < resetBtnReleaseTime) || (b.hover);
}
// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
// Draws the button texture with the piece shader, scaled around its center
GLvoid drawResetButton(const Shader& shader, const ButtonState& b, const GLuint& tex) {
	glm::vec2 size = resetBtnSize * promoScale(b, resetPulseAmount);
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(resetBtnCenter, 0.0f));
	model = glm::scale(model, glm::vec3(size, 1.0f));
	shader.setMat4("model", model);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	shader.setInt("pieceTexture", 0);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
GLvoid drawText(const Shader& shader, const GLuint tex[], GameState& gs) {
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(whosPlayingCenter, 0.0f));
	model = glm::scale(model, glm::vec3(whosPlayingSize, 0.0f));
	shader.setMat4("model", model);
	glActiveTexture(GL_TEXTURE0);
	{ glBindTexture(GL_TEXTURE_2D, tex[gs.board.stateToIndex()]); }
	shader.setInt("pieceTexture", 0);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
GLvoid drawPossibleMoves(const Shader& shader, GameState& gs, const GLuint dots) {
	if ((!gs.drag.active && !gs.drag.wasSelected) && (gs.selectedCol == -1 && gs.selectedRow == -1)) return;
	const std::vector<Board::Move> & moves = gs.board.generateMoves(gs.drag.fromCol, gs.drag.fromRow);

	auto findMove = [](std::vector<Board::Move> moves, int col, int row) {
		for (auto it : moves) { 
			if (it.col != col || it.row != row) continue;
			else {
				return true;
			}
		}
		return false;
	};
	for (GLint row = 0; row < BOARD_SIZE; row++) {
		for (GLint col = 0; col < BOARD_SIZE; col++) {
			if (!gs.board.findMove(moves, col, row)) continue; // if the move is not valid go to next loop iteration

			GLfloat centerX = boardStart + (col + 0.5f) * squareSize - 0.001f;
			GLfloat centerY = boardStart + (row + 0.5f) * squareSize - 0.006f;
			glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(centerX, centerY, 0.0f));
			model = glm::scale(model, glm::vec3(squareSize * 1.777f, squareSize, 0.2f));

			shader.setMat4("model", model);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, dots);
			shader.setInt("pieceTexture", 0);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		}
	}
}
// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------

// Centre of slot i (0..3). Used by both drawing and mouse picking, so they always agree.
glm::vec2 promotionSlotCenter(int i, const GLfloat& tBottom) {
	const GLfloat cell = promotionTraySize * 0.5f;
	int c = i % 2;        // 0 = left,     1 = right
	int r = 1 - i / 2;    // 1 = top row,  0 = bottom row
	return glm::vec2(promotionTrayLeft + (c + 0.5f) * cell,
		tBottom + (r + 0.5f) * cell);
}
// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
// Which slot a world-space point is over, or -1 for none
int promotionSlotAt(glm::vec2 world, const GLfloat& tBottom) {
	const GLfloat cell = promotionTraySize * 0.5f;
	for (int i = 0; i < 4; i++)
		if (insideRect(world, promotionSlotCenter(i, tBottom), glm::vec2(cell))) return i;
	return -1;
}
// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
GLvoid drawPromotionTray(const Shader& shader,
	const GameState& gs, const GLuint textures[], const GLfloat& tBottom, const GLuint promPanel) {
	if (!gs.board.promotionPending) return;

	const GLfloat cell = promotionTraySize * 0.5f;
	glm::vec2 center(promotionTrayLeft + promotionTraySize * 0.5f,
		tBottom + promotionTraySize * 0.5f);     // centre of the 2 x 2 slots


	glm::vec2 panelSize(promotionTraySize + 2.0f * promotionPanelPad);    // visible panel, world units
	glm::vec2 panelPx = promoPanelPxMax - promoPanelPxMin;                  // visible panel, pixels
	glm::vec2 worldPerPx = panelSize / panelPx;
	glm::vec2 quadSize = promoPanelImgSize * worldPerPx;                     // whole canvas, world units

	glm::vec2 panelCenterPx = (promoPanelPxMin + promoPanelPxMax) * 0.5f;
	glm::vec2 offsetPx(panelCenterPx.x - promoPanelImgSize.x * 0.5f,      // + = panel right of canvas centre
		promoPanelImgSize.y * 0.5f - panelCenterPx.y);                      // + = panel above (image y points down)
	glm::vec2 quadCenter = center - offsetPx * worldPerPx;

	shader.use();
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(quadCenter, 0.0f));
	model = glm::scale(model, glm::vec3(quadSize, 1.0f));
	shader.setMat4("model", model);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, promPanel);
	shader.setInt("pieceTexture", 0);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


	// 2. The four choices: textures, piece shader, in the promoting pawn's colour
	colors side = gs.board.at(gs.board.promoCol, gs.board.promoRow).currColor;
	for (int i = 0; i < 4; i++) {
		Board::Piece p;
		p.currPiece = promotionChoices[i];
		p.currColor = side;
		GLfloat s = cell * 0.9f * promoScale(gs.promoBtns[i], promoPulseAmount);
		drawPieceAt(shader, p, promotionSlotCenter(i, tBottom), s, textures);
	}
}
// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
GLfloat promoScale(const ButtonState& b, const GLfloat pulseAmount) {
	if (b.pressed && b.hover) return resetBtnPressScale;
	GLfloat rest = 1.0f;
	if (b.hover)
		rest += pulseAmount * (GLfloat)glm::sin((glfwGetTime() - b.hoverStart) * promoPulseSpeed);
	if (b.releaseTime >= 0.0) {
		double t = (glfwGetTime() - b.releaseTime) / resetBtnReleaseTime;
		if (t < 1.0) {
			GLfloat e = 1.0f - (GLfloat)((1.0 - t) * (1.0 - t));
			return glm::mix(resetBtnPressScale, rest, e);
		}
	}
	return rest;
}
// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
GLfloat promotionTrayBottom(const Board& b) {
	return (b.sideToMove == colors::WHITE) ? promotionTrayBLow : promotionTrayBHigh;
}
// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------