# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

This is a C++17 game using CMake with raylib (fetched via FetchContent).

```bash
# Configure (first time or after CMakeLists.txt changes)
cmake -B build -G "MinGW Makefiles"   # Windows with MinGW
cmake -B build                         # Linux/macOS

# Build
cmake --build build

# Run the game
./build/masquerade-panic.exe          # Windows
./build/masquerade-panic              # Linux/macOS
```

Alternative: Use VSCode build tasks (Ctrl+Shift+B) which use Premake/Make.

## Architecture

**Masquerade Panic** is a survival game where the player must avoid a killer among masked NPCs and reach the exit door before time runs out.

### Core Files (src/)

- **Entity.h** - Entity struct with position, velocity, type, and state. Entity types: `ENTITY_PLAYER`, `ENTITY_NPC`, `ENTITY_KILLER`, `ENTITY_EXIT_DOOR`
- **GameState.h** - Central state container holding all entities in a vector, camera, timer, and game constants. Quick access to player/killer/exit via stored indices
- **Utils.h** - Math helpers (distance, direction, collision), random generators, and position utilities
- **main.cpp** - Game loop, input handling, entity updates (player movement, NPC wander, killer tracking), rendering

### Game Constants (in GameState.h)

- Map: 2000x2000 pixels
- 50 NPCs with random wander behavior
- Killer uses "Panic Mode": speed increases from 70 to 120 as timer counts down
- 30-second timer

### Entity Pattern

Entities are stored in `GameState.entities` vector. Access specific entities via:
```cpp
Entity* player = GetPlayer(state);
Entity* killer = GetKiller(state);
Entity* exit = GetExitDoor(state);
```

### Rendering

Uses raylib's 2D mode with Camera2D for smooth follow. Entities drawn as simple stick figures (player plain, NPCs with masks, killer with creepy smile).
