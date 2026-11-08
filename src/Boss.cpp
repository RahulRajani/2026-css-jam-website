#include <raylib.h>
#include <algorithm>
#include <cmath>
#include <string>

#include "game.h"
#include "defines.h"

static const int   MAX_HP         = 100;
static const int   DMG_TO_CHRONOS = 10;     // 10 perfect hits to win
static const int   DMG_TO_PLAYER  = 10;     // 5 misses to lose
static const float HALF_START     = 0.080f; // gold zone half-width at full Chronos HP
static const float HALF_END       = 0.055f; //   ...and at almost dead
static const float TIMEOUT_START  = 2.8f;
static const float TIMEOUT_END    = 2.0f;
static const float RESULT_TIME    = 0.7f;
static const float MELTDOWN_SECS  = 10.0f;
static const int   START_POOL     = 10;     // keys available at the start

struct PromptKey { int key; const char* name; };
// Ordered easy -> hard. The available pool grows as Chronos weakens.
static const PromptKey PROMPTS[] = {
    { KEY_SPACE, "SPACE" }, { KEY_UP, "UP" }, { KEY_DOWN, "DOWN" },
    { KEY_LEFT, "LEFT" },   { KEY_RIGHT, "RIGHT" }, { KEY_ENTER, "ENTER" },
};
static const int PROMPT_COUNT = (int)(sizeof(PROMPTS) / sizeof(PROMPTS[0]));

static const char* CONFRONT[] = {
    "So. You found the keys I hid, little mortal.",
    "I am Chronos. Before the sun, before the sea, there was time, and I was its master.",
    "Your kind treats my gift as endless, and burns it on glowing screens and idle hours.",
    "So I gave you these islands and their loops, so every second would finally matter.",
};
static const int CONFRONT_COUNT = 4;

static const char* CHOICES[2] = { "We've learned. Let us go.", "Your time is up, Chronos." };
static const char* REPLY[2] = {
    "Learned?! Words are cheap, child. PROVE you value your time!",
    "How bold. Then face the master of time itself!",
};

static const char* DYING[] = {
    "The time is the means by which all is revealed.",
    "You understand the flow of time, and yet you still choose to defy me???",
    "If more humans were like you, maybe I'd be more sparing...",
    "...but it is too late now.",
};
static const int DYING_COUNT = 4;

static const char* OUTRO[] = {
    "Chronos shrieks, and is swept out of time.",
    "A golden watch clatters to the floor.",
    "A portal opens. We're going home.",
};
static const int OUTRO_COUNT = 3;

enum BossPhase {
    P_CONFRONT, P_CHOICE, P_REPLY, P_BATTLE, P_LOST,
    P_DYING, P_MELTDOWN, P_BOOM, P_SENT_AWAY, P_OUTRO
};

struct BossData {
    bool active = false;
    BossPhase phase = P_CONFRONT;
    int line = 0;
    int choice = 0;
    float t = 0.0f;                 // seconds in the current phase
    bool musicSwitched = false;

    int playerHp = MAX_HP, chronosHp = MAX_HP;
    int promptIndex = 0;
    float roundT = 0.0f, cursor = 0.0f;
    bool resolved = false, lastPerfect = false;
    float resultT = 0.0f;
    float shakeP = 0.0f, shakeC = 0.0f;
    float flashRed = 0.0f, flashWhite = 0.0f;
    float zoneCenter = 0.5f, zoneHalf = HALF_START;
    float speed = 3.4f, timeout = TIMEOUT_START;

    float countdown = MELTDOWN_SECS;
    bool ePressed = false;
};

static BossData boss;
static std::string voiceIdFor() {
    switch (boss.phase) {
    case P_CONFRONT: return "confront_" + std::to_string(boss.line);
    case P_CHOICE:   return "choice_prompt";
    case P_REPLY:    return (boss.line == 0 ? "choice_" : "reply_") + std::to_string(boss.choice);
    case P_DYING:    return "dying_" + std::to_string(boss.line);
    case P_LOST:     return "lost";
    case P_MELTDOWN: return boss.ePressed ? "meltdown_e" : "meltdown";
    case P_BOOM:     return "boom";
    case P_SENT_AWAY:return "sent_away";
    case P_OUTRO:    return "outro_" + std::to_string(boss.line);
    default:         return "";   // P_BATTLE is handled by barks
    }
}
static Music g_music;
static bool g_musicLoaded = false;
// ---- voice acting ----
static Sound g_voice;
static bool g_voiceLoaded = false;
static std::string g_lastVoice;
static const int BARK_VARIANTS = 3;

static bool voicePlaying() { return g_voiceLoaded && IsSoundPlaying(g_voice); }

static void stopVoice() {
    if (g_voiceLoaded) {
        StopSound(g_voice);
        UnloadSound(g_voice);
        g_voiceLoaded = false;
    }
}

static void speak(const std::string& id) {
    stopVoice();
    static const char* EXT[] = { ".ogg", ".wav", ".mp3" };
    for (const char* e : EXT) {
        const std::string path = "assets/voice/" + id + e;
        if (FileExists(path.c_str())) {
            g_voice = LoadSound(path.c_str());
            g_voiceLoaded = true;
            PlaySound(g_voice);
            return;
        }
    }
}

static std::string bark(const char* base) {
    return std::string(base) + "_" + std::to_string(GetRandomValue(0, BARK_VARIANTS - 1));
}

// ---------------------------------------------------------------------------
// helpers
// ---------------------------------------------------------------------------
static bool advance() {
    return IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
           IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

static bool blink() { return (int)(GetTime() * 2.0) % 2 == 0; }

static void setPhase(BossPhase p) {
    boss.phase = p;
    boss.line = 0;
    boss.t = 0.0f;
}

static void drawCentered(const char* text, int y, int size, Color c) {
    DrawText(text, (SCREEN_WIDTH - MeasureText(text, size)) / 2, y, size, c);
}

static void drawWrapped(const std::string& text, int x, int y, int maxW, int size, Color c) {
    std::string line, word;
    int cy = y;
    for (size_t i = 0; i <= text.size(); ++i) {
        if (i == text.size() || text[i] == ' ') {
            const std::string test = line.empty() ? word : line + " " + word;
            if (!line.empty() && MeasureText(test.c_str(), size) > maxW) {
                DrawText(line.c_str(), x, cy, size, c);
                cy += size + 3;
                line = word;
            } else {
                line = test;
            }
            word.clear();
        } else {
            word += text[i];
        }
    }
    if (!line.empty()) DrawText(line.c_str(), x, cy, size, c);
}

static void dialogueBox(const char* speaker, Color accent, const std::string& text) {
    DrawRectangle(10, 150, SCREEN_WIDTH - 20, 80, Color{ 12, 10, 16, 245 });
    DrawRectangleLines(10, 150, SCREEN_WIDTH - 20, 80, accent);
    DrawRectangle(16, 134, 80, 16, BLACK);
    DrawRectangleLines(16, 134, 80, 16, accent);
    DrawText(speaker, 20, 138, 8, accent);
    drawWrapped(text, 18, 160, SCREEN_WIDTH - 36, 8, RAYWHITE);
    if (blink()) DrawText("[CLICK / ENTER]", SCREEN_WIDTH - 92, 218, 6, accent);
}

static void drawBackdrop(bool hot) {
    ClearBackground(hot ? Color{ 26, 2, 6, 255 } : Color{ 10, 4, 14, 255 });
    const float t = (float)GetTime();
    for (int i = 0; i < 7; ++i) {
        const float x = std::fmod(i * 46.0f + t * 18.0f, 320.0f) - 40.0f;
        DrawLineEx({ x, 0.0f }, { x - 90.0f, (float)SCREEN_HEIGHT }, 14.0f,
                   hot ? Color{ 150, 12, 24, 70 } : Color{ 110, 12, 48, 60 });
    }
}

static void drawChronos(GameState* state, float cx, float cy, float height,
                        Color tint, float shake) {
    const Texture2D tex = state->assets.chronosSprite.getCurrentTexture();
    if (tex.height <= 0) return;
    const float scale = height / (float)tex.height;
    const float w = tex.width * scale, h = tex.height * scale;
    const float ox = shake > 0.0f ? (float)std::sin(GetTime() * 90.0) * shake : 0.0f;
    const float oy = shake > 0.0f ? (float)std::cos(GetTime() * 110.0) * shake * 0.6f : 0.0f;
    DrawTexturePro(tex, { 0, 0, (float)tex.width, (float)tex.height },
                   { cx + ox, cy + oy, w, h }, { w / 2.0f, h / 2.0f }, 0.0f, tint);
}

static void drawWatch(float cx, float cy) {
    DrawCircle((int)cx, (int)cy, 9, GOLD);
    DrawCircleLines((int)cx, (int)cy, 9, Color{ 140, 100, 10, 255 });
    DrawRectangle((int)cx - 1, (int)cy - 7, 2, 7, Color{ 90, 60, 5, 255 });
    DrawRectangle((int)cx - 1, (int)cy - 1, 5, 2, Color{ 90, 60, 5, 255 });
    DrawRectangle((int)cx - 2, (int)cy - 13, 4, 4, GOLD);
}

static void hpBar(int x, int y, int w, int hp, Color fill, const char* label) {
    DrawRectangle(x - 1, y - 1, w + 2, 10, BLACK);
    DrawRectangle(x, y, w, 8, Color{ 40, 10, 14, 255 });
    DrawRectangle(x, y, (int)(w * hp / (float)MAX_HP), 8, fill);
    DrawText(label, x, y - 8, 6, RAYWHITE);
    const std::string n = std::to_string(hp);
    DrawText(n.c_str(), x + w - MeasureText(n.c_str(), 6), y - 8, 6, RAYWHITE);
}

static const char* playerName(const GameState* s) {
    return s->selectedCharacter == 0 ? "JAMIE" : "CAROL";
}

static float rage() { return 1.0f - boss.chronosHp / (float)MAX_HP; }

static void newRound() {
    const float r = rage();
    boss.roundT = 0.0f;
    boss.resolved = false;

    const int pool = std::min(PROMPT_COUNT, START_POOL + (int)(r * (PROMPT_COUNT - START_POOL)));
    int idx;
    do { idx = GetRandomValue(0, pool - 1); } while (idx == boss.promptIndex && pool > 1);
    boss.promptIndex = idx;

    boss.zoneHalf   = HALF_START + (HALF_END - HALF_START) * r;
    boss.timeout    = TIMEOUT_START + (TIMEOUT_END - TIMEOUT_START) * r;
    boss.speed      = 3.4f + r * 1.2f + GetRandomValue(0, 100) / 100.0f * 0.6f;
    const float wander = 0.10f + 0.20f * r;   // the gold zone drifts off-centre
    boss.zoneCenter = 0.5f + (GetRandomValue(-100, 100) / 100.0f) * wander;

    while (GetKeyPressed() > 0) {}
}

static void startBattle(GameState* state) {
    speak(boss.musicSwitched ? "retry" : "battle_start");
    if (!boss.musicSwitched) {
        Music& area = (state->currentIsland == 2) ? state->assets.bgm2 : state->assets.bgm;
        StopMusicStream(area);
        if (!g_musicLoaded) {
            g_music = LoadMusicStream("assets/Boss.mp3");
            g_music.looping = true;
            SetMusicVolume(g_music, 0.8f);
            g_musicLoaded = true;
        }
        PlayMusicStream(g_music);
        boss.musicSwitched = true;
    }
    boss.playerHp = MAX_HP;
    boss.chronosHp = MAX_HP;
    newRound();
    setPhase(P_BATTLE);
}

static void endBoss(GameState* state) {
    stopVoice();                       // <-- add
    if (g_musicLoaded) {
        StopMusicStream(g_music);
        UnloadMusicStream(g_music);
        g_musicLoaded = false;
    }
    boss.active = false;
    state->victory = true;
    state->currentScreen = SCREEN_GAME;
}

bool bossActive() { return boss.active; }

void startBoss(GameState* state) {
    (void)state;
    stopVoice(); g_lastVoice.clear();  // <-- add
    boss = BossData();
    boss.active = true;
    SetRandomSeed((unsigned)(GetTime() * 1000.0));
    while (GetKeyPressed() > 0) {}
    while (GetCharPressed() > 0) {}
}

// ---------------------------------------------------------------------------
// the screen
// ---------------------------------------------------------------------------
void bossScreen(GameState* state, Vector2 mousePos, float dt) {
    boss.t += dt;
    boss.flashRed = std::max(0.0f, boss.flashRed - dt);
    boss.flashWhite = std::max(0.0f, boss.flashWhite - dt);
    boss.shakeP = std::max(0.0f, boss.shakeP - dt);
    boss.shakeC = std::max(0.0f, boss.shakeC - dt);
    state->assets.chronosSprite.updateFrame(dt);

    // Whichever track is playing has to be fed every frame.
    Music& area = (state->currentIsland == 2) ? state->assets.bgm2 : state->assets.bgm;
    UpdateMusicStream((boss.musicSwitched && g_musicLoaded) ? g_music : area);
    if (g_musicLoaded) SetMusicVolume(g_music, voicePlaying() ? 0.35f : 0.8f);

    const Color chronosAccent = { 220, 70, 70, 255 };
    const Color playerAccent  = { 0x23, 0x7b, 0x52, 255 };

    // ------------------------------ update ------------------------------
    const Rectangle choiceRects[2] = { { 20, 172, 200, 20 }, { 20, 196, 200, 20 } };
    const Rectangle stopwatchBtn = { 20, 178, 200, 32 };

    switch (boss.phase) {

    case P_CONFRONT:
        if (advance() && ++boss.line >= CONFRONT_COUNT) setPhase(P_CHOICE);
        break;

    case P_CHOICE: {
        bool confirm = false;
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))   boss.choice = 0;
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) boss.choice = 1;
        for (int i = 0; i < 2; ++i) {
            if (CheckCollisionPointRec(mousePos, choiceRects[i])) {
                boss.choice = i;
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) confirm = true;
            }
        }
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) confirm = true;
        if (confirm) setPhase(P_REPLY);
        break;
    }

    case P_REPLY:
    if (advance() && ++boss.line >= 2) startBattle(state);
    break;

    case P_BATTLE:
        if (!boss.resolved) {
            boss.roundT += dt;
            boss.cursor = (1.0f - std::cos(boss.roundT * boss.speed)) * 0.5f;

            bool judged = false, perfect = false;
            int k;
            while ((k = GetKeyPressed()) > 0) {
                if (judged) continue;
                judged = true;
                perfect = (k == PROMPTS[boss.promptIndex].key) &&
                          std::fabs(boss.cursor - boss.zoneCenter) <= boss.zoneHalf;
            }
            if (!judged && boss.roundT >= boss.timeout) judged = true;

            if (judged) {
                boss.resolved = true;
                boss.resultT = 0.0f;
                boss.lastPerfect = perfect;
                if (perfect) {
                    boss.chronosHp = std::max(0, boss.chronosHp - DMG_TO_CHRONOS);
                    boss.shakeC = 0.4f;
                    boss.flashWhite = 0.2f;
                } else {
                    boss.playerHp = std::max(0, boss.playerHp - DMG_TO_PLAYER);
                    boss.shakeP = 0.4f;
                    boss.flashRed = 0.25f;
                }
                if (!voicePlaying()) speak(perfect ? bark("hit") : bark("hurt"));
            }
        } else {
            boss.resultT += dt;
            if (boss.resultT >= RESULT_TIME) {
                if (boss.chronosHp <= 0)      setPhase(P_DYING);
                else if (boss.playerHp <= 0)  setPhase(P_LOST);
                else                          newRound();
            }
        }
        break;

    case P_LOST:
        if (advance()) startBattle(state);   // retry with full HP
        break;

    case P_DYING:
        if (advance() && ++boss.line >= DYING_COUNT) {
            setPhase(P_MELTDOWN);
            boss.countdown = MELTDOWN_SECS;
            boss.ePressed = false;
        }
        break;

    case P_MELTDOWN:
        boss.countdown -= dt;
        if (!boss.ePressed) {
            if (IsKeyPressed(KEY_E)) boss.ePressed = true;
        } else if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
                   (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
                    CheckCollisionPointRec(mousePos, stopwatchBtn))) {
            setPhase(P_SENT_AWAY);
            break;
        }
        if (boss.countdown <= 0.0f) setPhase(P_BOOM);
        break;

    case P_BOOM:
        if (boss.t > 1.5f && advance()) {
            setPhase(P_MELTDOWN);
            boss.countdown = MELTDOWN_SECS;
            boss.ePressed = false;
        }
        break;

    case P_SENT_AWAY:
        if (boss.t >= 2.2f) setPhase(P_OUTRO);
        break;

    case P_OUTRO:
        if (advance() && ++boss.line >= OUTRO_COUNT) {
            endBoss(state);
            return;
        }
        break;
    }

    // ---- voice: trigger a line whenever the phase/line changes ----
    const std::string vid = voiceIdFor();
    if (vid != g_lastVoice) {
        g_lastVoice = vid;
        if (!vid.empty()) speak(vid);
    }

    // ------------------------------ draw ------------------------------

    // ------------------------------ draw ------------------------------
    const Color calm = { 200, 200, 200, 255 };

    if (boss.phase == P_CONFRONT || boss.phase == P_CHOICE || boss.phase == P_REPLY) {
        drawBackdrop(false);
        drawChronos(state, 120, 72, 96, calm, 0.0f);
        if (boss.phase == P_CONFRONT) {
            dialogueBox("CHRONOS", chronosAccent, CONFRONT[std::min(boss.line, CONFRONT_COUNT - 1)]);
        } else if (boss.phase == P_REPLY) {
            if (boss.line == 0) dialogueBox(playerName(state), playerAccent, CHOICES[boss.choice]);
            else                dialogueBox("CHRONOS", chronosAccent, REPLY[boss.choice]);
        } else {
            DrawRectangle(10, 150, SCREEN_WIDTH - 20, 80, Color{ 12, 10, 16, 245 });
            DrawRectangleLines(10, 150, SCREEN_WIDTH - 20, 80, playerAccent);
            DrawText(playerName(state), 20, 156, 8, playerAccent);
            DrawText("How do I answer him?", 20, 166 - 2, 7, RAYWHITE);
            for (int i = 0; i < 2; ++i) {
                const bool sel = boss.choice == i;
                DrawRectangleRec(choiceRects[i], sel ? Color{ 20, 90, 60, 235 } : Color{ 20, 20, 30, 235 });
                DrawRectangleLines((int)choiceRects[i].x, (int)choiceRects[i].y,
                                   (int)choiceRects[i].width, (int)choiceRects[i].height, playerAccent);
                DrawText(CHOICES[i], (int)choiceRects[i].x + 8, (int)choiceRects[i].y + 6, 8, RAYWHITE);
            }
        }
    }
    else if (boss.phase == P_BATTLE || boss.phase == P_LOST) {
        drawBackdrop(true);

        // Chronos, top right
        const float shC = boss.shakeC > 0.0f ? boss.shakeC * 14.0f : 0.0f;
        drawChronos(state, 168, 66, 72,
                    boss.shakeC > 0.0f ? Color{ 255, 120, 120, 255 } : calm, shC);
        hpBar(100, 20, 128, boss.chronosHp, Color{ 220, 50, 50, 255 }, "CHRONOS");

        // Player, bottom left
        const float shP = boss.shakeP > 0.0f ? boss.shakeP * 14.0f : 0.0f;
        const float pox = shP > 0.0f ? (float)std::sin(GetTime() * 90.0) * shP : 0.0f;
        DrawRectangle(18, 90, 52, 52, Color{ 10, 10, 14, 220 });
        DrawRectangleLines(18, 90, 52, 52, RED);
        DrawTexturePro(state->assets.players[state->selectedCharacter],
                       { 0, 0, (float)PLAYER_RECT_SIZE, (float)PLAYER_RECT_SIZE },
                       { 22.0f + pox, 94.0f, 44.0f, 44.0f }, { 0, 0 }, 0.0f, WHITE);
        hpBar(20, 222, 110, boss.playerHp, Color{ 60, 200, 90, 255 }, playerName(state));

        // Prompt + timing bar
        const PromptKey& pk = PROMPTS[boss.promptIndex];
        drawCentered("PRESS", 96, 8, RAYWHITE);
        const int kw = MeasureText(pk.name, 16);
        DrawRectangle((SCREEN_WIDTH - kw) / 2 - 8 + 40, 106, kw + 16, 26, BLACK);
        DrawRectangleLines((SCREEN_WIDTH - kw) / 2 - 8 + 40, 106, kw + 16, 26, GOLD);
        DrawText(pk.name, (SCREEN_WIDTH - kw) / 2 + 40, 112, 16, GOLD);

        const int barX = 30, barY = 158, barW = SCREEN_WIDTH - 60, barH = 20;
        DrawRectangle(barX, barY, barW, barH, BLACK);
        DrawRectangleLines(barX, barY, barW, barH, RED);
        const int zoneW = (int)(barW * boss.zoneHalf * 2.0f);
        DrawRectangle(barX + (int)(boss.zoneCenter * barW) - zoneW / 2, barY, zoneW, barH,
                      Color{ 255, 214, 60, 200 });
        DrawRectangle(barX + (int)(boss.cursor * barW) - 2, barY - 5, 4, barH + 10, WHITE);

        // time-left bar under the timing bar
        if (!boss.resolved) {
            const float left = std::max(0.0f, 1.0f - boss.roundT / boss.timeout);
            DrawRectangle(barX, barY + barH + 6, (int)(barW * left), 3,
                          left < 0.3f ? RED : Color{ 190, 170, 170, 255 });
        }

        if (!boss.resolved) {
            drawCentered("Hit the key when the line is in the gold!", 196, 6, Color{ 190, 170, 170, 255 });
        } else if (boss.lastPerfect) {
            drawCentered("PERFECT!", 186, 14, GREEN);
            DrawText(("-" + std::to_string(DMG_TO_CHRONOS)).c_str(), 190, 92, 10, GREEN);
        } else {
            drawCentered("MISS!", 186, 14, RED);
            DrawText(("-" + std::to_string(DMG_TO_PLAYER)).c_str(), 24, 78, 10, RED);
        }

        if (boss.phase == P_LOST) {
            DrawRectangle(20, 90, SCREEN_WIDTH - 40, 70, Color{ 10, 0, 4, 245 });
            DrawRectangleLines(20, 90, SCREEN_WIDTH - 40, 70, RED);
            drawCentered("OVERWHELMED", 102, 16, RED);
            if (blink()) drawCentered("[CLICK / ENTER] to try again", 136, 8, RAYWHITE);
        }
    }
    else if (boss.phase == P_DYING) {
        drawBackdrop(true);
        const unsigned char g = (unsigned char)std::max(40, 190 - boss.line * 50);
        drawChronos(state, 120, 72, 96, Color{ 255, g, g, 255 }, 1.0f + boss.line * 1.2f);
        dialogueBox("CHRONOS", chronosAccent, DYING[std::min(boss.line, DYING_COUNT - 1)]);
    }
    else if (boss.phase == P_MELTDOWN || boss.phase == P_BOOM) {
        const float prog = 1.0f - std::max(0.0f, boss.countdown) / MELTDOWN_SECS;
        drawBackdrop(true);
        const float pulse = 0.5f + 0.5f * (float)std::sin(GetTime() * 16.0);
        drawChronos(state, 120, 84, 96,
                    Color{ 255, (unsigned char)(40 + 90 * pulse), (unsigned char)(40 + 60 * pulse), 255 },
                    2.0f + prog * 5.0f);
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{ 255, 0, 0, (unsigned char)(prog * 90.0f) });

        const std::string secs = std::to_string((int)std::ceil(std::max(0.0f, boss.countdown)));
        drawCentered(secs.c_str(), 12, 30, RED);

        if (!boss.ePressed) {
            drawCentered("Chronos is unravelling... do something!", 160, 7, RAYWHITE);
            if (blink()) drawCentered("PRESS [E]!", 178, 14, GOLD);
        } else {
            const bool hover = CheckCollisionPointRec(mousePos, stopwatchBtn);
            DrawRectangleRec(stopwatchBtn, hover ? Color{ 255, 230, 120, 255 } : GOLD);
            DrawRectangleLines((int)stopwatchBtn.x, (int)stopwatchBtn.y,
                               (int)stopwatchBtn.width, (int)stopwatchBtn.height, WHITE);
            const char* label = "The Stopwatch of Armageddon";
            DrawText(label, (SCREEN_WIDTH - MeasureText(label, 8)) / 2, (int)stopwatchBtn.y + 12, 8, BLACK);
            if (blink()) drawCentered("[CLICK / ENTER]", 216, 6, RAYWHITE);
        }

        if (boss.phase == P_BOOM) {
            const float a = std::min(1.0f, boss.t * 1.2f);
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{ 255, 255, 255, (unsigned char)(a * 255.0f) });
            if (boss.t > 1.0f) {
                drawCentered("PURE DESTRUCTION", 100, 14, BLACK);
                if (boss.t > 1.5f && blink()) drawCentered("[CLICK / ENTER] try again", 130, 8, DARKGRAY);
            }
        }
    }
    else if (boss.phase == P_SENT_AWAY || boss.phase == P_OUTRO) {
        drawBackdrop(false);
        if (boss.phase == P_SENT_AWAY) {
            const float fade = std::max(0.0f, 1.0f - boss.t / 2.2f);
            drawChronos(state, 120, 84 - boss.t * 30.0f, 96.0f * fade + 8.0f,
                        Color{ 255, 120, 120, (unsigned char)(255.0f * fade) }, 4.0f);
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                          Color{ 255, 255, 255, (unsigned char)(std::max(0.0f, 1.0f - boss.t * 3.0f) * 255.0f) });
            if (boss.t > 1.4f) drawWatch(120, 150);
        } else {
            drawWatch(120, 100);
            dialogueBox("-", playerAccent, OUTRO[std::min(boss.line, OUTRO_COUNT - 1)]);
        }
    }

    // Hit flashes
    if (boss.flashRed > 0.0f)
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                      Color{ 255, 0, 0, (unsigned char)(std::min(1.0f, boss.flashRed / 0.25f) * 110.0f) });
    if (boss.flashWhite > 0.0f)
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                      Color{ 255, 255, 255, (unsigned char)(std::min(1.0f, boss.flashWhite / 0.2f) * 140.0f) });
}
