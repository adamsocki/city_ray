#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"


#include "src/zayn.h"


int main(void)
{




    // Initialize window
    const int screenWidth = 800;
    const int screenHeight = 450;
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "raylib - Hello World");

    // SetTargetFPS(60);

    Zayn zaynInstance;
    Init_Zayn(&zaynInstance);

    bool showMessageBox = false;
    GuiLoadStyle("resources/raygui/style_jungle.rgs");

    // Main game loop
    while (!WindowShouldClose())
    {
        // Logic
        Update_Zayn(&zaynInstance);

        // Render
        BeginDrawing();
            ClearBackground(BLACK);

            if (zaynInstance.inZone_001)
            {
                if (GuiButton((Rectangle){ 24, 24, 120, 30 }, "#191#Show Message")) showMessageBox = true;

            }

            BeginMode3D(zaynInstance.camera);

            // Draw grid floor for orientation
            DrawGrid(20, 1.0f);

            // Draw a reference plane at y = 0
            // DrawPlane((Vector3){ 0.0f, -1.0f, 0.0f }, (Vector2){ 20.0f, 20.0f }, DARKGRAY);

            DrawModel(zaynInstance.car, (Vector3){ 1.0f, 0.0f, -5.5f }, 10.0f, WHITE);

            EndMode3D();

            // Draw crosshair at center of screen
            int centerX = screenWidth / 2;
            int centerY = screenHeight / 2;
            GuiDrawIcon(ICON_TARGET_POINT, centerX - 8, centerY - 8, 1, WHITE);

            // DrawText("Hello, raylib!", 190, 200, 20, LIGHTGRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
