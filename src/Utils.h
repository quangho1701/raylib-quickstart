#ifndef UTILS_H
#define UTILS_H

#include "raylib.h"
#include "raymath.h"

// Calculate distance between two points
inline float Distance(Vector2 a, Vector2 b) {
    return Vector2Distance(a, b);
}

// Calculate squared distance (faster, avoids sqrt)
inline float DistanceSquared(Vector2 a, Vector2 b) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    return dx * dx + dy * dy;
}

// Normalize a vector (safe version that handles zero-length)
inline Vector2 NormalizeSafe(Vector2 v) {
    float length = Vector2Length(v);
    if (length > 0.0f) {
        return Vector2Scale(v, 1.0f / length);
    }
    return {0.0f, 0.0f};
}

// Get direction vector from a to b (normalized)
inline Vector2 DirectionTo(Vector2 from, Vector2 to) {
    return NormalizeSafe(Vector2Subtract(to, from));
}

// Collision check between two circles
inline bool CheckCircleCollision(Vector2 center1, float radius1, Vector2 center2, float radius2) {
    return CheckCollisionCircles(center1, radius1, center2, radius2);
}

// Collision check between a point and a circle
inline bool CheckPointInCircle(Vector2 point, Vector2 center, float radius) {
    return CheckCollisionPointCircle(point, center, radius);
}

// Collision check between a point and a rectangle
inline bool CheckPointInRect(Vector2 point, Rectangle rect) {
    return CheckCollisionPointRec(point, rect);
}

// Clamp a position within bounds
inline Vector2 ClampPosition(Vector2 pos, float minX, float minY, float maxX, float maxY) {
    pos.x = Clamp(pos.x, minX, maxX);
    pos.y = Clamp(pos.y, minY, maxY);
    return pos;
}

// Random float in range [min, max]
inline float RandomFloat(float min, float max) {
    return min + (float)GetRandomValue(0, 10000) / 10000.0f * (max - min);
}

// Random position within bounds
inline Vector2 RandomPosition(float minX, float minY, float maxX, float maxY) {
    return {RandomFloat(minX, maxX), RandomFloat(minY, maxY)};
}

#endif // UTILS_H
