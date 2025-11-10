#include "zayn.h"
#include "camera.h"
#include "raymath.h"

void Init_Zayn(Zayn* zayn)
{
    zayn->isRunning = true;
    zayn->inZone_001 = false;

    Init_Camera(&zayn->camera);

    zayn->car = LoadModel("resources/MacintoshClassic.glb");
    
    // Initialize player bounding box
    zayn->playerSize = (Vector3){ 1.0f, 2.0f, 1.0f };  // Width, Height, Depth
    zayn->showBounds = true;  // Show bounds by default for debugging
    UpdatePlayerBounds(zayn);

    Init_Roadway(&zayn->roadway, 6, 8.0f, 1.5f);
}

void Update_Zayn(Zayn* zayn)
{
    Update_Camera(&zayn->camera);
    
    // Update player bounding box position
    UpdatePlayerBounds(zayn);
    
    // Toggle bounding box visibility with B key
    if (IsKeyPressed(KEY_B)) {
        zayn->showBounds = !zayn->showBounds;
    }
}

void UpdatePlayerBounds(Zayn* zayn)
{
    // Calculate bounding box min and max based on camera position and player size
    Vector3 halfSize = Vector3Scale(zayn->playerSize, 0.5f);
    
    zayn->playerBounds.min = Vector3Subtract(zayn->camera.position, halfSize);
    zayn->playerBounds.max = Vector3Add(zayn->camera.position, halfSize);
}

void DrawPlayerBounds(Zayn* zayn)
{
    if (!zayn->showBounds) return;
    
    // Draw wireframe bounding box
    Vector3 center = zayn->camera.position;
    Vector3 size = zayn->playerSize;
    
    // Draw the bounding box as a wireframe cube
    DrawCubeWires(center, size.x, size.y, size.z, GREEN);
    
    // Draw corner markers for better visibility
    Color cornerColor = { 255, 0, 0, 255 }; // Red corners
    float cornerSize = 0.1f;
    
    // Draw 8 corners of the bounding box
    Vector3 corners[8] = {
        { zayn->playerBounds.min.x, zayn->playerBounds.min.y, zayn->playerBounds.min.z },
        { zayn->playerBounds.max.x, zayn->playerBounds.min.y, zayn->playerBounds.min.z },
        { zayn->playerBounds.min.x, zayn->playerBounds.max.y, zayn->playerBounds.min.z },
        { zayn->playerBounds.max.x, zayn->playerBounds.max.y, zayn->playerBounds.min.z },
        { zayn->playerBounds.min.x, zayn->playerBounds.min.y, zayn->playerBounds.max.z },
        { zayn->playerBounds.max.x, zayn->playerBounds.min.y, zayn->playerBounds.max.z },
        { zayn->playerBounds.min.x, zayn->playerBounds.max.y, zayn->playerBounds.max.z },
        { zayn->playerBounds.max.x, zayn->playerBounds.max.y, zayn->playerBounds.max.z }
    };
    
    for (int i = 0; i < 8; i++) {
        DrawCube(corners[i], cornerSize, cornerSize, cornerSize, cornerColor);
    }
}

bool CheckCollisionWithBounds(Zayn* zayn, BoundingBox otherBounds)
{
    return CheckCollisionBoxes(zayn->playerBounds, otherBounds);
}

BoundingBox GetCarBounds(Zayn* zayn)
{
    // Get the car's bounding box (approximate size for the Macintosh Classic model)
    Vector3 carPosition = { 1.0f, 0.0f, -5.5f };
    float carScale = 10.0f;
    Vector3 carSize = { 2.0f * carScale, 1.5f * carScale, 1.0f * carScale }; // Approximate dimensions
    
    BoundingBox carBounds;
    Vector3 halfSize = Vector3Scale(carSize, 0.5f);
    carBounds.min = Vector3Subtract(carPosition, halfSize);
    carBounds.max = Vector3Add(carPosition, halfSize);
    
    return carBounds;
}

bool CheckPlayerCollision(Zayn* zayn, Vector3 newPosition)
{
    // Create a temporary bounding box for the new position
    Vector3 halfSize = Vector3Scale(zayn->playerSize, 0.5f);
    BoundingBox tempBounds;
    tempBounds.min = Vector3Subtract(newPosition, halfSize);
    tempBounds.max = Vector3Add(newPosition, halfSize);
    
    // Check collision with car
    BoundingBox carBounds = GetCarBounds(zayn);
    if (CheckCollisionBoxes(tempBounds, carBounds)) {
        return true; // Collision detected
    }
    
    // Add more collision checks here for other objects
    
    return false; // No collision
}

// Wrapper function for collision checking that can be passed to camera update
bool PlayerCollisionCheck(Vector3 newPosition)
{
    // This is a global function that will be used by the camera
    // We'll need to access the zayn instance somehow
    // For now, we'll use a simple approach with the car bounds
    Vector3 halfSize = { 0.5f, 1.0f, 0.5f }; // Default player size
    BoundingBox tempBounds;
    tempBounds.min = Vector3Subtract(newPosition, halfSize);
    tempBounds.max = Vector3Add(newPosition, halfSize);
    
    // Check collision with car (hardcoded for now)
    Vector3 carPosition = { 1.0f, 0.0f, -5.5f };
    float carScale = 10.0f;
    Vector3 carSize = { 2.0f * carScale, 1.5f * carScale, 1.0f * carScale };
    
    BoundingBox carBounds;
    Vector3 carHalfSize = Vector3Scale(carSize, 0.5f);
    carBounds.min = Vector3Subtract(carPosition, carHalfSize);
    carBounds.max = Vector3Add(carPosition, carHalfSize);
    
    if (CheckCollisionBoxes(tempBounds, carBounds)) {
        return true; // Collision detected
    }
    
    return false; // No collision
}
=======
}
>>>>>>> a1415fa0bcd23cba78fafd4a6624d711d725fe9a
