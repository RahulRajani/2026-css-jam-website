#pragma once
#include "Map.h"

// Island 3 is 60 x 45 tiles.
static const int ISLAND3_W = 60;
static const int ISLAND3_H = 45;

// Tile coordinates of the key spots (all kept clear of trees/walls).
static const int I3_SPAWN_X = 7,   I3_SPAWN_Y = 22;
static const int I3_EXIT_X  = 53,  I3_EXIT_Y  = 36;
static const int I3_CHRONOS_X = 8, I3_CHRONOS_Y = 8;
static const int I3_SHADOW_X = 30, I3_SHADOW_Y = 22;

// Builds the complete island 3 layout into the map (tiles only; load the
// placeholder tileset first, like main.cpp does for island 2).
void BuildIsland3Map(TileMap& map);

// Tile ID at (x, y) of the layout built by BuildIsland3Map, or -1 if out of range.
int Island3TileAt(int x, int y);
