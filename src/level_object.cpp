#include "level_object.h"
#include "raymath.h"
#include <string.h>

static Model LoadModelByName(const char* name) {
    // Minimal library: map names -> files in resources/models/
    if (strcmp(name, "Mac") == 0) {
        return LoadModel("resources/MacintoshClassic.glb"); // existing in your repo root
    }
    // Fallback: a cube
    Mesh m = GenMeshCube(1,1,1);
    Model cube = LoadModelFromMesh(m);
    return cube;
}

void LO_Init(LevelObjects* lo) {
    lo->count = 0;
    lo->nextId = 1;
    lo->showBounds = true;
}

static BoundingBox ComputeModelBounds(const Model* model, Vector3 position, Vector3 scale) {
    // Approximation: use model's mesh bounds scaled and translated (no rotation for bounds simplicity)
    BoundingBox bb = { {0,0,0},{0,0,0} };
    if (model->meshCount > 0) {
        bb = GetMeshBoundingBox(model->meshes[0]);
    } else {
        bb.min = (Vector3){-0.5f,-0.5f,-0.5f};
        bb.max = (Vector3){ 0.5f, 0.5f, 0.5f};
    }
    bb.min = Vector3Multiply(bb.min, scale);
    bb.max = Vector3Multiply(bb.max, scale);
    bb.min = Vector3Add(bb.min, position);
    bb.max = Vector3Add(bb.max, position);
    return bb;
}

void LO_RecalcBounds(LevelObject* o) {
    o->bounds = ComputeModelBounds(&o->model, o->position, o->scale);
}

int LO_Add(LevelObjects* lo, const char* name, Vector3 pos, Vector3 rot, Vector3 scale) {
    if (lo->count >= LO_MAX) return -1;
    int idx = lo->count++;
    LevelObject* o = &lo->items[idx];
    o->id = lo->nextId++;
    strncpy(o->name, name, LO_NAME_MAX-1);
    o->name[LO_NAME_MAX-1] = '\0';
    o->position = pos;
    o->rotation = rot;
    o->scale = scale;
    o->model = LoadModelByName(name);
    LO_RecalcBounds(o);
    return o->id;
}

bool LO_Remove(LevelObjects* lo, int id) {
    for (int i = 0; i < lo->count; i++) {
        if (lo->items[i].id == id) {
            // unload model to avoid leaks
            UnloadModel(lo->items[i].model);
            // compact
            lo->items[i] = lo->items[lo->count - 1];
            lo->count--;
            return true;
        }
    }
    return false;
}

int LO_Duplicate(LevelObjects* lo, int id) {
    const LevelObject* src = LO_GetByIdConst(lo, id);
    if (!src) return -1;
    int newId = LO_Add(lo, src->name, Vector3Add(src->position,(Vector3){0.5f,0,0}), src->rotation, src->scale);
    return newId;
}

LevelObject* LO_GetById(LevelObjects* lo, int id) {
    for (int i = 0; i < lo->count; i++) if (lo->items[i].id == id) return &lo->items[i];
    return NULL;
}

const LevelObject* LO_GetByIdConst(const LevelObjects* lo, int id) {
    for (int i = 0; i < lo->count; i++) if (lo->items[i].id == id) return &lo->items[i];
    return NULL;
}