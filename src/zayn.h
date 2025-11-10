#ifndef ZAYN_H
#define ZAYN_H

#include "raylib.h"
#include "roadway.h"

struct Zayn {
    bool isRunning;

    bool inZone_001;
    Camera3D camera;

    Model car;
    Roadway roadway;
};

void Init_Zayn(Zayn* zayn);
void Update_Zayn(Zayn* zayn);

#endif // ZAYN_H
