#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif
#include <raylib.h>
#include "chronos.h"
#include "lighting.h"
#include "Map.h"
#include "island_layout.h"
#include "animated_sprite.h"
#include "util.h"
#include "game.h"   // GameState, structs (+ defines.h: screen size, ScreenState)
#include "island2_layout.h"   // island 2 layout
#include "island3_layout.h"   // island 3 layout (used by the minimap)
#include "TilePalette.h"    // TileID::*
#include "wreck_props.h"    // trust.png plane-wreck props

// Title + credits screens live in main_menu.cpp
void menuScreen(GameState* state, Vector2 mousePos);
void creditsScreen(GameState* state, Vector2 mousePos);
void startBoss(GameState* state);
bool bossActive();
void bossScreen(GameState* state, Vector2 mousePos, float deltaTime);

#include <cstdint>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>

Vector2 getGameMousePos() {
    Vector2 mousePos = GetMousePosition();

    int scrWidth = GetScreenWidth();
    int scrHeight = GetScreenHeight();

    int scale = std::max(1, std::min(scrWidth / SCREEN_WIDTH, scrHeight / SCREEN_HEIGHT));

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

void selectScreen(GameState* state, Vector2 mousePos) {
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))  state->selectedCharacter = 0;
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) state->selectedCharacter = 1;

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        state->dialogueIndex = 0;
        state->currentScreen = SCREEN_DIALOGUE;
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        state->currentScreen = SCREEN_MENU;
    }

    Rectangle jamieHit = { 20.0f, 44.0f, 96.0f, 106.0f };
    Rectangle carolHit = { 124.0f, 44.0f, 96.0f, 106.0f };

    if (CheckCollisionPointRec(mousePos, jamieHit)) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (state->selectedCharacter == 0) {
                state->dialogueIndex = 0;
                state->currentScreen = SCREEN_DIALOGUE;
            } else {
                state->selectedCharacter = 0;
            }
        }
    }
    if (CheckCollisionPointRec(mousePos, carolHit)) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (state->selectedCharacter == 1) {
                state->dialogueIndex = 0;
                state->currentScreen = SCREEN_DIALOGUE;
            } else {
                state->selectedCharacter = 1;
            }
        }
    }

    float t = (float)GetTime();
    float jitterX = (std::sin(t * 35.0f) > 0.92f) ? 2.0f : 0.0f;
    float jitterY = (std::cos(t * 27.0f) < -0.94f) ? -1.0f : 0.0f;

    drawMenuBackdrop(t, jitterX);

    // Header panel
    DrawSlantedQuad(12.0f + jitterX, 14.0f, 220.0f, 22.0f, -15.0f, Color{ 150, 80, 200, 90 });
    DrawSlantedQuad(10.0f, 12.0f, 220.0f, 22.0f, -15.0f, Color{ 24, 14, 36, 255 });
    DrawSlantedQuad(12.0f, 12.0f, 6.0f, 22.0f, -15.0f, Color{ 212, 175, 55, 255 });
    DrawSlantedQuad(18.0f, 12.0f, 2.0f, 22.0f, -15.0f, Color{ 150, 80, 200, 255 });
    DrawText("Select your Character", 28 + (int)jitterX, 17, 10, Color{ 240, 210, 130, 255 });

    for (int i = 0; i < 2; i++) {
        bool selected = (state->selectedCharacter == i);
        float slant = (i == 0) ? -14.0f : -8.0f;
        float x = ((i == 0) ? 22.0f : 126.0f) + (selected ? jitterX : 0.0f);
        float y = (selected ? 41.0f : 47.0f) + (selected ? jitterY : 0.0f);
        float w = 92.0f;
        float h = selected ? 109.0f : 99.0f;

        if (selected) {
            DrawSlantedQuad(x + 5.0f, y + 4.0f, w, h, slant - 2.0f, Color{ 150, 80, 200, 110 });
            DrawSlantedQuad(x - 3.0f, y - 2.0f, w, h, slant + 1.0f, Color{ 212, 175, 55, 85 });
        } else {
            DrawSlantedQuad(x + 3.0f, y + 4.0f, w, h, slant, Color{ 5, 3, 8, 210 });
        }

        Color cardColor = selected ? Color{ 40, 24, 58, 255 } : Color{ 18, 11, 26, 255 };
        DrawSlantedQuad(x, y, w, h, slant, cardColor);

        Color accentColor = selected ? Color{ 150, 80, 200, 255 } : Color{ 96, 60, 130, 120 };
        DrawSlantedQuad(x + 1.0f, y, w - 2.0f, 4.0f, slant, accentColor);
        DrawSlantedQuad(x, y + h - 2.0f, w, 2.0f, slant,
                        selected ? Color{ 212, 175, 55, 255 } : Color{ 34, 22, 46, 255 });

        const char* numTag = (i == 0) ? "01" : "02";
        DrawText(numTag, (int)(x + 14.0f), (int)(y + 8.0f), 10,
                 selected ? Color{ 212, 175, 55, 255 } : Color{ 60, 44, 80, 255 });

        float spriteSize = selected ? 38.0f : 28.0f;
        float spriteY = y + (selected ? 22.0f : 24.0f);
        float spriteX = x + (w - spriteSize) / 2.0f - 2.0f;

        if (selected) {
            DrawTexturePro(
                state->assets.players[i],
                { 0, 0, (float)PLAYER_RECT_SIZE, (float)PLAYER_RECT_SIZE },
                { spriteX + 2.0f, spriteY, spriteSize, spriteSize },
                { 0, 0 }, 0.0f, Color{ 150, 80, 200, 160 }
            );
        }

        DrawTexturePro(
            state->assets.players[i],
            { 0, 0, (float)PLAYER_RECT_SIZE, (float)PLAYER_RECT_SIZE },
            { spriteX, spriteY, spriteSize, spriteSize },
            { 0, 0 }, 0.0f, selected ? WHITE : Color{ 130, 110, 150, 190 }
        );

        const char* name = (i == 0) ? "JAMIE" : "CAROL";
        Color nameColor = selected ? Color{ 200, 240, 180, 255 } : Color{ 85, 115, 95, 255 };

        if (selected) {
            DrawSlantedQuad(x + 8.0f, y + h - 26.0f, w - 16.0f, 16.0f, -6.0f, Color{ 150, 80, 200, 255 });
            int nw = MeasureText(name, 10);
            DrawText(name, (int)(x + (w - nw) / 2.0f), (int)(y + h - 23.0f), 10, nameColor);
        } else {
            int nw = MeasureText(name, 9);
            DrawText(name, (int)(x + (w - nw) / 2.0f), (int)(y + h - 20.0f), 9, nameColor);
        }
    }

    float descY = 162.0f;
    float descH = 50.0f;

    DrawSlantedQuad(18.0f - jitterX, descY + 4.0f, 210.0f, descH, -13.0f, Color{ 150, 80, 200, 80 });
    DrawSlantedQuad(14.0f, descY, 212.0f, descH, -11.0f, Color{ 20, 12, 30, 255 });
    DrawSlantedQuad(12.0f, descY, 5.0f, descH, -11.0f, Color{ 212, 175, 55, 255 });
    DrawSlantedQuad(17.0f, descY, 2.0f, descH, -11.0f, Color{ 150, 80, 200, 255 });

    if (state->selectedCharacter == 0) {
        DrawText("2nd Year Uni Student", 26 + (int)jitterX, (int)descY + 6, 8, Color{ 240, 210, 130, 255 });
        DrawText("Ability", 26, (int)descY + 18, 8, Color{ 212, 175, 55, 255 });
        DrawText(": Radar ping detects clues", 66, (int)descY + 18, 8, Color{ 190, 170, 200, 255 });
        DrawText("Trait", 26, (int)descY + 30, 8, Color{ 150, 80, 200, 255 });
        DrawText(": Lower field of vision", 66, (int)descY + 30, 8, Color{ 150, 130, 170, 255 });
    } else {
        DrawText("2nd Year Uni Student", 26 + (int)jitterX, (int)descY + 6, 8, Color{ 240, 210, 130, 255 });
        DrawText("Ability", 26, (int)descY + 18, 8, Color{ 212, 175, 55, 255 });
        DrawText(": Able to sprint for period", 66, (int)descY + 18, 8, Color{ 190, 170, 200, 255 });
        DrawText("Trait", 26, (int)descY + 30, 8, Color{ 150, 80, 200, 255 });
        DrawText(": Lower regular speed", 66, (int)descY + 30, 8, Color{ 150, 130, 170, 255 });
    }

    DrawText("Left/Right", 12, 222, 7, Color{ 212, 175, 55, 200 });
    DrawText("[ENTER] Start", 140, 222, 7, Color{ 220, 190, 120, 255 });
}

void dialogueScreen(GameState* state) {
 const char* lines[][2] = {
    { "GBP 1,000 to sit on a research",   "flight. Rent's due. I said yes." },
    { "Three other students were on board.", "I woke alone in the wreckage." },
    { "No broken bones, just a dark",     "island under a full moon..." },
    { "Something is watching us.",        "I need to find the others and escape." },
};
const int totalLines = 4;

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        state->dialogueIndex++;
        if (state->dialogueIndex >= totalLines) {
            state->currentScreen = SCREEN_GAME;
            return;
        }
    }

    ClearBackground(BLACK);

    Rectangle box = { 12.0f, 164.0f, SCREEN_WIDTH - 24.0f, 64.0f };
    DrawRectangleRec(box, Color{ 10, 14, 11, 255 });
    DrawRectangleLinesEx(box, 1.0f, Color{ 0x23, 0x7b, 0x52, 255 });

    const char* speaker = (state->selectedCharacter == 0) ? "JAMIE" : "CAROL";
    DrawText(speaker, (int)box.x + 8, (int)box.y + 6, 8, Color{ 0x33, 0x65, 0x02, 255 });

    DrawText(lines[state->dialogueIndex][0], (int)box.x + 8, (int)box.y + 20, 8, RAYWHITE);
    DrawText(lines[state->dialogueIndex][1], (int)box.x + 8, (int)box.y + 32, 8, RAYWHITE);

    if (static_cast<int>(GetTime() * 2.0) % 2 == 0) {
        DrawText("[ENTER]", (int)(box.x + box.width - 42), (int)(box.y + box.height - 12), 7, Color{ 0x23, 0x7b, 0x52, 255 });
    }
}

// The playable area (the whole tile map) in world coordinates.
Rectangle getWorldBounds(const GameState* state) {
    return {
        state->mapOrigin.x,
        state->mapOrigin.y,
        (float)(state->islandMap.GetWidth() * state->islandMap.GetWorldTileSize()),
        (float)(state->islandMap.GetHeight() * state->islandMap.GetWorldTileSize())
    };
}

// Keeps the player inside the map and stops them dead against the edge.
void clampPlayerToWorld(GameState* state) {
    const Rectangle bounds = getWorldBounds(state);
    const float half = PLAYER_RECT_SIZE * 0.5f;
    Player& p = state->player;

    if (p.x < bounds.x + half)                { p.x = bounds.x + half;                p.velx = 0.0f; }
    if (p.x > bounds.x + bounds.width - half) { p.x = bounds.x + bounds.width - half; p.velx = 0.0f; }
    if (p.y < bounds.y + half)                { p.y = bounds.y + half;                p.vely = 0.0f; }
    if (p.y > bounds.y + bounds.height - half){ p.y = bounds.y + bounds.height - half;p.vely = 0.0f; }
}

// Placeholder art: each terrain character in island_layout.cpp is drawn as a
// flat colour. A character's tile ID is its position in this list, so when
// the real spritesheet arrives, this becomes the table of symbol -> tile ID.
struct TerrainColour {
    char symbol;
    Color colour;
};

static const TerrainColour TERRAIN[] = {
    {'~', {8, 14, 40, 255}},     // deep water
    {',', {26, 56, 104, 255}},   // shallow water
    {'s', {196, 170, 120, 255}}, // sand
    {'c', {96, 72, 116, 255}},   // cliff
    {'g', {62, 104, 56, 255}},   // grass
    {'f', {30, 64, 44, 255}},    // dark grass
    {'T', {14, 44, 30, 255}},    // tree
    {'p', {156, 120, 78, 255}},  // dirt path
    {'k', {120, 120, 130, 255}}, // rock
    {'M', {92, 70, 126, 255}},   // mountain
    {'o', {112, 112, 128, 255}}, // stone floor
    {'q', {84, 104, 82, 255}},   // mossy cobble
    {'W', {54, 54, 76, 255}},    // stone wall
    {'S', {176, 166, 150, 255}}, // stairs
    {'E', {255, 190, 80, 255}},  // door / cave mouth
    {'l', {255, 214, 60, 255}},  // torch / candle
    {'r', {176, 96, 255, 255}},  // rune / portal
    {'i', {160, 160, 180, 255}}, // pillar
    {'b', {124, 88, 52, 255}},   // wooden dock
    {'B', {170, 116, 72, 255}},  // boat
    {'x', {214, 214, 220, 255}}, // plane wreck
    {'d', {132, 112, 92, 255}},  // debris
};

static int terrainTileId(char symbol) {
    for (size_t i = 0; i < sizeof(TERRAIN) / sizeof(TERRAIN[0]); ++i) {
        if (TERRAIN[i].symbol == symbol) {
            return static_cast<int>(i);
        }
    }
    return -1; // unknown character: leave the cell empty
}

// Fills the map from ISLAND_LAYOUT and gives it the placeholder tileset.
void loadIslandMap(TileMap& map) {
    std::vector<Color> colours;
    for (const TerrainColour& terrain : TERRAIN) {
        colours.push_back(terrain.colour);
    }

    map.LoadPlaceholderTileset(colours);
    map.SetTint({175, 180, 185, 255});

    for (int y = 0; y < ISLAND_HEIGHT; ++y) {
        for (int x = 0; x < ISLAND_WIDTH; ++x) {
            map.SetTile(x, y, terrainTileId(ISLAND_LAYOUT[y][x]));
        }
    }
}

// ---------------------------------------------------------------------------
// Island 1 art + collision
// ---------------------------------------------------------------------------
// The real tiled island (island_layout.cpp + assets/island_tiles.png).
// Define HAVE_ISLAND_ATLAS (top of this file, or add_compile_definitions in CMake)
// once the atlas version of island_layout.cpp, island_atlas*.h and
// assets/island_tiles.png are all in the project.
#ifdef HAVE_ISLAND_ATLAS
bool loadIslandTiles(TileMap& map);
#endif

// Layout characters you cannot walk through on island 1.
// Collision reads ISLAND_LAYOUT directly, so it keeps working whichever
// tileset (placeholder colours or the atlas) the map is drawn with.
static bool isSolidIsland1Char(char c) {
    switch (c) {
        case '~': case ',':   // deep / shallow water
        case 'c':             // cliffs
        case 'M':             // mountain rock
        case 'W':             // stone walls
        case 'k':             // rocks
        case 'i':             // pillars
        case 'B':             // boat
        case 'x':             // plane wreck
            return true;
        default:
            return false;
    }
}

// True if a small box centred on world (cx, cy) overlaps a solid island-1 tile
// or the temporary occlusion-test wall.
static bool island1Blocked(const GameState* state, float cx, float cy) {
    // Same small box as island 2 so narrow paths remain passable.
    const float half = 5.0f;
    for (int sy = -1; sy <= 1; sy += 2) {
        for (int sx = -1; sx <= 1; sx += 2) {
            const int tx = (int)std::floor((cx + sx * half - state->mapOrigin.x) / TILE_SIZE);
            const int ty = (int)std::floor((cy + sy * half - state->mapOrigin.y) / TILE_SIZE);
            if (tx < 0 || ty < 0 || tx >= ISLAND_WIDTH || ty >= ISLAND_HEIGHT) return true;
            if (isSolidIsland1Char(ISLAND_LAYOUT[ty][tx])) return true;
        }
    }

    const Rectangle playerBounds = {
        cx - half, cy - half, half * 2.0f, half * 2.0f
    };
    const Rectangle testWall = {140.0f, 58.0f, 4.0f, 108.0f};
    return CheckCollisionRecs(playerBounds, testWall);
}

// ---------------------------------------------------------------------------
// Island 2 (reached through the portal)
// ---------------------------------------------------------------------------
// The layout comes from IslandLayout.cpp. Its map origin is (0, 0), so tile
// (tx, ty) has its centre at world (tx * 16 + 8, ty * 16 + 8).
static const int ISLAND2_W = 120;
static const int ISLAND2_H = 90;
static const int LOOPS_PER_ISLAND = 2;
static const float ISLAND2_LOOP_TIME = 60.0f;   // seconds per loop on island 2

static const float CHRONOS_CHASE_SPEED = 28.0f;            // px/s
static const float CHRONOS_CHASE_SPEED_DANGEROUS = 72.0f;  // last 15 s of a loop

// Where things are on island 2 (tile coordinates, checked to be reachable).
static const int I2_SPAWN_X = 22,  I2_SPAWN_Y = 25;   // crash site
static const int I2_ITEM_X  = 29,  I2_ITEM_Y  = 63;   // luminous chamber
static const int I2_EXIT_X  = 96,  I2_EXIT_Y  = 71;   // Chronos arena
static const int I2_CHRONOS_X = 39, I2_CHRONOS_Y = 18;
static const int I2_SHADOW_X  = 58, I2_SHADOW_Y  = 40;
static const int I1_SHADOW_X = 80, I1_SHADOW_Y = 39;
static const int I1_PORTAL_KEY_X = 54, I1_PORTAL_KEY_Y = 66;
static const int I1_PORTAL_X = 125, I1_PORTAL_Y = 65;

static Vector2 islandTileCentre(int tx, int ty) {
    return { tx * (float)TILE_SIZE + TILE_SIZE / 2.0f,
             ty * (float)TILE_SIZE + TILE_SIZE / 2.0f };
}

static Vector2 island1TileCentre(const GameState* state, int tx, int ty) {
    const float tileSize = (float)state->islandMap.GetWorldTileSize();
    return {
        state->mapOrigin.x + tx * tileSize + tileSize / 2.0f,
        state->mapOrigin.y + ty * tileSize + tileSize / 2.0f
    };
}

// Colour for each TileID in TilePalette.h (placeholder art, same idea as TERRAIN).
static const Color ISLAND2_COLOURS[] = {
    {62, 104, 56, 255},    // GRASS
    {30, 64, 44, 255},     // DARK_GRASS
    {90, 124, 70, 255},    // FLOWER_GRASS
    {156, 120, 78, 255},   // DIRT
    {132, 130, 136, 255},  // STONE_PATH
    {196, 170, 120, 255},  // SAND
    {26, 56, 104, 255},    // WATER
    {96, 72, 116, 255},    // CLIFF
    {112, 112, 128, 255},  // STONE_FLOOR
    {54, 54, 76, 255},     // STONE_WALL
    {92, 70, 126, 255},    // MOUNTAIN
    {14, 44, 30, 255},     // TREE
    {18, 52, 40, 255},     // PINE_TREE
    {120, 120, 130, 255},  // ROCK
    {84, 84, 108, 255},    // RUIN
    {176, 96, 255, 255},   // RUNE
    {255, 214, 60, 255},   // TORCH
    {124, 88, 52, 255},    // BRIDGE
    {214, 214, 220, 255},  // WRECKAGE
    {190, 110, 255, 255},  // PURPLE_MAGIC
};

static bool isSolidTileId(int id) {
    switch (id) {
        case -1:                // outside the map
        case TileID::WATER:
        case TileID::CLIFF:
        case TileID::STONE_WALL:
        case TileID::MOUNTAIN:
        case TileID::ROCK:
        case TileID::RUIN:
        case TileID::WRECKAGE:
            return true;
        default:
            return false;
    }
}

// True if a small box centred on (cx, cy) overlaps a solid island-2 tile.
// The box is smaller than a tile (10 px) so 1-tile gaps in the maze are passable.
static bool island2Blocked(float cx, float cy) {
    const float half = 5.0f;
    for (int sy = -1; sy <= 1; sy += 2) {
        for (int sx = -1; sx <= 1; sx += 2) {
            const int tx = (int)std::floor((cx + sx * half) / TILE_SIZE);
            const int ty = (int)std::floor((cy + sy * half) / TILE_SIZE);
            if (isSolidTileId(Island2TileAt(tx, ty))) return true;
        }
    }
    return false;
}

static bool playerBlocked(const GameState* state, float cx, float cy) {
    return state->currentIsland == 1
        ? island1Blocked(state, cx, cy)
        : island2Blocked(cx, cy);
}

static void moveChronosAlongMap(GameState* state, Vector2 target, float speed, float deltaTime) {
    const float dx = target.x - state->chronos.position.x;
    const float dy = target.y - state->chronos.position.y;
    const float distance = std::sqrt(dx * dx + dy * dy);
    if (distance <= 0.001f) return;

    const float step = std::min(distance, speed * deltaTime);
    state->chronos.position.x += dx / distance * step;
    state->chronos.position.y += dy / distance * step;
}

static void drawEdgeIndicator(
    Vector2 target, Vector2 from, const char* label, Color colour, float edgeInset = 12.0f
) {
    const float dx = target.x - from.x;
    const float dy = target.y - from.y;
    const float length = std::sqrt(dx * dx + dy * dy);
    if (length <= 0.001f) return;

    const Vector2 direction = {dx / length, dy / length};
    const float edgeX = SCREEN_WIDTH / 2.0f - edgeInset;
    const float edgeY = SCREEN_HEIGHT / 2.0f - edgeInset;
    const float scaleX = std::abs(direction.x) > 0.001f
        ? edgeX / std::abs(direction.x)
        : 100000.0f;
    const float scaleY = std::abs(direction.y) > 0.001f
        ? edgeY / std::abs(direction.y)
        : 100000.0f;
    const float edgeDistance = std::min(scaleX, scaleY);
    const Vector2 tip = {
        SCREEN_WIDTH / 2.0f + direction.x * edgeDistance,
        SCREEN_HEIGHT / 2.0f + direction.y * edgeDistance
    };
    const Vector2 perpendicular = {-direction.y, direction.x};
    const Vector2 base = {tip.x - direction.x * 8.0f, tip.y - direction.y * 8.0f};
    DrawTriangle(
        {base.x + perpendicular.x * 4.0f, base.y + perpendicular.y * 4.0f},
        tip,
        {base.x - perpendicular.x * 4.0f, base.y - perpendicular.y * 4.0f},
        colour
    );

    const int labelWidth = MeasureText(label, 8);
    const int labelX = std::max(3, std::min(
        SCREEN_WIDTH - labelWidth - 3,
        (int)(tip.x - direction.x * 16.0f - labelWidth / 2.0f)
    ));
    const int labelY = std::max(16, std::min(
        SCREEN_HEIGHT - 16,
        (int)(tip.y - direction.y * 16.0f - 4.0f)
    ));
    DrawText(label, labelX, labelY, 8, colour);
}

static void drawKeyMarker(Vector2 position) {
    const Vector2 ring = {position.x - 4.0f, position.y};
    const Vector2 shaftStart = {position.x - 1.0f, position.y};
    const Vector2 shaftEnd = {position.x + 10.0f, position.y};
    DrawCircleV(ring, 5.0f, Color{65, 42, 12, 255});
    DrawCircleV(ring, 3.5f, GOLD);
    DrawCircleV(ring, 1.5f, Color{35, 24, 12, 255});
    DrawLineEx(shaftStart, shaftEnd, 4.0f, Color{65, 42, 12, 255});
    DrawLineEx(shaftStart, shaftEnd, 2.5f, GOLD);
    DrawRectangle((int)position.x + 4, (int)position.y, 3, 5, GOLD);
    DrawRectangle((int)position.x + 8, (int)position.y, 3, 4, GOLD);
}

static void drawPortalMarker(Vector2 position, bool unlocked) {
    const Rectangle frame = {position.x - 9.0f, position.y - 11.0f, 18.0f, 22.0f};
    const Color frameColour = unlocked ? Color{190, 110, 255, 255} : Color{105, 75, 125, 255};
    const Color portalColour = unlocked ? Color{190, 110, 255, 190} : Color{85, 60, 100, 190};
    DrawRectangleRec(frame, Color{30, 16, 42, 255});
    DrawRectangleRec({position.x - 6.0f, position.y - 8.0f, 12.0f, 17.0f}, Color{8, 6, 18, 255});
    DrawEllipse((int)position.x, (int)position.y - 2, 5, 7, portalColour);
    DrawLineEx({position.x, position.y - 7.0f}, {position.x, position.y + 7.0f}, 1.5f,
               Color{225, 190, 255, 220});
    DrawRectangleLinesEx(frame, 2.0f, frameColour);
    DrawCircleV({position.x + 5.0f, position.y + 4.0f}, 1.5f, GOLD);
}

static void drawExitMarker(Vector2 position, bool unlocked) {
    const Rectangle frame = {position.x - 9.0f, position.y - 11.0f, 18.0f, 22.0f};
    const Color frameColour = unlocked ? GREEN : Color{95, 100, 95, 255};
    DrawRectangleRec(frame, Color{20, 32, 25, 255});
    DrawRectangleRec({position.x - 6.0f, position.y - 8.0f, 12.0f, 17.0f}, Color{8, 14, 10, 255});
    DrawRectangleLinesEx(frame, 2.0f, frameColour);
    DrawLineEx({position.x - 3.0f, position.y + 5.0f}, {position.x + 3.0f, position.y + 5.0f},
               1.5f, frameColour);
    DrawCircleV({position.x + 4.0f, position.y + 1.0f}, 1.5f, GOLD);
}

static void buildIsland2Map(TileMap& map) {
    std::vector<Color> colours(ISLAND2_COLOURS,
                               ISLAND2_COLOURS + sizeof(ISLAND2_COLOURS) / sizeof(ISLAND2_COLOURS[0]));
    map.LoadPlaceholderTileset(colours);
    map.SetTint({175, 180, 185, 255});
    BuildIsland2Map(map);
}

static const char* const ISLAND_1_LOOP_GOALS[] = {
    "Rescue Samarth", "Find the portal key and reach the portal"
};
static const char* const ISLAND_2_LOOP_GOALS[] = {
    "Solve the shadow's riddle", "Find the exit key and escape"
};

// Puts the player, Chronos, the shadows and the item back at their starting
// spots for the current island. Used on restart and at the start of each loop.
static void respawnForLoop(GameState* state) {
    state->player.velx = 0.0f;
    state->player.vely = 0.0f;
    if (state->currentIsland == 1) {
        state->portalKey.collected = false;
        state->portalExit.unlocked = false;
        state->player.x = SCREEN_WIDTH / 2.0f;
        state->player.y = SCREEN_HEIGHT / 2.0f;
        state->chronosPatrolWaypointIndex = 0;
        state->chronos.position = state->currentLoop == 1
            ? island1TileCentre(state, 35, 42)
            : state->chronos.waypointA;
        state->chronos.movingToB = true;
        if (!state->shadowDefeated) {
            state->patroller.position = island1TileCentre(state, I1_SHADOW_X, I1_SHADOW_Y);
        }
    } else {
        state->exitKey.collected = false;
        state->finalExit.unlocked = false;
        const Vector2 spawn = islandTileCentre(I2_SPAWN_X, I2_SPAWN_Y);
        state->player.x = spawn.x;
        state->player.y = spawn.y;
        state->chronos.position = islandTileCentre(I2_CHRONOS_X, I2_CHRONOS_Y);
        if (!state->yellowShadowDefeated) {
            state->yellowShadow.position = islandTileCentre(I2_SHADOW_X, I2_SHADOW_Y);
        }
    }
}

// ---------------------------------------------------------------------------
// Minimap: one pixel per tile, built from the SAME layout the main map uses.
// [M] toggles between the corner minimap and the full-screen map.
// ---------------------------------------------------------------------------
static Texture2D gMiniTex = {};
static bool gBigMap = false;

static Color miniTileColour(int island, int x, int y) {
    if (island == 1) {
        const int id = terrainTileId(ISLAND_LAYOUT[y][x]);
        return id < 0 ? BLACK : TERRAIN[id].colour;
    }
    const int id = (island == 2) ? Island2TileAt(x, y) : Island3TileAt(x, y);
    const int n = (int)(sizeof(ISLAND2_COLOURS) / sizeof(ISLAND2_COLOURS[0]));
    if (id < 0 || id >= n) return BLACK;
    return ISLAND2_COLOURS[id];
}

// Call after the island's map has been built.
static void buildMinimap(const GameState* state) {
    if (gMiniTex.id != 0) UnloadTexture(gMiniTex);
    const int w = state->islandMap.GetWidth();
    const int h = state->islandMap.GetHeight();
    Image img = GenImageColor(w, h, BLACK);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            ImageDrawPixel(&img, x, y, miniTileColour(state->currentIsland, x, y));
    gMiniTex = LoadTextureFromImage(img);
    UnloadImage(img);
    SetTextureFilter(gMiniTex, TEXTURE_FILTER_POINT);
}

static void drawMinimap(const GameState* state) {
    if (gMiniTex.id == 0) return;
    const float mw = (float)gMiniTex.width, mh = (float)gMiniTex.height;

    float scale, ox, oy;
    if (gBigMap) {                                  // full-screen map
        scale = std::min((SCREEN_WIDTH - 20.0f) / mw, (SCREEN_HEIGHT - 20.0f) / mh);
        ox = (SCREEN_WIDTH - mw * scale) / 2.0f;
        oy = (SCREEN_HEIGHT - mh * scale) / 2.0f;
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{ 0, 0, 0, 200 });
    } else {                                        // corner minimap
        scale = 56.0f / mw;
        ox = SCREEN_WIDTH - mw * scale - 6.0f;
        oy = 6.0f;
    }

    DrawRectangle((int)ox - 2, (int)oy - 2, (int)(mw * scale) + 4, (int)(mh * scale) + 4, BLACK);
    DrawTexturePro(gMiniTex, { 0, 0, mw, mh }, { ox, oy, mw * scale, mh * scale },
                   { 0, 0 }, 0.0f, WHITE);
    DrawRectangleLines((int)ox - 2, (int)oy - 2, (int)(mw * scale) + 4, (int)(mh * scale) + 4,
                       Color{ 0x23, 0x7b, 0x52, 255 });

    auto toMap = [&](Vector2 world) {
        return Vector2{ ox + (world.x - state->mapOrigin.x) / TILE_SIZE * scale,
                        oy + (world.y - state->mapOrigin.y) / TILE_SIZE * scale };
    };

    const bool island1 = state->currentIsland == 1;
    const Collectible& key = island1 ? state->portalKey : state->exitKey;
    const Exit& exitObj = island1 ? state->portalExit : state->finalExit;
    const bool shadowGone = island1 ? state->shadowDefeated : state->yellowShadowDefeated;
    const Vector2 shadowPos = island1 ? state->patroller.position : state->yellowShadow.position;
    const float dot = gBigMap ? 2.0f : 1.0f;

    if (!shadowGone)
        DrawCircleV(toMap(shadowPos), dot, ORANGE);
    if (!key.collected)
        DrawCircleV(toMap(key.position), dot, GOLD);
    DrawCircleV(toMap(exitObj.position), dot, exitObj.unlocked ? GREEN : GRAY);
    DrawCircleV(toMap(state->chronos.position), dot, RED);

    if (static_cast<int>(GetTime() * 4.0) % 2 == 0)  // player blinks
        DrawCircleV(toMap({ state->player.x, state->player.y }), gBigMap ? 2.5f : 1.5f, WHITE);

    if (!gBigMap) DrawText("[M] map", (int)ox, (int)(oy + mh * scale + 4), 6, Color{ 0x23, 0x7b, 0x52, 255 });
    else          DrawText("[M] close", 8, SCREEN_HEIGHT - 12, 7, Color{ 0x23, 0x7b, 0x52, 255 });
}

// Swaps the world over to island 2 and starts a fresh set of loops there.
static void enterIsland2(GameState* state) {
    TraceLog(LOG_WARNING, "I2 item tile=%d solid=%d | exit tile=%d solid=%d",
    Island2TileAt(I2_ITEM_X, I2_ITEM_Y), isSolidTileId(Island2TileAt(I2_ITEM_X, I2_ITEM_Y)),
    Island2TileAt(I2_EXIT_X, I2_EXIT_Y), isSolidTileId(Island2TileAt(I2_EXIT_X, I2_EXIT_Y)));
    state->islandMap.Unload();
    state->islandMap = TileMap(ISLAND2_W, ISLAND2_H, TILE_SIZE, TILE_SIZE);
    buildIsland2Map(state->islandMap);

    state->currentIsland = 2;
    state->mapOrigin = {0.0f, 0.0f};
    state->lightingMap = LightingMap(ISLAND2_W * TILE_SIZE, ISLAND2_H * TILE_SIZE);

    state->loopDuration = ISLAND2_LOOP_TIME;
    state->currentLoop = 1;
    state->timeRemaining = state->loopDuration;
    state->gameOver = false;
    state->victory = false;
    state->gameOverReason = GAME_OVER_NONE;
    state->chronosDangerous = false;
    state->chronosHitCooldown = 0.0f;
    state->playerHitFlash = 0.0f;
    state->playerShakeTimer = 0.0f;

    state->exitKey.position = islandTileCentre(I2_ITEM_X, I2_ITEM_Y);
    state->finalExit.position = islandTileCentre(I2_EXIT_X, I2_EXIT_Y);

    state->yellowShadowDefeated = false;

    buildMinimap(state);
    respawnForLoop(state);
}

void portalScreen(GameState* state, float deltaTime) {
    const float duration = 3.0f;
    state->portalTimer += deltaTime;
    const float t = state->portalTimer;
    const float grow = std::min(1.0f, t / duration);

    ClearBackground(Color{ 6, 2, 14, 255 });
    const Vector2 c = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };

    // Rings rushing inwards
    for (int i = 0; i < 9; ++i) {
        float r = 112.0f - std::fmod(i * 13.0f + t * 45.0f, 112.0f);
        unsigned char a = (unsigned char)(40 + 150 * (1.0f - r / 112.0f));
        DrawRing(c, r, r + 2.0f, 0.0f, 360.0f, 48, Color{ 176, 96, 255, a });
    }

    // Spinning arms
    for (int k = 0; k < 6; ++k) {
        float ang = t * 3.5f + k * (2.0f * PI / 6.0f);
        float len = 30.0f + 70.0f * grow;
        Vector2 tip = { c.x + std::cos(ang) * len, c.y + std::sin(ang) * len };
        DrawLineEx(c, tip, 2.0f, Color{ 230, 140, 255, 200 });
    }

    // Bright core swallowing the screen
    const float core = 6.0f + grow * grow * 150.0f;
    DrawCircleV(c, core, Color{ 235, 215, 255, (unsigned char)(60 + 195 * grow * grow) });

    if (t < duration * 0.7f) {
        const char* msg = "The portal pulls you in...";
        int w = MeasureText(msg, 8);
        DrawText(msg, (SCREEN_WIDTH - w) / 2, 214, 8, Color{ 220, 190, 255, 255 });
    }

    if (t >= duration) {
        state->portalTimer = 0.0f;
        enterIsland2(state);
        state->currentScreen = SCREEN_GAME;
    }
}

static Vector2 chronosPatrolPos(float t) {
    static const int WP[][2] = { {39,18}, {54,22}, {65,29}, {79,35}, {91,39} };
    const int n = 5;
    const float speed = CHRONOS_CHASE_SPEED;
    float segmentLengths[4];
    float total = 0.0f;
    for (int i = 0; i < n - 1; ++i) {
        const Vector2 a = islandTileCentre(WP[i][0], WP[i][1]);
        const Vector2 b = islandTileCentre(WP[i + 1][0], WP[i + 1][1]);
        segmentLengths[i] = std::hypot(b.x - a.x, b.y - a.y);
        total += segmentLengths[i];
    }
    float d = std::fmod(t * speed, total * 2.0f);
    if (d > total) d = total * 2.0f - d;
    for (int i = 0; i < n - 1; ++i) {
        if (d <= segmentLengths[i]) {
            Vector2 a = islandTileCentre(WP[i][0], WP[i][1]);
            Vector2 b = islandTileCentre(WP[i + 1][0], WP[i + 1][1]);
            float k = d / segmentLengths[i];
            return { a.x + (b.x - a.x) * k, a.y + (b.y - a.y) * k };
        }
        d -= segmentLengths[i];
    }
    return islandTileCentre(WP[n - 1][0], WP[n - 1][1]);
}

static bool clueIntroSeen = false;
static int  cluePage = -1;   // -1 = not showing

void gameScreen(GameState* state, Vector2 mousePos, float deltaTime) {
    if (IsKeyPressed(KEY_M)) gBigMap = !gBigMap;
    if (IsKeyPressed(KEY_R) && !state->gameOver) { startBoss(state); return; }
    if (cluePage >= 0) {
        deltaTime = 0.0f;   // freezes timer, movement and enemies
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (++cluePage >= 3) cluePage = -1;
        }
    }

    if (state->gameOver && IsKeyPressed(KEY_R)) {
        state->currentLoop = 1;
        state->timeRemaining = state->loopDuration;
        state->gameOver = false;
        state->victory = false;
        state->gameOverReason = GAME_OVER_NONE;
        state->chronosDangerous = false;
        state->chronosHitCooldown = 0.0f;
        state->playerHitFlash = 0.0f;
        state->playerShakeTimer = 0.0f;
        respawnForLoop(state);
    }

    if (!state->gameOver && !state->victory && state->timeRemaining > 0.0f) {
        state->timeRemaining -= deltaTime;
        if (state->timeRemaining < 0.0f) {
            state->timeRemaining = 0.0f;
        }

        if (state->timeRemaining <= 0.0f) {
            if (state->currentLoop < LOOPS_PER_ISLAND) {
                state->currentLoop++;
                state->timeRemaining = state->loopDuration;
                respawnForLoop(state);
                state->chronosHitCooldown = 0.0f;
                state->playerHitFlash = 0.0f;
                state->playerShakeTimer = 0.0f;
            } else {
                state->gameOver = true;
                state->gameOverReason = GAME_OVER_TIME;
            }
        }
    }

    state->chronosDangerous =
        !state->gameOver &&
        !state->victory &&
        state->timeRemaining > 0.0f &&
        state->timeRemaining <= 15.0f;

    Collectible& activeKey = state->currentIsland == 1
        ? state->portalKey
        : state->exitKey;
    Exit& activeExit = state->currentIsland == 1
        ? state->portalExit
        : state->finalExit;

    if (!state->victory) {
        if (!state->gameOver) {
            Vector2 target;
            float speed;
            if (state->chronosDangerous) {
                target = {state->player.x, state->player.y};
                speed = CHRONOS_CHASE_SPEED_DANGEROUS;
            } else if (state->currentIsland == 1) {
                if (state->currentLoop == 1) {
                    static const int FIRST_LOOP_ROUTE[][2] = {
                        {35, 42}, {39, 42}, {39, 50}, {35, 50}
                    };
                    const int routeCount = sizeof(FIRST_LOOP_ROUTE) / sizeof(FIRST_LOOP_ROUTE[0]);
                    state->chronosPatrolWaypointIndex %= routeCount;
                    target = island1TileCentre(
                        state,
                        FIRST_LOOP_ROUTE[state->chronosPatrolWaypointIndex][0],
                        FIRST_LOOP_ROUTE[state->chronosPatrolWaypointIndex][1]
                    );
                    const float dx = target.x - state->chronos.position.x;
                    const float dy = target.y - state->chronos.position.y;
                    if (dx * dx + dy * dy <= 4.0f) {
                        state->chronosPatrolWaypointIndex =
                            (state->chronosPatrolWaypointIndex + 1) % routeCount;
                        target = island1TileCentre(
                            state,
                            FIRST_LOOP_ROUTE[state->chronosPatrolWaypointIndex][0],
                            FIRST_LOOP_ROUTE[state->chronosPatrolWaypointIndex][1]
                        );
                    }
                    speed = 40.0f;
                } else {
                    const Vector2 waypoint = state->chronos.movingToB
                        ? state->chronos.waypointB
                        : state->chronos.waypointA;
                    const float dx = waypoint.x - state->chronos.position.x;
                    const float dy = waypoint.y - state->chronos.position.y;
                    if (dx * dx + dy * dy <= 4.0f) {
                        state->chronos.movingToB = !state->chronos.movingToB;
                    }
                    target = state->chronos.movingToB
                        ? state->chronos.waypointB
                        : state->chronos.waypointA;
                    speed = state->chronos.normalSpeed;
                }
            } else {
                target = chronosPatrolPos(state->loopDuration - state->timeRemaining);
                speed = CHRONOS_CHASE_SPEED;
            }
            moveChronosAlongMap(state, target, speed, deltaTime);
        }
    }

    // The camera keeps the player at the centre of the screen, and mousePos is
    // in screen space, so the aim angle is measured from the screen centre.
    float playerMouseAngle = std::atan2(
        mousePos.y - SCREEN_HEIGHT / 2.0f,
        mousePos.x - SCREEN_WIDTH / 2.0f
    );

    if (!state->gameOver && !state->victory) {
        state->chronosHitCooldown = std::max(0.0f, state->chronosHitCooldown - deltaTime);
        state->playerHitFlash = std::max(0.0f, state->playerHitFlash - deltaTime);
        state->playerShakeTimer = std::max(0.0f, state->playerShakeTimer - deltaTime);

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

        const float nextX = state->player.x + state->player.velx * deltaTime;
        if (playerBlocked(state, nextX, state->player.y)) {
            state->player.velx = 0.0f;
        } else {
            state->player.x = nextX;
        }

        const float nextY = state->player.y + state->player.vely * deltaTime;
        if (playerBlocked(state, state->player.x, nextY)) {
            state->player.vely = 0.0f;
        } else {
            state->player.y = nextY;
        }

        clampPlayerToWorld(state);
    }

    if (!state->gameOver && !state->victory && state->chronosHitCooldown <= 0.0f) {
        const float dx = state->player.x - state->chronos.position.x;
        const float dy = state->player.y - state->chronos.position.y;
        const float distance = std::sqrt(dx * dx + dy * dy);
        const Texture2D chronosTexture = state->assets.chronosSprite.getCurrentTexture();
        const float chronosRadius =
            std::max(chronosTexture.width, chronosTexture.height) * 0.5f;
        const Rectangle playerBounds = {
            state->player.x - PLAYER_RECT_SIZE * 0.5f,
            state->player.y - PLAYER_RECT_SIZE * 0.5f,
            PLAYER_RECT_SIZE,
            PLAYER_RECT_SIZE
        };

        if (CheckCollisionCircleRec(state->chronos.position, chronosRadius, playerBounds)) {
            state->chronosHitCooldown = 0.5f;

            if (state->chronosDangerous) {
                state->gameOver = true;
                state->gameOverReason = GAME_OVER_CAUGHT;
            } else {
                const float safeDistance = std::max(distance, 0.001f);
                const float pushX = dx / safeDistance;
                const float pushY = dy / safeDistance;
                const float oldX = state->player.x;
                const float oldY = state->player.y;
                const float knockbackX = pushX * 30.0f;
                const float knockbackY = pushY * 30.0f;
                if (state->currentIsland == 1) {
                    const float nextX = oldX + knockbackX;
                    const float nextY = oldY + knockbackY;
                    const bool canMoveX = !island1Blocked(state, nextX, oldY);
                    const bool canMoveY = !island1Blocked(state, oldX, nextY);

                    if (!island1Blocked(state, nextX, nextY)) {
                        state->player.x = nextX;
                        state->player.y = nextY;
                        state->player.velx = pushX * 80.0f;
                        state->player.vely = pushY * 80.0f;
                    } else {
                        state->player.x = canMoveX ? nextX : oldX;
                        state->player.y = canMoveY ? nextY : oldY;
                        state->player.velx = canMoveX ? pushX * 80.0f : 0.0f;
                        state->player.vely = canMoveY ? pushY * 80.0f : 0.0f;
                    }
                } else {
                    state->player.x = oldX + knockbackX;
                    state->player.y = oldY + knockbackY;
                    if (island2Blocked(state->player.x, state->player.y)) {
                        state->player.x = oldX;
                        state->player.y = oldY;
                    }
                    state->player.velx = pushX * 80.0f;
                    state->player.vely = pushY * 80.0f;
                }
                clampPlayerToWorld(state);
                state->playerHitFlash = 0.25f;
                state->playerShakeTimer = 0.35f;
            }
        }
    }

    if (!state->gameOver && !state->victory && IsKeyPressed(KEY_E)) {
        if (state->currentIsland == 1 && !state->shadowDefeated) {
            const float dx = state->player.x - state->patroller.position.x;
            const float dy = state->player.y - state->patroller.position.y;
            if (dx * dx + dy * dy <= 20.0f * 20.0f) {
                state->currentScreen = SCREEN_BATTLE;
                state->battleCursorPos = 0.0f;
                state->battleEnded = false;
                state->battleWon = false;
                state->battleAttemptsRemaining = 3;
                state->battleResultTimer = 0.0f;
            }
        } else if (state->currentIsland == 2 && !state->yellowShadowDefeated) {
            const float dx = state->player.x - state->yellowShadow.position.x;
            const float dy = state->player.y - state->yellowShadow.position.y;
            if (dx * dx + dy * dy <= 20.0f * 20.0f) {
                startRiddle(state);
            }
        }
    }

    if (!state->gameOver && !state->victory &&
        !activeKey.collected && IsKeyPressed(KEY_E)) {
        const float dx = state->player.x - activeKey.position.x;
        const float dy = state->player.y - activeKey.position.y;
        const float distance = std::sqrt(dx * dx + dy * dy);

        if (distance <= 16.0f) {
            const bool blockerCleared = state->currentIsland == 1
                ? state->shadowDefeated
                : state->yellowShadowDefeated;
            if (blockerCleared) {
                activeKey.collected = true;
                if (!clueIntroSeen) {
                    clueIntroSeen = true;
                    cluePage = 0;
                }
            }
        }
    }

    const bool blockerCleared = state->currentIsland == 1
        ? state->shadowDefeated
        : state->yellowShadowDefeated;
    activeExit.unlocked = activeKey.collected && blockerCleared;

    if (activeExit.unlocked && IsKeyPressed(KEY_E)) {
        const float dx = state->player.x - activeExit.position.x;
        const float dy = state->player.y - activeExit.position.y;
        const float distance = std::sqrt(dx * dx + dy * dy);

        if (distance <= 18.0f) {
            if (state->currentIsland == 1) {
                state->portalTimer = 0.0f;
                state->currentScreen = SCREEN_PORTAL;   // step through to island 2
            } else {
                startBoss(state);
                return;
            }
        }
    }

    // Camera: its target is the player and its offset is the screen centre, so
    // the player never moves on screen, the world scrolls past instead.
    // Rounding the target keeps pixel art on whole pixels (no shimmering).
    Camera2D camera = {};
    camera.offset = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    camera.target = { std::round(state->player.x), std::round(state->player.y) };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    // The part of the world currently on screen, used to skip off-screen tiles.
    const Rectangle view = {
        camera.target.x - camera.offset.x,
        camera.target.y - camera.offset.y,
        (float)SCREEN_WIDTH,
        (float)SCREEN_HEIGHT
    };

    ClearBackground(Color{20, 20, 24, 255});

    BeginMode2D(camera);

    state->islandMap.DrawView(view, state->mapOrigin, false);

    // Plane-wreck props (trust.png), drawn on top of the ground, under the actors.
    if (state->currentIsland == 1) {
        drawCrashSite(
            state->mapOrigin.x + ISLAND_SPAWN_X * TILE_SIZE + TILE_SIZE / 2.0f,
            state->mapOrigin.y + ISLAND_SPAWN_Y * TILE_SIZE + TILE_SIZE / 2.0f
        );
    } else if (state->currentIsland == 2) {
        const Vector2 c = islandTileCentre(I2_SPAWN_X, I2_SPAWN_Y);
        drawCrashSiteSmall(c.x, c.y);
    }

    // Temporary occlusion-test wall; replace with the map's obstacle data later.
    if (state->currentIsland == 1) {
        DrawRectangle(140, 58, 4, 108, DARKGRAY);
    }

    if (state->currentIsland == 1 && !state->shadowDefeated) {
        DrawTexturePro(
            state->assets.shadowSprite,
            {0, 0, 32, 32},
            {state->patroller.position.x, state->patroller.position.y, 20, 20},
            {10, 10},
            0.0f,
            Color{145, 55, 65, 255}
        );
    }

    if (state->currentIsland == 2 && !state->yellowShadowDefeated) {
        const bool flicker = static_cast<int>(GetTime() * 6.0) % 2 == 0;
        DrawTexturePro(
            state->assets.shadowSprite,
            {0, 0, 32, 32},
            {state->yellowShadow.position.x, state->yellowShadow.position.y, 20, 20},
            {10, 10},
            0.0f,
            flicker ? Color{255, 220, 90, 255} : Color{210, 170, 40, 255}
        );
    }

    state->assets.chronosSprite.updateFrame(deltaTime);
    DrawTexturePro(
        state->assets.chronosSprite.getCurrentTexture(),
        {
            0, 0,
            (float)state->assets.chronosSprite.getCurrentTexture().width,
            (float)state->assets.chronosSprite.getCurrentTexture().height
        },
        {
            state->chronos.position.x,
            state->chronos.position.y,
            (float)state->assets.chronosSprite.getCurrentTexture().width,
            (float)state->assets.chronosSprite.getCurrentTexture().height
        },
        {
            (float)state->assets.chronosSprite.getCurrentTexture().width / 2,
            (float)state->assets.chronosSprite.getCurrentTexture().height / 2
        },
        0.0f,
        {180, 180, 180, 255}
    );

    const float shakeX = state->playerShakeTimer > 0.0f
        ? std::sin(GetTime() * 70.0f) * 3.0f
        : 0.0f;
    const float shakeY = state->playerShakeTimer > 0.0f
        ? std::cos(GetTime() * 85.0f) * 2.0f
        : 0.0f;

    DrawTexturePro(
        state->assets.players[state->selectedCharacter],
        {0, 0, (float)PLAYER_RECT_SIZE, (float)PLAYER_RECT_SIZE},
        {camera.target.x + shakeX, camera.target.y + shakeY,
            (float)PLAYER_RECT_SIZE, (float)PLAYER_RECT_SIZE},
        {PLAYER_RECT_SIZE / 2.0f, PLAYER_RECT_SIZE / 2.0f},
        playerMouseAngle * RAD2DEG,
        state->playerHitFlash > 0.0f ? ORANGE : WHITE
    );

    EndMode2D();

    // LightingMap works in map-local coordinates (0,0 = the map's top-left
    // corner), so subtract mapOrigin from every world position we give it.
    // It draws in screen space, so this call stays outside BeginMode2D.
    const int originX = (int)state->mapOrigin.x;
    const int originY = (int)state->mapOrigin.y;

    const std::vector<LightSource> lightSources{
        LightSource{
            (int)camera.target.x - originX,
            (int)camera.target.y - originY,
            PLAYER_LIGHT_SIZE / 2
        }
    };
    state->lightingMap.calcAndDrawLights(
        lightSources,
        (int)view.x - originX, (int)view.y - originY,   // top-left of the screen, map-local
        SCREEN_WIDTH, SCREEN_HEIGHT
    );

    // Item and exit live in the world, so they go through the camera too.
    BeginMode2D(camera);

    if (!activeKey.collected) {
        drawKeyMarker(activeKey.position);
    }

    if (state->currentIsland == 1) {
        drawPortalMarker(activeExit.position, activeExit.unlocked);
    } else {
        drawExitMarker(activeExit.position, activeExit.unlocked);
    }

    EndMode2D();

    // HUD (screen space)
    const char* keyStatus = state->currentIsland == 1
        ? (activeKey.collected ? "Portal key: 1/1" : "Portal key: 0/1")
        : (activeKey.collected ? "Exit key: 1/1" : "Exit key: 0/1");
    DrawText(keyStatus, 8, 8, 10, RAYWHITE);

    if (!state->gameOver && !state->victory) {
        const float pdx = state->player.x - activeKey.position.x;
        const float pdy = state->player.y - activeKey.position.y;
        const float edx2 = state->player.x - activeExit.position.x;
        const float edy2 = state->player.y - activeExit.position.y;
        const bool searchingForShadow = state->currentIsland == 1
            ? !state->shadowDefeated
            : !state->yellowShadowDefeated;
        const bool lookingForKey = !activeKey.collected;
        const Vector2 shadowPosition = state->currentIsland == 1
            ? state->patroller.position
            : state->yellowShadow.position;
        const Vector2 navigationTarget = searchingForShadow
            ? shadowPosition
            : lookingForKey ? activeKey.position : activeExit.position;
        const float targetHalfSize = searchingForShadow ? 6.0f : lookingForKey ? 4.0f : 6.0f;
        const float targetScreenX = SCREEN_WIDTH / 2.0f + navigationTarget.x - camera.target.x;
        const float targetScreenY = SCREEN_HEIGHT / 2.0f + navigationTarget.y - camera.target.y;
        const bool targetOnScreen =
            targetScreenX + targetHalfSize >= 0.0f &&
            targetScreenX - targetHalfSize <= SCREEN_WIDTH &&
            targetScreenY + targetHalfSize >= 0.0f &&
            targetScreenY - targetHalfSize <= SCREEN_HEIGHT;
        if (!targetOnScreen) {
            drawEdgeIndicator(
                navigationTarget,
                {camera.target.x, camera.target.y},
                searchingForShadow ? "SHADOW"
                    : lookingForKey ? "KEY" : (state->currentIsland == 1 ? "PORTAL" : "EXIT"),
                searchingForShadow ? ORANGE : lookingForKey ? GOLD : GREEN
            );
        }

        const Texture2D chronosTexture = state->assets.chronosSprite.getCurrentTexture();
        const float chronosScreenX = SCREEN_WIDTH / 2.0f + state->chronos.position.x - camera.target.x;
        const float chronosScreenY = SCREEN_HEIGHT / 2.0f + state->chronos.position.y - camera.target.y;
        const bool chronosOnScreen =
            chronosScreenX + chronosTexture.width * 0.5f >= 0.0f &&
            chronosScreenX - chronosTexture.width * 0.5f <= SCREEN_WIDTH &&
            chronosScreenY + chronosTexture.height * 0.5f >= 0.0f &&
            chronosScreenY - chronosTexture.height * 0.5f <= SCREEN_HEIGHT;
        if (!chronosOnScreen) {
            const bool flashRed = state->chronosDangerous &&
                static_cast<int>(GetTime() * 4.0f) % 2 == 0;
            drawEdgeIndicator(
                state->chronos.position,
                {camera.target.x, camera.target.y},
                "CHRONOS",
                flashRed ? RED : Color{190, 110, 255, 255},
                28.0f
            );
        }

        const float shadowDx = state->player.x - shadowPosition.x;
        const float shadowDy = state->player.y - shadowPosition.y;
        if (searchingForShadow && shadowDx * shadowDx + shadowDy * shadowDy <= 28.0f * 28.0f) {
            const char* prompt = state->currentIsland == 1
                ? "Press E: challenge shadow"
                : "Press E: solve riddle";
            DrawText(prompt, (SCREEN_WIDTH - MeasureText(prompt, 8)) / 2, 190, 8, ORANGE);
        } else if (!activeKey.collected && std::sqrt(pdx * pdx + pdy * pdy) <= 28.0f) {
            if (!blockerCleared) {
                DrawText(
                    state->currentIsland == 1
                        ? "Rescue Samarth first"
                        : "Solve the riddle first",
                    54, 190, 10, GOLD
                );
            } else {
                DrawText(
                    state->currentIsland == 1
                        ? "Press E: take portal key"
                        : "Press E: take exit key",
                    43, 190, 10, GOLD
                );
            }
        } else if (std::sqrt(edx2 * edx2 + edy2 * edy2) <= 30.0f) {
            if (activeExit.unlocked) {
                DrawText(state->currentIsland == 1 ? "Press E to enter portal" : "Press E to escape",
                         state->currentIsland == 1 ? 46 : 68, 190, 10, GREEN);
            } else if (!blockerCleared) {
                DrawText(
                    state->currentIsland == 1
                        ? "Rescue Samarth to open portal"
                        : "Solve the riddle to reach the key",
                    34, 190, 10, LIGHTGRAY
                );
            } else {
                DrawText(
                    state->currentIsland == 1
                        ? "Find the portal key first"
                        : "Find the exit key first",
                    49, 190, 10, LIGHTGRAY
                );
            }
        }
    }

    if (state->victory) {
        DrawRectangle(20, 70, SCREEN_WIDTH - 40, 100, Color{10, 30, 18, 245});
        DrawRectangleLines(20, 70, SCREEN_WIDTH - 40, 100, GOLD);
        DrawText("VICTORY!", 82, 82, 16, GOLD);
        DrawText("CHRONOS IS DEFEATED", 54, 108, 9, RAYWHITE);
        DrawText("You escaped and made it home.", 35, 128, 8, RAYWHITE);
        DrawText("GAME COMPLETE", 82, 151, 8, GREEN);
    } else if (state->gameOver) {
        DrawRectangle(30, 82, SCREEN_WIDTH - 60, 76, Color{35, 10, 12, 235});
        DrawRectangleLines(30, 82, SCREEN_WIDTH - 60, 76, RED);
        if (state->gameOverReason == GAME_OVER_CAUGHT) {
            DrawText("CAUGHT!", 80, 96, 22, RED);
            DrawText("Chronos caught you.", 63, 124, 10, RAYWHITE);
        } else if (state->gameOverReason == GAME_OVER_SHADOW) {
            DrawText("CAUGHT!", 80, 96, 22, RED);
            DrawText("The shadow took you.", 60, 124, 10, RAYWHITE);
        } else {
            DrawText("TIME OVER", 76, 96, 22, RED);
            DrawText("You failed to escape.", 58, 124, 10, RAYWHITE);
        }
        DrawText("Press R to restart", 66, 140, 9, LIGHTGRAY);
    }

    int secondsLeft = static_cast<int>(std::ceil(state->timeRemaining));
    std::string timerText = "Time: " + std::to_string(secondsLeft);

    bool isUrgent = (state->timeRemaining <= 5.0f && state->timeRemaining > 0.0f);

    int fontSize = isUrgent ? 14 : 10;
    Color textColor = isUrgent ? RED : RAYWHITE;

    int textWidth = MeasureText(timerText.c_str(), fontSize);
    int posX = (SCREEN_WIDTH - textWidth) / 2;
    int posY = 6;

    if (isUrgent) {
        float t = (float)GetTime();
        posX += (int)(std::sin(t * 50.0f) * 3.0f);
        posY += (int)(std::cos(t * 45.0f) * 2.0f);

        DrawText(timerText.c_str(), posX - 2, posY, fontSize, SKYBLUE);
        DrawText(timerText.c_str(), posX + 2, posY, fontSize, Color{255, 50, 50, 180});
    }

    DrawText(timerText.c_str(), posX, posY, fontSize, textColor);

    std::string loopText = "Loop: " + std::to_string(state->currentLoop) + "/" +
        std::to_string(LOOPS_PER_ISLAND);
    drawMinimap(state);

    DrawText(loopText.c_str(), 8, 22, 10, RAYWHITE);
    const char* const* loopGoals = state->currentIsland == 1
        ? ISLAND_1_LOOP_GOALS
        : ISLAND_2_LOOP_GOALS;
    DrawText(loopGoals[state->currentLoop - 1], 8, 36, 8, GOLD);
    if (cluePage >= 0) {
        static const char* clueLines[3][2] = {
            { "A key marked with Chronos's symbol.", "The metal is warm to the touch." },
            { "It must unlock the way forward.", "Someone wants us to reach the portal." },
            { "Chronos is leading us somewhere.", "Keep moving before the loop resets." },
        };
        DrawRectangle(12, 164, SCREEN_WIDTH - 24, 64, Color{10, 14, 11, 245});
        DrawRectangleLines(12, 164, SCREEN_WIDTH - 24, 64, Color{0x23, 0x7b, 0x52, 255});
        DrawText(state->selectedCharacter == 0 ? "JAMIE" : "CAROL", 20, 170, 8, Color{0x33, 0x65, 0x02, 255});
        DrawText(clueLines[cluePage][0], 20, 184, 8, RAYWHITE);
        DrawText(clueLines[cluePage][1], 20, 196, 8, RAYWHITE);
        if (static_cast<int>(GetTime() * 2.0) % 2 == 0) {
            DrawText("[ENTER]", SCREEN_WIDTH - 54, 216, 7, Color{0x23, 0x7b, 0x52, 255});
        }
    }
}

void battleScreen(GameState* state, float deltaTime) {
    if (!state->battleEnded) {
        state->battleCursorPos = (std::sin((float)GetTime() * 6.0f * 0.9f) + 1.0f) * 0.5f;

        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            state->battleEnded = true;
            if (state->battleCursorPos >= 0.32f && state->battleCursorPos <= 0.68f) {
                state->battleWon = true;
            } else {
                state->battleWon = false;
            }
        }
    } else {
        state->battleResultTimer += deltaTime;
        if (state->battleResultTimer >= 1.5f) {
            if (state->battleWon) {
                state->currentScreen = SCREEN_SAMARTH_DIALOGUE;
                state->samarthLineIndex = 0;
                state->samarthChoiceActive = false;
                state->samarthResponded = false;
                state->samarthResponseTimer = 0.0f;
            } else if (state->battleAttemptsRemaining > 1) {
                state->battleAttemptsRemaining--;
                state->battleEnded = false;
            } else {
                state->gameOver = true;
                state->gameOverReason = GAME_OVER_SHADOW;
                state->currentScreen = SCREEN_GAME;
            }
            state->battleResultTimer = 0.0f;
        }
    }

    ClearBackground(Color{ 15, 10, 25, 255 });
    DrawTriangle((Vector2){0, 0}, (Vector2){240, 0}, (Vector2){0, 240}, Color{ 180, 15, 15, 25 });

    DrawRectangle(130, 12, 100, 24, BLACK);
    DrawRectangleLines(130, 12, 100, 24, RED);
    DrawText("TIME SHADOW", 136, 18, 8, RAYWHITE);
    const std::string attemptsText = "TRIES LEFT: " + std::to_string(state->battleAttemptsRemaining);
    DrawText(attemptsText.c_str(), 12, 18, 8, GOLD);

    DrawRectangle(180, 38, 48, 48, Color{ 40, 10, 30, 255 });
    DrawRectangleLines(180, 38, 48, 48, RED);
    DrawText("SHADOW", 185, 58, 6, RED);

    DrawRectangle(10, SCREEN_HEIGHT - 65, 90, 22, BLACK);
    DrawRectangleLines(10, SCREEN_HEIGHT - 65, 90, 22, RED);
    const char* charName = (state->selectedCharacter == 0) ? "JAMIE" : "CAROL";
    DrawText(charName, 16, SCREEN_HEIGHT - 58, 8, RAYWHITE);

    if (state->assets.players[state->selectedCharacter].id != 0) {
        DrawTexturePro(
            state->assets.players[state->selectedCharacter],
            {0, 0, (float)PLAYER_RECT_SIZE, (float)PLAYER_RECT_SIZE},
            {26.0f, (float)SCREEN_HEIGHT - 118.0f, 48.0f, 48.0f},
            {0, 0}, 0.0f, WHITE
        );
    }

    int barX = 30;
    int barY = 110;
    int barW = SCREEN_WIDTH - 60;
    int barH = 22;

    DrawRectangle(barX, barY, barW, barH, BLACK);
    DrawRectangleLines(barX, barY, barW, barH, RED);

    int sweetZoneW = (int)(barW * 0.36f);
    int sweetZoneX = barX + (barW - sweetZoneW) / 2;
    DrawRectangle(sweetZoneX, barY, sweetZoneW, barH, Color{ 50, 220, 50, 190 });

    int cursorX = barX + (int)(state->battleCursorPos * barW);
    DrawRectangle(cursorX - 2, barY - 5, 4, barH + 10, YELLOW);

    if (!state->battleEnded) {
        DrawText("Press [SPACE] in green zone!", 42, 85, 8, RAYWHITE);
    } else {
        if (state->battleWon) {
            DrawRectangle(30, 145, SCREEN_WIDTH - 60, 42, Color{ 10, 40, 20, 245 });
            DrawRectangleLines(30, 145, SCREEN_WIDTH - 60, 42, GREEN);
            DrawText("Shadow Defeated!", 70, 161, 8, GREEN);
        } else {
            DrawRectangle(30, 145, SCREEN_WIDTH - 60, 42, Color{ 40, 10, 10, 245 });
            DrawRectangleLines(30, 145, SCREEN_WIDTH - 60, 42, RED);
            DrawText(
                state->battleAttemptsRemaining > 1 ? "Missed - try again!" : "No chances left.",
                state->battleAttemptsRemaining > 1 ? 70 : 72,
                161,
                9,
                RED
            );
        }
    }
}

void samarthDialogueScreen(GameState* state, float deltaTime) {
    const char* lines[] = {
        "Hey there, I was trapped inside that shadow, thanks for freeing me",
        "Chronos has trapped me and several other humans on this island",
        "Chronos has witnessed humans wasting time and resources",
        "Thus he has decided that humans shall spend all of eternity in a loop",
        state->selectedCharacter == 0
            ? "You're our only hope, Jamie. Find the portal key and reach the next island."
            : "You're our only hope, Carol. Find the portal key and reach the next island."
    };
    const int totalLines = 5;

    if (!state->samarthChoiceActive && !state->samarthResponded) {
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            state->samarthLineIndex++;
            if (state->samarthLineIndex >= totalLines) {
                state->samarthChoiceActive = true;
            }
        }
    } else if (state->samarthChoiceActive && !state->samarthResponded) {
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            state->samarthChoiceSelected = 0;
        }
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            state->samarthChoiceSelected = 1;
        }

        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (state->samarthChoiceSelected == 0) {
                state->luckPoints += 1;
                state->acceptedChallenge = true;
            } else {
                state->acceptedChallenge = false;
            }
            state->samarthChoiceActive = false;
            state->samarthResponded = true;
            state->samarthResponseTimer = 0.0f;
        }
    } else {
        state->samarthResponseTimer += deltaTime;
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || state->samarthResponseTimer >= 3.5f) {
            state->currentScreen = SCREEN_GAME;
            state->samarthResponded = false;
            state->samarthLineIndex = 0;
            state->shadowDefeated = true; // Permanently mark shadow as defeated
            state->patroller.position = { -999.0f, -999.0f }; // Send shadow off-screen
            state->samarthResponseTimer = 0.0f;
        }
    }

    ClearBackground(Color{ 10, 10, 15, 255 });

    state->assets.samarthSprite.updateFrame(deltaTime);
    DrawTexturePro(
        state->assets.samarthSprite.getCurrentTexture(),
        {
            0, 0,
            (float)state->assets.samarthSprite.getCurrentTexture().width,
            (float)state->assets.samarthSprite.getCurrentTexture().height
        },
        {
            40, 70,
            (float)state->assets.samarthSprite.getCurrentTexture().width * 4,
            (float)state->assets.samarthSprite.getCurrentTexture().height * 4
        },
        {
            (float)state->assets.samarthSprite.getCurrentTexture().width * 2,
            (float)state->assets.samarthSprite.getCurrentTexture().height * 2
        },
        0.0f,
        {255, 255, 255, 255}
    );

    // 1. Dialogue Box Background
    DrawRectangle(10, 145, SCREEN_WIDTH - 20, 85, Color{ 15, 15, 25, 240 });
    DrawRectangleLines(10, 145, SCREEN_WIDTH - 20, 85, BLUE);

    // 2. Speaker Nameplate floating properly above the dialogue box
    DrawRectangle(16, 127, 72, 18, BLACK);
    DrawRectangleLines(16, 127, 72, 18, BLUE);
    DrawText("SAMARTH", 20, 132, 8, BLUE);

    if (!state->samarthChoiceActive && !state->samarthResponded) {
        std::string currentText = lines[state->samarthLineIndex];
        int maxCharsPerLine = 34;
        std::string line1 = (currentText.length() > maxCharsPerLine) ? currentText.substr(0, maxCharsPerLine) : currentText;
        std::string line2 = (currentText.length() > maxCharsPerLine) ? currentText.substr(maxCharsPerLine) : "";

        DrawText(line1.c_str(), 20, 160, 8, RAYWHITE);
        DrawText(line2.c_str(), 20, 175, 8, RAYWHITE);

        if (static_cast<int>(GetTime() * 3.0) % 2 == 0) {
            DrawText("[CLICK / ENTER]", SCREEN_WIDTH - 90, 215, 6, BLUE);
        }
    } else if (state->samarthChoiceActive && !state->samarthResponded) {
        DrawText("Make your choice:", 20, 152, 8, SKYBLUE);

        Rectangle opt1Rect = { 35.0f, 168.0f, 170.0f, 20.0f };
        DrawRectangleRec(opt1Rect, state->samarthChoiceSelected == 0 ? Color{ 20, 80, 160, 230 } : Color{ 20, 20, 30, 230 });
        DrawRectangleLines((int)opt1Rect.x, (int)opt1Rect.y, (int)opt1Rect.width, (int)opt1Rect.height, BLUE);
        DrawText("Yes I'm in", (int)opt1Rect.x + 10, (int)opt1Rect.y + 6, 8, RAYWHITE);

        Rectangle opt2Rect = { 35.0f, 192.0f, 170.0f, 20.0f };
        DrawRectangleRec(opt2Rect, state->samarthChoiceSelected == 1 ? Color{ 160, 40, 40, 230 } : Color{ 20, 20, 30, 230 });
        DrawRectangleLines((int)opt2Rect.x, (int)opt2Rect.y, (int)opt2Rect.width, (int)opt2Rect.height, RED);
        DrawText("I don't care", (int)opt2Rect.x + 10, (int)opt2Rect.y + 6, 8, RAYWHITE);
    } else {
        if (state->acceptedChallenge) {
            DrawText("Thats the spirit, I will be cheering", 20, 156, 8, SKYBLUE);
            DrawText("you on all the way! Chronos repeats", 20, 168, 8, SKYBLUE);
            DrawText("the same route every loop.", 20, 180, 8, GOLD);
        } else {
            DrawText("I understand. The portal key is", 20, 160, 8, LIGHTGRAY);
            DrawText("still our way to the next island.", 20, 172, 8, LIGHTGRAY);
            DrawText("Chronos repeats his route each loop.", 20, 184, 7, GOLD);
        }

        if (static_cast<int>(GetTime() * 3.0) % 2 == 0) {
            DrawText("[CLICK / ENTER]", SCREEN_WIDTH - 90, 215, 6, BLUE);
        }
    }
}

void mainLoop(void* data) {
    GameState* state = (GameState*)data;
    Music& bgm = (state->currentIsland == 2) ? state->assets.bgm2 : state->assets.bgm;
    if (!bossActive()) {
        if (IsAudioDeviceReady() && !IsMusicStreamPlaying(bgm)) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || GetKeyPressed() != 0) {
                PlayMusicStream(bgm);
            }
        }
        UpdateMusicStream(bgm);
    }

    float deltaTime = GetFrameTime();
    Vector2 gameMousePos = getGameMousePos();

    BeginTextureMode(state->screenTexture);
        if (bossActive()) {
            bossScreen(state, gameMousePos, deltaTime);
        } else if (state->currentScreen == SCREEN_MENU) {
            menuScreen(state, gameMousePos);
        } else if (state->currentScreen == SCREEN_CREDITS) {
            creditsScreen(state, gameMousePos);
        } else if (state->currentScreen == SCREEN_SELECT) {
            selectScreen(state, gameMousePos);
        } else if (state->currentScreen == SCREEN_DIALOGUE) {
            dialogueScreen(state);
        } else if (state->currentScreen == SCREEN_GAME) {
            gameScreen(state, gameMousePos, deltaTime);
        } else if (state->currentScreen == SCREEN_BATTLE) {
            battleScreen(state, deltaTime);
        } else if (state->currentScreen == SCREEN_SAMARTH_DIALOGUE) {
            samarthDialogueScreen(state, deltaTime);
        } else if (state->currentScreen == SCREEN_PORTAL) {
            portalScreen(state, deltaTime);
        } else if (state->currentScreen == SCREEN_RIDDLE) {
            riddleScreen(state, deltaTime);
        }
    EndTextureMode();

    BeginDrawing();
        ClearBackground(BLACK);

        int scrWidth = GetScreenWidth();
        int scrHeight = GetScreenHeight();

        int scale = std::max(1, std::min(scrWidth / SCREEN_WIDTH, scrHeight / SCREEN_HEIGHT));

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
    assets.shadowSprite = LoadTexture("assets/TIles/Character.png");

    loadWreckProps("assets/trust.png");

    assets.bgm = LoadMusicStream("assets/AreaOne.mp3");
    assets.bgm.looping = true;
    SetMusicVolume(assets.bgm, 0.7f);
    assets.bgm2 = LoadMusicStream("assets/AreaTwo.mp3");
    assets.bgm2.looping = true;
    SetMusicVolume(assets.bgm2, 0.7f);
    PlayMusicStream(assets.bgm);

    assets.chronosSprite = AnimatedSprite(
        {
            "assets/chronos-anim1.png",
            "assets/chronos-anim2.png",
            "assets/chronos-anim3.png",
            "assets/chronos-anim4.png",
            "assets/chronos-anim5.png"
        },
        0.1f
    );

    assets.cogSprite = AnimatedSprite(
        {
            "assets/cog-anim1.png",
            "assets/cog-anim2.png",
            "assets/cog-anim3.png",
            "assets/cog-anim4.png",
            "assets/cog-anim5.png",
            "assets/cog-anim6.png"
        },
        0.1f
    );

    assets.samarthSprite = AnimatedSprite(
        {
            "assets/samarth-anim1.png",
            "assets/samarth-anim2.png",
            "assets/samarth-anim3.png",
            "assets/samarth-anim4.png",
            "assets/samarth-anim5.png",
            "assets/samarth-anim6.png"
        },
        0.1f
    );

    return assets;
}

void unloadAssets(GameAssets& assets) {
    UnloadShader(assets.crtShader);

    unloadWreckProps();
    UnloadTexture(assets.players[0]);
    UnloadTexture(assets.players[1]);
    UnloadTexture(assets.shadowSprite);

    UnloadMusicStream(assets.bgm);
    UnloadMusicStream(assets.bgm2);

    assets.chronosSprite.unloadTextures();

    assets = {};
}

int main(int argc, char** argv) {
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(1280, 720, "THE LAST LOOP");
    InitAudioDevice();

    GameState state = {};
    state.currentIsland = 1;
    state.player = {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
    state.portalKey.collected = false;
    state.exitKey.position = {0.0f, 0.0f};
    state.exitKey.collected = false;

    // Initialize patroller shadow position & speed
    state.shadowDefeated = false;

    state.currentScreen = SCREEN_SELECT;
    state.selectedCharacter = 0;
    state.selectAnim = 0.0f;
    state.assets = loadAssets();
    state.currentScreen = SCREEN_MENU;
    state.portalExit.unlocked = false;
    state.finalExit.position = {0.0f, 0.0f};
    state.finalExit.unlocked = false;

    state.islandMap = TileMap(ISLAND_WIDTH, ISLAND_HEIGHT, TILE_SIZE, TILE_SIZE);
#ifdef HAVE_ISLAND_ATLAS
    if (!loadIslandTiles(state.islandMap)) {
        loadIslandMap(state.islandMap);   // fallback: flat colours if the atlas is missing
    }
#else
    loadIslandMap(state.islandMap);
#endif

    // Centre the spawn tile on world (120, 120). Item and exit positions are
    // converted from map tiles once this origin is known; other test objects
    // keep their world coordinates. Negative map coordinates are fine here.
    state.mapOrigin = {
        SCREEN_WIDTH / 2.0f - ISLAND_SPAWN_X * TILE_SIZE - TILE_SIZE / 2.0f,
        SCREEN_HEIGHT / 2.0f - ISLAND_SPAWN_Y * TILE_SIZE - TILE_SIZE / 2.0f
    };
    state.chronos.position = island1TileCentre(&state, 35, 42);
    state.chronos.movingToB = true;
    state.patroller.position = island1TileCentre(&state, I1_SHADOW_X, I1_SHADOW_Y);
    state.portalKey.position = island1TileCentre(&state, I1_PORTAL_KEY_X, I1_PORTAL_KEY_Y);
    state.portalExit.position = island1TileCentre(&state, I1_PORTAL_X, I1_PORTAL_Y);

    buildMinimap(&state);

    // One lighting cell per world pixel, covering the whole map. Obstacles use
    // map-local coordinates, so the test wall (world 140,58 to 143,165) is
    // shifted by mapOrigin.
    state.lightingMap = LightingMap(
        ISLAND_WIDTH * TILE_SIZE, ISLAND_HEIGHT * TILE_SIZE
    );
    for (int y = 58; y < 166; ++y) {
        for (int x = 140; x < 144; ++x) {
            state.lightingMap.setObstacle(
                x - (int)state.mapOrigin.x, y - (int)state.mapOrigin.y, 1
            );
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
    if (gMiniTex.id != 0) UnloadTexture(gMiniTex);

    state.islandMap.Unload();
    unloadAssets(state.assets);

    CloseAudioDevice();
    CloseWindow();

    return 0;
}
