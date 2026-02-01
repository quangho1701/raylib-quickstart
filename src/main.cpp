#include "raylib.h"

int main() {
    InitWindow(800, 600, "Masquerade Panic");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Raylib + CMake Ready!", 250, 280, 30, DARKGRAY);
        DrawRectangle(375, 350, 50, 50, RED);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
