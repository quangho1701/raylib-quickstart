#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "Entity.h"
#include <vector>

// Game constants
const float MAP_WIDTH = 2000.0f;
const float MAP_HEIGHT = 2000.0f;
const float PLAYER_SPEED = 200.0f;
const float CAMERA_SMOOTHING = 5.0f;

struct GameState {
    float timer;
    bool gameOver;
    bool gameWon;
    std::vector<Entity> entities;

    // Camera
    Camera2D camera;

    // Player index in entities vector for quick access
    int playerIndex;
};

// Initialize a new game state with default values
inline GameState CreateGameState() {
    GameState state;
    state.timer = 30.0f;  // 30 second survival timer
    state.gameOver = false;
    state.gameWon = false;
    state.playerIndex = -1;

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

#endif // GAMESTATE_H
