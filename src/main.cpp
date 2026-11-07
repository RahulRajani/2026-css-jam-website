#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#include <raylib.h>

#include <cstdint>
#include <vector>
#include <cmath>

#define PLAYER_RECT_SIZE 50
#define PLAYER_VELOCITY_MAX 300
#define PLAYER_ACCELERATION 15

struct Player {
	float x, y;
	float velx, vely;
};

struct GameState {
	Player player;
};

// main update loop - add stuff here
void mainLoop(void* data) {
	GameState* state = (GameState*)data;

	float deltaTime = GetFrameTime();

	Vector2 mousePos = GetMousePosition();
	
	float playerMouseAngle = std::atan2(
		mousePos.y - state->player.y,
		mousePos.x - state->player.x
	);

	float accelerateX = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);
	float accelerateY = IsKeyDown(KEY_S) - IsKeyDown(KEY_W);

	state->player.velx = std::lerp(
		state->player.velx, PLAYER_VELOCITY_MAX * accelerateX, PLAYER_ACCELERATION * deltaTime
	);
	state->player.vely = std::lerp(
		state->player.vely, PLAYER_VELOCITY_MAX * accelerateY, PLAYER_ACCELERATION * deltaTime
	);

	state->player.x += state->player.velx * deltaTime;
	state->player.y += state->player.vely * deltaTime;

	BeginDrawing();

	ClearBackground(Color{255, 255, 255, 255});

	DrawRectanglePro(
		Rectangle{state->player.x, state->player.y, PLAYER_RECT_SIZE, PLAYER_RECT_SIZE},
		Vector2{PLAYER_RECT_SIZE / 2.0f, PLAYER_RECT_SIZE / 2.0f},
		playerMouseAngle * RAD2DEG,
		Color{140, 140, 140, 255}
	);

	DrawFPS(10, 10);

	EndDrawing();
}

int main(int argc, char** argv) {
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);

	InitWindow(1280, 720, "game"); // change title

	GameState state = {}; // create inital state
	state.player = {400, 400};

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
