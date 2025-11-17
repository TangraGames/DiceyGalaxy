
#include "raylib.h"
#include "raymath.h"        // Required for: Vector2Clamp()
#include "utils_hexmap.c"

#define MAX(a, b) ((a)>(b)? (a) : (b))
#define MIN(a, b) ((a)<(b)? (a) : (b))
#define MAP_RADIUS 5

//------------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------------
static int gameScreenWidth = 640;
static int gameScreenHeight = 480;

static Layout hexLayout;
static Hex selectedHex;
static Map map;

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

static void initGame(void)
{
    // Initialize hex layout with pointy-top orientation, 24 pixel size, centered
    Point size = { 24.0f, 24.0f };
    Point origin = { gameScreenWidth / 2.0f, gameScreenHeight / 2.0f };
    hexLayout = MakeLayout(layout_pointy, size, origin);
    
    // Initialize selected hex to an out-of-bounds value
    selectedHex = MakeHex(100, -100, 0);
    
    // Create map using the utility function
    map = CreateMap(origin, size, MAP_RADIUS);
}

static void updateGame(void)
{
    // Handle mouse click to select hex
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        Vector2 virtualMouse = getVirtualMouse();
        Point mousePoint = { virtualMouse.x, virtualMouse.y };
        selectedHex = PixelToHex(hexLayout, mousePoint);
    }
}

static void drawGame(void)
{
    ClearBackground(RAYWHITE);
    
    // Draw all hexes from the map
    for (int i = 0; i < map.hexCount; i++)
    {
        Hex hex = map.hexes[i];
        
        // Get hex center for DrawPoly
        Point center = HexToPixel(hexLayout, hex);
        
        // Determine fill color based on hex state
        Color fillColor = LIGHTGRAY;
        if (HexEquals(hex, selectedHex))
        {
            fillColor = YELLOW;  // Selected hex
        }
        if (hex.q == 0 && hex.r == 0 && hex.s == 0)
        {
            fillColor = SKYBLUE;  // Center hex (override selected)
        }
        
        // Draw filled hexagon using DrawPoly
        DrawPoly((Vector2){center.x, center.y}, 6, hexLayout.size.x, 30.0f, fillColor);
        
        // Draw hex outline
        DrawPolyLines((Vector2){center.x, center.y}, 6, hexLayout.size.x, 30.0f, DARKGRAY);
    }
    
    DrawText(TextFormat("Hexagonal Map - Click to select | Selected: (%d,%d,%d) | Hexes: %d", 
             selectedHex.q, selectedHex.r, selectedHex.s, map.hexCount), 10, 10, 20, BLACK);
}

//------------------------------------------------------------------------------------
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
    DestroyMap(&map);                   // Free map memory
    UnloadRenderTexture(target);        // Unload render texture

    CloseWindow();                      // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}