#include <raylib.h>
#include <cmath>

#include <cmath>

#include "game.h"

#include "defines.h"
#include "util.h"

static void drawMenuBackdrop(float t, float jitterX) {
    ClearBackground(Color{ 8, 12, 9, 255 });

    DrawSlantedQuad(-35.0f, 0.0f, 55.0f, (float)SCREEN_HEIGHT, 55.0f, Color{ 14, 24, 16, 255 });
    DrawSlantedQuad(115.0f, 0.0f, 75.0f, (float)SCREEN_HEIGHT, 48.0f, Color{ 16, 28, 20, 255 });

    float scanY = std::fmod(t * 36.0f, (float)SCREEN_HEIGHT);
    DrawRectangle(0, (int)scanY, SCREEN_WIDTH, 2, Color{ 0x23, 0x7b, 0x52, 45 });
}

static void drawMenuButton(const char* label, float x, float y, float w, float h,
                           bool selected, float jitterX, float jitterY) {
    float slant = -10.0f;
    float bx = x + (selected ? jitterX : 0.0f);
    float by = y + (selected ? jitterY : 0.0f);

    if (selected) {
        DrawSlantedQuad(bx + 5.0f, by + 4.0f, w, h, slant - 2.0f, Color{ 0x33, 0x65, 0x02, 110 });
        DrawSlantedQuad(bx - 3.0f, by - 2.0f, w, h, slant + 1.0f, Color{ 0x23, 0x7b, 0x52, 85 });
    } else {
        DrawSlantedQuad(bx + 3.0f, by + 4.0f, w, h, slant, Color{ 3, 6, 4, 210 });
    }

    Color cardColor = selected ? Color{ 20, 32, 24, 255 } : Color{ 12, 17, 14, 255 };
    DrawSlantedQuad(bx, by, w, h, slant, cardColor);

    Color accentColor = selected ? Color{ 0x33, 0x65, 0x02, 255 } : Color{ 0x23, 0x7b, 0x52, 120 };
    DrawSlantedQuad(bx + 1.0f, by, w - 2.0f, 4.0f, slant, accentColor);
    DrawSlantedQuad(bx, by + h - 2.0f, w, 2.0f, slant,
                    selected ? Color{ 0x23, 0x7b, 0x52, 255 } : Color{ 25, 40, 30, 255 });

    int fontSize = selected ? 12 : 10;
    if (selected) {
        DrawText(">", (int)(bx + 12.0f), (int)(by + h / 2.0f - 6.0f), 10, Color{ 0x23, 0x7b, 0x52, 255 });
    }
    int nw = MeasureText(label, fontSize);
    DrawText(label, (int)(bx + (w - nw) / 2.0f), (int)(by + h / 2.0f - fontSize / 2.0f), fontSize,
             selected ? Color{ 200, 240, 180, 255 } : Color{ 85, 115, 95, 255 });
}

static const float BTN_W = 200.0f;
static const float BTN_H = 36.0f;
static const float BTN_X = (SCREEN_WIDTH - BTN_W) / 2.0f;
static const float PLAY_Y = 110.0f;
static const float CREDITS_Y = 158.0f;

static void activateMenuItem(GameState* state, int item) {
    if (item == 0) {
        state->selectedCharacter = 0;
        state->currentScreen = SCREEN_SELECT;
    } else {
        state->currentScreen = SCREEN_CREDITS;
    }
}

void menuScreen(GameState* state, Vector2 mousePos) {
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))   state->menuSelection = 0;
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) state->menuSelection = 1;

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        activateMenuItem(state, state->menuSelection);
    }

    Rectangle playHit    = { BTN_X, PLAY_Y, BTN_W, BTN_H };
    Rectangle creditsHit = { BTN_X, CREDITS_Y, BTN_W, BTN_H };

    if (CheckCollisionPointRec(mousePos, playHit)) {
        state->menuSelection = 0;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) activateMenuItem(state, 0);
    }
    if (CheckCollisionPointRec(mousePos, creditsHit)) {
        state->menuSelection = 1;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) activateMenuItem(state, 1);
    }

    float t = (float)GetTime();
    float jitterX = (std::sin(t * 35.0f) > 0.92f) ? 2.0f : 0.0f;
    float jitterY = (std::cos(t * 27.0f) < -0.94f) ? -1.0f : 0.0f;

    drawMenuBackdrop(t, jitterX);

    // Title panel
    DrawSlantedQuad(12.0f + jitterX, 26.0f, 296.0f, 52.0f, -8.0f, Color{ 0x33, 0x65, 0x02, 90 });
    DrawSlantedQuad(10.0f, 24.0f, 296.0f, 52.0f, -8.0f, Color{ 16, 24, 19, 255 });
    DrawSlantedQuad(12.0f, 24.0f, 6.0f, 52.0f, -8.0f, Color{ 0x33, 0x65, 0x02, 255 });
    DrawSlantedQuad(18.0f, 24.0f, 2.0f, 52.0f, -8.0f, Color{ 0x23, 0x7b, 0x52, 255 });

    const char* title = "THE LAST LOOP";
    int tw = MeasureText(title, 20);
    int tx = (int)((SCREEN_WIDTH - tw) / 2.0f) + (int)jitterX;
    DrawText(title, tx + 2, 43, 20, Color{ 0x33, 0x65, 0x02, 200 });
    DrawText(title, tx, 41, 20, Color{ 190, 230, 170, 255 });

    drawMenuButton("PLAY", BTN_X, PLAY_Y, BTN_W, BTN_H,
                   state->menuSelection == 0, jitterX, jitterY);
    drawMenuButton("CREDITS", BTN_X, CREDITS_Y, BTN_W, BTN_H,
                   state->menuSelection == 1, jitterX, jitterY);

    DrawText("Up/Down", 12, 222, 7, Color{ 0x23, 0x7b, 0x52, 200 });
    DrawText("[ENTER] Select", 140, 222, 7, Color{ 170, 220, 140, 255 });
}

void creditsScreen(GameState* state, Vector2 mousePos) {
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ESCAPE)) {
        state->currentScreen = SCREEN_MENU;
    }

    Rectangle backHit = { BTN_X, 180.0f, BTN_W, BTN_H };
    if (CheckCollisionPointRec(mousePos, backHit) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        state->currentScreen = SCREEN_MENU;
    }

    float t = (float)GetTime();
    float jitterX = (std::sin(t * 35.0f) > 0.92f) ? 2.0f : 0.0f;

    drawMenuBackdrop(t, jitterX);

    // Header
    DrawSlantedQuad(12.0f + jitterX, 14.0f, 220.0f, 22.0f, -15.0f, Color{ 0x33, 0x65, 0x02, 90 });
    DrawSlantedQuad(10.0f, 12.0f, 220.0f, 22.0f, -15.0f, Color{ 16, 24, 19, 255 });
    DrawSlantedQuad(12.0f, 12.0f, 6.0f, 22.0f, -15.0f, Color{ 0x33, 0x65, 0x02, 255 });
    DrawSlantedQuad(18.0f, 12.0f, 2.0f, 22.0f, -15.0f, Color{ 0x23, 0x7b, 0x52, 255 });
    DrawText("Credits", 28 + (int)jitterX, 17, 10, Color{ 190, 230, 170, 255 });

    // Credits panel
    DrawSlantedQuad(18.0f, 50.0f + 4.0f, 280.0f, 110.0f, -6.0f, Color{ 0x33, 0x65, 0x02, 80 });
    DrawSlantedQuad(14.0f, 50.0f, 282.0f, 110.0f, -6.0f, Color{ 13, 20, 15, 255 });
    DrawSlantedQuad(12.0f, 50.0f, 5.0f, 110.0f, -6.0f, Color{ 0x33, 0x65, 0x02, 255 });
    DrawSlantedQuad(17.0f, 50.0f, 2.0f, 110.0f, -6.0f, Color{ 0x23, 0x7b, 0x52, 255 });

    DrawText("Game Design", 26, 62, 8, Color{ 0x23, 0x7b, 0x52, 255 });
    DrawText("Your Name", 26, 74, 8, Color{ 165, 190, 175, 255 });

    DrawText("Programming", 26, 92, 8, Color{ 0x23, 0x7b, 0x52, 255 });
    DrawText("Your Name", 26, 104, 8, Color{ 165, 190, 175, 255 });

    DrawText("Art and Audio", 26, 122, 8, Color{ 0x23, 0x7b, 0x52, 255 });
    DrawText("Your Name", 26, 134, 8, Color{ 165, 190, 175, 255 });

    drawMenuButton("BACK", BTN_X, 180.0f, BTN_W, BTN_H, true, jitterX, 0.0f);

    DrawText("[ENTER] Back", 140, 222, 7, Color{ 170, 220, 140, 255 });
}
