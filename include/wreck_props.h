#pragma once
#include <raylib.h>

// Plane-wreck prop atlas (assets/trust.png, 1024x1024, irregular layout).
// Not a tile grid, so every piece has its own source rectangle.
// The rectangles were estimated by eye: nudge them if a piece looks cropped.

struct WreckProp {
    Rectangle src;   // region of trust.png
    float scale;     // world pixels per source pixel (tiles are 16 px)
};

static const WreckProp W_NOSE   = {{  35,  80, 275, 170}, 0.20f};
static const WreckProp W_MIDDLE = {{ 310, 100, 385, 130}, 0.20f};
static const WreckProp W_TAIL   = {{ 700,  30, 300, 215}, 0.20f};
static const WreckProp W_ENGINE = {{  60, 260, 180, 100}, 0.18f};
static const WreckProp W_WING   = {{ 265, 265, 230, 100}, 0.18f};
static const WreckProp W_SEATS  = {{ 435, 380, 130, 110}, 0.18f};
static const WreckProp W_CARGO  = {{ 580, 388, 145,  95}, 0.18f};
static const WreckProp W_CRATE  = {{  58, 498, 155,  95}, 0.15f};
static const WreckProp W_EXTING = {{ 895, 618,  55, 100}, 0.20f};

static Texture2D g_wreck = {};

// Loads the atlas and turns its near-white background transparent.
inline void loadWreckProps(const char* path = "assets/trust.png") {
    Image img = LoadImage(path);
    if (img.data == nullptr) return;
    ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    unsigned char* px = (unsigned char*)img.data;
    for (int i = 0; i < img.width * img.height; ++i) {
        unsigned char* p = px + i * 4;
        if (p[0] > 238 && p[1] > 238 && p[2] > 238) p[3] = 0;
    }
    g_wreck = LoadTextureFromImage(img);
    UnloadImage(img);
    SetTextureFilter(g_wreck, TEXTURE_FILTER_POINT);
}

inline void unloadWreckProps() {
    if (g_wreck.id) { UnloadTexture(g_wreck); g_wreck = {}; }
}

// Draws a prop with its bottom-centre at world (x, y), so it sits on the ground.
inline void drawWreck(const WreckProp& p, float x, float y) {
    if (g_wreck.id == 0) return;
    const float w = p.src.width * p.scale;
    const float h = p.src.height * p.scale;
    DrawTexturePro(g_wreck, p.src, { x - w / 2.0f, y - h, w, h }, { 0, 0 }, 0.0f, WHITE);
}

// The whole crash scene, centred on a world position (the crash-site tile).
inline void drawCrashSite(float cx, float cy) {
    drawWreck(W_TAIL,    cx + 78,  cy - 18);
    drawWreck(W_MIDDLE,  cx - 10,  cy - 40);
    drawWreck(W_NOSE,    cx - 96,  cy - 8);
    drawWreck(W_ENGINE,  cx + 40,  cy + 44);
    drawWreck(W_WING,    cx - 60,  cy + 58);
    drawWreck(W_SEATS,   cx - 20,  cy + 70);
    drawWreck(W_CARGO,   cx + 100, cy + 30);
    drawWreck(W_CRATE,   cx + 70,  cy + 78);
    drawWreck(W_EXTING,  cx - 120, cy + 40);
}

// Smaller scatter for island 2's crash area.
inline void drawCrashSiteSmall(float cx, float cy) {
    drawWreck(W_WING,   cx + 40, cy);
    drawWreck(W_CARGO,  cx - 30, cy + 30);
    drawWreck(W_ENGINE, cx + 10, cy - 30);
}
