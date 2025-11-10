#ifndef ZAYN_H
#define ZAYN_H

#include "raylib.h"


struct Zayn {
    bool isRunning;

    bool inZone_001;
    Camera3D camera;

    Model car;
    
    // Player bounding box
    BoundingBox playerBounds;
    Vector3 playerSize;  // Width, Height, Depth of the bounding box
    bool showBounds;     // Debug flag to show bounding box
};

void Init_Zayn(Zayn* zayn);
void Update_Zayn(Zayn* zayn);

// Bounding box functions
void UpdatePlayerBounds(Zayn* zayn);
void DrawPlayerBounds(Zayn* zayn);
bool CheckCollisionWithBounds(Zayn* zayn, BoundingBox otherBounds);

#endif // ZAYN_H