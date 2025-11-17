/*
    This is a utility file to help working with hexmaps using cube coordinates for c games in raylib.

    The implementation is based on the article "Hexagonal Grids" by Red Blob Games.
    (https://www.redblobgames.com/grids/hexagons/)

    Functions provided in this file include:
    ------------------------------------------------------------------------
    - MakeHex: Creates a hex with given cube coordinates.
    - HexDistance: Calculates the distance between two hexes.
    - HexNeighbor: Gets the neighboring hex in a specified direction.
    - HexAdd: Adds two hexes together.
    - HexSubtract: Subtracts one hex from another.
    - HexScale: Scales a hex by a given factor.
    - HexLength: Computes the length of a hex from the origin.
    - HexEquals: Checks if two hexes are equal.
    - HexDirection: Returns the direction vector for a given direction index.
    - CreateMap: Creates a map with tiles in a given radius around center.
    - DestroyMap: Frees the memory allocated for the map.
    - GetTileAt: Retrieves a tile at a specific hex position.
    - SetTileType: Changes the terrain type of a tile.
    - SetTileSelected: Sets the selection state of a tile.
    - GetTileColor: Returns the color associated with a tile type.

    Data Structures and definitions provided in this file:
    ------------------------------------------------------------------------
    - Point: 2D coordinate structure (x, y).
    - Hex: Cube coordinate structure (q, r, s).
    - TileType: Enumeration of terrain types (GRASS, WATER, ROCKS, etc.).
    - Tile: Game tile with position, type, and properties.
    - Map: Structure to hold a hexagonal map with center, size, and tile array.
    - Direction vectors for hex neighbors.
    - Layout: Structure to define hex layout (orientation, size, origin).
    - Orientation: Structure to define hex orientation (flat-topped or pointy-topped).
    - Predefined orientations for flat-topped and pointy-topped hexes.
*/

#include <raylib.h>
#include <stdlib.h>

#define SQRT3 1.73205080757f // Square root of 3

typedef struct {
    float x;
    float y;
} Point;

typedef struct Hex {
    int q; // Cube coordinate q
    int r; // Cube coordinate r
    int s; // Cube coordinate s
} Hex;

typedef struct FractionalHex {
    float q;
    float r;
    float s;
} FractionalHex;

typedef enum TileType {
    TILE_GRASS = 0,
    TILE_WATER,
    TILE_ROCKS,
    TILE_SAND,
    TILE_FOREST
} TileType;

typedef struct Tile {
    Hex position;       // Hex coordinate (q, r, s)
    TileType type;      // Terrain type
    bool isWalkable;    // Can units move through?
    bool isSelected;    // Is this tile currently selected?
} Tile;

typedef struct Map {
    Point center;       // Center position (origin) of hex (0,0,0)
    Point hexSize;      // Size of each hex
    int radius;         // Map radius (tiles from center)
    Tile* tiles;        // Dynamic array of tiles
    int tileCount;      // Number of tiles in the map
} Map;

FractionalHex MakeFractionalHex(float q, float r, float s) {
    FractionalHex hex;

    if (fabsf(q + r + s) > 0.0001f) {
        TraceLog(LOG_ERROR, "Invalid fractional cube coordinates: q=%f, r=%f, s=%f (sum must be 0)", q, r, s);
        hex.q = 0;
        hex.r = 0;
        hex.s = 0;
    }   
    
    else {
        hex.q = q;
        hex.r = r;
        hex.s = s;
    }
    return hex;
}

Hex MakeHex(int q, int r, int s) {
    Hex hex;

    if (q + r + s != 0) {
        TraceLog(LOG_ERROR, "Invalid cube coordinates: q=%d, r=%d, s=%d (sum must be 0)", q, r, s);
        hex.q = 0;
        hex.r = 0;
        hex.s = 0;
    }

    else {
        hex.q = q;
        hex.r = r;
        hex.s = s;
    }

    return hex;
}

bool HexEquals(Hex a, Hex b) {
    return (a.q == b.q) && (a.r == b.r) && (a.s == b.s);
}

Hex HexAdd(Hex a, Hex b) {
    return MakeHex(a.q + b.q, a.r + b.r, a.s + b.s);
}

Hex HexSubtract(Hex a, Hex b) {
    return MakeHex(a.q - b.q, a.r - b.r, a.s - b.s);
}

Hex HexScale(Hex hex, int k) {
    return MakeHex(hex.q * k, hex.r * k, hex.s * k);
}

int HexLength(Hex hex) {
    return (abs(hex.q) + abs(hex.r) + abs(hex.s)) / 2;
}

int HexDistance(Hex a, Hex b) {
    return HexLength(HexSubtract(a, b));
}

Hex HexRound(FractionalHex h) {
    int q = roundf(h.q);
    int r = roundf(h.r);
    int s = roundf(h.s);

    float q_diff = fabsf(q - h.q);
    float r_diff = fabsf(r - h.r);
    float s_diff = fabsf(s - h.s);

    if (q_diff > r_diff && q_diff > s_diff) {
        q = -r - s;
    } 
    else if (r_diff > s_diff) {
        r = -q - s;
    } 
    else {
        s = -q - r;
    }

    return MakeHex(q, r, s);
}

static const Hex directions[] = {
    { 1, -1, 0}, 
    { 1, 0, -1}, 
    { 0, 1, -1},
    {-1, 1, 0}, 
    {-1, 0, 1}, 
    { 0, -1, 1}
};

Hex HexDirection(int direction) {
    if (direction < 0 || direction >= 6) {
        TraceLog(LOG_ERROR, "Invalid direction: %d (must be between 0 and 5)", direction);
    }
    int dir = (6 + (direction % 6)) % 6; // Ensure direction is within 0-5
    return directions[dir]; 
}

Hex HexNeighbor(Hex hex, int direction) {
    return HexAdd(hex, HexDirection(direction));
}

typedef struct Orientation {
    float f0, f1, f2, f3;
    float b0, b1, b2, b3;
    float start_angle; // in multiples of 60°
} Orientation;

static const Orientation layout_flat = {
    3.0f / 2.0f, 0.0f,
    SQRT3 / 2.0f, SQRT3,
    2.0f / 3.0f, 0.0f,
    -1.0f / 3.0f, SQRT3 / 3.0f,
    0.0f
};

static const Orientation layout_pointy = {
    SQRT3, SQRT3 / 2.0f,
    0.0f, 3.0f / 2.0f,
    SQRT3 / 3.0f, -1.0f / 3.0f,
    0.0f, 2.0f / 3.0f,
    0.5f
};

typedef struct Layout {
    Orientation orientation;
    Point size;
    Point origin;
} Layout;

Layout MakeLayout(Orientation orientation, Point size, Point origin) {
    Layout layout;
    layout.orientation = orientation;
    layout.size = size;
    layout.origin = origin;
    return layout;
}

Point HexToPixel(Layout layout, Hex hex) {
    Orientation M = layout.orientation;
    float x = (M.f0 * hex.q + M.f1 * hex.r) * layout.size.x;
    float y = (M.f2 * hex.q + M.f3 * hex.r) * layout.size.y;
    return (Point){ x + layout.origin.x, y + layout.origin.y };
}

Hex PixelToHex(Layout layout, Point p) {
    Orientation M = layout.orientation;
    Point pt = { (p.x - layout.origin.x) / layout.size.x,
                 (p.y - layout.origin.y) / layout.size.y };
    float q = M.b0 * pt.x + M.b1 * pt.y;
    float r = M.b2 * pt.x + M.b3 * pt.y;
    return HexRound(MakeFractionalHex(q, r, -q - r));
}

Point HexCornerOffset(Layout layout, int corner) {
    Orientation M = layout.orientation;
    float angle = 2.0f * 3.14159265359f * (M.start_angle + corner) / 6.0f;
    return (Point){ layout.size.x * cosf(angle), layout.size.y * sinf(angle) };
}

void PolygonCorners(Layout layout, Hex hex, Point* corners) {
    Point center = HexToPixel(layout, hex);
    for (int i = 0; i < 6; i++) {
        Point offset = HexCornerOffset(layout, i);
        corners[i] = (Point){ center.x + offset.x, center.y + offset.y };
    }
}

// Map creation and management functions

Map CreateMap(Point center, Point hexSize, int radius) {
    Map map;
    map.center = center;
    map.hexSize = hexSize;
    map.radius = radius;
    
    // Calculate maximum number of tiles (3 * radius^2 + 3 * radius + 1)
    int maxTiles = 3 * radius * radius + 3 * radius + 1;
    map.tiles = (Tile*)malloc(maxTiles * sizeof(Tile));
    map.tileCount = 0;
    
    // Populate map with tiles
    for (int q = -radius; q <= radius; q++) {
        for (int r = -radius; r <= radius; r++) {
            int s = -q - r;
            if (abs(s) <= radius) {
                Tile tile;
                tile.position = MakeHex(q, r, s);
                tile.type = TILE_GRASS;  // Default terrain type
                tile.isWalkable = true;  // Grass is walkable by default
                tile.isSelected = false;
                map.tiles[map.tileCount] = tile;
                map.tileCount++;
            }
        }
    }
    
    return map;
}

void DestroyMap(Map* map) {
    if (map->tiles != NULL) {
        free(map->tiles);
        map->tiles = NULL;
        map->tileCount = 0;
    }
}

// Tile helper functions

Tile* GetTileAt(Map* map, Hex position) {
    for (int i = 0; i < map->tileCount; i++) {
        if (HexEquals(map->tiles[i].position, position)) {
            return &map->tiles[i];
        }
    }
    return NULL;  // Tile not found
}

void SetTileType(Map* map, Hex position, TileType type) {
    Tile* tile = GetTileAt(map, position);
    if (tile != NULL) {
        tile->type = type;
        // Update walkability based on type
        tile->isWalkable = (type != TILE_WATER && type != TILE_ROCKS);
    }
}

void SetTileSelected(Map* map, Hex position, bool selected) {
    // First deselect all tiles if selecting a new one
    if (selected) {
        for (int i = 0; i < map->tileCount; i++) {
            map->tiles[i].isSelected = false;
        }
    }
    
    // Set the target tile's selection state
    Tile* tile = GetTileAt(map, position);
    if (tile != NULL) {
        tile->isSelected = selected;
    }
}

Color GetTileColor(TileType type) {
    switch(type) {
        case TILE_GRASS:  return (Color){34, 139, 34, 255};   // Forest green
        case TILE_WATER:  return (Color){30, 144, 255, 255};  // Dodger blue
        case TILE_ROCKS:  return (Color){128, 128, 128, 255}; // Gray
        case TILE_SAND:   return (Color){244, 164, 96, 255};  // Sandy brown
        case TILE_FOREST: return (Color){0, 100, 0, 255};     // Dark green
        default:          return LIGHTGRAY;
    }
}
