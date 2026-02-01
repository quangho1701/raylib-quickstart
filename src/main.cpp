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

// Draw a simple stick figure for the player
void DrawPlayer(Entity& player) {
    float x = player.pos.x;
    float y = player.pos.y;

    // Head (circle)
    DrawCircleLines((int)x, (int)(y - 20), 10, BLACK);

    // Body (line)
    DrawLine((int)x, (int)(y - 10), (int)x, (int)(y + 15), BLACK);

    // Arms (line)
    DrawLine((int)(x - 12), (int)y, (int)(x + 12), (int)y, BLACK);

    // Legs (lines)
    DrawLine((int)x, (int)(y + 15), (int)(x - 10), (int)(y + 30), BLACK);
    DrawLine((int)x, (int)(y + 15), (int)(x + 10), (int)(y + 30), BLACK);
}

// Draw NPC (stick figure with mask over face)
void DrawNPC(Entity& npc) {
    float x = npc.pos.x;
    float y = npc.pos.y;

    // Head (circle)
    DrawCircleLines((int)x, (int)(y - 20), 10, BLACK);

    // Mask (rectangle over face)
    DrawRectangleLines((int)(x - 8), (int)(y - 26), 16, 12, BLACK);

    // Body (line)
    DrawLine((int)x, (int)(y - 10), (int)x, (int)(y + 15), BLACK);

    // Arms (line)
    DrawLine((int)(x - 12), (int)y, (int)(x + 12), (int)y, BLACK);

    // Legs (lines)
    DrawLine((int)x, (int)(y + 15), (int)(x - 10), (int)(y + 30), BLACK);
    DrawLine((int)x, (int)(y + 15), (int)(x + 10), (int)(y + 30), BLACK);
}

// Draw Killer (stick figure with creepy smile)
void DrawKiller(Entity& killer) {
    float x = killer.pos.x;
    float y = killer.pos.y;

    // Head (circle)
    DrawCircleLines((int)x, (int)(y - 20), 10, BLACK);

    // Creepy smile (curved line using multiple segments)
    // Draw a wide curved smile
    DrawLine((int)(x - 6), (int)(y - 18), (int)(x - 3), (int)(y - 15), BLACK);
    DrawLine((int)(x - 3), (int)(y - 15), (int)(x + 3), (int)(y - 15), BLACK);
    DrawLine((int)(x + 3), (int)(y - 15), (int)(x + 6), (int)(y - 18), BLACK);

    // Eyes (small dots)
    DrawCircle((int)(x - 4), (int)(y - 22), 2, BLACK);
    DrawCircle((int)(x + 4), (int)(y - 22), 2, BLACK);

    // Body (line)
    DrawLine((int)x, (int)(y - 10), (int)x, (int)(y + 15), BLACK);

    // Arms (line)
    DrawLine((int)(x - 12), (int)y, (int)(x + 12), (int)y, BLACK);

    // Legs (lines)
    DrawLine((int)x, (int)(y + 15), (int)(x - 10), (int)(y + 30), BLACK);
    DrawLine((int)x, (int)(y + 15), (int)(x + 10), (int)(y + 30), BLACK);
}

// Draw Exit Door
void DrawExitDoor(Entity& door) {
    float x = door.pos.x;
    float y = door.pos.y;

    // Door rectangle (green outline as per plan)
    Rectangle doorRect = {
        x - EXIT_DOOR_WIDTH / 2.0f,
        y - EXIT_DOOR_HEIGHT / 2.0f,
        EXIT_DOOR_WIDTH,
        EXIT_DOOR_HEIGHT
    };
    DrawRectangleLinesEx(doorRect, 3.0f, GREEN);

    // Door knob (positioned relative to smaller door)
    DrawCircle((int)(x + 20), (int)y, 3, GREEN);

    // "EXIT" text (smaller to fit the door)
    DrawText("EXIT", (int)(x - 18), (int)(y - 8), 16, GREEN);
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

// Draw the game world (map bounds indicator)
void DrawWorld() {
    // Draw map boundary
    DrawRectangleLines(0, 0, (int)MAP_WIDTH, (int)MAP_HEIGHT, LIGHTGRAY);

    // Draw grid for visual reference
    for (int x = 0; x <= (int)MAP_WIDTH; x += 200) {
        DrawLine(x, 0, x, (int)MAP_HEIGHT, LIGHTGRAY);
    }
    for (int y = 0; y <= (int)MAP_HEIGHT; y += 200) {
        DrawLine(0, y, (int)MAP_WIDTH, y, LIGHTGRAY);
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
