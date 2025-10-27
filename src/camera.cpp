#include <iostream>
#include "camera.h"
#include "raymath.h"

// Camera rotation variables
float cameraPitch = 0.0f;
float cameraYaw = -90.0f;  // Start facing forward

void Init_Camera(Camera3D* camera)
{
    camera->position = (Vector3){ 2.0f, 0.5f, 6.0f };
    camera->target = (Vector3){ 0.0f, 0.5f, 0.0f };
    camera->up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera->fovy = 45.0f;
    camera->projection = CAMERA_PERSPECTIVE;

    // Disable cursor for FPS experience
    DisableCursor();
}


void Update_Camera(Camera3D* camera)
{
    float mouseSensitivity = 0.1f;
    float cameraSpeed = 5.0f * GetFrameTime(); // Speed in units per second

    // Mouse look
    Vector2 mouseDelta = GetMouseDelta();
    cameraYaw += mouseDelta.x * mouseSensitivity;
    cameraPitch -= mouseDelta.y * mouseSensitivity;

    // Clamp pitch to prevent camera flipping
    if (cameraPitch > 89.0f) cameraPitch = 89.0f;
    if (cameraPitch < -89.0f) cameraPitch = -89.0f;

    // Calculate direction vector from yaw and pitch
    Vector3 direction;
    direction.x = cosf(DEG2RAD * cameraYaw) * cosf(DEG2RAD * cameraPitch);
    direction.y = sinf(DEG2RAD * cameraPitch);
    direction.z = sinf(DEG2RAD * cameraYaw) * cosf(DEG2RAD * cameraPitch);

    // Normalize the direction
    direction = Vector3Normalize(direction);

    // Calculate right vector for strafing
    Vector3 forward = { direction.x, 0.0f, direction.z };
    forward = Vector3Normalize(forward);
    Vector3 right = Vector3CrossProduct(forward, camera->up);

    // WASD movement relative to camera direction
    if (IsKeyDown(KEY_W)) camera->position = Vector3Add(camera->position, Vector3Scale(forward, cameraSpeed));
    if (IsKeyDown(KEY_S)) camera->position = Vector3Subtract(camera->position, Vector3Scale(forward, cameraSpeed));
    if (IsKeyDown(KEY_A)) camera->position = Vector3Subtract(camera->position, Vector3Scale(right, cameraSpeed));
    if (IsKeyDown(KEY_D)) camera->position = Vector3Add(camera->position, Vector3Scale(right, cameraSpeed));

    // Q/E for up/down
    if (IsKeyDown(KEY_Q)) camera->position.y -= cameraSpeed;
    if (IsKeyDown(KEY_E)) camera->position.y += cameraSpeed;

    // std::cout << "Camera Position: (" << camera->position.x << ", " << camera->position.y << ", " << camera->position.z << ")\n";

    // Update camera target based on direction
    camera->target = Vector3Add(camera->position, direction);
}