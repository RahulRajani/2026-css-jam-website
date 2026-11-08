#include "island3_layout.h"
#include "TilePalette.h"
#include <vector>
#include <cstdlib>

static std::vector<int> gIsland3;

int Island3TileAt(int x, int y) {
    if (gIsland3.empty() || x < 0 || y < 0 || x >= ISLAND3_W || y >= ISLAND3_H) return -1;
    return gIsland3[y * ISLAND3_W + x];
}

void BuildIsland3Map(TileMap& map) {
    const int W = ISLAND3_W, H = ISLAND3_H;
    gIsland3.assign(W * H, (int)TileID::WATER);

    auto set = [&](int x, int y, int id) {
        if (x >= 0 && y >= 0 && x < W && y < H) gIsland3[y * W + x] = id;
    };
    auto nearPt = [](int x, int y, int kx, int ky) {
        return std::abs(x - kx) <= 4 && std::abs(y - ky) <= 4;
    };

    // Ground: sand beach ring, grass inside, scattered flowers/dark grass/trees
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            if (x < 3 || y < 3 || x >= W - 3 || y >= H - 3) continue;
            const bool edge = (x == 3 || y == 3 || x == W - 4 || y == H - 4);
            int id = edge ? (int)TileID::SAND : (int)TileID::GRASS;

            if (!edge) {
                unsigned h = (unsigned)x * 73856093u ^ (unsigned)y * 19349663u;
                h ^= h >> 13; h *= 0x5bd1e995u; h ^= h >> 15;
                const int r = (int)(h % 100);
                if (r < 10) id = (int)TileID::DARK_GRASS;
                else if (r < 16) id = (int)TileID::FLOWER_GRASS;

                const bool inner = x >= 5 && y >= 5 && x < W - 5 && y < H - 5;
                const bool clear = nearPt(x, y, I3_SPAWN_X, I3_SPAWN_Y) ||
                                   nearPt(x, y, I3_SHADOW_X, I3_SHADOW_Y) ||
                                   nearPt(x, y, I3_EXIT_X, I3_EXIT_Y) ||
                                   std::abs(x - 20) <= 2 || std::abs(x - 40) <= 2 || x < 10;
                if (inner && !clear && ((h >> 8) % 100) < 7) {
                    id = (h & 1) ? (int)TileID::TREE : (int)TileID::PINE_TREE;
                }
            }
            set(x, y, id);
        }
    }

    // Two ruined walls with gaps you have to find
    for (int y = 6; y < H - 6; ++y) {
        if (!(y >= 20 && y <= 24)) set(20, y, (int)TileID::STONE_WALL);
        if (!((y >= 9 && y <= 13) || (y >= 31 && y <= 35))) set(40, y, (int)TileID::STONE_WALL);
    }
    set(19, 19, (int)TileID::TORCH); set(19, 25, (int)TileID::TORCH);
    set(41, 8,  (int)TileID::TORCH); set(41, 14, (int)TileID::TORCH);
    set(41, 30, (int)TileID::TORCH); set(41, 36, (int)TileID::TORCH);

    // Shadow arena (stone floor), spawn clearing (dirt), exit rune
    for (int dy = -2; dy <= 2; ++dy)
        for (int dx = -2; dx <= 2; ++dx)
            set(I3_SHADOW_X + dx, I3_SHADOW_Y + dy, (int)TileID::STONE_FLOOR);
    for (int dy = -1; dy <= 1; ++dy)
        for (int dx = -1; dx <= 1; ++dx)
            set(I3_SPAWN_X + dx, I3_SPAWN_Y + dy, (int)TileID::DIRT);
    set(I3_EXIT_X, I3_EXIT_Y, (int)TileID::RUNE);
    set(I3_EXIT_X + 1, I3_EXIT_Y, (int)TileID::PURPLE_MAGIC);
    set(I3_EXIT_X - 1, I3_EXIT_Y, (int)TileID::PURPLE_MAGIC);
    set(I3_EXIT_X, I3_EXIT_Y + 1, (int)TileID::PURPLE_MAGIC);
    set(I3_EXIT_X, I3_EXIT_Y - 1, (int)TileID::PURPLE_MAGIC);

    for (int y = 0; y < H; ++y)
        for (int x = 0; x < W; ++x)
            map.SetTile(x, y, gIsland3[y * W + x]);
}
