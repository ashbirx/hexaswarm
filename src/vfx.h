/*******************************************************************************************
*   
*   NOTE: This file was generated with the help of AI (Claude). 
*   I have absolutely no idea what's going on here, but it works, the game looks polished, and I'm happy.
*
*   vfx.h - minimal, subtle visual-effects module for HEXASWARM (header-only)
*
*   Header-only: every function is `static`, so #include "vfx.h" from main.c is
*   the entire integration - no second .c file to add to your build.
*
*   Usage from main.c:
*     #include "vfx.h"
*     VFX_Init();                                  // once, after InitWindow
*     VFX_Update();                                // once per frame, gameplay only
*     VFX_Draw();                                  // draws muzzle flash / death poof / merge pulse
*     VFX_SpawnMuzzleFlash(pos, COL_BULLET);
*     VFX_SpawnDeathPoof(pos, enemyColor);
*     VFX_SpawnMergePulse(pos, COL_ENEMY_MERGED);
*     VFX_TriggerPlayerHit();
*     VFX_DrawHitVignette(COL_ENEMY_MERGED);        // draw last, on top of everything
*     VFX_TriggerHeartPop(slotIndex);
*     VFX_IsHeartPopping(&idx, &scale, &alpha);     // main.c draws the heart itself
*     VFX_DrawHitFlash(tex, dst, origin, rot, t);   // additive white flash for hit enemies
*
********************************************************************************************/

#ifndef VFX_H
#define VFX_H

#include "raylib.h"
#include "raymath.h"
#include <stdbool.h>

// Tunable durations (frames, at 60fps) - safe to tweak
#define VFX_PLAYER_HIT_DURATION   20.0f
#define VFX_HEART_POP_DURATION    16.0f
#define VFX_HIT_FLASH_DURATION     4.0f   // suggested duration for the caller's own hit timer

//----------------------------------------------------------------------------------
// Internal state (private to whichever .c file includes this header)
//----------------------------------------------------------------------------------
typedef enum {
    FX_MUZZLE_FLASH,
    FX_DEATH_POOF,
    FX_MERGE_PULSE
} VFX_EffectType;

typedef struct VFX_Effect
{
    VFX_EffectType type;
    Vector2 position;
    Vector2 velocity;      // only used by FX_DEATH_POOF particles
    Color color;
    float timer;           // counts down to 0
    float duration;
    float startRadius;
    float endRadius;
    bool active;
} VFX_Effect;

#define VFX_MAX_EFFECTS 64
static VFX_Effect vfx_effects[VFX_MAX_EFFECTS] = { 0 };

static float vfx_playerHitTimer = 0.0f;

static float vfx_heartPopTimer = 0.0f;
static int vfx_heartPopSlotIndex = -1;

//----------------------------------------------------------------------------------
// Internal helper
//----------------------------------------------------------------------------------
static inline void VFX_SpawnEffect(VFX_EffectType type, Vector2 pos, Color color, float duration,
                                    float startRadius, float endRadius, Vector2 velocity)
{
    for (int i = 0; i < VFX_MAX_EFFECTS; i++)
    {
        if (!vfx_effects[i].active)
        {
            vfx_effects[i].type = type;
            vfx_effects[i].position = pos;
            vfx_effects[i].velocity = velocity;
            vfx_effects[i].color = color;
            vfx_effects[i].timer = duration;
            vfx_effects[i].duration = duration;
            vfx_effects[i].startRadius = startRadius;
            vfx_effects[i].endRadius = endRadius;
            vfx_effects[i].active = true;
            return;
        }
    }
    // pool full: silently drop - effects are purely cosmetic, never gameplay-critical
}

//----------------------------------------------------------------------------------
// Public API
//----------------------------------------------------------------------------------
static inline void VFX_Init(void)
{
    for (int i = 0; i < VFX_MAX_EFFECTS; i++) vfx_effects[i].active = false;
    vfx_playerHitTimer = 0.0f;
    vfx_heartPopTimer = 0.0f;
    vfx_heartPopSlotIndex = -1;
}

static inline void VFX_SpawnMuzzleFlash(Vector2 pos, Color color)
{
    VFX_SpawnEffect(FX_MUZZLE_FLASH, pos, color, 6.0f, 10.0f, 2.0f, (Vector2){ 0, 0 });
}

static inline void VFX_SpawnDeathPoof(Vector2 pos, Color color)
{
    int n = 6;
    for (int i = 0; i < n; i++)
    {
        float ang = (360.0f / n) * i + (float)GetRandomValue(-10, 10);
        float speed = (float)GetRandomValue(60, 120) / 60.0f;
        Vector2 vel = { cosf(ang * DEG2RAD) * speed, sinf(ang * DEG2RAD) * speed };
        VFX_SpawnEffect(FX_DEATH_POOF, pos, color, 18.0f, 4.0f, 0.0f, vel);
    }
}

static inline void VFX_SpawnMergePulse(Vector2 pos, Color color)
{
    VFX_SpawnEffect(FX_MERGE_PULSE, pos, color, 20.0f, 8.0f, 50.0f, (Vector2){ 0, 0 });
}

static inline void VFX_Update(void)
{
    for (int i = 0; i < VFX_MAX_EFFECTS; i++)
    {
        if (!vfx_effects[i].active) continue;

        if (vfx_effects[i].type == FX_DEATH_POOF)
        {
            vfx_effects[i].position = Vector2Add(vfx_effects[i].position, vfx_effects[i].velocity);
            vfx_effects[i].velocity = Vector2Scale(vfx_effects[i].velocity, 0.9f); // soft drag
        }

        vfx_effects[i].timer -= 1.0f;
        if (vfx_effects[i].timer <= 0.0f) vfx_effects[i].active = false;
    }

    if (vfx_playerHitTimer > 0.0f) vfx_playerHitTimer -= 1.0f;
    if (vfx_heartPopTimer > 0.0f) vfx_heartPopTimer -= 1.0f;
}

static inline void VFX_Draw(void)
{
    for (int i = 0; i < VFX_MAX_EFFECTS; i++)
    {
        if (!vfx_effects[i].active) continue;

        float t = 1.0f - (vfx_effects[i].timer / vfx_effects[i].duration); // 0 -> 1
        float radius = vfx_effects[i].startRadius + (vfx_effects[i].endRadius - vfx_effects[i].startRadius) * t;
        Color c = vfx_effects[i].color;
        c.a = (unsigned char)(255 * (1.0f - t));

        switch (vfx_effects[i].type)
        {
        case FX_MUZZLE_FLASH:
        case FX_DEATH_POOF:
            DrawCircleV(vfx_effects[i].position, radius, c);
            break;
        case FX_MERGE_PULSE:
            DrawRing(vfx_effects[i].position, radius - 3.0f, radius, 0, 360, 24, c);
            break;
        }
    }
}

static inline void VFX_TriggerPlayerHit(void)
{
    vfx_playerHitTimer = VFX_PLAYER_HIT_DURATION;
}

static inline void VFX_DrawHitVignette(Color color)
{
    if (vfx_playerHitTimer <= 0.0f) return;

    float alpha01 = vfx_playerHitTimer / VFX_PLAYER_HIT_DURATION;
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    Color edge  = Fade(color, 0.5f * alpha01);
    Color clear = Fade(color, 0.0f);
    int band = 60;

    DrawRectangleGradientV(0, 0, screenW, band, edge, clear);
    DrawRectangleGradientV(0, screenH - band, screenW, band, clear, edge);
    DrawRectangleGradientH(0, 0, band, screenH, edge, clear);
    DrawRectangleGradientH(screenW - band, 0, band, screenH, clear, edge);
}

static inline void VFX_TriggerHeartPop(int slotIndex)
{
    vfx_heartPopTimer = VFX_HEART_POP_DURATION;
    vfx_heartPopSlotIndex = slotIndex;
}

static inline bool VFX_IsHeartPopping(int *outSlotIndex, float *outScale, float *outAlpha)
{
    if (vfx_heartPopTimer <= 0.0f) return false;

    float t = 1.0f - (vfx_heartPopTimer / VFX_HEART_POP_DURATION); // 0 -> 1
    if (outSlotIndex) *outSlotIndex = vfx_heartPopSlotIndex;
    if (outScale) *outScale = 1.0f - t;
    if (outAlpha) *outAlpha = 1.0f - t;
    return true;
}

static inline void VFX_DrawHitFlash(Texture2D tex, Rectangle dst, Vector2 origin, float rotation, float intensity01)
{
    if (intensity01 <= 0.0f) return;

    Rectangle src = { 0, 0, (float)tex.width, (float)tex.height };

    // additive blend: brightens only the sprite's own opaque pixels toward white,
    // transparent pixels stay untouched since they contribute nothing additively
    BeginBlendMode(BLEND_ADDITIVE);
    DrawTexturePro(tex, src, dst, origin, rotation, Fade(WHITE, intensity01));
    EndBlendMode();
}

#endif // VFX_H