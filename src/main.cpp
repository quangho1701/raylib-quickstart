#include "raylib.h"
#include "raymath.h"
#include "Entity.h"
#include "GameState.h"
#include "Utils.h"

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

    // Initialize game state
    GameState state = CreateGameState();

    // Create player at center of map
    Entity player = CreateEntity({MAP_WIDTH / 2.0f, MAP_HEIGHT / 2.0f}, ENTITY_PLAYER);
    state.entities.push_back(player);
    state.playerIndex = 0;

    // Set initial camera target to player position
    state.camera.target = state.entities[state.playerIndex].pos;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        // Update
        UpdatePlayer(state, deltaTime);
        UpdateCamera(state, deltaTime);

        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode2D(state.camera);

        // Draw world
        DrawWorld();

        // Draw player
        Entity* playerPtr = GetPlayer(state);
        if (playerPtr) {
            DrawPlayer(*playerPtr);
        }

        EndMode2D();

        // UI overlay
        DrawText("WASD or Arrow Keys to move", 10, 10, 20, DARKGRAY);
        DrawText(TextFormat("Player: (%.0f, %.0f)",
            state.entities[state.playerIndex].pos.x,
            state.entities[state.playerIndex].pos.y), 10, 35, 16, GRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
