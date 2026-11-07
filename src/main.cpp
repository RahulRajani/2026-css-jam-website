#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#include <raylib.h>

#include <cstdint>
#include <vector>
#include <cmath>

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240

#define PLAYER_RECT_SIZE 16
#define PLAYER_VELOCITY_MAX 100
#define PLAYER_ACCELERATION 15

struct Player {
	float x, y;
	float velx, vely;
};

struct GameAssets {
	Shader crtShader;

	Texture players[2];
};

struct GameState {
	Player player;
	bool carolMode;

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

// main update loop - add stuff here
void mainLoop(void* data) {
	GameState* state = (GameState*)data;

	float deltaTime = GetFrameTime();

	Vector2 gameMousePos = getGameMousePos();
	
	// player movement

	float playerMouseAngle = std::atan2(
		gameMousePos.y - state->player.y,
		gameMousePos.x - state->player.x
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

	// player mode switch
	if (IsKeyPressed(KEY_SPACE)) {
		state->carolMode = !state->carolMode;
	}

	BeginTextureMode(state->screenTexture);
		ClearBackground(Color{255, 255, 255, 255});

		DrawTexturePro(
			state->assets.players[state->carolMode ? 1 : 0],
			{0, 0, PLAYER_RECT_SIZE, PLAYER_RECT_SIZE},
			{state->player.x - PLAYER_RECT_SIZE / 2.0f, state->player.y - PLAYER_RECT_SIZE / 2.0f, PLAYER_RECT_SIZE, PLAYER_RECT_SIZE},
			{PLAYER_RECT_SIZE / 2.0f, PLAYER_RECT_SIZE / 2.0f},
			playerMouseAngle * RAD2DEG, {255, 255, 255, 255}
		);
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
	GameAssets assets;

	assets.crtShader = LoadShader(0, "assets/crt.frag");

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

	return assets;
}

void unloadAssets(GameAssets& assets) {
	UnloadShader(assets.crtShader);

	UnloadTexture(assets.players[0]);
	UnloadTexture(assets.players[1]);

	assets = {};
}

int main(int argc, char** argv) {
	SetTraceLogLevel(LOG_WARNING);
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);

	InitWindow(1280, 720, "game"); // change title

	GameState state = {}; // create inital state
	state.player = {SCREEN_WIDTH / 2 - PLAYER_RECT_SIZE / 2, SCREEN_HEIGHT / 2 - PLAYER_RECT_SIZE / 2};
	state.carolMode = false;

	state.assets = loadAssets();

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

	CloseWindow();

	return 0;
}
