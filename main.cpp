#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "src/zayn.h"
#include "src/editor/editor.h"
#include "src/level_object.h"       
#include "src/level_serializer.h"

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "raylib - Hello World");

    GuiLoadStyle("resources/raygui/style_jungle.rgs");

    // Play mode state
    Zayn zaynInstance;
    Init_Zayn(&zaynInstance);

    // Editor state
    EditorState editor;
    Editor_Init(&editor, screenWidth, screenHeight);

    LevelObjects level;
    LO_Init(&level);

    bool showMessageBox = false;

    while (!WindowShouldClose())
    {
        // Toggle editor with Tab
        if (IsKeyPressed(KEY_TAB)) {
            printf("Editor toggled\n");
            Editor_Toggle(&editor);
        }

        // Logic
        if (editor.enabled) {
            Editor_Update(&editor, &level);
        } else {
            Update_Zayn(&zaynInstance);
        }

        BeginDrawing();
            ClearBackground(BLACK);

            if (!editor.enabled)
            {
                if (zaynInstance.inZone_001)
                {
                    if (GuiButton((Rectangle){ 24, 24, 120, 30 }, "#191#Show Message")) showMessageBox = true;
                }

                BeginMode3D(zaynInstance.camera);

                // Draw grid floor for orientation
                DrawGrid(20, 1.0f);

                DrawModel(zaynInstance.car, (Vector3){ 1.0f, 0.0f, -5.5f }, 10.0f, WHITE);

                // Draw bounding boxes
                DrawPlayerBounds(&zaynInstance);

                EndMode3D();

                // Draw crosshair at center of screen
                int centerX = screenWidth / 2;
                int centerY = screenHeight / 2;
                GuiDrawIcon(ICON_TARGET_POINT, centerX - 8, centerY - 8, 1, WHITE);

                // Draw controls info
                DrawText("Play: WASD | Q/E up/down | B bounds | Tab editor", 10, screenHeight - 30, 16, LIGHTGRAY);
                if (zaynInstance.showBounds) {
                    DrawText("Bounding Box: ON", 10, screenHeight - 50, 16, GREEN);
                } else {
                    DrawText("Bounding Box: OFF", 10, screenHeight - 50, 16, RED);
                }
            }
            else
            {
                // Editor 3D + UI
                Editor_Render3D(&editor, &level);
                Editor_RenderUI(&editor, &level, screenWidth, screenHeight);
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
