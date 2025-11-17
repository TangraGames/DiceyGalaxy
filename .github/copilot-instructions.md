# Raylib Template Project - AI Agent Instructions

## Project Overview
Minimal raylib 5.5 C template for creating cross-platform games and graphics applications. Uses static linking with pre-compiled raylib library.

## Project Structure
- `src/main.c` - Single source file for application code (currently contains letterbox window example)
- `include/` - Raylib headers (raylib.h, raymath.h, rlgl.h)
- `lib/libraylib.a` - Pre-compiled static raylib library (~7.5MB)
- `bin/` - Build output directory (generated, git-ignored)
- `resources/` - Asset storage (currently empty)
- `run.sh` - Build and run script

**Unity Build Approach**: For small games, prefer a single-file unity build where `main.c` includes other `.c` files directly. This simplifies compilation and enables better compiler optimizations. Only split into separate compilation units when build times become problematic.

## Build System
**Always use `run.sh` to build and run.** This script:
1. Creates `bin/` directory if missing
2. Removes old executable
3. Compiles with gcc using strict flags: `-Wall -Wextra -Werror -std=c99 -pedantic-errors`
4. Links against: raylib, OpenGL, X11, pthread, math, dl, rt
5. Automatically runs the executable

**Do NOT** suggest Makefiles, CMake, or automated build systems - `run.sh` is the canonical build method. Keep builds simple and manual.

## Code Conventions
- **C99 standard** - strict compliance enforced via `-std=c99 -pedantic-errors`
- **Zero warnings policy** - `-Werror` treats all warnings as errors
- **Raylib idioms**: Use raylib's built-in functions (`GetRandomValue`, `TextFormat`, `Vector2Clamp`)
- **Math operations**: Prefer `raymath.h` functions over manual math (e.g., `Vector2Add`, `Vector2Scale`, `Clamp`)
- **UI**: Use `raygui.h` for immediate-mode GUI elements (add to `include/` if needed)
- **Manual memory management**: Always pair `Load*` with `Unload*` (e.g., `LoadRenderTexture`/`UnloadRenderTexture`)
- **Preprocessor macros**: Use uppercase (e.g., `MAX`, `MIN`) - see `main.c` for examples
- **Naming conventions**: 
  - Functions: `PascalCase` for public, `camelCase` for static helpers
  - Variables: `camelCase` for locals, `PascalCase` for structs
  - Constants/macros: `UPPER_SNAKE_CASE`

## Typical Raylib Structure
Structure code with separate initialization, update, and draw functions to keep the main loop simple:

```c
void InitGame(void) {
    // Load resources, initialize game state
}

void UpdateGame(void) {
    // Game logic, input handling, physics
}

void DrawGame(void) {
    // All rendering calls
}

void UnloadGame(void) {
    // Unload all loaded resources (textures, sounds, models, etc.)
}

int main(void) {
    InitWindow(width, height, "title");
    SetTargetFPS(60);
    
    int gameScreenWidth = 640;
    int gameScreenHeight = 480;
    RenderTexture2D target = LoadRenderTexture(gameScreenWidth, gameScreenHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);
    
    InitGame();
    
    while (!WindowShouldClose()) {
        // Compute scaling for letterboxing
        float scale = MIN((float)GetScreenWidth()/gameScreenWidth, (float)GetScreenHeight()/gameScreenHeight);
        
        UpdateGame();
        
        // Draw to render texture
        BeginTextureMode(target);
            ClearBackground(COLOR);
            DrawGame();
        EndTextureMode();
        
        // Draw render texture to screen with scaling
        BeginDrawing();
            ClearBackground(BLACK);
            DrawTexturePro(target.texture, 
                          (Rectangle){ 0.0f, 0.0f, (float)target.texture.width, (float)-target.texture.height },
                          (Rectangle){ (GetScreenWidth() - ((float)gameScreenWidth*scale))*0.5f, 
                                      (GetScreenHeight() - ((float)gameScreenHeight*scale))*0.5f,
                                      (float)gameScreenWidth*scale, (float)gameScreenHeight*scale }, 
                          (Vector2){ 0, 0 }, 0.0f, WHITE);
        EndDrawing();
    }
    
    UnloadGame();
    UnloadRenderTexture(target);
    CloseWindow();
    return 0;
}
```

## Development Workflow
1. Edit `src/main.c` (use unity build by including other `.c` files directly)
2. Run `./run.sh` to compile and execute
3. If compilation fails, address gcc errors (zero-warning policy)
4. Place assets in `resources/` and reference via relative paths

**Adding Code**: For small projects, add new functionality by creating `.c` files in `src/` and including them at the top of `main.c`. This unity build approach keeps compilation simple without modifying `run.sh`.

### Debugging and Sanitizers
Use `debug.sh` to build with debug symbols and sanitizers enabled:
```bash
./debug.sh
```

This script compiles with `-g` for GDB debugging and enables AddressSanitizer (`-fsanitize=address`) and UndefinedBehaviorSanitizer (`-fsanitize=undefined`) to catch memory errors, undefined behavior, and memory leaks at runtime.

## Key Dependencies
- **Required system libraries** (Linux): OpenGL, X11, pthread, math, dl, rt
- **Include path**: `-Iinclude` (raylib headers are local, not system-wide)
- **Library path**: `-Llib` (static linking to `libraylib.a`)

## Common Patterns
- **Responsive rendering**: Use `RenderTexture2D` for letterboxing (see current `main.c`)
- **Config flags**: Set before `InitWindow()` (e.g., `FLAG_WINDOW_RESIZABLE`, `FLAG_VSYNC_HINT`)
- **Virtual coordinates**: Scale mouse input to logical game resolution (see `virtualMouse` in `main.c`)

## Game Development Best Practices
- **Delta time**: Use `GetFrameTime()` for frame-independent movement/animations
- **Resource loading**: Load textures/sounds in initialization, unload in cleanup
- **Game state**: Use enums for state machines (`typedef enum { MENU, GAMEPLAY, PAUSE } GameState`)
- **Entity management**: Use struct arrays for game objects with consistent update/draw loops
- **Input handling**: Check `IsKeyPressed()` for one-shot actions, `IsKeyDown()` for continuous
- **Performance**: Use `DrawFPS()` during development; profile with `GetFPS()` and `GetFrameTime()`

## When Adding New Files
**Prefer unity build**: Include additional `.c` files directly in `main.c` at the top after raylib includes:

```c
// Example: player.c, enemy.c, collision.c included in main.c
```

This keeps `run.sh` unchanged and provides faster compilation. Use header guards in included files if needed. Only switch to separate compilation when project becomes large enough that full rebuilds are slow.
