#ifndef ENTITY_H
#define ENTITY_H

#include "raylib.h"

// Entity type constants
enum EntityType {
    ENTITY_PLAYER = 0,
    ENTITY_NPC,
    ENTITY_KILLER,
    ENTITY_EXIT_DOOR
};

struct Entity {
    Vector2 pos;
    Vector2 velocity;
    int type;
    bool active;
    bool hasMask;
    float wanderTimer;
};

// Helper to create a new entity with default values
inline Entity CreateEntity(Vector2 position, int entityType) {
    Entity e;
    e.pos = position;
    e.velocity = {0.0f, 0.0f};
    e.type = entityType;
    e.active = true;
    e.hasMask = (entityType == ENTITY_NPC); // Only NPCs have masks
    e.wanderTimer = 0.0f;
    return e;
}

#endif // ENTITY_H
