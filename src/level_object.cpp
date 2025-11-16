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

static BoundingBox ComputeModelBounds(const Model* model, Vector3 position, Vector3 scale, Vector3 rotation) {
    // Get the base mesh bounding box
    BoundingBox baseBB = { {0,0,0},{0,0,0} };
    if (model->meshCount > 0) {
        baseBB = GetMeshBoundingBox(model->meshes[0]);
    } else {
        baseBB.min = (Vector3){-0.5f,-0.5f,-0.5f};
        baseBB.max = (Vector3){ 0.5f, 0.5f, 0.5f};
    }

    // Create transformation matrix with scale and rotation
    Matrix matScale = MatrixScale(scale.x, scale.y, scale.z);
    Matrix matRotation = MatrixRotateXYZ((Vector3){
        rotation.x * DEG2RAD,
        rotation.y * DEG2RAD,
        rotation.z * DEG2RAD
    });
    Matrix transform = MatrixMultiply(matScale, matRotation);

    // Transform all 8 corners of the bounding box
    Vector3 corners[8] = {
        {baseBB.min.x, baseBB.min.y, baseBB.min.z},
        {baseBB.max.x, baseBB.min.y, baseBB.min.z},
        {baseBB.min.x, baseBB.max.y, baseBB.min.z},
        {baseBB.max.x, baseBB.max.y, baseBB.min.z},
        {baseBB.min.x, baseBB.min.y, baseBB.max.z},
        {baseBB.max.x, baseBB.min.y, baseBB.max.z},
        {baseBB.min.x, baseBB.max.y, baseBB.max.z},
        {baseBB.max.x, baseBB.max.y, baseBB.max.z}
    };

    // Find the min and max of all transformed corners
    Vector3 minBounds = {1e9f, 1e9f, 1e9f};
    Vector3 maxBounds = {-1e9f, -1e9f, -1e9f};

    for (int i = 0; i < 8; i++) {
        Vector3 transformed = Vector3Transform(corners[i], transform);

        if (transformed.x < minBounds.x) minBounds.x = transformed.x;
        if (transformed.y < minBounds.y) minBounds.y = transformed.y;
        if (transformed.z < minBounds.z) minBounds.z = transformed.z;
        if (transformed.x > maxBounds.x) maxBounds.x = transformed.x;
        if (transformed.y > maxBounds.y) maxBounds.y = transformed.y;
        if (transformed.z > maxBounds.z) maxBounds.z = transformed.z;
    }

    // Translate to final position
    BoundingBox result;
    result.min = Vector3Add(minBounds, position);
    result.max = Vector3Add(maxBounds, position);
    return result;
}

void LO_RecalcBounds(LevelObject* o) {
    o->bounds = ComputeModelBounds(&o->model, o->position, o->scale, o->rotation);
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