#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader.h"
#include "ChessLogic.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb_image.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <iostream>

const GLuint SCR_WIDTH = 1920;
const GLuint SCR_HEIGHT = 1080;

const std::string startFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";
const std::string revStart = "RNBQKBNR/PPPPPPPP/8/8/8/8/pppppppp/rnbqkbnr";
const std::string susString = "4kb1r/p4ppp/4q3/8/8/1B6/PPP2PPP/2KR4";

const GLint BOARD_SIZE = 8;

// Board layout constants, shared between rendering and mouse picking so
// they can never drift out of sync with one another.
const GLfloat squareSize = 0.2f;
const GLfloat boardExtent = BOARD_SIZE * squareSize;
const GLfloat boardStart = -boardExtent / 2.0f;

struct DragState {
	bool active = false;        // a piece is currently lifted
	bool wasSelected = false;   // square was already selected at press (click again = deselect)
	GLint fromCol = -1;
	GLint fromRow = -1;
	Pieces::Piece piece;        // copy of the lifted piece, drawn at the cursor
	glm::vec2 cursor{ 0.0f };   // cursor position in world space
};

struct GameState {
	Pieces board;
	DragState drag;
	GLint selectedCol = -1, selectedRow = -1;
	GLint hoverCol = -1, hoverRow = -1;
};

// ---------------------------------------------------------------------------------------------------------------------
// Function declarations
GLuint loadTexture(const char* path);
GLvoid framebuffer_size_callback(GLFWwindow* window, int width, int height);
GLvoid processInput(GLFWwindow* window);
GLvoid mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
GLvoid cursor_pos_callback(GLFWwindow* window, double x, double y);
glm::mat4 makeProjection(GLFWwindow* window);
glm::vec2 cursorToWorld(GLFWwindow* window, double mouseX, double mouseY);
GLboolean worldToSquare(glm::vec2 world, GLint& outCol, GLint& outRow);
GLvoid drawBoard(const Shader& shader, const GameState& gs);
GLvoid drawPieces(const Shader& shader, const GameState& gs, const GLuint textures[]);
GLvoid drawPieceAt(const Shader& shader, const Pieces::Piece& p, glm::vec2 center, GLfloat size, const GLuint textures[]);
GLuint textureFor(const Pieces::Piece& p, const GLuint textures[]);
GLvoid resetGame(GameState& gs);
GLvoid drawSidePanel(GLFWwindow* window, GameState& gs);
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
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Window", nullptr, nullptr);

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
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	GameState game;
	game.board.FENdecoder(revStart);   // row 0 is drawn at the bottom, so white goes first in the string
	glfwSetWindowUserPointer(window, &game);   // lets callbacks reach the game without globals
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_pos_callback);


	// Load glad so it configures OpenGL
	if (!gladLoadGL()) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 440");

	Shader boardShader("boardShaders/bShader.vert", "boardShaders/bShader.frag");
	Shader pieceShader("pieceShaders/pShader.vert", "pieceShaders/pShader.frag");

	std::cout << boardShader.ID << std::endl;
	std::cout << pieceShader.ID << std::endl;

	// Flip piece on load since piece shader renders upside-down
	stbi_set_flip_vertically_on_load(true);

	// Create black pieces texture objects 

	GLuint bRookTex = loadTexture("ChessPieces/b-rook.png");
	GLuint bPawnTex = loadTexture("ChessPieces/b-pawn.png");
	GLuint bKnightTex = loadTexture("ChessPieces/b-knight.png");
	GLuint bKingTex = loadTexture("ChessPieces/b-king.png");
	GLuint bQueenTex = loadTexture("ChessPieces/b-queen.png");
	GLuint bBishopTex = loadTexture("ChessPieces/b-bishop.png");

		// Create white pieces texture objects 
	GLuint wRookTex = loadTexture("ChessPieces/w-rook.png");
	GLuint wPawnTex = loadTexture("ChessPieces/w-pawn.png");
	GLuint wKnightTex = loadTexture("ChessPieces/w-knight.png");
	GLuint wKingTex = loadTexture("ChessPieces/w-king.png");
	GLuint wQueenTex = loadTexture("ChessPieces/w-queen.png");
	GLuint wBishopTex = loadTexture("ChessPieces/w-bishop.png");

	// Create array of texture objects
	GLuint textureArr[] = {
		bRookTex, bPawnTex, bKnightTex, bKingTex, bQueenTex, bBishopTex,
		wRookTex, wPawnTex, wKnightTex, wKingTex, wQueenTex, wBishopTex
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


	//Specify the viewport of openGL from x = 0, y = 0, to x = 800, y = 800
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
	


	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	// glBindVertexArray(0);

	// Swaps the current and back buffer to show the new color
	glfwSwapBuffers(window);

	// Keeps window open until user clicks exit, 
	// or some other function tells window to close
		while (!glfwWindowShouldClose(window)) {

			processInput(window);
			glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			glm::mat4 projection = makeProjection(window);
			glBindVertexArray(VAO);

			boardShader.use();
			boardShader.setMat4("projection", projection);
			drawBoard(boardShader, game);

			pieceShader.use();
			pieceShader.setMat4("projection", projection);
			drawPieces(pieceShader, game, textureArr);

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			drawSidePanel(window, game);
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			glfwSwapBuffers(window);
			glfwWaitEventsTimeout(0.1);

		}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// Destroy the GLFW window object
	glfwDestroyWindow(window);
	// Terminate GLFW
	glfwTerminate();

	return 0;
}
// ----------------------------------------------------------------------------------------------------------
// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
GLvoid processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}
//  ----------------------------------------------------------------------------------------------------------
// GLFW cursor position callback: updates the global hover square based on the mouse position
glm::mat4 makeProjection(GLFWwindow* window) {
	int fbWidth, fbHeight;
	glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
	if (fbWidth == 0 || fbHeight == 0) return glm::mat4(1.0f);   // minimized
	GLfloat aspect = (GLfloat)fbWidth / (GLfloat)fbHeight;
	return (aspect >= 1.0f) ? glm::ortho(-aspect, aspect, -1.0f, 1.0f) : glm::ortho(-1.0f, 1.0f, -1.0f / aspect, 1.0f / aspect);
}
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
// Converts world space coordinates to a square on the chessboard, returning false if outside the board
GLboolean worldToSquare(glm::vec2 world, GLint& outCol, GLint& outRow) {
	GLint col = (GLint)glm::floor((world.x - boardStart) / squareSize);
	GLint row = (GLint)glm::floor((world.y - boardStart) / squareSize);
	if (col < 0 || col >= BOARD_SIZE || row < 0 || row >= BOARD_SIZE) return GL_FALSE;
	outCol = col;
	outRow = row;
	return GL_TRUE;
}
// ----------------------------------------------------------------------------------------------------------
// GLFW framebuffer size callback: resizes the viewport when the window is resized
GLvoid framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}
// ----------------------------------------------------------------------------------------------------------

/* Loads a texture from a .png file and returns a pointer to the start of the data*/
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
		std::cout << "Loaded " << path << " - channels: " << nrChannels << std::endl;
	}
	else {
		std::cout << "ERROR::TEXTURE FAILED TO LOAD: " << path << std::endl;
	}
	stbi_image_free(data);
	return textureID;
}
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

			shader.setVec3("squareColor", squareColor);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}
	}
}
// --------------------------------------------------------------------------------------------------------
// textures[] order: b-rook, b-pawn, b-knight, b-king, b-queen, b-bishop, then the same six for white.
GLuint textureFor(const Pieces::Piece& p, const GLuint textures[]) {
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
// Draws a single piece at the given center position, with the given size (in world units)
GLvoid drawPieceAt(const Shader& shader, const Pieces::Piece& p, glm::vec2 center,
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
// Draws all pieces in the game state, skipping the currently dragged piece (which is drawn last)
GLvoid drawPieces(const Shader& shader, const GameState& gs, const GLuint textures[]) {
	for (GLint row = 0; row < BOARD_SIZE; row++) {
		for (GLint col = 0; col < BOARD_SIZE; col++) {
			const Pieces::Piece& p = gs.board.at(col, row);
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
// Clears the selected square in the game state
static void clearSelection(GameState& gs) { gs.selectedCol = gs.selectedRow = -1; }
// ---------------------------------------------------------------------------------------------------------
// GLFW mouse button callback: handles piece selection, lifting, and dropping
GLvoid mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	GameState& gs = *static_cast<GameState*>(glfwGetWindowUserPointer(window));
	DragState& d = gs.drag;

	// Right click while dragging cancels: the piece snaps home.
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS && d.active) {
		d.active = false;
		return;
	}
	if (button != GLFW_MOUSE_BUTTON_LEFT) return;

	if (action == GLFW_PRESS && ImGui::GetIO().WantCaptureMouse) return;

	double mx, my;
	glfwGetCursorPos(window, &mx, &my);
	glm::vec2 world = cursorToWorld(window, mx, my);
	GLint col = -1, row = -1;
	bool onBoard = worldToSquare(world, col, row);

	if (action == GLFW_PRESS) {
		if (!onBoard) { clearSelection(gs); return; }

		bool hasSel = gs.selectedCol != -1;
		bool sameAsSel = hasSel && col == gs.selectedCol && row == gs.selectedRow;

		// Click-to-move: a piece is selected and this square is empty or an enemy.
		if (hasSel && !sameAsSel && !gs.board.canPickUp(col, row)) {
			gs.board.movePiece(gs.selectedCol, gs.selectedRow, col, row);  // illegal -> no-op
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
			gs.board.movePiece(d.fromCol, d.fromRow, col, row);  // illegal -> snaps back
			clearSelection(gs);
		}
		else if (sameSquare && d.wasSelected) {
			clearSelection(gs);   // clicked an already-selected piece: toggle off
		}
		// Otherwise a plain click: the piece stays selected for click-to-move.
	}
}
// ---------------------------------------------------------------------------------------------------------
// GLFW cursor position callback: updates the global hover square based on the mouse position
GLvoid cursor_pos_callback(GLFWwindow* window, double x, double y) {
	GameState& gs = *static_cast<GameState*>(glfwGetWindowUserPointer(window));
	if (!gs.drag.active) return;
	gs.drag.cursor = cursorToWorld(window, x, y);
	if (!worldToSquare(gs.drag.cursor, gs.hoverCol, gs.hoverRow)) {
		gs.hoverCol = gs.hoverRow = -1;
	}
}
// ---------------------------------------------------------------------------------------------------------
// Resets the game state to a fresh board, white to move, no selection, no drag
GLvoid resetGame(GameState& gs) {
	gs = GameState{};                 // fresh board, white to move, no selection, no drag
	gs.board.FENdecoder(revStart);
}
// ---------------------------------------------------------------------------------------------------------
// ImGui panel placed just to the right of the board
GLvoid drawSidePanel(GLFWwindow* window, GameState& gs) {
	int winWidth, winHeight;
	glfwGetWindowSize(window, &winWidth, &winHeight);
	GLfloat aspect = (winHeight > 0) ? (GLfloat)winWidth / (GLfloat)winHeight : 1.0f;

	// Board's right edge (world x = 0.8) converted to window pixels
	GLfloat halfWidth = (aspect >= 1.0f) ? aspect : 1.0f;
	GLfloat boardRightPx = (-boardStart / halfWidth + 1.0f) * 0.5f * winWidth;
	GLfloat panelX = glm::min(boardRightPx + 20.0f, (GLfloat)winWidth - 200.0f);

	ImGui::SetNextWindowPos(ImVec2(panelX, 20.0f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(180.0f, 0.0f), ImGuiCond_Always);
	ImGui::Begin("Game", nullptr,
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

	ImGui::Text("%s to move", gs.board.sideToMove == colors::WHITE ? "White" : "Black");
	ImGui::Separator();
	if (ImGui::Button("Reset board", ImVec2(-1.0f, 0.0f))) {
		resetGame(gs);
	}

	ImGui::End();
}
// ---------------------------------------------------------------------------------------------------------