# Pixel Spell and Magic FX (32x32)
47 free animated pixel-art spell effects, 338 frames in total: projectiles (9), impacts (14), circles (6), shields (4), vortex (6), pillars (4), beams (4).
- PNG/frames_32x32/<category>/<effect>/<effect>_00.png ... : individual transparent frames (native)
- PNG/frames_128x128_x4/ : the same frames at 4x (nearest neighbour)
- PNG/strips_32x32/ and strips_128x128_x4/ : one horizontal strip per effect
- GIF/<effect>.gif : 128x128 preview of every effect, transparent
- spritesheet.png (8 columns, one effect per row) + spritesheet.json (x,y,w,h of every frame, fps, loop flag), spritesheet_x4.png
- demo_grid.gif : all effects playing
Playback: impacts and pillars play once; projectiles, magic circles, shields, vortexes and beams are seamless loops (loop=true in the JSON). Suggested fps is in the JSON.
Engine tips (Unity/Godot/GameMaker/Construct): Filter = Point/Nearest, no compression, integer scaling. Use additive blending for extra glow if you like.
Credit appreciated: "Pixel Spell and Magic FX by reodesuxz". See LICENSE.txt.
