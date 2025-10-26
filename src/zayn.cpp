#include "zayn.h"

void Init_Zayn(Zayn* zayn) 
{
    zayn->isRunning = true;

    zayn->camera.position = (Vector3){ 2.0f, -1.0f, 6.0f };
    zayn->camera.target = (Vector3){ 0.0f, 0.5f, 0.0f };
    zayn->camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    zayn->camera.fovy = 45.0f;
    zayn->camera.projection = CAMERA_PERSPECTIVE;

    zayn->car = LoadModel("resources/MacintoshClassic.glb");
}

void Update_Zayn(Zayn* zayn) 
{
    float cameraSpeed = 1.0f * GetFrameTime(); // Speed in units per second
    
    // WASD movement
    if (IsKeyDown(KEY_W)) zayn->camera.position.z -= cameraSpeed;
    if (IsKeyDown(KEY_S)) zayn->camera.position.z += cameraSpeed;
    if (IsKeyDown(KEY_A)) zayn->camera.position.x -= cameraSpeed;
    if (IsKeyDown(KEY_D)) zayn->camera.position.x += cameraSpeed;
    
    // Q/E for up/down
    if (IsKeyDown(KEY_Q)) zayn->camera.position.y -= cameraSpeed;
    if (IsKeyDown(KEY_E)) zayn->camera.position.y += cameraSpeed;
    
    // Update camera target to follow position
    zayn->camera.target = (Vector3){ 
        zayn->camera.position.x, 
        zayn->camera.position.y, 
        zayn->camera.position.z - 1.0f 
    };
}