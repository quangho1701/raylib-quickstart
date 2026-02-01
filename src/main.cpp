#include "raylib.h"
#include "raymath.h"
#include "Entity.h"
#include "GameState.h"
#include "Utils.h"

// Initialize/reset the game with all entities
void InitGame(GameState& state) {
    // Clear existing entities
    state.entities.clear();
    state.timer = GAME_MAX_TIME;
    state.gameOver = false;
    state.gameWon = false;

    // Spawn Player at center
    Vector2 playerPos = {MAP_WIDTH / 2.0f, MAP_HEIGHT / 2.0f};
    Entity player = CreateEntity(playerPos, ENTITY_PLAYER);
    state.entities.push_back(player);
    state.playerIndex = 0;

    // Spawn 50 NPCs at random positions
    for (int i = 0; i < NPC_COUNT; i++) {
        Vector2 npcPos = RandomPosition(50.0f, 50.0f, MAP_WIDTH - 50.0f, MAP_HEIGHT - 50.0f);
        Entity npc = CreateEntity(npcPos, ENTITY_NPC);
        npc.wanderTimer = RandomFloat(0.0f, NPC_WANDER_MAX_TIME);  // Stagger initial timers
        npc.velocity = RandomVelocity(NPC_SPEED);
        state.entities.push_back(npc);
    }

    // Spawn Killer at random position > 400px away from player
    Vector2 killerPos;
    do {
        killerPos = RandomPosition(50.0f, 50.0f, MAP_WIDTH - 50.0f, MAP_HEIGHT - 50.0f);
    } while (Distance(killerPos, playerPos) < KILLER_MIN_SPAWN_DISTANCE);

    Entity killer = CreateEntity(killerPos, ENTITY_KILLER);
    state.entities.push_back(killer);
    state.killerIndex = (int)state.entities.size() - 1;

    // Spawn Exit Door at random edge, but far enough from player
    Vector2 exitPos;
    do {
        exitPos = RandomEdgePosition(MAP_WIDTH, MAP_HEIGHT, EXIT_DOOR_WIDTH, EXIT_DOOR_HEIGHT);
    } while (Distance(exitPos, playerPos) < EXIT_DOOR_MIN_SPAWN_DISTANCE);

    Entity exitDoor = CreateEntity(exitPos, ENTITY_EXIT_DOOR);
    state.entities.push_back(exitDoor);
    state.exitDoorIndex = (int)state.entities.size() - 1;

    // Set initial camera target to player position
    state.camera.target = playerPos;
}

// Update player movement based on WASD input
void UpdatePlayer(GameState& state, float deltaTime) {
    Entity* player = GetPlayer(state);
    if (!player || !player->active) return;

    // Reset velocity
    player->velocity = {0.0f, 0.0f};

    // WASD input
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
        player->velocity.y = -1.0f;
    }
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
        player->velocity.y = 1.0f;
    }
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
        player->velocity.x = -1.0f;
    }
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
        player->velocity.x = 1.0f;
    }

    // Normalize diagonal movement
    player->velocity = NormalizeSafe(player->velocity);

    // Apply velocity with speed and delta time
    player->pos.x += player->velocity.x * PLAYER_SPEED * deltaTime;
    player->pos.y += player->velocity.y * PLAYER_SPEED * deltaTime;

    // Constrain player to map bounds (0,0 to 2000,2000)
    player->pos = ClampPosition(player->pos, 0.0f, 0.0f, MAP_WIDTH, MAP_HEIGHT);
}

// Update camera to follow player with smooth lerp
void UpdateCamera(GameState& state, float deltaTime) {
    Entity* player = GetPlayer(state);
    if (!player) return;

    // Lerp camera target toward player position
    float lerpFactor = CAMERA_SMOOTHING * deltaTime;
    lerpFactor = Clamp(lerpFactor, 0.0f, 1.0f);

    state.camera.target.x = Lerp(state.camera.target.x, player->pos.x, lerpFactor);
    state.camera.target.y = Lerp(state.camera.target.y, player->pos.y, lerpFactor);

    // Clamp camera so visual area stays within map bounds
    // Camera offset is the screen center point
    float halfScreenWidth = state.camera.offset.x / state.camera.zoom;
    float halfScreenHeight = state.camera.offset.y / state.camera.zoom;

    state.camera.target.x = Clamp(state.camera.target.x, halfScreenWidth, MAP_WIDTH - halfScreenWidth);
    state.camera.target.y = Clamp(state.camera.target.y, halfScreenHeight, MAP_HEIGHT - halfScreenHeight);
}

// Update NPC wander behavior
void UpdateNPCs(GameState& state, float deltaTime) {
    for (Entity& entity : state.entities) {
        if (entity.type != ENTITY_NPC || !entity.active) continue;

        // Decrease wander timer
        entity.wanderTimer -= deltaTime;

        // When timer expires, pick a new random direction
        if (entity.wanderTimer <= 0.0f) {
            entity.velocity = RandomVelocity(NPC_SPEED);
            entity.wanderTimer = RandomFloat(NPC_WANDER_MIN_TIME, NPC_WANDER_MAX_TIME);
        }

        // Move NPC
        entity.pos.x += entity.velocity.x * deltaTime;
        entity.pos.y += entity.velocity.y * deltaTime;

        // Bounce off map edges (reverse velocity when hitting bounds)
        if (entity.pos.x < 50.0f || entity.pos.x > MAP_WIDTH - 50.0f) {
            entity.velocity.x = -entity.velocity.x;
            entity.pos.x = Clamp(entity.pos.x, 50.0f, MAP_WIDTH - 50.0f);
        }
        if (entity.pos.y < 50.0f || entity.pos.y > MAP_HEIGHT - 50.0f) {
            entity.velocity.y = -entity.velocity.y;
            entity.pos.y = Clamp(entity.pos.y, 50.0f, MAP_HEIGHT - 50.0f);
        }
    }
}

// Update Killer tracking behavior with Panic Mode
void UpdateKiller(GameState& state, float deltaTime) {
    Entity* killer = GetKiller(state);
    Entity* player = GetPlayer(state);
    if (!killer || !player || !killer->active || !player->active) return;

    // Calculate direction from killer to player
    Vector2 direction = DirectionTo(killer->pos, player->pos);

    // Panic Mode: killer speed increases as timer decreases
    // Speed formula: baseSpeed + (1 - timer/maxTimer) * bonusSpeed
    float panicFactor = 1.0f - (state.timer / GAME_MAX_TIME);
    float currentSpeed = KILLER_BASE_SPEED + (panicFactor * KILLER_BONUS_SPEED);

    // Apply velocity
    killer->velocity.x = direction.x * currentSpeed;
    killer->velocity.y = direction.y * currentSpeed;

    // Move killer
    killer->pos.x += killer->velocity.x * deltaTime;
    killer->pos.y += killer->velocity.y * deltaTime;

    // Constrain killer to map bounds
    killer->pos = ClampPosition(killer->pos, 0.0f, 0.0f, MAP_WIDTH, MAP_HEIGHT);
}

// Sketchbook style constants - "Diary of a Wimpy Kid" aesthetic
const float SKETCH_LINE_THICK = 2.0f;      // Bold sketchy lines
const float SKETCH_LINE_THIN = 1.5f;       // Thinner detail lines
const float FIGURE_SCALE = 1.3f;           // Slightly larger figures

// Draw a simple stick figure for the player (plain, no mask)
void DrawPlayer(Entity& player) {
    float x = player.pos.x;
    float y = player.pos.y;
    float s = FIGURE_SCALE;

    // Head (bold circle outline - drawn twice for sketchy effect)
    DrawCircleLines((int)x, (int)(y - 25*s), (int)(12*s), BLACK);
    DrawCircleLines((int)x, (int)(y - 25*s), (int)(11*s), BLACK);

    // Simple dot eyes
    DrawCircle((int)(x - 4*s), (int)(y - 27*s), 2*s, BLACK);
    DrawCircle((int)(x + 4*s), (int)(y - 27*s), 2*s, BLACK);

    // Body (thick line)
    DrawLineEx({x, y - 13*s}, {x, y + 18*s}, SKETCH_LINE_THICK, BLACK);

    // Arms (slightly angled for more character)
    DrawLineEx({x, y - 5*s}, {x - 15*s, y + 5*s}, SKETCH_LINE_THICK, BLACK);
    DrawLineEx({x, y - 5*s}, {x + 15*s, y + 5*s}, SKETCH_LINE_THICK, BLACK);

    // Legs (thick lines)
    DrawLineEx({x, y + 18*s}, {x - 12*s, y + 38*s}, SKETCH_LINE_THICK, BLACK);
    DrawLineEx({x, y + 18*s}, {x + 12*s, y + 38*s}, SKETCH_LINE_THICK, BLACK);
}

// Draw NPC (stick figure with masquerade mask over face)
void DrawNPC(Entity& npc) {
    float x = npc.pos.x;
    float y = npc.pos.y;
    float s = FIGURE_SCALE;

    // Head (bold circle outline - sketchy double line)
    DrawCircleLines((int)x, (int)(y - 25*s), (int)(12*s), BLACK);
    DrawCircleLines((int)x, (int)(y - 25*s), (int)(11*s), BLACK);

    // Masquerade mask (rectangle with eye holes) - covers upper face
    Rectangle maskRect = {x - 10*s, y - 32*s, 20*s, 12*s};
    DrawRectangleLinesEx(maskRect, SKETCH_LINE_THICK, BLACK);
    // Eye holes in mask (small circles)
    DrawCircle((int)(x - 5*s), (int)(y - 26*s), 2*s, BLACK);
    DrawCircle((int)(x + 5*s), (int)(y - 26*s), 2*s, BLACK);

    // Body (thick line)
    DrawLineEx({x, y - 13*s}, {x, y + 18*s}, SKETCH_LINE_THICK, BLACK);

    // Arms (slightly angled)
    DrawLineEx({x, y - 5*s}, {x - 15*s, y + 5*s}, SKETCH_LINE_THICK, BLACK);
    DrawLineEx({x, y - 5*s}, {x + 15*s, y + 5*s}, SKETCH_LINE_THICK, BLACK);

    // Legs (thick lines)
    DrawLineEx({x, y + 18*s}, {x - 12*s, y + 38*s}, SKETCH_LINE_THICK, BLACK);
    DrawLineEx({x, y + 18*s}, {x + 12*s, y + 38*s}, SKETCH_LINE_THICK, BLACK);
}

// Draw Killer (stick figure with creepy bezier smile - the horror element!)
void DrawKiller(Entity& killer) {
    float x = killer.pos.x;
    float y = killer.pos.y;
    float s = FIGURE_SCALE;

    // Head (bold circle outline - sketchy double line)
    DrawCircleLines((int)x, (int)(y - 25*s), (int)(12*s), BLACK);
    DrawCircleLines((int)x, (int)(y - 25*s), (int)(11*s), BLACK);

    // Creepy wide eyes (slightly larger, unsettling)
    DrawCircle((int)(x - 5*s), (int)(y - 28*s), 3*s, BLACK);
    DrawCircle((int)(x + 5*s), (int)(y - 28*s), 3*s, BLACK);
    // Tiny white pupils for that unhinged look
    DrawCircle((int)(x - 5*s), (int)(y - 28*s), 1*s, WHITE);
    DrawCircle((int)(x + 5*s), (int)(y - 28*s), 1*s, WHITE);

    // CREEPY BEZIER SMILE - the signature horror element
    // Wide, curved smile using DrawLineBezier for smooth creepiness
    Vector2 smileStart = {x - 8*s, y - 20*s};
    Vector2 smileEnd = {x + 8*s, y - 20*s};
    // Draw the smile curve (bezier with control point below for upward curve)
    DrawLineBezier(smileStart, smileEnd, SKETCH_LINE_THICK, BLACK);
    // Add curve endpoints going up for that sinister grin
    DrawLineEx({x - 8*s, y - 20*s}, {x - 10*s, y - 23*s}, SKETCH_LINE_THIN, BLACK);
    DrawLineEx({x + 8*s, y - 20*s}, {x + 10*s, y - 23*s}, SKETCH_LINE_THIN, BLACK);

    // Body (thick line)
    DrawLineEx({x, y - 13*s}, {x, y + 18*s}, SKETCH_LINE_THICK, BLACK);

    // Arms (slightly raised, menacing pose)
    DrawLineEx({x, y - 5*s}, {x - 18*s, y - 2*s}, SKETCH_LINE_THICK, BLACK);
    DrawLineEx({x, y - 5*s}, {x + 18*s, y - 2*s}, SKETCH_LINE_THICK, BLACK);

    // Legs (thick lines)
    DrawLineEx({x, y + 18*s}, {x - 12*s, y + 38*s}, SKETCH_LINE_THICK, BLACK);
    DrawLineEx({x, y + 18*s}, {x + 12*s, y + 38*s}, SKETCH_LINE_THICK, BLACK);
}

// Draw Exit Door (sketchy style - green stands out as the goal)
void DrawExitDoor(Entity& door) {
    float x = door.pos.x;
    float y = door.pos.y;

    // Door rectangle (sketchy double outline for hand-drawn feel)
    Rectangle doorRect = {
        x - EXIT_DOOR_WIDTH / 2.0f,
        y - EXIT_DOOR_HEIGHT / 2.0f,
        EXIT_DOOR_WIDTH,
        EXIT_DOOR_HEIGHT
    };
    DrawRectangleLinesEx(doorRect, SKETCH_LINE_THICK + 1, DARKGREEN);
    // Inner rectangle for sketchy effect
    Rectangle innerRect = {
        doorRect.x + 3, doorRect.y + 3,
        doorRect.width - 6, doorRect.height - 6
    };
    DrawRectangleLinesEx(innerRect, SKETCH_LINE_THIN, DARKGREEN);

    // Door panels (sketchy rectangles inside)
    float panelW = 30.0f;
    float panelH = 50.0f;
    DrawRectangleLinesEx({x - panelW/2, y - EXIT_DOOR_HEIGHT/2 + 15, panelW, panelH}, SKETCH_LINE_THIN, DARKGREEN);
    DrawRectangleLinesEx({x - panelW/2, y + 10, panelW, panelH}, SKETCH_LINE_THIN, DARKGREEN);

    // Door knob (sketchy circle)
    DrawCircleLines((int)(x + 25), (int)y, 5, DARKGREEN);
    DrawCircle((int)(x + 25), (int)y, 2, DARKGREEN);

    // "EXIT" text with arrow (hand-drawn style)
    DrawText("EXIT", (int)(x - 20), (int)(y - EXIT_DOOR_HEIGHT/2 - 25), 20, DARKGREEN);
    // Arrow pointing down
    DrawLineEx({x, y - EXIT_DOOR_HEIGHT/2 - 8}, {x, y - EXIT_DOOR_HEIGHT/2 + 5}, SKETCH_LINE_THICK, DARKGREEN);
    DrawLineEx({x - 5, y - EXIT_DOOR_HEIGHT/2}, {x, y - EXIT_DOOR_HEIGHT/2 + 5}, SKETCH_LINE_THIN, DARKGREEN);
    DrawLineEx({x + 5, y - EXIT_DOOR_HEIGHT/2}, {x, y - EXIT_DOOR_HEIGHT/2 + 5}, SKETCH_LINE_THIN, DARKGREEN);
}

// Draw all entities
void DrawEntities(GameState& state) {
    // Draw exit door first (so it's behind other entities)
    Entity* exitDoor = GetExitDoor(state);
    if (exitDoor && exitDoor->active) {
        DrawExitDoor(*exitDoor);
    }

    // Draw all entities
    for (Entity& entity : state.entities) {
        if (!entity.active) continue;

        switch (entity.type) {
            case ENTITY_PLAYER:
                DrawPlayer(entity);
                break;
            case ENTITY_NPC:
                DrawNPC(entity);
                break;
            case ENTITY_KILLER:
                DrawKiller(entity);
                break;
            default:
                break;
        }
    }
}

// Draw the game world - sketchbook paper style
void DrawWorld() {
    // Subtle paper texture - faint ruled lines like notebook paper
    Color lineColor = {220, 220, 220, 255};  // Very light gray

    // Horizontal ruled lines (like notebook paper)
    for (int y = 0; y <= (int)MAP_HEIGHT; y += 40) {
        DrawLineEx({0, (float)y}, {MAP_WIDTH, (float)y}, 1.0f, lineColor);
    }

    // Margin line on left (red, like real notebook)
    Color marginColor = {255, 200, 200, 255};  // Faint red/pink
    DrawLineEx({80, 0}, {80, MAP_HEIGHT}, 1.5f, marginColor);

    // Map boundary - sketchy double border
    DrawRectangleLinesEx({0, 0, MAP_WIDTH, MAP_HEIGHT}, 3.0f, LIGHTGRAY);
    DrawRectangleLinesEx({4, 4, MAP_WIDTH - 8, MAP_HEIGHT - 8}, 1.0f, LIGHTGRAY);

    // Corner doodles (like someone drew on their notebook)
    // Top-left corner scribble
    DrawLineEx({20, 20}, {50, 25}, SKETCH_LINE_THIN, LIGHTGRAY);
    DrawLineEx({50, 25}, {30, 40}, SKETCH_LINE_THIN, LIGHTGRAY);

    // Bottom-right corner spiral
    float cx = MAP_WIDTH - 60;
    float cy = MAP_HEIGHT - 60;
    for (int i = 0; i < 3; i++) {
        float r = 10.0f + i * 8.0f;
        DrawCircleLines((int)cx, (int)cy, (int)r, LIGHTGRAY);
    }
}

int main() {
    InitWindow(800, 600, "Masquerade Panic");
    SetTargetFPS(60);

    // Initialize game state and spawn all entities
    GameState state = CreateGameState();
    InitGame(state);

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        // Update game logic (only if game is still running)
        if (!state.gameOver && !state.gameWon) {
            UpdatePlayer(state, deltaTime);
            UpdateNPCs(state, deltaTime);
            UpdateKiller(state, deltaTime);
            UpdateCamera(state, deltaTime);
        }

        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode2D(state.camera);

        // Draw world
        DrawWorld();

        // Draw all entities
        DrawEntities(state);

        EndMode2D();

        // UI overlay - Timer display
        Entity* killer = GetKiller(state);
        float panicFactor = 1.0f - (state.timer / GAME_MAX_TIME);
        DrawText(TextFormat("SURVIVE: %.1fs", state.timer), 320, 10, 24, BLACK);

        // Debug info
        DrawText(TextFormat("Entities: %d", (int)state.entities.size()), 10, 10, 16, GRAY);
        if (killer) {
            DrawText(TextFormat("Killer Speed: %.0f", KILLER_BASE_SPEED + panicFactor * KILLER_BONUS_SPEED), 10, 30, 16, GRAY);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
