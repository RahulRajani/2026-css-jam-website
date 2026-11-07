# Island game: C++ tile-map starter

This starter assumes **Raylib** because the project screenshot shows a C++ game project with shader files and build scripts, but the screenshot does not prove which graphics library is configured. The code below uses Raylib APIs. If your project uses SDL2 or SFML, tell me and this needs adapting.

## Files

- `Map.h` — TileMap class declaration.
- `Map.cpp` — loads a tilesheet, draws a grid map, lets you paint tiles, and saves/loads a simple text map.

## Where to put the files

Copy `Map.h` and `Map.cpp` into your existing `src/` folder:

```text
2026-css-jam/
├── assets/
│   ├── Tileset.png
│   └── Tiles/
│       ├── Free_Pixel_Portals_and_Runes/
│       ├── kenney_tiny-dungeon/
│       ├── Pixel Art Top Down - Basic v1/
│       ├── Pixel_Beach_Island_Tileset_v1/
│       ├── Pixel_Dungeon_Tileset_v1/
│       ├── Pixel_Spell_Magic_FX_v1/
│       ├── Pixel_Top-down_Forest_and_Village.../
│       └── Raven Fantasy - Pixel Art Tileset - .../
└── src/
    ├── main.cpp
    ├── Map.h
    ├── Map.cpp
    ├── chronos.cpp
    └── chronos.h
```

The code uses your existing `assets/Tileset.png` path. If that image is not the atlas you want to use, change the path in `main.cpp`.

## Add Map.cpp to your build

If your CMakeLists.txt explicitly lists source files, add `src/Map.cpp` to the executable's source list. If it already compiles all `.cpp` files in `src`, you may not need to change CMake.

## Example main.cpp integration

Keep your existing window and game setup if you already have it. The essential integration looks like this:

```cpp
#include "raylib.h"
#include "Map.h"

int main() {
    InitWindow(1000, 800, "Island Map");
    SetTargetFPS(60);

    // sourceTileSize = size of a tile inside the PNG.
    // worldTileSize = size it appears on screen.
    // Change sourceTileSize to 16 if your chosen atlas uses 16x16 tiles.
    TileMap island(50, 40, 32, 32);

    island.LoadTileset("assets/Tileset.png");

    // Temporary test tile IDs. Change these after inspecting the atlas.
    // -1 means empty.
    island.SetTile(10, 10, 0);
    island.SetTile(11, 10, 1);
    island.SetTile(12, 10, 2);

    int selectedTile = 0;
    const Vector2 mapOffset{20.0f, 20.0f};

    while (!WindowShouldClose()) {
        // Number keys select tile IDs 0 to 9 for quick testing.
        for (int key = KEY_ZERO; key <= KEY_NINE; ++key) {
            if (IsKeyPressed(key)) {
                selectedTile = key - KEY_ZERO;
            }
        }

        // Left mouse button paints; right mouse button erases.
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            island.PaintAt(GetMousePosition(), mapOffset, selectedTile);
        }
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            island.PaintAt(GetMousePosition(), mapOffset, -1);
        }

        if (IsKeyPressed(KEY_S)) {
            island.Save("assets/maps/island.map");
        }
        if (IsKeyPressed(KEY_L)) {
            island.Load("assets/maps/island.map");
        }

        BeginDrawing();
        ClearBackground(Color{12, 15, 26, 255});

        island.Draw(mapOffset, true);

        DrawText(TextFormat("Selected tile ID: %d", selectedTile),
                 20, 750, 20, RAYWHITE);
        DrawText("1-0: select tile | Left click: paint | Right click: erase",
                 20, 775, 16, LIGHTGRAY);
        DrawText("S: save | L: load", 20, 795, 16, LIGHTGRAY);

        EndDrawing();
    }

    island.Unload();
    CloseWindow();
    return 0;
}
```

If your window is only 800x600, use a smaller map or draw it with a camera/scrolling. A 50x40 map at 32 pixels per tile is 1600x1280 pixels.

## How tile IDs work

The atlas is treated as a regular grid. If the atlas has 10 columns:

- ID `0` is column 0, row 0 (top-left).
- ID `1` is column 1, row 0.
- ID `10` is column 0, row 1.
- ID `11` is column 1, row 1.

The sample IDs `0`, `1`, and `2` are placeholders. They may not be grass, dirt or stone in your actual PNG. Inspect the atlas and change the IDs to match.

## Important tile-size setting

The constructor is:

```cpp
TileMap island(50, 40, 32, 32);
```

The third number is the tile size **inside the source PNG**. The fourth is the displayed world tile size.

For a 16x16 source atlas displayed at 32x32:

```cpp
TileMap island(50, 40, 16, 32);
```

Do not change the source size unless the PNG is actually arranged on that grid.

## Notes

- The code expects Raylib to already be configured in your project.
- Paths such as `assets/Tileset.png` are relative to the program's working directory. Run the game with the project root (`2026-css-jam/`) as the working directory.
- Create the `assets/maps/` folder before saving.
- This is a basic map renderer/editor, not yet a Tiled JSON importer.
- Collision, animated water, multiple tileset atlases, camera scrolling and object layers can be added next.
