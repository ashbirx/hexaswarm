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
#include "vfx.h"

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

// Color palette
#define COL_GREEN             (Color){  80, 200, 120, 255 }
#define COL_RED               (Color){ 220,  50,  70, 255 }

#define COL_BG                (Color){  20,  20,  30, 255 }
#define COL_BG_LIGHT          (Color){  35,  35,  50, 255 }
#define COL_TEXT              (Color){ 220, 225, 240, 255 }
#define COL_TEXT_DIM          (Color){ 130, 135, 150, 255 }
#define COL_OVERLAY           (Color){  10,  10,  20, 160 }

#define COL_TURRET            (Color){ 200, 210, 230, 255 }
#define COL_BULLET            (Color){ 255, 180,  50, 255 }

#define COL_GAME_TITLE        COL_GREEN
#define COL_ENEMY             COL_GREEN
#define COL_ENEMY_MERGED      COL_RED
#define COL_HP                COL_GREEN

// Font sizes
// heading sizes
#define HEAD_HERO             88.0f   // main game title
#define HEAD_BIG              56.0f   // big reveals: pause text, final score
#define HEAD_MED              28.0f   // screen sub-headers, panel section titles, menu buttons
#define HEAD_SMALL            20.0f   // HUD score, small countdown labels

// body sizes
#define BODY_LG               18.0f   // subtitle lines (high score, "POWERED BY")
#define BODY_MD               16.0f   // paragraph/description lines, controls hint row

// Letter spacings
#define SPACING_HEAD(sz)      ((sz) * 0.12f)
#define SPACING_BODY(sz)      ((sz) * 0.08f)

// Fonts
#define FONT_BODY             "resources/fonts/m5x7.ttf"
#define FONT_HEADING          "resources/fonts/m6x11.ttf"

// Textures
#define TEX_LOGO              "resources/textures/raylib_logo.png"
#define TEX_HOW_TO_PLAY       "resources/textures/howtoplay_panel.png"
#define TEX_LIFE              "resources/textures/life_heart.png"
#define TEX_PLAYER            "resources/textures/player.png"
#define TEX_ENEMY_REGULAR     "resources/textures/enemy_regular.png"
#define TEX_ENEMY_MERGED      "resources/textures/enemy_merged.png"

// Audio
#define FX_SHOOT              "resources/audio/fx/shoot.ogg"
#define FX_KILL               "resources/audio/fx/kill.wav"
#define FX_MERGE              "resources/audio/fx/merge.wav"
#define FX_HURT               "resources/audio/fx/hurt.ogg"
#define FX_SELECT             "resources/audio/fx/select.ogg"
#define MUS_BG                "resources/audio/music/bg_music.ogg"

// Game related constants
#define MAX_BULLETS            64
#define FIRE_RATE              4 // 4 shots in 1 second
#define MAX_ENEMIES            32
#define MERGE_DIST             60.0f
#define PLAYER_LIFES           5

#define TURRET_SPRITE_ROTATION_OFFSET 180.0f

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
    float hitFlashTimer;
} Enemy;

typedef enum Edge
{
    EDGE_TOP = 0,
    EDGE_BOTTOM,
    EDGE_LEFT,
    EDGE_RIGHT
} Edge;

typedef struct Fonts
{
    Font body;
    Font heading;
} Fonts;

typedef struct Textures
{
    Texture2D logo;
    Texture2D player;
    Texture2D enemyRegular;
    Texture2D enemyMerged;
    Texture2D howToPlay;
    Texture2D life;
} Textures;

typedef struct Sounds
{
    Sound fxShoot;
    Sound fxKill;
    Sound fxMerge;
    Sound fxHurt;
    Sound fxSelect;
} Sounds;

typedef struct Buttons
{
    Rectangle play;
    Rectangle howToPlay;
    Rectangle quitFromHome;

    Rectangle restartFromPause;
    Rectangle goHomeFromPause;
    Rectangle quitFromPause;

    Rectangle restartFromEnd;
    Rectangle goHomeFromEnd;
    Rectangle quitFromEnd;

    Rectangle backFromHowtoplay;
} Buttons;

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
static Buttons buttons = { 0 };

static Fonts fonts = { 0 };
static Textures textures = { 0 };
static Sounds sounds = { 0 };

static Player player = { 0 };
static int score = 0;

static Bullet bullets[MAX_BULLETS] = { 0 };
static int bulletIndex = 0;
static int fireInterval = 60 / FIRE_RATE; //  1 shot per 15 frames, so 4 shots in 60 frames (1 sec)
static int fireCooldown = 0;

static Enemy enemies[MAX_ENEMIES] = { 0 };
static int enemySpawnInterval = 54; // enemies will spwan after every 0.9 sec
static int enemySpawnCooldown = 0;
static int mergeInterval = 600; // enemies will merge after every 10 sec, given they are within merge distance
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

// update gameplay screen utils
static void UpdatePlayer(void);
static void UpdateBullets(void);
static void UpdateEnemies(void);
static void SpawnEnemy(void);
static Vector2 GetSpawnPosition(void);
static void MergeEnemies(int a, int b);

// draw gameplay screen utils
static void DrawPlayerSprite(void);
static void DrawEnemySprite(Texture2D tex, Enemy *e);

static void UpdateEndingScreen(void);
static void DrawEndingScreen(void);

static void UpdateHowToPlayScreen(void);
static void DrawHowToPlayScreen(void);

// menu button utils
static bool IsMenuButtonActivated(Rectangle rect, int hotkey);
static void DrawMenuButtonRow(const char **labels, int count, float startY, Rectangle *outRects);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
#if !defined(_DEBUG)
    SetTraceLogLevel(LOG_ALL);         // Disable raylib trace log messages
#endif

    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "HEXASWARM");
    InitAudioDevice();
    
    // TODO: Load resources / Initialize variables at this point

    // load fonts
    fonts.body = LoadFontEx(FONT_BODY, 64, NULL, 0);
    fonts.heading = LoadFontEx(FONT_HEADING, 64, NULL, 0);

    SetTextureFilter(fonts.body.texture, TEXTURE_FILTER_POINT);
    SetTextureFilter(fonts.heading.texture, TEXTURE_FILTER_POINT);

    // load textures
    textures.logo = LoadTexture(TEX_LOGO);
    textures.howToPlay = LoadTexture(TEX_HOW_TO_PLAY);
    textures.player = LoadTexture(TEX_PLAYER);
    textures.life = LoadTexture(TEX_LIFE);
    textures.enemyRegular = LoadTexture(TEX_ENEMY_REGULAR);
    textures.enemyMerged = LoadTexture(TEX_ENEMY_MERGED);

    SetTextureFilter(textures.player, TEXTURE_FILTER_POINT);
    SetTextureFilter(textures.life, TEXTURE_FILTER_POINT);
    SetTextureFilter(textures.enemyRegular, TEXTURE_FILTER_POINT);
    SetTextureFilter(textures.enemyMerged, TEXTURE_FILTER_POINT);

    // load sounds
    sounds.fxSelect = LoadSound(FX_SELECT);
    sounds.fxShoot = LoadSound(FX_SHOOT);
    sounds.fxKill = LoadSound(FX_KILL);
    sounds.fxMerge = LoadSound(FX_MERGE);
    sounds.fxHurt = LoadSound(FX_HURT);

    // init game
    InitGame();
    VFX_Init();
    
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
    
    // Unload fonts
    UnloadFont(fonts.body);
    UnloadFont(fonts.heading);

    // unload textures
    UnloadTexture(textures.logo);
    UnloadTexture(textures.howToPlay);
    UnloadTexture(textures.player);
    UnloadTexture(textures.life);
    UnloadTexture(textures.enemyRegular);
    UnloadTexture(textures.enemyMerged);

    // unload sounds
    UnloadSound(sounds.fxSelect);
    UnloadSound(sounds.fxShoot);
    UnloadSound(sounds.fxKill);
    UnloadSound(sounds.fxMerge);
    UnloadSound(sounds.fxHurt);

    CloseAudioDevice();

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
    gamePaused = false;
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
    player.color = COL_TURRET;

    // init bullets
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        bullets[i].position = (Vector2){0.0f, 0.0f};
        bullets[i].direction = (Vector2){0.0f, 0.0f};
        bullets[i].radius = 5.0f;
        bullets[i].speed = 12.0f;
        bullets[i].color = COL_BULLET;
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
        enemies[i].color = COL_ENEMY;
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
        ClearBackground(COL_BG);
        
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
        ClearBackground(COL_BG);
        
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
    const char *text1 = "POWERED BY";
    Vector2 s1 = MeasureTextEx(fonts.body, text1, BODY_LG, SPACING_BODY(BODY_LG));
    Vector2 p1 = (Vector2){screenWidth/2.0f - s1.x/2.0f, screenHeight/2.0f - textures.logo.height/2.0f - s1.y - 8.0f};

    int secondsLeft = 3 - (frameCounter/60);
    const char *text2 = TextFormat("LOADING IN %d...", secondsLeft);
    Vector2 s2 = MeasureTextEx(fonts.heading, text2, HEAD_SMALL, SPACING_HEAD(HEAD_SMALL));
    Vector2 p2 = (Vector2){screenWidth/2.0f - s2.x/2.0f, screenHeight/2.0f + textures.logo.height/2.0f + 32.0f};

    // draw logo
    DrawTextEx(fonts.body, text1, p1, BODY_LG, SPACING_BODY(BODY_LG), COL_TEXT_DIM);
    DrawTexture(textures.logo, screenWidth/2 - textures.logo.width/2, screenHeight/2 - textures.logo.height/2, COL_TEXT);
    // draw loading counter
    DrawTextEx(fonts.heading, text2, p2, HEAD_SMALL, SPACING_HEAD(HEAD_SMALL), COL_TEXT);    
}

//----------------------------------------------------------------------------------
// Screen: Title
//----------------------------------------------------------------------------------
static void UpdateTitleScreen(void)
{
    if (IsMenuButtonActivated(buttons.play, KEY_P))
    {
        InitGame();
        currentScreen = SCREEN_GAMEPLAY;
    }
    else if (IsMenuButtonActivated(buttons.howToPlay, KEY_H))
    {
        currentScreen = SCREEN_HOWTOPLAY;
    }
    else if (IsMenuButtonActivated(buttons.quitFromHome, KEY_Q))
    {
        shouldClose = true;
    }
}

static void DrawTitleScreen(void)
{
    // draw game title
    const char *title = "HEXASWARM";
    Vector2 s = MeasureTextEx(fonts.heading, title, HEAD_HERO, SPACING_HEAD(HEAD_HERO));
    float titleX = screenWidth/2.0f - s.x/2.0f;
    float titleY = screenHeight/2.0f - s.y/2.0f;
    DrawTextEx(fonts.heading, title, (Vector2){titleX, titleY}, HEAD_HERO, SPACING_HEAD(HEAD_HERO), COL_GAME_TITLE);

    // draw menu buttons row
    const char *labels[] = { "[P] PLAY", "[H] HOW TO PLAY", "[Q] QUIT" };
    float startY = titleY + s.y + 60.0f;
    Rectangle rects[3] = { 0 };
    DrawMenuButtonRow(labels, 3, startY, rects);
    buttons.play = rects[0];
    buttons.howToPlay = rects[1];
    buttons.quitFromHome = rects[2];
}

//----------------------------------------------------------------------------------
// Screen: Gameplay
//----------------------------------------------------------------------------------
// Update gameplay screen
static void UpdateGamePlayScreen(void)
{
    if (IsKeyPressed(KEY_P)) gamePaused = !gamePaused;

    if (gamePaused)
    {
        if (IsMenuButtonActivated(buttons.restartFromPause, KEY_R))
        {
            InitGame();
        }
        if (IsMenuButtonActivated(buttons.goHomeFromPause, KEY_H))
        {
            currentScreen = SCREEN_TITLE;
        }
        else if (IsMenuButtonActivated(buttons.quitFromPause, KEY_Q))
        {
            shouldClose = true;
        }
    }
    else
    {
        UpdatePlayer();
        UpdateBullets();
        UpdateEnemies();
        VFX_Update();
    }
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
        PlaySound(sounds.fxShoot);
        VFX_SpawnMuzzleFlash(barrelTipPosition, COL_BULLET);
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
                    VFX_SpawnDeathPoof(enemies[j].position, enemies[j].color);
                    enemies[j].active = false;
                    score += enemies[j].points;
                    if (enemies[j].isMergedEnemy) PlaySound(sounds.fxKill);
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
        if (enemies[i].hitFlashTimer > 0.0f) enemies[i].hitFlashTimer -= 1.0f;

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
            VFX_TriggerPlayerHit();
            VFX_TriggerHeartPop(player.lifes);
            PlaySound(sounds.fxHurt);
            if (player.lifes <= 0) currentScreen = SCREEN_ENDING;
        }

        for (int j = i + 1; j < MAX_ENEMIES; j++)
        {
            if (!enemies[j].active) continue;

            // merge detection + resolution
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
                    PlaySound(sounds.fxMerge);
                    mergeCooldown = mergeInterval;
                    break;
                }
            }
            
            // collision detection & resolution: enemy vs enemy
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
    enemies[slot].color = COL_ENEMY;
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
    mergedEnemy.color = COL_ENEMY_MERGED;

    VFX_SpawnMergePulse(position, COL_ENEMY_MERGED);

    enemies[b].active = false;
    enemies[a] = mergedEnemy;
}

// Draw gameplay screen
static void DrawGamePlayScreen(void)
{
    // turret
    DrawPlayerSprite();

    // bullets
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (bullets[i].active)
            DrawCircleV(bullets[i].position, bullets[i].radius, bullets[i].color);
    }

    // enemies
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!enemies[i].active) continue;
        Texture2D tex = enemies[i].isMergedEnemy ? textures.enemyMerged : textures.enemyRegular;
        DrawEnemySprite(tex, &enemies[i]);
        if (enemies[i].hitFlashTimer > 0.0f)
        {
            float d = enemies[i].radius * 2.0f;
            Rectangle dst = { enemies[i].position.x, enemies[i].position.y, d, d };
            VFX_DrawHitFlash(tex, dst, (Vector2){ d * 0.5f, d * 0.5f }, 0.0f,
                            enemies[i].hitFlashTimer / VFX_HIT_FLASH_DURATION);
        }
    }

    VFX_Draw();   // muzzle flashes, death poofs, merge pulses - on top of entities

    // heads-up-display
    DrawRectangleRec((Rectangle){0.0f, 0.0f, screenWidth, 60.0f}, COL_BG_LIGHT);

    const char *scoreText = TextFormat("SCORE: %d", score);
    DrawTextEx(fonts.heading, scoreText, (Vector2){20.0f, 20.0f}, HEAD_SMALL, SPACING_HEAD(HEAD_SMALL), COL_TEXT);

    // player lifes display
    for (int i = 0; i < player.lifes; i++)
    {
        if (textures.life.id > 0)
        {
            float h = 24.0f;
            float w = h * ((float)textures.life.width / (float)textures.life.height);
            Rectangle src = { 0, 0, (float)textures.life.width, (float)textures.life.height };
            Rectangle dst = { screenWidth - 215.0f + 40 * i, 20.0f, w, h };
            DrawTexturePro(textures.life, src, dst, (Vector2){0, 0}, 0.0f, WHITE);
        }
        else
        {
            DrawRectangle(screenWidth - 215 + 40*i, 20, 35, 20, COL_HP);
        }
    }

    int poppedSlot; float poppedScale, poppedAlpha;
    if (VFX_IsHeartPopping(&poppedSlot, &poppedScale, &poppedAlpha))
    {
        float h = 24.0f * poppedScale;
        float w = h * ((float)textures.life.width / (float)textures.life.height);
        Rectangle src = { 0, 0, (float)textures.life.width, (float)textures.life.height };
        Rectangle dst = { screenWidth - 215.0f + 40 * poppedSlot, 20.0f, w, h };
        DrawTexturePro(textures.life, src, dst, (Vector2){0,0}, 0.0f, Fade(WHITE, poppedAlpha));
    }

    // merge cooldown display
    const char *nextMergeText = TextFormat("NEXT MERGE IN: %ds", mergeCooldown/60);
    if (mergeCooldown > 0)
    {
        Vector2 s = MeasureTextEx(fonts.heading, nextMergeText, HEAD_SMALL, SPACING_HEAD(HEAD_SMALL));
        Vector2 p = {screenWidth/2.0f - s.x/2.0f, 20.0f};
        DrawTextEx(fonts.heading, nextMergeText, p, HEAD_SMALL, SPACING_HEAD(HEAD_SMALL), COL_ENEMY_MERGED);
    }

    if (gamePaused) {
        DrawRectangle(0, 0, screenWidth, screenHeight, COL_OVERLAY);
        const char* pauseText = "GAME PAUSED";
        Vector2 s = MeasureTextEx(fonts.heading, pauseText, HEAD_BIG, SPACING_HEAD(HEAD_BIG));
        Vector2 p = {screenWidth/2.0f - s.x/2.0f, screenHeight/2.0f - s.y/2.0f};
        DrawTextEx(fonts.heading, pauseText, p, HEAD_BIG, SPACING_HEAD(HEAD_BIG), COL_TEXT);

        const char *labels[] = { "[R] RESTART", "[H] HOME", "[Q] QUIT" };
        float startY = p.y + s.y + 60.0f;
        Rectangle rects[3] = { 0 };
        DrawMenuButtonRow(labels, 3, startY, rects);
        buttons.restartFromPause = rects[0];
        buttons.goHomeFromPause = rects[1];
        buttons.quitFromPause = rects[2];
    }

    VFX_DrawHitVignette(COL_ENEMY_MERGED);   // draw last, on top of everything
}

static void DrawPlayerSprite(void)
{
    if (textures.player.id > 0)
    {
        float h = player.radius * 2.4f; // a bit taller than the base circle's
                                         // diameter, to fit the barrel/cone above it
        float w = h * ((float)textures.player.width / (float)textures.player.height);
        Rectangle src = { 0, 0, (float)textures.player.width, (float)textures.player.height };
        Rectangle dst = { player.position.x, player.position.y, w, h };
        Vector2 origin = { w * 0.5f, h * 0.6875f }; // ~ base circle's position within the sprite
        DrawTexturePro(textures.player, src, dst, origin,
                       player.rotation + TURRET_SPRITE_ROTATION_OFFSET, WHITE);
    }
    else
    {
        // --- original fallback, unchanged ---
        DrawCircleV(player.position, player.radius, player.color);
        DrawRectanglePro(
            (Rectangle){player.position.x, player.position.y, player.barrelSize.x, player.barrelSize.y},
            (Vector2){player.barrelSize.x/2, 0.0f},
            player.rotation,
            player.color
        );
    }
}

static void DrawEnemySprite(Texture2D tex, Enemy *e)
{
    if (tex.id > 0)
    {
        float d = e->radius * 2.0f; // diameter
        Rectangle src = { 0, 0, (float)tex.width, (float)tex.height };
        Rectangle dst = { e->position.x, e->position.y, d, d };
        Vector2 origin = { d * 0.5f, d * 0.5f };
        DrawTexturePro(tex, src, dst, origin, 0.0f, WHITE);
    }
    else
    {
        DrawPoly(e->position, 6, e->radius, 0.0f, e->color);
    }
}

//----------------------------------------------------------------------------------
// Screen: Ending
//----------------------------------------------------------------------------------
static void UpdateEndingScreen(void)
{
    if (IsMenuButtonActivated(buttons.restartFromEnd, KEY_R))
    {
        InitGame();
        currentScreen = SCREEN_GAMEPLAY;
    }
    if (IsMenuButtonActivated(buttons.goHomeFromEnd, KEY_H))
    {
        currentScreen = SCREEN_TITLE;
    }
    else if (IsMenuButtonActivated(buttons.quitFromEnd, KEY_Q))
    {
        shouldClose = true;
    }
}

static void DrawEndingScreen(void)
{
    // display score
    const char *scoreText = TextFormat("SCORE: %d", score);
    Vector2 s1 = MeasureTextEx(fonts.heading, scoreText, HEAD_BIG, SPACING_HEAD(HEAD_BIG));
    Vector2 p1 = {screenWidth/2.0f - s1.x/2.0f, screenHeight/2.0f - s1.y/2.0f};
    DrawTextEx(fonts.heading, scoreText, p1, HEAD_BIG, SPACING_HEAD(HEAD_BIG), COL_TEXT);

    // display highest score
    const char *highScoreText = "HIGH SCORE: 1200";
    Vector2 s2 = MeasureTextEx(fonts.heading, highScoreText, HEAD_SMALL, SPACING_BODY(HEAD_SMALL));
    Vector2 p2 = {screenWidth/2.0f - s2.x/2.0f, screenHeight/2.0f + s1.y/2.0f + 32.0f};
    DrawTextEx(fonts.heading, highScoreText, p2, HEAD_SMALL, SPACING_BODY(HEAD_SMALL), COL_TEXT_DIM);

    // draw menu buttons row
    const char *labels[] = { "[R] RESTART", "[H] HOME", "[Q] QUIT" };
    float startY = p2.y + s2.y + 60.0f;
    Rectangle rects[3] = { 0 };
    DrawMenuButtonRow(labels, 3, startY, rects);
    buttons.restartFromEnd = rects[0];
    buttons.goHomeFromEnd = rects[1];
    buttons.quitFromEnd = rects[2];
}

//----------------------------------------------------------------------------------
// Screen: How To Play
//----------------------------------------------------------------------------------
static void UpdateHowToPlayScreen(void)
{
    if (IsMenuButtonActivated(buttons.backFromHowtoplay, KEY_B))
        currentScreen = SCREEN_TITLE;
}

static void DrawHowToPlayScreen(void)
{
    DrawTexture(textures.howToPlay, 40, 40, WHITE);

    const char *controlsHdr = "CONTROLS";
    Vector2 s = MeasureTextEx(fonts.heading, controlsHdr, HEAD_SMALL, SPACING_HEAD(HEAD_SMALL));
    DrawTextEx(fonts.heading, controlsHdr, (Vector2){360.0f - s.x/2.0f, 60.0f}, HEAD_SMALL, SPACING_HEAD(HEAD_SMALL), COL_TEXT);

    const char *hint = "[LEFT/RIGHT] Rotate    [SPACE] Shoot    [P] Pause";
    Vector2 s2 = MeasureTextEx(fonts.body, hint, BODY_LG, SPACING_BODY(BODY_LG));
    DrawTextEx(fonts.body, hint, (Vector2){360.0f - s2.x/2.0f, 96.0f}, BODY_LG, SPACING_BODY(BODY_LG), COL_TEXT_DIM);

    DrawTextEx(fonts.heading, "YOUR WEAPON", (Vector2){170.0f, 150.0f}, HEAD_SMALL, SPACING_HEAD(HEAD_SMALL), COL_TEXT);
    DrawTextEx(fonts.body, "Rotate to aim and shoot incoming", (Vector2){170.0f, 184.0f}, BODY_LG, SPACING_BODY(BODY_LG), COL_TEXT);
    DrawTextEx(fonts.body, "enemies before they reach you.", (Vector2){170.0f, 208.0f}, BODY_LG, SPACING_BODY(BODY_LG), COL_TEXT);

    DrawTextEx(fonts.heading, "HEX ENEMY (REGULAR)", (Vector2){170.0f, 266.0f}, HEAD_SMALL, SPACING_HEAD(HEAD_SMALL), COL_TEXT);
    DrawTextEx(fonts.body, "Spawns from the arena edges, wanders", (Vector2){170.0f, 300.0f}, BODY_LG, SPACING_BODY(BODY_LG), COL_TEXT);
    DrawTextEx(fonts.body, "randomly and bounces off the outer walls.", (Vector2){170.0f, 324.0f}, BODY_LG, SPACING_BODY(BODY_LG), COL_TEXT);

    DrawTextEx(fonts.heading, "HEX ENEMY (MERGED)", (Vector2){170.0f, 382.0f}, HEAD_SMALL, SPACING_HEAD(HEAD_SMALL), COL_TEXT);
    DrawTextEx(fonts.body, "Forms when enemies merge. Heads", (Vector2){170.0f, 416.0f}, BODY_LG, SPACING_BODY(BODY_LG), COL_TEXT);
    DrawTextEx(fonts.body, "straight for your position to kill you.", (Vector2){170.0f, 440.0f}, BODY_LG, SPACING_BODY(BODY_LG), COL_TEXT);

    DrawTextEx(fonts.heading, "YOUR LIVES", (Vector2){170.0f, 498.0f}, HEAD_SMALL, SPACING_HEAD(HEAD_SMALL), COL_TEXT);
    DrawTextEx(fonts.body, "You begin with 5 lives. Lose one", (Vector2){170.0f, 532.0f}, BODY_LG, SPACING_BODY(BODY_LG), COL_TEXT);
    DrawTextEx(fonts.body, "whenever an enemy reaches you.", (Vector2){170.0f, 556.0f}, BODY_LG, SPACING_BODY(BODY_LG), COL_TEXT);

    const char *labels[] = { "[B] BACK" };
    Rectangle rects[1] = { 0 };
    DrawMenuButtonRow(labels, 1, 620.0f, rects);
    buttons.backFromHowtoplay = rects[0];
}

//----------------------------------------------------------------------------------
// Menu Button Utils
//----------------------------------------------------------------------------------
static void DrawMenuButtonRow(const char **labels, int count, float startY, Rectangle *outRects)
{    
    const float padX = 18.0f;
    const float padY = 10.0f;
    const float gap  = 32.0f;

    Vector2 sizes[8];
    float widths[8];
    float totalWidth = 0.0f;

    for (int i = 0; i < count; i++)
    {
        sizes[i] = MeasureTextEx(fonts.heading, labels[i], HEAD_MED, SPACING_HEAD(HEAD_MED));
        widths[i] = sizes[i].x + 2*padX;
        totalWidth += widths[i];
    }
    totalWidth += gap * (count - 1);

    float startX = screenWidth/2.0f - totalWidth/2.0f;
    float cursorX = startX;

    for (int i = 0; i < count; i++)
    {
        float w = widths[i];
        float h = sizes[i].y + 2*padY;
        float centerX = cursorX + w/2.0f;

        Rectangle rec = { cursorX, startY, w, h };

        if (outRects != NULL) outRects[i] = rec;

        bool hovered = CheckCollisionPointRec(GetMousePosition(), rec);
        
        if (hovered)
            DrawRectangleRec(rec, COL_BG_LIGHT);

        DrawTextEx(fonts.heading, labels[i],
            (Vector2){ centerX - sizes[i].x/2.0f, startY + h/2.0f - sizes[i].y/2.0f },
            HEAD_MED, SPACING_HEAD(HEAD_MED), COL_TEXT);

        cursorX += w + gap;
    }
}

static bool IsMenuButtonActivated(Rectangle rect, int hotkey)
{
    bool hovered = CheckCollisionPointRec(GetMousePosition(), rect);
    bool clicked = hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    bool hotPressed = IsKeyPressed(hotkey);
    if (clicked || hotPressed)
    {
        PlaySound(sounds.fxSelect);
        return true;
    }
    return false;
}