/*
    Credits: 
    Raylib letterbox example: https://www.raylib.com/examples/core/loader.html?name=core_window_letterbox
    RedBlob Games hex grid guide: (https://www.redblobgames.com/grids/hexagons/)
*/
#include "raylib.h"
#include "raymath.h"        // Required for: Vector2Clamp()
#include "utils_hexmap.c"

#define MAX(a, b) ((a)>(b)? (a) : (b))
#define MIN(a, b) ((a)<(b)? (a) : (b))
#define MAP_RADIUS 5

// Tileset configuration for terrain.png
#define TILE_WIDTH 120
#define TILE_HEIGHT 140
#define TILE_PADDING 1
#define TILESET_COLUMNS 7
#define TILESET_ROWS 14
#define SCALE_FACTOR 2 // Scale image to 1/5th size

// Scaled tile dimensions
#define SCALED_TILE_WIDTH (TILE_WIDTH / SCALE_FACTOR)
#define SCALED_TILE_HEIGHT (TILE_HEIGHT / SCALE_FACTOR)

//------------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------------
static int gameScreenWidth = 800;
static int gameScreenHeight = 800;

static Layout hexLayout;
static Map map;
static Texture2D tilesetTexture;

//------------------------------------------------------------------------------------
// Module Functions
//------------------------------------------------------------------------------------
static float getScreenScale(void)
{
    return MIN((float)GetScreenWidth()/gameScreenWidth, (float)GetScreenHeight()/gameScreenHeight);
}

static Vector2 getVirtualMouse(void)
{
    float scale = getScreenScale();
    Vector2 mouse = GetMousePosition();
    Vector2 virtualMouse = {0};
    
    virtualMouse.x = (mouse.x - (GetScreenWidth() - (gameScreenWidth*scale))*0.5f)/scale;
    virtualMouse.y = (mouse.y - (GetScreenHeight() - (gameScreenHeight*scale))*0.5f)/scale;
    virtualMouse = Vector2Clamp(virtualMouse, (Vector2){0, 0}, (Vector2){(float)gameScreenWidth, (float)gameScreenHeight});
    
    return virtualMouse;
}


// Get the source rectangle from tileset for a given tile type
// Tileset has tiles in 7 columns × 14 rows, each 120×140 pixels with 1px padding
// After scaling to 1/5th, tiles are 24×28 pixels with padding scaled down
// Tile types map to grid positions: GRASS=0, WATER=1, ROCKS=2, SAND=3, FOREST=4
static Rectangle getTileSourceRect(TileType type)
{
    // Calculate row and column from tile type (reading left-to-right, top-to-bottom)
    int col = type % TILESET_COLUMNS;
    int row = type / TILESET_COLUMNS;
    
    // Account for scaled padding between tiles
    float scaledPadding = (float)TILE_PADDING / SCALE_FACTOR;
    float x = col * (SCALED_TILE_WIDTH + scaledPadding);
    float y = row * (SCALED_TILE_HEIGHT + scaledPadding);
    
    return (Rectangle){ x, y, (float)SCALED_TILE_WIDTH, (float)SCALED_TILE_HEIGHT };
}

static void initGame(void)
{
    // Load tileset texture from terrain.png (7 columns × 14 rows, 120×140px tiles, 1px padding)
    Image tilesetImage = LoadImage("resources/terrain.png");
    if (!IsImageValid(tilesetImage))
    {
        TraceLog(LOG_ERROR, "Failed to load tileset image: resources/terrain.png");
    }
    else
    {
        // Scale image to 1/10th size
        ImageResize(&tilesetImage, tilesetImage.width / SCALE_FACTOR, tilesetImage.height / SCALE_FACTOR);
        tilesetTexture = LoadTextureFromImage(tilesetImage);
        UnloadImage(tilesetImage);
    }
    
    // Initialize hex layout with pointy-top orientation
    // For pointy-top hexagons, we need to calculate proper spacing:
    // - Horizontal spacing between centers = √3 * size.x (should equal tile width)
    // - Vertical spacing between centers = 1.5 * size.y
    // To decrease vertical spacing (more overlap), increase the divisor
    Point size = { 
        (float)SCALED_TILE_WIDTH / SQRT3,   // √3 ≈ 1.732
        (float)SCALED_TILE_HEIGHT / 2.0f    // Increased divisor from 1.5 to 2 to decrease vertical spacing between tiles
    };
    Point origin = { gameScreenWidth / 2.0f, gameScreenHeight / 2.0f };
    hexLayout = MakeLayout(layout_pointy, size, origin);
    
    // Create map using the utility function
    map = CreateMap(origin, size, MAP_RADIUS);
    
    // Set center tile to a different type for testing
    Hex centerHex = MakeHex(0, 0, 0);
    SetTileType(&map, centerHex, TILE_WATER);
    
    // Add some variety to the map
    SetTileType(&map, MakeHex(1, -1, 0), TILE_ROCKS);
    SetTileType(&map, MakeHex(-1, 1, 0), TILE_SAND);
    SetTileType(&map, MakeHex(0, 1, -1), TILE_FOREST);
}

static void updateGame(void)
{
    // Handle mouse click to select tile
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        Vector2 virtualMouse = getVirtualMouse();
        Point mousePoint = { virtualMouse.x, virtualMouse.y };
        Hex clickedHex = PixelToHex(hexLayout, mousePoint);
        SetTileSelected(&map, clickedHex, true);
    }
    
    // Handle right click to change tile type (for testing)
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        Vector2 virtualMouse = getVirtualMouse();
        Point mousePoint = { virtualMouse.x, virtualMouse.y };
        Hex clickedHex = PixelToHex(hexLayout, mousePoint);
        
        Tile* tile = GetTileAt(&map, clickedHex);
        if (tile != NULL) {
            // Cycle through tile types
            tile->type = (tile->type + 1) % 5;
            tile->isWalkable = (tile->type != TILE_WATER && tile->type != TILE_ROCKS);
        }
    }
}

static void drawGame(void)
{
    ClearBackground(RAYWHITE);
    
    if (!IsTextureValid(tilesetTexture))
    {
        DrawText("ERROR: Tileset not loaded!", 10, 10, 20, RED);
        return;
    }
    
    // Track selected tile for info display
    Tile* selectedTile = NULL;
    Point selectedCenter = {0, 0};
    
    // Draw all tiles from the map
    for (int i = 0; i < map.tileCount; i++)
    {
        Tile tile = map.tiles[i];
        
        // Get hex center
        Point center = HexToPixel(hexLayout, tile.position);
        
        // Track selected tile
        if (tile.isSelected)
        {
            selectedTile = &map.tiles[i];
            selectedCenter = center;
        }
        
        // Get source rectangle from tileset
        Rectangle sourceRect = getTileSourceRect(tile.type);
        
        // Calculate destination rectangle (centered on hex position)
        // Use scaled tile dimensions
        Rectangle destRect = {
            center.x - (float)SCALED_TILE_WIDTH / 2.0f,
            center.y - (float)SCALED_TILE_HEIGHT / 2.0f,
            (float)SCALED_TILE_WIDTH,
            (float)SCALED_TILE_HEIGHT
        };
        
        // Tint color (highlight if selected)
        Color tint = WHITE;
        if (tile.isSelected)
        {
            tint = YELLOW;  // Tint selected tiles yellow
        }
        
        // Draw the tile texture
        DrawTexturePro(tilesetTexture, sourceRect, destRect, (Vector2){0, 0}, 0.0f, tint);
    }
    
    // Top info
    DrawText(TextFormat("Map Tiles: %d | Left-click: select | Right-click: change terrain", 
             map.tileCount), 10, 10, 20, BLACK);
    
    // Bottom info - show selected tile coordinates
    if (selectedTile != NULL)
    {
        DrawText(TextFormat("Selected Tile - Cube: (q:%d, r:%d, s:%d) | Screen: (%.1f, %.1f)", 
                 selectedTile->position.q, selectedTile->position.r, selectedTile->position.s,
                 selectedCenter.x, selectedCenter.y), 
                 10, gameScreenHeight - 30, 20, DARKGREEN);
    }
    else
    {
        DrawText("No tile selected", 10, gameScreenHeight - 30, 20, GRAY);
    }
}
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    // Enable config flags for resizable window and vertical synchro
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(screenWidth, screenHeight, "responsive window-letterbox example");
    SetWindowMinSize(320, 240);

    // Render texture initialization, used to hold the rendering result so we can easily resize it
    RenderTexture2D target = LoadRenderTexture(gameScreenWidth, gameScreenHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);  // Texture scale filter to use

    initGame();

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
  
    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Compute scaling for letterboxing
        float scale = getScreenScale();

        updateGame();

        // Draw to render texture
        BeginTextureMode(target);
            drawGame();
        EndTextureMode();

        // Draw render texture to screen with scaling
        BeginDrawing();
            ClearBackground(BLACK);     // Clear screen background

            DrawTexturePro(target.texture, (Rectangle){ 0.0f, 0.0f, (float)target.texture.width, (float)-target.texture.height },
                           (Rectangle){ (GetScreenWidth() - ((float)gameScreenWidth*scale))*0.5f, (GetScreenHeight() - ((float)gameScreenHeight*scale))*0.5f,
                           (float)gameScreenWidth*scale, (float)gameScreenHeight*scale }, (Vector2){ 0, 0 }, 0.0f, WHITE);
        EndDrawing();
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(tilesetTexture);      // Unload tileset texture
    DestroyMap(&map);                   // Free map memory
    UnloadRenderTexture(target);        // Unload render texture

    CloseWindow();                      // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}