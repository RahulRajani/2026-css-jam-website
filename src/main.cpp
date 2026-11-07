#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#include <raylib.h>
#include "chronos.h"
#include "lighting.h"

#include <cstdint>
#include <vector>
#include <cmath>
#include <algorithm>

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240

#define PLAYER_RECT_SIZE 16
#define PLAYER_VELOCITY_MAX 100
#define PLAYER_ACCELERATION 15
#define PLAYER_LIGHT_SIZE 100

enum ScreenState {
    SCREEN_SELECT,
    SCREEN_GAME
};

struct Player {
    float x, y;
    float velx, vely;
};

struct GameAssets {
    Shader crtShader;

    Texture players[2];
    Music bgm;
};

struct GameState {
    ScreenState currentScreen;
    int selectedCharacter;
    float selectAnim;

    Player player;
    Chronos chronos;
    LightingMap lightingMap;

    int currentLoop = 1;
    float timeRemaining = 60.0f;
    bool gameOver = false;
    bool chronosDangerous = false;

    GameAssets assets;

    RenderTexture2D screenTexture;
};

Vector2 getGameMousePos() {
    Vector2 mousePos = GetMousePosition();

    int scrWidth = GetScreenWidth();
    int scrHeight = GetScreenHeight();

    int scale = std::min(scrWidth / SCREEN_WIDTH, scrHeight / SCREEN_HEIGHT);

    int scrBufferWidth = SCREEN_WIDTH * scale;
    int scrBufferHeight = SCREEN_HEIGHT * scale;

    const Rectangle gameRect = {
        (scrWidth - scrBufferWidth) / 2.0f,
        (scrHeight - scrBufferHeight) / 2.0f,
        (float)scrBufferWidth, (float)scrBufferHeight
    };

    if (!CheckCollisionPointRec(mousePos, gameRect)) {
        return (Vector2){-1.0f, -1.0f};
    }

    return (Vector2){
        (mousePos.x - gameRect.x) / scale,
        (mousePos.y - gameRect.y) / scale
    };
}

void DrawSlantedQuad(float x, float y, float w, float h, float slant, Color color) {
    Vector2 v1 = { x + slant, y };
    Vector2 v2 = { x, y + h };
    Vector2 v3 = { x + w + slant, y };
    Vector2 v4 = { x + w, y + h };

    DrawTriangle(v1, v2, v3, color);
    DrawTriangle(v3, v2, v4, color);
}

void selectScreen(GameState* state, Vector2 mousePos) {
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))  state->selectedCharacter = 0;
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) state->selectedCharacter = 1;

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        state->currentScreen = SCREEN_GAME;
    }

    Rectangle jamieHit = { 20.0f, 44.0f, 96.0f, 106.0f };
    Rectangle carolHit = { 124.0f, 44.0f, 96.0f, 106.0f };

    if (CheckCollisionPointRec(mousePos, jamieHit)) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (state->selectedCharacter == 0) state->currentScreen = SCREEN_GAME;
            else state->selectedCharacter = 0;
        }
    }
    if (CheckCollisionPointRec(mousePos, carolHit)) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (state->selectedCharacter == 1) state->currentScreen = SCREEN_GAME;
            else state->selectedCharacter = 1;
        }
    }

    float t = (float)GetTime();
    float jitterX = (std::sin(t * 35.0f) > 0.92f) ? 2.0f : 0.0f;
    float jitterY = (std::cos(t * 27.0f) < -0.94f) ? -1.0f : 0.0f;

    ClearBackground(Color{ 8, 12, 9, 255 });

    DrawSlantedQuad(-35.0f, 0.0f, 55.0f, (float)SCREEN_HEIGHT, 55.0f, Color{ 14, 24, 16, 255 });
    DrawSlantedQuad(115.0f, 0.0f, 75.0f, (float)SCREEN_HEIGHT, 48.0f, Color{ 16, 28, 20, 255 });

    float scanY = std::fmod(t * 36.0f, (float)SCREEN_HEIGHT);
    DrawRectangle(0, (int)scanY, SCREEN_WIDTH, 2, Color{ 0x23, 0x7b, 0x52, 45 });

    DrawSlantedQuad(12.0f + jitterX, 14.0f, 220.0f, 22.0f, -15.0f, Color{ 0x33, 0x65, 0x02, 90 });
    DrawSlantedQuad(10.0f, 12.0f, 220.0f, 22.0f, -15.0f, Color{ 16, 24, 19, 255 });
    DrawSlantedQuad(12.0f, 12.0f, 6.0f, 22.0f, -15.0f, Color{ 0x33, 0x65, 0x02, 255 });
    DrawSlantedQuad(18.0f, 12.0f, 2.0f, 22.0f, -15.0f, Color{ 0x23, 0x7b, 0x52, 255 });
    DrawText("Select your Character", 28 + (int)jitterX, 17, 10, Color{ 190, 230, 170, 255 });
    DrawText("Fugit Hora", 180, 18, 8, Color{ 0x23, 0x7b, 0x52, 255 });

    for (int i = 0; i < 2; i++) {
        bool selected = (state->selectedCharacter == i);
        float slant = (i == 0) ? -14.0f : -8.0f;
        float x = ((i == 0) ? 22.0f : 126.0f) + (selected ? jitterX : 0.0f);
        float y = (selected ? 41.0f : 47.0f) + (selected ? jitterY : 0.0f); 
        float w = 92.0f;
        float h = selected ? 109.0f : 99.0f;

        if (selected) {
            DrawSlantedQuad(x + 5.0f, y + 4.0f, w, h, slant - 2.0f, Color{ 0x33, 0x65, 0x02, 110 });
            DrawSlantedQuad(x - 3.0f, y - 2.0f, w, h, slant + 1.0f, Color{ 0x23, 0x7b, 0x52, 85 });
        } else {
            DrawSlantedQuad(x + 3.0f, y + 4.0f, w, h, slant, Color{ 3, 6, 4, 210 });
        }

        Color cardColor = selected ? Color{ 20, 32, 24, 255 } : Color{ 12, 17, 14, 255 };
        DrawSlantedQuad(x, y, w, h, slant, cardColor);

        Color accentColor = selected ? Color{ 0x33, 0x65, 0x02, 255 } : Color{ 0x23, 0x7b, 0x52, 120 };
        DrawSlantedQuad(x + 1.0f, y, w - 2.0f, 4.0f, slant, accentColor);
        DrawSlantedQuad(x, y + h - 2.0f, w, 2.0f, slant, selected ? Color{ 0x23, 0x7b, 0x52, 255 } : Color{ 25, 40, 30, 255 });

        const char* numTag = (i == 0) ? "01" : "02";
        DrawText(numTag, (int)(x + 14.0f), (int)(y + 8.0f), 10, selected ? Color{ 0x23, 0x7b, 0x52, 255 } : Color{ 40, 60, 48, 255 });

        float spriteSize = selected ? 38.0f : 28.0f;
        float spriteY = y + (selected ? 22.0f : 24.0f);
        float spriteX = x + (w - spriteSize) / 2.0f - 2.0f;

        if (selected) {
            DrawTexturePro(
                state->assets.players[i],
                { 0, 0, (float)PLAYER_RECT_SIZE, (float)PLAYER_RECT_SIZE },
                { spriteX + 2.0f, spriteY, spriteSize, spriteSize },
                { 0, 0 }, 0.0f, Color{ 0x33, 0x65, 0x02, 160 }
            );
        }

        DrawTexturePro(
            state->assets.players[i],
            { 0, 0, (float)PLAYER_RECT_SIZE, (float)PLAYER_RECT_SIZE },
            { spriteX, spriteY, spriteSize, spriteSize },
            { 0, 0 }, 0.0f, selected ? WHITE : Color{ 110, 140, 120, 190 }
        );

        const char* name = (i == 0) ? "JAMIE" : "CAROL";
        Color nameColor = selected ? Color{ 200, 240, 180, 255 } : Color{ 85, 115, 95, 255 };
        
        if (selected) {
            DrawSlantedQuad(x + 8.0f, y + h - 26.0f, w - 16.0f, 16.0f, -6.0f, Color{ 0x33, 0x65, 0x02, 255 });
            int nw = MeasureText(name, 10);
            DrawText(name, (int)(x + (w - nw) / 2.0f), (int)(y + h - 23.0f), 10, nameColor);
        } else {
            int nw = MeasureText(name, 9);
            DrawText(name, (int)(x + (w - nw) / 2.0f), (int)(y + h - 20.0f), 9, nameColor);
        }
    }

    float descY = 162.0f;
    float descH = 50.0f;

    DrawSlantedQuad(18.0f - jitterX, descY + 4.0f, 210.0f, descH, -13.0f, Color{ 0x33, 0x65, 0x02, 80 });
    DrawSlantedQuad(14.0f, descY, 212.0f, descH, -11.0f, Color{ 13, 20, 15, 255 });
    DrawSlantedQuad(12.0f, descY, 5.0f, descH, -11.0f, Color{ 0x33, 0x65, 0x02, 255 });
    DrawSlantedQuad(17.0f, descY, 2.0f, descH, -11.0f, Color{ 0x23, 0x7b, 0x52, 255 });

    if (state->selectedCharacter == 0) {
        DrawText("2nd Year Uni Student", 26 + (int)jitterX, (int)descY + 6, 8, Color{ 150, 215, 110, 255 });
        DrawText("Ability", 26, (int)descY + 18, 8, Color{ 0x23, 0x7b, 0x52, 255 });
        DrawText(": Radar ping detects clues", 66, (int)descY + 18, 8, Color{ 165, 190, 175, 255 });
        DrawText("Trait", 26, (int)descY + 30, 8, Color{ 0x33, 0x65, 0x02, 255 });
        DrawText(" Lower Field of vision", 66, (int)descY + 30, 8, Color{ 120, 145, 130, 255 });
    } else {
        DrawText("2nd Year Uni Student", 26 + (int)jitterX, (int)descY + 6, 8, Color{ 150, 215, 110, 255 });
        DrawText("Ability", 26, (int)descY + 18, 8, Color{ 0x23, 0x7b, 0x52, 255 });
        DrawText(": Able to sprint for period", 66, (int)descY + 18, 8, Color{ 165, 190, 175, 255 });
        DrawText("Trait", 26, (int)descY + 30, 8, Color{ 0x33, 0x65, 0x02, 255 });
        DrawText(": Lower regular speed", 66, (int)descY + 30, 8, Color{ 120, 145, 130, 255 });
    }

    DrawText("Left/Right", 12, 222, 7, Color{ 0x23, 0x7b, 0x52, 200 });
    DrawText("[ENTER] Start", 140, 222, 7, Color{ 170, 220, 140, 255 });
}

void gameScreen(GameState* state, Vector2 mousePos, float deltaTime) {
    if (!state->gameOver && state->timeRemaining > 0.0f) {
        state->timeRemaining -= deltaTime;

        if (state->timeRemaining < 0.0f) {
            state->timeRemaining = 0.0f;
        }

        if (state->timeRemaining <= 0.0f) {
            if (state->currentLoop < 3) {
                state->currentLoop++;
                state->timeRemaining = 60.0f;

                state->player.x = SCREEN_WIDTH / 2.0f;
                state->player.y = SCREEN_HEIGHT / 2.0f;
                state->player.velx = 0.0f;
                state->player.vely = 0.0f;
            } else {
                state->gameOver = true;
            }
        }
    }

    state->chronosDangerous =
        !state->gameOver &&
        state->timeRemaining > 0.0f &&
        state->timeRemaining <= 15.0f;

    updateChronos(state->chronos, deltaTime, state->chronosDangerous);

    float playerMouseAngle = std::atan2(
        mousePos.y - state->player.y,
        mousePos.x - state->player.x
    );

    if (!state->gameOver) {
        float accelerateX = (float)(IsKeyDown(KEY_D) - IsKeyDown(KEY_A));
        float accelerateY = (float)(IsKeyDown(KEY_S) - IsKeyDown(KEY_W));

        state->player.velx = std::lerp(
            state->player.velx, PLAYER_VELOCITY_MAX * accelerateX,
            PLAYER_ACCELERATION * deltaTime
        );
        state->player.vely = std::lerp(
            state->player.vely, PLAYER_VELOCITY_MAX * accelerateY,
            PLAYER_ACCELERATION * deltaTime
        );

        state->player.x += state->player.velx * deltaTime;
        state->player.y += state->player.vely * deltaTime;
    }

    ClearBackground(Color{20, 20, 24, 255});
    // Temporary occlusion-test wall; replace with the map's obstacle data later.
    DrawRectangle(140, 58, 4, 108, DARKGRAY);

    const std::vector<LightSource> lightSources{
        LightSource{
            static_cast<int>(state->player.x),
            static_cast<int>(state->player.y),
            PLAYER_LIGHT_SIZE / 2
        }
    };
    state->lightingMap.calcAndDrawLights(
        lightSources, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT
    );

    Color chronosColor = DARKGRAY;

    if (state->chronosDangerous &&
        static_cast<int>(GetTime() * 4.0) % 2 == 0) {
        chronosColor = RED;
    }

    DrawCircleV(
        state->chronos.position,
        8.0f,
        chronosColor
    );

    DrawTexturePro(
        state->assets.players[state->selectedCharacter],
        {0, 0, (float)PLAYER_RECT_SIZE, (float)PLAYER_RECT_SIZE},
        {state->player.x, state->player.y, (float)PLAYER_RECT_SIZE, (float)PLAYER_RECT_SIZE},
        {PLAYER_RECT_SIZE / 2.0f, PLAYER_RECT_SIZE / 2.0f},
        playerMouseAngle * RAD2DEG, {255, 255, 255, 255}
    );
}

void mainLoop(void* data) {
    GameState* state = (GameState*)data;
    if (IsAudioDeviceReady()) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || GetKeyPressed() != 0) {
            if (!IsMusicStreamPlaying(state->assets.bgm)) {
                PlayMusicStream(state->assets.bgm);
            }
        }
    }

    UpdateMusicStream(state->assets.bgm);

    float deltaTime = GetFrameTime();
    Vector2 gameMousePos = getGameMousePos();

    BeginTextureMode(state->screenTexture);
        if (state->currentScreen == SCREEN_SELECT) {
            selectScreen(state, gameMousePos);
        } else if (state->currentScreen == SCREEN_GAME) {
            gameScreen(state, gameMousePos, deltaTime);
        }
    EndTextureMode();

    BeginDrawing();
        ClearBackground(BLACK);

        int scrWidth = GetScreenWidth();
        int scrHeight = GetScreenHeight();

        int scale = std::min(scrWidth / SCREEN_WIDTH, scrHeight / SCREEN_HEIGHT);

        int scrBufferWidth = SCREEN_WIDTH * scale;
        int scrBufferHeight = SCREEN_HEIGHT * scale;

        float crtRes[2] = {(float)scrBufferWidth, (float)scrBufferHeight};
        SetShaderValue(state->assets.crtShader, GetShaderLocation(state->assets.crtShader, "resolution"), crtRes, SHADER_UNIFORM_VEC2);

        BeginShaderMode(state->assets.crtShader); {
            DrawTexturePro(
                state->screenTexture.texture,
                (Rectangle){0, 0, (float)state->screenTexture.texture.width, (float)-state->screenTexture.texture.height},
                (Rectangle){
                    (float)(int)((scrWidth - scrBufferWidth) / 2.0f),
                    (float)(int)((scrHeight - scrBufferHeight) / 2.0f),
                    (float)scrBufferWidth, (float)scrBufferHeight
                },
                (Vector2){0.0f, 0.0f}, 0.0f, WHITE
            );
        } EndShaderMode();

        DrawFPS(10, 10);
    EndDrawing();
}

GameAssets loadAssets() {
    GameAssets assets = {};

    #ifdef __EMSCRIPTEN__
    assets.crtShader = LoadShader(0, "assets/crt.frag");
    #else
    assets.crtShader = LoadShader(0, "assets/crt_desktop.frag");
    #endif

    float crtCurve = 1.2f;
    float crtVignette = 0.4f;

    SetShaderValue(
        assets.crtShader, GetShaderLocation(assets.crtShader, "curvature"),
        &crtCurve, SHADER_UNIFORM_FLOAT
    );
    SetShaderValue(
        assets.crtShader, GetShaderLocation(assets.crtShader, "vignette"),
        &crtVignette, SHADER_UNIFORM_FLOAT
    );

    assets.players[0] = LoadTexture("assets/player_jamie.png");
    assets.players[1] = LoadTexture("assets/player_carol.png");

    assets.bgm = LoadMusicStream("assets/oooscary.mp3");
    assets.bgm.looping = true;
    SetMusicVolume(assets.bgm, 0.7f); 
    PlayMusicStream(assets.bgm);

    return assets;
}

void unloadAssets(GameAssets& assets) {
    UnloadShader(assets.crtShader);

    UnloadTexture(assets.players[0]);
    UnloadTexture(assets.players[1]);

    UnloadMusicStream(assets.bgm);

    assets = {};
}

int main(int argc, char** argv) {
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(1280, 720, "game");
    InitAudioDevice();

    GameState state = {};
    state.player = {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
    state.currentScreen = SCREEN_SELECT;
    state.selectedCharacter = 0;
    state.selectAnim = 0.0f;
    state.assets = loadAssets();

    state.lightingMap = LightingMap(SCREEN_WIDTH, SCREEN_HEIGHT);
    for (int y = 58; y < 166; ++y) {
        for (int x = 140; x < 144; ++x) {
            state.lightingMap.setObstacle(x, y, 1);
        }
    }

    state.screenTexture = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    SetTextureWrap(state.screenTexture.texture, TEXTURE_WRAP_CLAMP);

    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(mainLoop, &state, 0, true);
    #else
    while (!WindowShouldClose()) {
        mainLoop(&state);
    }
    #endif

    UnloadRenderTexture(state.screenTexture);

    unloadAssets(state.assets);

    CloseAudioDevice();
    CloseWindow();

    return 0;
}
