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
#define MERGE_DIST 60.0f
#define PLAYER_LIFES 5

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum { 
    SCREEN_LOGO = 0, 
    SCREEN_TITLE, 
    SCREEN_GAMEPLAY, 
    SCREEN_ENDING,
    SCREEN_HOWTOPLAY
} GameScreen;

// TODO: Define your custom data types here
typedef struct Player
{
    Vector2 position;
    float radius;
    Vector2 barrelSize;
    float rotationSpeed;
    float rotation;
    int lifes;
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
    int points;
    Color color;
    bool active;
    bool isMergedEnemy;
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
static bool shouldClose = false;
static int score = 0;

static Player player = { 0 };

static Bullet bullets[MAX_BULLETS] = { 0 };
static int bulletIndex = 0;
static int fireInterval = 60 / FIRE_RATE; //  1 shot per 15 frames, so 4 shots in 60 frames (1 sec)
static int fireCooldown = 0;

static Enemy enemies[MAX_ENEMIES] = { 0 };
static int enemySpawnInterval = 60; // enemies will spwan after every 1 sec
static int enemySpawnCooldown = 0;
static int mergeInterval = 600;
static int mergeCooldown = 0;

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

// gameplay modules
static void UpdatePlayer(void);
static void UpdateBullets(void);
static void UpdateEnemies(void);

// gameplay utils
static void SpawnEnemy(void);
static Vector2 GetSpawnPosition(void);
static void MergeEnemies(int a, int b);

static void UpdateEndingScreen(void);
static void DrawEndingScreen(void);

static void UpdateHowToPlayScreen(void);
static void DrawHowToPlayScreen(void);

// menu utils
static Rectangle GetMenuButtonRect(const char *label, int centerX, int y);
static bool IsMenuButtonActivated(Rectangle rect, int hotkey);
static void DrawMenuButton(const char *label, Rectangle rect);

// menu layouts
static void GetTitleScreenButtons(Rectangle *rPlay, Rectangle *rHowTo, Rectangle *rQuit);
static void GetEndingScreenButtons(Rectangle *rRestart, Rectangle *rQuit);
static void GetHowToPlayScreenButton(Rectangle *rBack);

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
    while (!WindowShouldClose() && !shouldClose)    // Detect window close button or quit action
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
static void InitGame(void)
{
    // init score, bulletIndex and cooldown timers
    score = 0;
    bulletIndex = 0;
    fireCooldown = 0;
    enemySpawnCooldown = 0;
    mergeCooldown = 0;

    // init player
    player.radius = 25.0f;
    player.position = (Vector2){
        (float)screenWidth/2.0f, 
        (float)screenHeight/2.0f
    }; 
    player.barrelSize = (Vector2){20.0f, 50.0f};
    player.rotationSpeed = 5.0f;
    player.rotation = 0.0f;
    player.lifes = PLAYER_LIFES;
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

    // init enemies
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        enemies[i].position = (Vector2){0.0f, 0.0f};
        enemies[i].direction = (Vector2){0.0f, 0.0f};
        enemies[i].radius = 20.0f;
        enemies[i].speed = 2.0f;
        enemies[i].hp = 1;
        enemies[i].points = enemies[i].hp*10;
        enemies[i].active = false;
        enemies[i].isMergedEnemy = false;
        enemies[i].color = DARKGREEN;
    }
}

// Update and draw frame
static void UpdateDrawFrame(void)
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
    case SCREEN_HOWTOPLAY:
        UpdateHowToPlayScreen();
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
        // DrawRectangleLinesEx((Rectangle){ 0, 0, screenWidth, screenHeight }, 16, BLACK);
        
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
        case SCREEN_HOWTOPLAY:
            DrawHowToPlayScreen();
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

//----------------------------------------------------------------------------------
// Screen: Logo
//----------------------------------------------------------------------------------
static void UpdateLogoScreen(void)
{
    if (frameCounter > 180)
    {
        currentScreen = SCREEN_TITLE;
        frameCounter = 0;
    }
}

static void DrawLogoScreen(void)
{
    int secondsLeft = 3 - (frameCounter/60);
    const char *title = "LOGO SCREEN";
    const char *subTitle = TextFormat("LOADING IN %d...", secondsLeft);

    DrawText(title, screenWidth/2 - MeasureText(title, fontSize*3)/2, screenHeight/2 - fontSize*3/2, fontSize*3, BLACK);
    DrawText(subTitle, screenWidth/2 - MeasureText(subTitle, fontSize)/2, screenHeight/2 + fontSize*3/2 + 20, fontSize, BLACK);
}

//----------------------------------------------------------------------------------
// Screen: Title
//----------------------------------------------------------------------------------
static void UpdateTitleScreen(void)
{
    Rectangle rPlay, rHowTo, rQuit;
    GetTitleScreenButtons(&rPlay, &rHowTo, &rQuit);

    if (IsMenuButtonActivated(rPlay, KEY_P))
    {
        InitGame();
        currentScreen = SCREEN_GAMEPLAY;
    }
    else if (IsMenuButtonActivated(rHowTo, KEY_H))
    {
        currentScreen = SCREEN_HOWTOPLAY;
    }
    else if (IsMenuButtonActivated(rQuit, KEY_Q))
    {
        shouldClose = true;
    }
}

static void DrawTitleScreen(void)
{
    const char *title = "TITLE SCREEN";
    int titleY = screenHeight/2 - fontSize*3/2 - 60;

    DrawText(title, screenWidth/2 - MeasureText(title, fontSize*3)/2, titleY, fontSize*3, BLACK);

    Rectangle rPlay, rHowTo, rQuit;
    GetTitleScreenButtons(&rPlay, &rHowTo, &rQuit);

    DrawMenuButton("[P] PLAY", rPlay);
    DrawMenuButton("[H] HOW TO PLAY", rHowTo);
    DrawMenuButton("[Q] QUIT", rQuit);
}

//----------------------------------------------------------------------------------
// Screen: Gameplay
//----------------------------------------------------------------------------------
// Update gameplay screen
static void UpdateGamePlayScreen(void)
{
    if (IsKeyPressed(KEY_P))
        gamePaused = !gamePaused;
    if (gamePaused)
        return;

    UpdatePlayer();
    UpdateBullets();
    UpdateEnemies();
}

// Update player (rotation + firing)
static void UpdatePlayer(void)
{
    // player rotation about centre of window
    if (IsKeyDown(KEY_RIGHT))
        player.rotation += player.rotationSpeed;
    else if (IsKeyDown(KEY_LEFT))
        player.rotation -= player.rotationSpeed;

    // fire cooldown
    if (fireCooldown > 0) fireCooldown--;

    // fire bullet
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
}

// Update bullets (movement + collision)
static void UpdateBullets(void)
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (!bullets[i].active) continue;

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

        // collision detection and resolution: bullet vs enemy
        for (int j = 0; j < MAX_ENEMIES; j++)
        {
            if (!enemies[j].active) continue;

            if (CheckCollisionCircles(
                bullets[i].position, bullets[i].radius, 
                enemies[j].position, enemies[j].radius)
            )
            {
                enemies[j].hp--;
                if (enemies[j].hp <= 0)
                {
                    enemies[j].active = false;
                    score += enemies[j].points;
                }
                bullets[i].active = false;
                break;
            }
        }  
    }
}

// Update enemies (spawning, movement, wall bounce, player collision)
static void UpdateEnemies(void)
{
    // enemy spawn cooldown
    if (enemySpawnCooldown > 0) enemySpawnCooldown--;

    // enemy merge cooldown
    if (mergeCooldown > 0) mergeCooldown--;

    // enemy spawning
    if (enemySpawnCooldown == 0)
    {
        SpawnEnemy();
        enemySpawnCooldown = enemySpawnInterval;
    }

    // enemy movement + collision
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!enemies[i].active) continue;

        // move towards player
        enemies[i].position.x += enemies[i].direction.x * enemies[i].speed;
        enemies[i].position.y += enemies[i].direction.y * enemies[i].speed;

        // collision detection & resolution: enemy vs walls (x-axis)
        if (
            (enemies[i].position.x - enemies[i].radius <= 0 && enemies[i].direction.x < 0) ||
            (enemies[i].position.x + enemies[i].radius >= screenWidth && enemies[i].direction.x > 0)
        )
            enemies[i].direction.x *= -1;

        // collision detection & resolution: enemy vs walls (y-axis)
        if (
            (enemies[i].position.y - enemies[i].radius <= 0 && enemies[i].direction.y < 0) ||
            (enemies[i].position.y + enemies[i].radius>= screenHeight && enemies[i].direction.y > 0)
        )
            enemies[i].direction.y *= -1;

        // collision detection & resolution: enemy vs player
        if (CheckCollisionCircles(enemies[i].position, enemies[i].radius, player.position, player.radius))
        {
            enemies[i].active = false;
            player.lifes--;
            if (player.lifes <= 0) currentScreen = SCREEN_ENDING;
        }

        // merge detection + resolution
        for (int j = i + 1; j < MAX_ENEMIES; j++)
        {
            if (!enemies[j].active) continue;

            if (mergeCooldown == 0)
            {
                float mergeDistance = enemies[i].radius + enemies[j].radius + MERGE_DIST;
            
                float dist = Vector2Distance(
                    enemies[i].position,
                    enemies[j].position
                );
                
                if (dist <= mergeDistance)
                {
                    MergeEnemies(i, j);
                    mergeCooldown = mergeInterval;
                    break;
                }
            }
            
            Vector2 toJ = Vector2Subtract(enemies[j].position, enemies[i].position);
            float approaching = Vector2DotProduct(enemies[i].direction, toJ);

            if (
                CheckCollisionCircles(enemies[i].position, enemies[i].radius, enemies[j].position, enemies[j].radius) &&
                approaching > 0
            )
            {
                // both normal: both bounce
                if (!enemies[i].isMergedEnemy && !enemies[j].isMergedEnemy)
                {
                    enemies[i].direction.x *= -1;
                    enemies[i].direction.y *= -1;

                    enemies[j].direction.x *= -1;
                    enemies[j].direction.y *= -1;
                }
                // i is elite, j is normal: only j bounces
                else if (enemies[i].isMergedEnemy && !enemies[j].isMergedEnemy)
                {
                    enemies[j].direction.x *= -1;
                    enemies[j].direction.y *= -1;
                }
                // i is normal, j is elite: only i bounces
                else if (!enemies[i].isMergedEnemy && enemies[j].isMergedEnemy)
                {
                    enemies[i].direction.x *= -1;
                    enemies[i].direction.y *= -1;
                }
                // both elite: no bounce, they pass through each other
            }
        }
    }
}

static void SpawnEnemy(void)
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

    // direction: from spawn point toward a random target point
    Vector2 dir = Vector2Normalize(
        Vector2Subtract((Vector2){
            GetRandomValue(0, screenWidth), 
            GetRandomValue(0, screenHeight)
        }, 
        spawnPos
    ));

    enemies[slot].position = spawnPos;
    enemies[slot].direction = dir;
    enemies[slot].radius = 20.0f;
    enemies[slot].speed = 2.0f;
    enemies[slot].hp = 1;
    enemies[slot].points = enemies[slot].hp*10;
    enemies[slot].active = true;
    enemies[slot].isMergedEnemy = false;
    enemies[slot].color = DARKGREEN;
}

static Vector2 GetSpawnPosition(void)
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

static void MergeEnemies(int a, int b)
{
    Enemy mergedEnemy = { 0 };

    Vector2 position = (Vector2){
        (enemies[a].position.x + enemies[b].position.x) * 0.5f,
        (enemies[a].position.y + enemies[b].position.y) * 0.5f,
    };

    float radius = sqrtf(
        enemies[a].radius * enemies[a].radius +
        enemies[b].radius * enemies[b].radius
    );

    Vector2 direction = Vector2Normalize(
        Vector2Subtract(player.position, position)
    );

    mergedEnemy.position = position;
    mergedEnemy.radius = radius;
    mergedEnemy.direction = direction;
    mergedEnemy.hp = enemies[a].hp + enemies[b].hp;
    mergedEnemy.points = mergedEnemy.hp*10;
    mergedEnemy.speed = 3.0f;
    mergedEnemy.active = true;
    mergedEnemy.isMergedEnemy = true;
    mergedEnemy.color = MAROON;

    enemies[b].active = false;
    enemies[a] = mergedEnemy;
}

// Draw gameplay screen
static void DrawGamePlayScreen(void)
{
    DrawText(TextFormat("SCORE: %d", score), 20, 20, fontSize, BLACK);

    for (int i = 0; i < player.lifes; i++)
        DrawRectangle(screenWidth - 215 + 40*i, 20, 35, 20, GREEN);

    const char *text = TextFormat("NEXT MERGE IN: %ds", mergeCooldown/60);
    if (mergeCooldown > 0)
        DrawText(text, screenWidth/2 - MeasureText(text, fontSize)/2, 20, fontSize, BLACK);

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
            DrawCircleV(bullets[i].position, bullets[i].radius, bullets[i].color);
    }

    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].active)
            DrawPoly(enemies[i].position, 6, enemies[i].radius, 0.0f, enemies[i].color);
    }

    // pause overlay (drawn on top of everything)
    if (gamePaused) {
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.5f));
        DrawText("GAME PAUSED", screenWidth/2 - MeasureText("GAME PAUSED", fontSize*3)/2, screenHeight/2 - fontSize*3/2, fontSize*3, WHITE);
    }
}

//----------------------------------------------------------------------------------
// Screen: Ending
//----------------------------------------------------------------------------------
static void UpdateEndingScreen(void)
{
    Rectangle rRestart, rQuit;
    GetEndingScreenButtons(&rRestart, &rQuit);

    if (IsMenuButtonActivated(rRestart, KEY_R))
    {
        InitGame();
        currentScreen = SCREEN_GAMEPLAY;
    }
    else if (IsMenuButtonActivated(rQuit, KEY_Q))
    {
        shouldClose = true;
    }
}

static void DrawEndingScreen(void)
{
    const char *title = TextFormat("SCORE: %d", score);
    int titleY = screenHeight/2 - fontSize*3/2 - 40;

    DrawText(title, screenWidth/2 - MeasureText(title, fontSize*3)/2, titleY, fontSize*3, BLACK);

    Rectangle rRestart, rQuit;
    GetEndingScreenButtons(&rRestart, &rQuit);

    DrawMenuButton("[R] RESTART", rRestart);
    DrawMenuButton("[Q] QUIT", rQuit);
}

//----------------------------------------------------------------------------------
// Screen: How To Play
//----------------------------------------------------------------------------------
static void UpdateHowToPlayScreen(void)
{
    Rectangle rBack;
    GetHowToPlayScreenButton(&rBack);

    if (IsMenuButtonActivated(rBack, KEY_B))
    {
        currentScreen = SCREEN_TITLE;
    }
}

static void DrawHowToPlayScreen(void)
{
    const char *title = "HOW TO PLAY";
    int titleY = 100;

    DrawText(title, screenWidth/2 - MeasureText(title, fontSize*2)/2, titleY, fontSize*2, BLACK);

    int y = titleY + fontSize*2 + 40;
    int leftX = 160;
    int lineSpacing = 30;

    DrawText("[LEFT] [RIGHT]", leftX, y, fontSize, DARKGRAY);
    DrawText("Rotate turret", leftX + 220, y, fontSize, BLACK);

    DrawText("[SPACE]", leftX, y + lineSpacing, fontSize, DARKGRAY);
    DrawText("Fire", leftX + 220, y + lineSpacing, fontSize, BLACK);

    DrawText("[P]", leftX, y + lineSpacing*2, fontSize, DARKGRAY);
    DrawText("Pause", leftX + 220, y + lineSpacing*2, fontSize, BLACK);

    int descY = y + lineSpacing*3 + 30;
    const char *line1 = "Destroy hexagonal enemies.";
    const char *line2 = "Enemies merge into stronger elites!";
    const char *line3 = "Don't let them reach you.";

    DrawText(line1, screenWidth/2 - MeasureText(line1, fontSize)/2, descY, fontSize, BLACK);
    DrawText(line2, screenWidth/2 - MeasureText(line2, fontSize)/2, descY + lineSpacing, fontSize, BLACK);
    DrawText(line3, screenWidth/2 - MeasureText(line3, fontSize)/2, descY + lineSpacing*2, fontSize, BLACK);

    Rectangle rBack;
    GetHowToPlayScreenButton(&rBack);
    DrawMenuButton("[B] Back", rBack);
}

//----------------------------------------------------------------------------------
// Menu Utils
//----------------------------------------------------------------------------------
static Rectangle GetMenuButtonRect(const char *label, int centerX, int y)
{
    int textWidth = MeasureText(label, fontSize);
    int padX = 20;
    int padY = 8;
    return (Rectangle){
        (float)centerX - textWidth/2.0f - padX,
        (float)y - padY,
        (float)(textWidth + padX*2),
        (float)(fontSize + padY*2)
    };
}

static bool IsMenuButtonActivated(Rectangle rect, int hotkey)
{
    bool hovered = CheckCollisionPointRec(GetMousePosition(), rect);
    bool clicked = hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    bool hotPressed = IsKeyPressed(hotkey);
    return clicked || hotPressed;
}

static void DrawMenuButton(const char *label, Rectangle rect)
{
    bool hovered = CheckCollisionPointRec(GetMousePosition(), rect);

    // draw background highlight on hover
    if (hovered)
        DrawRectangleRec(rect, Fade(BLACK, 0.15f));

    // draw text (change colour on hover)
    Color textColor = hovered ? DARKGRAY : BLACK;
    int textWidth = MeasureText(label, fontSize);
    DrawText(label, (int)(rect.x + (rect.width - textWidth)/2.0f), (int)(rect.y + (rect.height - fontSize)/2.0f), fontSize, textColor);
}

//----------------------------------------------------------------------------------
// Menu Layouts
//----------------------------------------------------------------------------------
static void GetTitleScreenButtons(Rectangle *rPlay, Rectangle *rHowTo, Rectangle *rQuit)
{
    int titleY = screenHeight/2 - fontSize*3/2 - 60;
    int baseY = titleY + fontSize*3 + 45;
    int gap = 30;

    int w1 = MeasureText("[P] PLAY", fontSize) + 40;
    int w2 = MeasureText("[H] HOW TO PLAY", fontSize) + 40;
    int w3 = MeasureText("[Q] QUIT", fontSize) + 40;
    int totalWidth = w1 + w2 + w3 + gap * 2;
    int startX = (screenWidth - totalWidth) / 2;

    int c1 = startX + w1 / 2;
    int c2 = startX + w1 + gap + w2 / 2;
    int c3 = startX + w1 + gap + w2 + gap + w3 / 2;

    *rPlay = GetMenuButtonRect("[P] PLAY", c1, baseY);
    *rHowTo = GetMenuButtonRect("[H] HOW TO PLAY", c2, baseY);
    *rQuit = GetMenuButtonRect("[Q] QUIT", c3, baseY);
}

static void GetEndingScreenButtons(Rectangle *rRestart, Rectangle *rQuit)
{
    int titleY = screenHeight/2 - fontSize*3/2 - 40;
    int baseY = titleY + fontSize*3 + 45;
    int gap = 40;

    int w1 = MeasureText("[R] RESTART", fontSize) + 40;
    int w2 = MeasureText("[Q] QUIT", fontSize) + 40;
    int totalWidth = w1 + w2 + gap;
    int startX = (screenWidth - totalWidth) / 2;

    int c1 = startX + w1 / 2;
    int c2 = startX + w1 + gap + w2 / 2;

    *rRestart = GetMenuButtonRect("[R] RESTART", c1, baseY);
    *rQuit = GetMenuButtonRect("[Q] QUIT", c2, baseY);
}

static void GetHowToPlayScreenButton(Rectangle *rBack)
{
    int titleY = 100;
    int y = titleY + fontSize*2 + 40;
    int lineSpacing = 30;
    int descY = y + lineSpacing*3 + 30;
    int baseY = descY + lineSpacing*3 + 40;

    *rBack = GetMenuButtonRect("[B] Back", screenWidth/2, baseY);
}