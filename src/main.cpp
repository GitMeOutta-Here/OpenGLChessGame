#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader.h"
#include "ChessLogic.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb_image.h"

#include <iostream>

GLuint loadTexture(const char* path);
GLvoid framebuffer_size_callback(GLFWwindow* window, int width, int height);
GLvoid processInput(GLFWwindow* window);
GLvoid drawBoard(Shader shader, GLfloat sqSize, GLfloat bdStart);
GLvoid drawPieces(Shader shader, GLfloat sqSize, GLfloat bdStart, Pieces pieceLayout, GLuint textures[]);
GLvoid screenToWorld();

const GLuint SCR_WIDTH = 1920;
const GLuint SCR_HEIGHT = 1080;

const std::string startFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";
const std::string revStart = "RNBQKBNR/PPPPPPPP/8/8/8/8/pppppppp/rnbqkbnr";
const std::string susString = "4kb1r/p4ppp/4q3/8/8/1B6/PPP2PPP/2KR4";

const GLint BOARD_SIZE = 8;

GLdouble xPos;
GLdouble yPos;


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


	// Load glad so it configures OpenGL
	if (!gladLoadGL()) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

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
		// render
		// ------
		glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// render the triangle
		boardShader.use();
		glBindVertexArray(VAO);

		GLint fbWidth, fbHeight;
		glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

		GLfloat aspectRatio = (GLfloat)fbWidth / (GLfloat)fbHeight;

		glm::mat4 projection = glm::mat4(1.0f);
		if (aspectRatio >= 1.0f) {
			projection = glm::ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f);
		}
		else {
			projection = glm::ortho(-1.0f, 1.0f, -1.0f / aspectRatio, 1.0f / aspectRatio);
		}



		boardShader.setMat4("projection", projection);
		GLfloat squareSize = 0.2f;
		GLfloat boardExtent = BOARD_SIZE * squareSize;
		GLfloat boardStart = -boardExtent / 2.0f;


		drawBoard(boardShader, squareSize, boardStart);
		// Create pieces
		Pieces initSetup;
		// Decode FEN string for start position
		initSetup.FENdecoder(susString);


		pieceShader.use();
		glBindVertexArray(VAO);


		pieceShader.setMat4("projection", projection);
		drawPieces(pieceShader, squareSize, boardStart, initSetup, textureArr);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwGetCursorPos(window, &xPos, &yPos);


		glfwWaitEvents();
	}


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
// ----------------------------------------------------------------------------------------------------------
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
GLvoid drawBoard(Shader shader, GLfloat sqSize, GLfloat bdStart) {
	for (GLint row = 0; row < BOARD_SIZE; row++) {
		for (GLint col = 0; col < BOARD_SIZE; col++) {
			GLfloat centerX = bdStart + (col + 0.5f) * sqSize;
			GLfloat centerY = bdStart + (row + 0.5f) * sqSize;

			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(centerX, centerY, 0.0f));
			model = glm::scale(model, glm::vec3(sqSize, sqSize, 1.0f));
			shader.setMat4("model", model);
			if ((col + row) % 2 == 0) {
				shader.setVec3("squareColor", 0.412f, 0.263f, 0.0f);
			}
			else {
				shader.setVec3("squareColor", 0.961f, 0.831f, 0.6f);
			}
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}
	}
}
// ---------------------------------------------------------------------------------------------------------
GLvoid drawPieces(Shader shader, GLfloat sqSize, GLfloat bdStart, Pieces pieceLayout, GLuint textures[]) {
	GLint iterator = 0;
	for (const auto& it : pieceLayout.piecePos) {
		GLint col = iterator % 8;
		GLint row = iterator / 8;

		GLfloat pieceCenterX = bdStart + (col + 0.5f) * sqSize;
		GLfloat pieceCenterY = bdStart + (row + 0.5f) * sqSize;


		if (it.currColor == colors::NONE) {

		}


		else if (it.currColor == colors::WHITE) {
			glm::mat4 pieceModel = glm::mat4(1.0f);
			pieceModel = glm::translate(pieceModel, glm::vec3(pieceCenterX, pieceCenterY, 0.0f));
			pieceModel = glm::scale(pieceModel, glm::vec3(sqSize, sqSize, 1.0f));
			shader.setMat4("model", pieceModel);

			glActiveTexture(GL_TEXTURE0);

			switch (it.currPiece) {
				case pieces::ROOK:
					glBindTexture(GL_TEXTURE_2D, textures[0]);
					break;
				case pieces::PAWN:
					glBindTexture(GL_TEXTURE_2D, textures[1]);
					break;
				case pieces::KNIGHT:
					glBindTexture(GL_TEXTURE_2D, textures[2]);
					break;
				case pieces::KING:
					glBindTexture(GL_TEXTURE_2D, textures[3]);
					break;
				case pieces::QUEEN:
					glBindTexture(GL_TEXTURE_2D, textures[4]);
					break;
				case pieces::BISHOP:
					glBindTexture(GL_TEXTURE_2D, textures[5]);
					break;
			}

			shader.setInt("pieceTexture", 0);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}


		else if (it.currColor == colors::BLACK) {
			glm::mat4 pieceModel = glm::mat4(1.0f);
			pieceModel = glm::translate(pieceModel, glm::vec3(pieceCenterX, pieceCenterY, 0.0f));
			pieceModel = glm::scale(pieceModel, glm::vec3(sqSize, sqSize, 1.0f));
			shader.setMat4("model", pieceModel);

			glActiveTexture(GL_TEXTURE0);

			switch (it.currPiece) {
			case pieces::ROOK:
				glBindTexture(GL_TEXTURE_2D, textures[6]);
				break;
			case pieces::PAWN:
				glBindTexture(GL_TEXTURE_2D, textures[7]);
				break;
			case pieces::KNIGHT:
				glBindTexture(GL_TEXTURE_2D, textures[8]);
				break;
			case pieces::KING:
				glBindTexture(GL_TEXTURE_2D, textures[9]);
				break;
			case pieces::QUEEN:
				glBindTexture(GL_TEXTURE_2D, textures[10]);
				break;
			case pieces::BISHOP:
				glBindTexture(GL_TEXTURE_2D, textures[11]);
				break;
			}

			shader.setInt("pieceTexture", 0);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}




		iterator++;
	}
}
// ---------------------------------------------------------------------------------------------------------
void screenToWorld() {
	
}