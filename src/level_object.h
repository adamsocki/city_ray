#ifndef LEVEL_OBJECT_H
#define LEVEL_OBJECT_H

#include "raylib.h"
#include <stdbool.h>

#define LO_MAX 512
#define LO_NAME_MAX 32

typedef struct {
    int id;
    char name[LO_NAME_MAX];
    Vector3 position;
    Vector3 rotation;   // Euler degrees
    Vector3 scale;
    Model model;
    BoundingBox bounds;
} LevelObject;

typedef struct {
    LevelObject items[LO_MAX];
    int count;
    int nextId;
    bool showBounds;
} LevelObjects;

void LO_Init(LevelObjects* lo);
int  LO_Add(LevelObjects* lo, const char* name, Vector3 pos, Vector3 rot, Vector3 scale);
bool LO_Remove(LevelObjects* lo, int id);
int  LO_Duplicate(LevelObjects* lo, int id);
LevelObject* LO_GetById(LevelObjects* lo, int id);
const LevelObject* LO_GetByIdConst(const LevelObjects* lo, int id);
void LO_RecalcBounds(LevelObject* o);

#endif // LEVEL_OBJECT_H