#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#include <raylib.h>

#include <cstdint>
#include <vector>
#include <cmath>

struct GameState {
	// add stuff
	
	// e.g.
	std::vector<Vector2> rectanglesPos;
};

// main update loop - add stuff here
void mainLoop(void* data) {
	GameState* state = (GameState*)data;

	// add randomly positioned rectangles to array
	for (int i = 0; i < 10; i++) {
		state->rectanglesPos.push_back(
			Vector2{
				(float)GetRandomValue(0, GetRenderWidth()),
				(float)GetRandomValue(0, GetRenderHeight())
			}
		);
	}

	BeginDrawing();

	ClearBackground(Color{255, 255, 255, 255});

	// draw random rectangles
	for (const Vector2& vec : state->rectanglesPos) {
		uint8_t greyscale = 70 + (int)vec.x % 60;

		DrawRectanglePro(
			Rectangle{vec.x, vec.y, 40, 40},
			Vector2{20, 20},
			GetTime() * 75.0f * (std::fmod((float)vec.x * 0.02f, 0.5f) + 0.5f),
			Color{greyscale, greyscale, greyscale, 150}
		);
	}

	// rotating + scaling text on a sine wave
	float fontSize = 42.0f * (std::sin(GetTime()) + 2) / 2.0f;

	const char* text = "C++ game on the web :)";
	Vector2 textSize = MeasureTextEx(GetFontDefault(), text, fontSize, fontSize / 8.0f);

	DrawTextPro(
		GetFontDefault(),
		text,
		Vector2{GetRenderWidth() / 2.0f, GetRenderHeight() / 2.0f},
		Vector2{textSize.x / 2, textSize.y / 2},
		GetTime() * 45.0f,
		fontSize,
		fontSize / 8.0f,
		Color{0, 0, 0, 255}
	);

	DrawFPS(10, 10);

	EndDrawing();
}

int main(int argc, char** argv) {
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);

	InitWindow(1280, 720, "game"); // change title

	GameState state = {}; // create inital state

	#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop_arg(mainLoop, &state, 0, true);
	#else
	while (!WindowShouldClose()) {
		mainLoop(&state);
	}
	#endif

	CloseWindow();

	return 0;
}
