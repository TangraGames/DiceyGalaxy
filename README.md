# DiceyGalaxy
A 4X game of skill and luck on a galactic scale! Work in progress...

## Project Structure

```
DiceyGalaxy/
├── src/
│   ├── main.c           # Main game loop and rendering
│   └── utils_hexmap.c   # Hexagonal grid utilities and tile system
├── include/
│   ├── raylib.h         # Raylib header
│   ├── raymath.h        # Raylib math utilities
│   └── rlgl.h           # Raylib OpenGL abstraction
├── lib/
│   └── libraylib.a      # Pre-compiled static raylib library
├── bin/                 # Build output (git-ignored)
├── resources/           # Game assets (textures, sounds, etc.)
├── run.sh               # Build and run script
└── debug.sh             # Build with debug symbols and sanitizers
```

## Build System

**Always use `./run.sh` to build and run the game.**

```bash
./run.sh        # Compile and run
./debug.sh      # Compile with debug symbols and sanitizers
```

The build script:
- Creates `bin/` directory if needed
- Compiles with strict flags: `-Wall -Wextra -Werror -std=c99 -pedantic-errors`
- Links against: raylib, OpenGL, X11, pthread, math, dl, rt
- Automatically runs the executable

**Do NOT use Makefiles or CMake** - the shell script approach keeps builds simple and manual.

## Architecture

### Unity Build Pattern
The project uses a **unity build** approach where `main.c` includes `utils_hexmap.c` directly. This:
- Simplifies compilation (single compilation unit)
- Enables better compiler optimizations
- Keeps the build process straightforward
- Is ideal for small to medium projects

Only split into separate compilation units when build times become problematic.

### Hexagonal Grid System (`utils_hexmap.c`)

**Coordinate System**: Uses **cube coordinates** (q, r, s) where `q + r + s = 0`
- Based on [Red Blob Games hexagonal grid guide](https://www.redblobgames.com/grids/hexagons/)
- Supports both flat-top and pointy-top orientations
- Non-uniform hex sizes supported for coordinate calculations

**Key Structures:**
```c
Hex      - Cube coordinate position (q, r, s)
Tile     - Game tile with position, type, walkability, selection state
TileType - Enum of terrain types (GRASS, WATER, ROCKS, SAND, FOREST)
Map      - Hexagonal map with dynamic tile array
Layout   - Defines hex orientation, size, and origin for rendering
```

**Key Functions:**
- `MakeHex()` - Create hex with validation
- `CreateMap()` - Generate hexagonal map with radius
- `DestroyMap()` - Free map memory
- `GetTileAt()` - Find tile by hex coordinates
- `SetTileType()` - Change terrain type
- `SetTileSelected()` - Manage tile selection
- `HexToPixel()` / `PixelToHex()` - Coordinate conversion
- `HexDistance()` - Calculate distance between hexes
- `HexNeighbor()` - Get adjacent hex in a direction

**Layout Configuration:**
The `Layout` struct defines how hexagons are positioned and sized. The `size` parameter represents the hex radius and affects spacing:

For pointy-top hexagons:
- Horizontal spacing between centers = `√3 × size.x`
- Vertical spacing between centers = `1.5 × size.y`

**Important:** When using texture-based tiles, the mathematical ratios (`size.x = tile_width / √3`, `size.y = tile_height / 1.5`) may need adjustment to eliminate visual gaps. Typical adjustments:
- Keep horizontal: `size.x = tile_width / 1.732`
- Adjust vertical: `size.y = tile_height / (1.8 to 2.2)` - experiment to find the best fit for your specific tile artwork

This is because tile artwork often has transparent areas or overlapping edges that don't match perfect hexagonal geometry.

### Current Game State

**Implemented Features:**
- ✅ Hexagonal map with 5-tile radius (91 tiles)
- ✅ Tile system with terrain types and properties
- ✅ Color-coded terrain rendering
- ✅ Interactive tile selection (left-click)
- ✅ Terrain type cycling (right-click, for testing)
- ✅ Responsive window with letterboxing
- ✅ Virtual mouse coordinate mapping
- ✅ Proper memory management

**Rendering Note:**
Current implementation uses `DrawPoly()` which only supports **uniform (circular) hexagons**. For non-uniform hex sizes or detailed sprites, use texture-based rendering in production. The coordinate system in `utils_hexmap.c` fully supports non-uniform scaling.

### Memory Management
- Map uses **dynamic memory allocation** (`malloc`/`free`)
- Always pair `CreateMap()` with `DestroyMap()`
- No hardcoded tile count limits
- Zero memory leaks with proper cleanup

### Code Conventions
- **C99 standard** with strict compliance
- **Zero warnings policy** (`-Werror`)
- Raylib idioms and built-in functions preferred
- Manual memory management (always pair Load/Unload)
- Naming: `PascalCase` for types/functions, `camelCase` for variables, `UPPER_SNAKE_CASE` for macros

## Controls
- **Left-click**: Select tile (brightens color, yellow outline)
- **Right-click**: Cycle terrain types (testing feature)
- **ESC**: Exit game

## Next Steps
- Add game units and entities
- Implement texture-based tile rendering
- Add pathfinding using `HexDistance()` and walkability
- Implement fog of war
- Add game mechanics (resources, buildings, combat)
- Sound effects and music

## Dependencies
- **raylib 5.5+** (statically linked)
- **System libraries** (Linux): OpenGL, X11, pthread, math, dl, rt
- **C99 compiler** (gcc recommended)
