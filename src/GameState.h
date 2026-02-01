#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "Entity.h"
#include <vector>

// Game constants
const float MAP_WIDTH = 2000.0f;
const float MAP_HEIGHT = 2000.0f;
const float PLAYER_SPEED = 200.0f;
const float CAMERA_SMOOTHING = 5.0f;

// AI constants
const int NPC_COUNT = 50;
const float NPC_SPEED = 50.0f;
const float NPC_WANDER_MIN_TIME = 1.0f;
const float NPC_WANDER_MAX_TIME = 3.0f;

// Killer constants
const float KILLER_BASE_SPEED = 70.0f;
const float KILLER_BONUS_SPEED = 50.0f;  // Added to base speed as timer decreases
const float KILLER_MIN_SPAWN_DISTANCE = 400.0f;

// Exit door constants
const float EXIT_DOOR_WIDTH = 60.0f;
const float EXIT_DOOR_HEIGHT = 100.0f;
const float EXIT_DOOR_MIN_SPAWN_DISTANCE = 800.0f;

// Game timer
const float GAME_MAX_TIME = 30.0f;

struct GameState {
    float timer;
    bool gameOver;
    bool gameWon;
    std::vector<Entity> entities;

    // Camera
    Camera2D camera;

    // Entity indices for quick access
    int playerIndex;
    int killerIndex;
    int exitDoorIndex;
};

// Initialize a new game state with default values
inline GameState CreateGameState() {
    GameState state;
    state.timer = GAME_MAX_TIME;
    state.gameOver = false;
    state.gameWon = false;
    state.playerIndex = -1;
    state.killerIndex = -1;
    state.exitDoorIndex = -1;

    // Initialize camera
    state.camera.target = {MAP_WIDTH / 2.0f, MAP_HEIGHT / 2.0f};
    state.camera.offset = {400.0f, 300.0f};  // Center of 800x600 window
    state.camera.rotation = 0.0f;
    state.camera.zoom = 1.0f;

    return state;
}

// Get pointer to player entity (returns nullptr if no player)
inline Entity* GetPlayer(GameState& state) {
    if (state.playerIndex >= 0 && state.playerIndex < (int)state.entities.size()) {
        return &state.entities[state.playerIndex];
    }
    return nullptr;
}

// Get pointer to killer entity (returns nullptr if no killer)
inline Entity* GetKiller(GameState& state) {
    if (state.killerIndex >= 0 && state.killerIndex < (int)state.entities.size()) {
        return &state.entities[state.killerIndex];
    }
    return nullptr;
}

// Get pointer to exit door entity (returns nullptr if no exit door)
inline Entity* GetExitDoor(GameState& state) {
    if (state.exitDoorIndex >= 0 && state.exitDoorIndex < (int)state.entities.size()) {
        return &state.entities[state.exitDoorIndex];
    }
    return nullptr;
}

#endif // GAMESTATE_H
