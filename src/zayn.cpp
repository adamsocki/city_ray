#include "zayn.h"
#include "camera.h"

void Init_Zayn(Zayn* zayn) 
{
    zayn->isRunning = true;
    zayn->inZone_001 = false;

    Init_Camera(&zayn->camera);
    
    zayn->car = LoadModel("resources/MacintoshClassic.glb");
}

void Update_Zayn(Zayn* zayn) 
{
    
    Update_Camera(&zayn->camera);
    
}