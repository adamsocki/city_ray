#include "raylib.h"
#include "src/zayn.h"


int main(void)
{




    // Initialize window
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "raylib - Hello World");

    SetTargetFPS(60);


    Zayn zaynInstance;
    Init_Zayn(&zaynInstance);

    // Main game loop
    while (!WindowShouldClose())
    {
        Update_Zayn(&zaynInstance);

        
        BeginDrawing();
            ClearBackground(BLACK);

            BeginMode3D(zaynInstance.camera);
            // ClearBackground(RAYWHITE);
            DrawModel(zaynInstance.car, (Vector3){ 0.0f, 0.0f, 0.0f }, 0.25f, WHITE);  
            EndMode3D(); 
            DrawText("Hello, raylib!", 190, 200, 20, LIGHTGRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
