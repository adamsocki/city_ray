#ifndef ZAYN_H
#define ZAYN_H

#include "raylib.h"


struct Zayn {
    bool isRunning;

    Camera3D camera;

    Model car;
};

void Init_Zayn(Zayn* zayn);
void Update_Zayn(Zayn* zayn);

#endif // ZAYN_H