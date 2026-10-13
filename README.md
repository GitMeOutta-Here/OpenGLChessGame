# OpenGL Chess

A two-player chess board written in C++17 with modern OpenGL (4.4 core profile), GLFW, GLM, stb_image and Dear ImGui. You move pieces by dragging them with the mouse or by clicking, and a side panel shows whose turn it is and has a button that resets the board.

The project builds on Windows (Visual Studio + CMake) and on Linux (Fedora 43 + CMake).

![Starting position with the ImGui side panel](docs/screenshot.png)

> **Status:** the board, the pieces, drag-and-drop and turn order work. The game does not check chess rules yet: a piece can move to any square that is empty or holds an enemy piece. Rules are the next milestone (see [Remaining tasks](#remaining-tasks)).

---

## Contents

- [Features so far](#features-so-far)
- [Controls](#controls)
- [Project layout](#project-layout)
- [Dependencies](#dependencies)
- [Building on Windows](#building-on-windows-visual-studio)
- [Building on Linux (Fedora 43)](#building-on-linux-fedora-43)
- [How it works](#how-it-works)
- [Remaining tasks](#remaining-tasks)
- [Housekeeping](#housekeeping)

---

## Features so far

| Area | What works |
| --- | --- |
| Rendering | 8×8 board drawn as 64 scaled copies of one unit quad; 12 PNG piece textures with alpha blending; an aspect-correct orthographic projection, so the board stays square when the window is resized |
| Board state | `Pieces` class holding a 64-square array, loaded from a FEN-style placement string; square lookup with `at(col, row)` |
| Input | Mouse picking that converts window pixels to board squares, including resized and high-DPI windows |
| Drag-and-drop | Press to lift a piece, which follows the cursor drawn on top of the others at 110% size. The target square is highlighted, and releasing drops the piece there. An illegal drop, a drop off the board or a right-click sends the piece back to its square |
| Click-to-move | Click a piece to select it (red square), then click the destination square. Clicking the selected piece again deselects it |
| Turn order | Only the side to move can pick up pieces; the turn passes after each move |
| Captures | Moving onto an enemy piece removes it; moving onto your own piece is refused |
| UI | Dear ImGui panel beside the board showing "White/Black to move" and a **Reset board** button |
| Platforms | Windows (MSVC, prebuilt GLFW) and Linux (GCC, system GLFW) |

## Controls

| Action | Input |
| --- | --- |
| Move a piece | Left-drag it to the target square, **or** click it, then click the target square |
| Cancel a drag | Right-click while dragging, or release outside the board |
| Deselect | Click the selected piece again, or click off the board |
| Reset the game | **Reset board** button in the side panel |
| Quit | `Esc` |

## Project layout

```
OpenGL/
├── CMakeLists.txt          Windows build (MSVC + prebuilt GLFW in Dependencies/GLFW)
├── linux/
│   └── CMakeLists.txt      Linux build (system GLFW + OpenGL packages)
├── README.md
├── docs/
│   └── screenshot.png
├── src/
│   ├── main.cpp            Window, render loop, input callbacks, drawing, ImGui panel
│   ├── ChessLogic.h        pieces/colors enums, Pieces board class, FEN decoder, move rules
│   ├── shader.h            Shader loader (compile, link, uniform setters)
│   ├── boardShaders/       bShader.vert / bShader.frag: solid colour per square
│   ├── pieceShaders/       pShader.vert / pShader.frag: textured quad, discards transparent pixels
│   └── ChessPieces/        b-*.png and w-*.png piece images (12 files)
└── Dependencies/
    ├── glad/  KHR/         OpenGL 4.4 core function loader
    ├── GLFW/               Headers + prebuilt Windows libraries (lib-vc2022 is used)
    ├── glm/                Header-only maths library
    ├── stb/                stb_image (PNG loading); stb_image.cpp holds the implementation
    └── imGui/imgui-master/ Dear ImGui 1.92.x, with the GLFW and OpenGL3 backends
```

## Dependencies

| Library | Purpose | Windows | Linux |
| --- | --- | --- | --- |
| OpenGL 4.4 core | Rendering | `opengl32` (system) | `mesa-libGL-devel` or your GPU driver |
| GLFW 3.4 | Window, context, mouse and keyboard | Bundled `Dependencies/GLFW/lib-vc2022/glfw3.lib` | `glfw-devel` package |
| GLAD | Loads OpenGL function pointers | Bundled source | Bundled source |
| GLM | Matrices and vectors (`ortho`, `translate`, `inverse`) | Bundled headers | Bundled headers |
| stb_image | Loads the piece PNGs | Bundled header | Bundled header |
| Dear ImGui | Side panel UI | Bundled source | Bundled source |

## Building on Windows (Visual Studio)

1. Open the `OpenGL` folder in Visual Studio with **File → Open → Folder**. Visual Studio detects `CMakeLists.txt`.
2. Select the `x64-Debug` configuration and build (**Build → Build All**).
3. Run `OpenGL.exe`. The build output is in `out/build/x64-Debug/`, and the post-build steps copy `boardShaders/`, `pieceShaders/` and `ChessPieces/` next to the executable.

The root `CMakeLists.txt` currently asks for CMake 4.0 or newer and links the Visual Studio 2022 build of GLFW. `Dependencies/GLFW/include/GLFW` needs both `glfw3.h` and `glfw3native.h`, because the ImGui GLFW backend includes the latter on Windows.

## Building on Linux (Fedora 43)

The Linux build uses `linux/CMakeLists.txt`, which links the system GLFW and OpenGL libraries instead of the Windows `.lib` files. Everything else (GLAD, GLM, stb_image, ImGui) is compiled from `Dependencies/`, the same as on Windows.

```bash
# one-time setup
sudo dnf install cmake gcc-c++ glfw-devel mesa-libGL-devel

# configure and build, from the project root
cmake -S linux -B build-linux -DCMAKE_BUILD_TYPE=Debug
cmake --build build-linux -j

# run from the build folder (assets are loaded with relative paths)
cd build-linux
./OpenGL
```
---

## How it works

### Coordinates

- The board covers world coordinates **−0.8 to 0.8** on both axes (`squareSize = 0.2`, `boardStart = −0.8`).
- `makeProjection()` builds an orthographic projection that keeps squares square for any window shape. Drawing and mouse picking both call it, so they always agree.
- **Row 0 is drawn at the bottom.** The board array index is `row * 8 + col`, so index 0 is a1 (bottom-left from White's side) and index 63 is h8.

### Board state and FEN

`Pieces::FENdecoder` fills the array **in string order starting at index 0**. Standard FEN lists rank 8 first, but index 0 is a1, so the game loads `revStart` (`"RNBQKBNR/PPPPPPPP/8/8/8/8/pppppppp/rnbqkbnr"`), the starting position with the ranks reversed. Loading a standard FEN string directly would put Black's pieces on ranks 1–2 and White's on ranks 7–8. Making the decoder read standard FEN is one of the tasks below.

### Game state

```cpp
struct GameState {
    Pieces    board;                     // 64 squares + sideToMove
    DragState drag;                      // active, origin square, lifted piece, cursor (world)
    GLint     selectedCol, selectedRow;  // red highlight / click-to-move origin
    GLint     hoverCol, hoverRow;        // drop-target highlight while dragging
};
```

One `GameState` lives in `main()`. It is registered with `glfwSetWindowUserPointer` so the GLFW callbacks can reach it without global variables. **Reset board** assigns a fresh `GameState` in place, so that pointer stays valid.

### Input flow

1. **Press** on a piece of the side to move: the piece is *lifted*. The board array is not changed; the piece is only hidden on its square.
2. **Cursor moves**: `cursor_pos_callback` updates the lifted piece's world position and the hovered square.
3. **Release** on another square: `Pieces::movePiece()` is called. If the move is refused, nothing changed, so the piece reappears on its original square ("snap back" needs no code of its own).
4. Clicks over the ImGui panel are ignored by the board (`ImGui::GetIO().WantCaptureMouse`).

All rule decisions go through two functions in `ChessLogic.h`, `canPickUp()` and `isMoveAllowed()`. The chess rules below can be added there without touching input or rendering.

### Render order (each frame)

1. Board squares, with selection and drop-target tints (`boardShader`)
2. All pieces except the lifted one (`pieceShader`)
3. The lifted piece at the cursor, drawn last so it is on top
4. The ImGui panel

The loop waits on `glfwWaitEventsTimeout(0.1)`, so the game uses almost no CPU while idle but still gives ImGui the extra frames it needs.

---

## Remaining tasks

The tasks are listed roughly in build order. The move-legality tasks all go into `ChessLogic.h`, mostly inside `isMoveAllowed()`.

### 1. Correct movement for each piece

- [ ] **Pawn:** one square forward onto an empty square; two squares from its starting rank if both squares are empty; diagonal one square only when capturing. Direction depends on colour (White goes up, toward row 7).
- [ ] **Knight:** the eight L-shaped jumps; it jumps over pieces.
- [ ] **Bishop / Rook / Queen:** slide along diagonals, ranks/files, or both. Every square between start and target must be empty.
- [ ] **King:** one square in any direction (plus castling, below).
- [ ] Suggested structure: a `generateMoves(col, row)` function returning a list of target squares. `isMoveAllowed()` then checks whether the target is in that list, and the same list can drive legal-move highlighting.

### 2. Check, checkmate and stalemate

- [ ] `isSquareAttacked(col, row, byColor)`: does any enemy piece reach this square?
- [ ] Reject any move that leaves your own king in check (make the move on a copy of the board, test, discard).
- [ ] After each move: no legal moves and in check means **checkmate**; no legal moves and not in check means **stalemate**.
- [ ] Show the result in the ImGui panel and highlight a king that is in check.

### 3. Castling

- [ ] Track castling rights: four flags (White/Black, king side/queen side), cleared when the king moves, when that rook moves, or when that rook is captured.
- [ ] Allowed only if the squares between king and rook are empty, the king is not in check, and the king does not pass through or land on an attacked square.
- [ ] The king moves two squares. `movePiece()` must also move the rook (h1→f1 / a1→d1 for White, h8→f8 / a8→d8 for Black).
- [ ] Dragging the king two squares should trigger castling. Dropping the king onto its own rook could also be accepted (some games allow this).

### 4. En passant

- [ ] Store an en passant target square after every two-square pawn move (the square the pawn skipped over); clear it after the next move.
- [ ] A pawn may capture diagonally onto that square. The captured pawn is removed from the square *beside* the capturing pawn, not from the target square.
- [ ] The capture must also pass the king-safety check from task 2 (en passant can expose a king along a rank).

### 5. Pawn promotion

- [ ] When a pawn reaches the last rank (row 7 for White, row 0 for Black), replace it with a queen, rook, bishop or knight.
- [ ] Ask with an ImGui modal popup (`ImGui::OpenPopup` / `BeginPopupModal`) showing the four piece images (`ImGui::ImageButton` with the existing textures). While the choice is open, the game waits and the board ignores clicks.
- [ ] A simpler first step: always promote to a queen, then add the popup.

### 6. Render the board from Black's side

- [ ] Add a `bool flipped` to `GameState` and a **Flip board** button or checkbox in the ImGui panel.
- [ ] Keep the board array as it is and flip only the display, with a single mapping used everywhere:
  `displayCol = flipped ? 7 - col : col;  displayRow = flipped ? 7 - row : row;`
- [ ] Apply it in three places: `drawBoard` and `drawPieces` (where squares are drawn) and `worldToSquare` (which converts the mouse back to a board square). The drag code needs no changes.
- [ ] Square colours stay correct automatically, because `(col + row) % 2` does not change under the flip.
- [ ] Optional: flip automatically when a game starts as Black, or after every move for a pass-and-play mode.

### 7. Full FEN support

- [ ] Make `FENdecoder` read standard FEN (rank 8 first) by writing rank `r` into row `7 - r`. Then `startFEN` can be used directly and `revStart` can be removed.
- [ ] Parse the remaining FEN fields: side to move, castling rights, en passant square, half-move clock, full-move number.
- [ ] Add `toFEN()` for saving and loading positions, and add bounds checks so malformed strings cannot write past square 63.

### 8. Draw rules

- [ ] Fifty-move rule (half-move clock reaches 100).
- [ ] Threefold repetition (keep a history of positions, for example FEN strings without the clocks, or Zobrist hashes).
- [ ] Insufficient material (K vs K, K+B vs K, K+N vs K, and same-colour bishops).

### 9. Game experience

- [ ] Show legal target squares (dots or tints) while a piece is dragged or selected.
- [ ] Highlight the last move's origin and target squares.
- [ ] Move list in the ImGui panel using algebraic notation (e4, Nf3, O-O, exd5, e8=Q+).
- [ ] Undo / redo using a move history (store each move plus what it captured and the previous castling/en passant state).
- [ ] Optional: a faded "ghost" piece on the origin square while dragging (add an `alpha` uniform to `pShader.frag`), cancel a drag when the window loses focus (`glfwSetWindowFocusCallback`), captured-piece trays, clocks, a simple computer opponent.

### 10. Testing

- [ ] Perft tests (count all legal moves to a given depth from known positions and compare with published totals) to catch move-generation bugs, especially castling, en passant and promotion.
- [ ] Keep `ChessLogic.h` free of OpenGL so it can be tested in a small console program.

---

## Housekeeping

Small clean-ups, none of which block the tasks above:

- [ ] Delete the stale `x64/shader.h` copy (the build uses `src/shader.h`).
- [ ] Remove or use the leftover `susString` and the declared-but-never-defined `pieceToChar()` in `ChessLogic.h`.
- [ ] Add `build-linux/` and `x64/` to `.gitignore`.
- [ ] Consider merging both CMake files into one with `if(WIN32) … else() … endif()` once the Linux build is confirmed on Fedora.
- [ ] Load assets relative to the executable's folder instead of the working directory, so the game can be launched from anywhere.
