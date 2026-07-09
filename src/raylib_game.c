/*******************************************************************************************
*
*   raylib gamejam template
*
*   Code licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2022-2026 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>      // Emscripten library
#endif

#include <stdio.h>                          // Required for: printf()
#include <stdlib.h>                         // Required for: 
#include <string.h>                         // Required for:

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Simple log system to avoid printf() calls if required
// NOTE: Avoiding those calls, also avoids const strings memory usage
#define SUPPORT_LOG_INFO
#if defined(SUPPORT_LOG_INFO)
    #define LOG(...) printf(__VA_ARGS__)
#else
    #define LOG(...)
#endif

#define fontSize 20
#define MAX_BULLETS 64
#define FIRE_RATE 4 // 4 shots in 1 second
#define MAX_ENEMIES 32

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum { 
    SCREEN_LOGO = 0, 
    SCREEN_TITLE, 
    SCREEN_GAMEPLAY, 
    SCREEN_ENDING
} GameScreen;

// TODO: Define your custom data types here
typedef struct Player
{
    Vector2 position;
    float radius;
    Vector2 barrelSize;
    float rotationSpeed;
    float rotation;
    Color color;
} Player;

typedef struct Bullet
{
    Vector2 position;
    Vector2 direction;
    float radius;
    float speed;
    Color color;
    bool active;
} Bullet;

typedef struct Enemy
{
    Vector2 position;
    Vector2 direction;
    float speed;
    float radius;
    int hp;
    Color color;
    bool active;
} Enemy;

typedef enum Edge
{
    EDGE_TOP = 0,
    EDGE_BOTTOM,
    EDGE_LEFT,
    EDGE_RIGHT
} Edge;

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------
static const int screenWidth = 720;
static const int screenHeight = 720;

static RenderTexture2D target = { 0 };  // Render texture to render our game
static int frameCounter = 0;

// TODO: Define global variables here, recommended to make them static
static GameScreen currentScreen = SCREEN_LOGO;
static bool gamePaused = false;
static Player player = { 0 };
static Bullet bullets[MAX_BULLETS] = { 0 };
static int bulletIndex = 0;
static int fireInterval = 60 / FIRE_RATE; //  1 shot per 15 frames, so 4 shots in 60 frames (1 sec)
static int fireCooldown = 0;
static Enemy enemies[MAX_ENEMIES] = { 0 };
static int enemySpawnInterval = 90; // enemies will spwan after every 1.5 sec
static int enemySpawnCooldown = 0;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void InitGame(void);

static void UpdateDrawFrame(void);      // Update and Draw one frame

static void UpdateLogoScreen();
static void DrawLogoScreen(void);

static void UpdateTitleScreen(void);
static void DrawTitleScreen(void);

static void UpdateGamePlayScreen(void);
static void DrawGamePlayScreen(void);

static void UpdateEndingScreen(void);
static void DrawEndingScreen(void);

static void SpawnEnemy(void);
static Vector2 GetSpawnPosition(void);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
#if !defined(_DEBUG)
    SetTraceLogLevel(LOG_NONE);         // Disable raylib trace log messages
#endif

    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib gamejam template");
    
    // TODO: Load resources / Initialize variables at this point
    InitGame();
    
    // Render texture to draw, enables screen scaling
    // NOTE: If screen is scaled, mouse input should be scaled proportionally
    target = LoadRenderTexture(screenWidth, screenHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);     // Set our game frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadRenderTexture(target);
    
    // TODO: Unload all loaded resources at this point

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//--------------------------------------------------------------------------------------------
// Module Functions Definition
//--------------------------------------------------------------------------------------------
// Init game
void InitGame(void)
{
    // init player
    player.radius = 25.0f;
    player.position = (Vector2){
        (float)screenWidth/2.0f, 
        (float)screenHeight/2.0f
    }; 
    player.barrelSize = (Vector2){20.0f, 50.0f};
    player.rotationSpeed = 5.0f;
    player.rotation = 0.0f;
    player.color = BLACK;

    // init bullets
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        bullets[i].position = (Vector2){0.0f, 0.0f};
        bullets[i].direction = (Vector2){0.0f, 0.0f};
        bullets[i].radius = 5.0f;
        bullets[i].speed = 12.0f;
        bullets[i].color = ORANGE;
        bullets[i].active = false;
    }

    // init enemy
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        enemies[i].position = (Vector2){0.0f, 0.0f};
        enemies[i].direction = (Vector2){0.0f, 0.0f};
        enemies[i].radius = 20.0f;
        enemies[i].speed = 1.5f;
        enemies[i].hp = 1;
        enemies[i].active = false;
        enemies[i].color = DARKGREEN;
    }
}

// Update and draw frame
void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    // TODO: Update variables / Implement example logic at this point
   
    frameCounter++;

    switch (currentScreen)
    {
    case SCREEN_LOGO:
        UpdateLogoScreen();
        break;
    case SCREEN_TITLE:
        UpdateTitleScreen();
        break;
    case SCREEN_GAMEPLAY:
        UpdateGamePlayScreen();
        break;
    case SCREEN_ENDING:
        UpdateEndingScreen();
        break;
    default:
        break;
    }
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    // Render game screen to a texture, 
    // it could be useful for scaling or further shader postprocessing
    BeginTextureMode(target);
        ClearBackground(RAYWHITE);
        DrawRectangleLinesEx((Rectangle){ 0, 0, screenWidth, screenHeight }, 16, BLACK);
        
        // TODO: Draw your game screen here
        switch (currentScreen)
        {
        case SCREEN_LOGO:
            DrawLogoScreen();
            break;
        case SCREEN_TITLE:
            DrawTitleScreen();
            break;
        case SCREEN_GAMEPLAY:
            DrawGamePlayScreen();
            break;
        case SCREEN_ENDING:
            DrawEndingScreen();
            break;
        default:
            break;
        }        
        
    EndTextureMode();
    
    // Render to screen (main framebuffer)
    BeginDrawing();
        ClearBackground(RAYWHITE);
        
        // Draw render texture to screen, scaled if required
        DrawTexturePro(target.texture, (Rectangle){ 0, 0, (float)target.texture.width, -(float)target.texture.height }, 
            (Rectangle){ 0, 0, (float)target.texture.width, (float)target.texture.height }, (Vector2){ 0, 0 }, 0.0f, WHITE);

        // TODO: Draw everything that requires to be drawn at this point, maybe UI?

    EndDrawing();
    //----------------------------------------------------------------------------------  
}

// Update logo screen
static void UpdateLogoScreen(void)
{
    if (frameCounter > 180)
    {
        currentScreen = SCREEN_TITLE;
        frameCounter = 0;
    }
}

// Draw logo screen
static void DrawLogoScreen(void)
{
    int secondsLeft = 3 - (frameCounter/60);
    const char *title = "LOGO SCREEN";
    const char *subTitle = TextFormat("LOADING IN %d...", secondsLeft);

    DrawText(title, screenWidth/2 - MeasureText(title, fontSize*3)/2, screenHeight/2 - fontSize*3/2, fontSize*3, BLACK);
    DrawText(subTitle, screenWidth/2 - MeasureText(subTitle, fontSize)/2, screenHeight/2 + fontSize*3/2 + 20, fontSize, BLACK);
}

// Update title screen
static void UpdateTitleScreen(void)
{
    if (IsKeyPressed(KEY_ENTER))
        currentScreen = SCREEN_GAMEPLAY;
}

// Draw title screen
static void DrawTitleScreen(void)
{
    const char *title = "TITLE SCREEN";
    const char *subTitle = "PRESS [ENTER] to START";

    DrawText(title, screenWidth/2 - MeasureText(title, fontSize*3)/2, screenHeight/2 - fontSize*3/2, fontSize*3, BLACK);
    if ((frameCounter/20)%2) 
        DrawText(subTitle, screenWidth/2 - MeasureText(subTitle, fontSize)/2, screenHeight/2 + fontSize*3/2 + 20, fontSize, BLACK);
}

// Update gameplay screen
static void UpdateGamePlayScreen(void)
{
    if (IsKeyPressed(KEY_ENTER))
        currentScreen = SCREEN_ENDING;
    if (IsKeyPressed(KEY_P))
        gamePaused = !gamePaused;
    if (gamePaused)
        return;

    // player rotation about centre of window
    if (IsKeyDown(KEY_RIGHT))
        player.rotation += player.rotationSpeed;
    else if (IsKeyDown(KEY_LEFT))
        player.rotation -= player.rotationSpeed;

    // position the bullet ready to be fired at the tip of the barrel
    if (fireCooldown > 0) fireCooldown--;

    if (IsKeyDown(KEY_SPACE) && fireCooldown <= 0) 
    {
        Vector2 localOffset = { 0.0f, player.barrelSize.y };
        Vector2 barrelTipPosition = Vector2Add(player.position, Vector2Rotate(localOffset, player.rotation*DEG2RAD));
    
        Vector2 localForward = { 0.0f, 1.0f };
        Vector2 direction = Vector2Rotate(localForward, player.rotation*DEG2RAD);

        bullets[bulletIndex].active = true;
        bullets[bulletIndex].position = barrelTipPosition;
        bullets[bulletIndex].direction = direction;
        bulletIndex = (bulletIndex + 1) % MAX_BULLETS;
        fireCooldown = fireInterval;
    }

    // bullet movement logic
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (bullets[i].active)
        {
            bullets[i].position.x += bullets[i].direction.x * bullets[i].speed;
            bullets[i].position.y += bullets[i].direction.y * bullets[i].speed;

            // deactivate once fully off-screen (with a small margin for radius)
            if (bullets[i].position.x < 0 - bullets[i].radius ||
                bullets[i].position.x > screenWidth + bullets[i].radius ||
                bullets[i].position.y < 0 - bullets[i].radius ||
                bullets[i].position.y > screenHeight + bullets[i].radius
            )
            {
                bullets[i].active = false;
            }
        }
    }

    // enemy spawning
    if (enemySpawnCooldown > 0)
    {
        enemySpawnCooldown--;
    }

    if (enemySpawnCooldown == 0)
    {
        SpawnEnemy();
        enemySpawnCooldown = enemySpawnInterval;
    }

    // enemy movement logic
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].active)
        {
            // move towards player
            enemies[i].position.x += enemies[i].direction.x * enemies[i].speed;
            enemies[i].position.y += enemies[i].direction.y * enemies[i].speed;

            // collision detection & resolution: enemy vs walls
            if (
                (enemies[i].position.x - enemies[i].radius <= 0 && enemies[i].direction.x < 0) ||
                (enemies[i].position.x + enemies[i].radius >= screenWidth && enemies[i].direction.x > 0)
            )
                enemies[i].direction.x *= -1;
           if (
                (enemies[i].position.y - enemies[i].radius <= 0 && enemies[i].direction.y < 0) ||
                (enemies[i].position.y + enemies[i].radius>= screenHeight && enemies[i].direction.y > 0)
            )
                enemies[i].direction.y *= -1;

            // collision detection & resolution: enemy vs player
            if (CheckCollisionCircles(enemies[i].position, enemies[i].radius, player.position, player.radius))
            {
                enemies[i].active = false;
                LOG("Enemy reached player!\n");
            }
        }
    }
}

void SpawnEnemy(void)
{
    // find an inactive slot
    int slot = -1;
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!enemies[i].active) 
        {
            slot = i;
            break;
        }
    }
    if (slot == -1) return; // no free slot, skip spawning this time

    Vector2 spawnPos = GetSpawnPosition();

    // direction: from spawn point toward the player
    Vector2 toPlayer = Vector2Subtract(player.position, spawnPos);
    Vector2 dir = Vector2Normalize(toPlayer);

    enemies[slot].position = spawnPos;
    enemies[slot].direction = dir;
    enemies[slot].active = true;
}

Vector2 GetSpawnPosition(void)
{
    Edge edge = GetRandomValue(0, 3);
    Vector2 pos = {0.0f, 0.0f};

    switch (edge)
    {
    case EDGE_TOP:
        pos = (Vector2){GetRandomValue(0, screenWidth), 0.0f};
        break;
    case EDGE_BOTTOM:
        pos = (Vector2){GetRandomValue(0, screenWidth), screenHeight};
        break;
    case EDGE_LEFT:
        pos = (Vector2){0.0f, GetRandomValue(0, screenHeight)};
        break;
    case EDGE_RIGHT:
        pos = (Vector2){screenWidth, GetRandomValue(0, screenHeight)};
        break;
    default:
        break;
    }

    return pos;
}

// Draw gameplay screen
static void DrawGamePlayScreen(void)
{
    if (gamePaused) {
        DrawText("GAME PAUSED", screenWidth/2 - MeasureText("GAME PAUSED", fontSize*3)/2, screenHeight/2 - fontSize*3/2, fontSize*3, BLACK);
        return;
    }

    // turret body
    DrawCircleV(player.position, player.radius, player.color);

    // turret barrel
    DrawRectanglePro(
        (Rectangle){player.position.x, player.position.y, player.barrelSize.x, player.barrelSize.y},
        (Vector2){player.barrelSize.x/2, 0.0f},
        player.rotation,
        player.color
    );

    // draw bullet if active
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (bullets[i].active)
        DrawCircleV(
            bullets[i].position, 
            bullets[i].radius, 
            bullets[i].color
        );
    }

    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].active)
            DrawPoly(enemies[i].position, 6, enemies[i].radius, 0.0f, enemies[i].color);
    }
    
}

// Update ending screen
static void UpdateEndingScreen(void)
{
    if (IsKeyPressed(KEY_ENTER))
        currentScreen = SCREEN_TITLE;
}

// Draw ending screen
static void DrawEndingScreen(void)
{
    const char *title = "ENDING SCREEN";
    const char *subTitle = "PRESS [ENTER] TO PLAY AGAIN";

    DrawText(title, screenWidth/2 - MeasureText(title, fontSize*3)/2, screenHeight/2 - fontSize*3/2, fontSize*3, BLACK);
    if ((frameCounter/20)%2) 
        DrawText(subTitle, screenWidth/2 - MeasureText(subTitle, fontSize)/2, screenHeight/2 + fontSize*3/2 + 20, fontSize, BLACK);
}