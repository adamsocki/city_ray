#include "zayn.h"
#include "camera.h"

void Init_Zayn(Zayn* zayn)
{
    zayn->isRunning = true;
    zayn->inZone_001 = false;

    Init_Camera(&zayn->camera);

    zayn->car = LoadModel("resources/MacintoshClassic.glb");

    Init_Roadway(&zayn->roadway, 6, 8.0f, 1.5f);
}

void Update_Zayn(Zayn* zayn)
{
    Update_Camera(&zayn->camera);
}
