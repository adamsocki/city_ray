#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "src/zayn.h"

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "raylib - Hello World");

    Zayn zaynInstance;
    Init_Zayn(&zaynInstance);

    bool showMessageBox = false;
    GuiLoadStyle("resources/raygui/style_jungle.rgs");

    while (!WindowShouldClose())
    {
        Update_Zayn(&zaynInstance);

        BeginDrawing();
            ClearBackground(BLACK);

            if (zaynInstance.inZone_001)
            {
                if (GuiButton((Rectangle){ 24, 24, 120, 30 }, "#191#Show Message")) showMessageBox = true;
            }

            BeginMode3D(zaynInstance.camera);

            DrawGrid(20, 1.0f);
            Render_Roadway(zaynInstance.roadway, DARKGRAY);

            DrawModel(zaynInstance.car, (Vector3){ 1.0f, 0.0f, -5.5f }, 10.0f, WHITE);

            EndMode3D();

            int centerX = screenWidth / 2;
            int centerY = screenHeight / 2;
            GuiDrawIcon(ICON_TARGET_POINT, centerX - 8, centerY - 8, 1, WHITE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
