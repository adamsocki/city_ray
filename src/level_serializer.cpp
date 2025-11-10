#include "level_serializer.h"
#include "raylib.h"
#include <stdio.h>
#include <string.h>


// Minimal manual JSON (no external deps). Rotation only Y for now.

static void fvec3(FILE* f, const char* k, Vector3 v) {
    fprintf(f, "\"%s\":{\"x\":%.4f,\"y\":%.4f,\"z\":%.4f}", k, v.x, v.y, v.z);
}

bool Level_Save(const LevelObjects* lo, const char* path) {
    FILE* f = fopen(path, "wb");
    if (!f) return false;
    fprintf(f, "{");
    fprintf(f, "\"version\":1,");
    fprintf(f, "\"objects\":[");
    for (int i = 0; i < lo->count; i++) {
        const LevelObject* o = &lo->items[i];
        fprintf(f, "%s{", (i==0)?"":",");
        fprintf(f, "\"id\":%d,", o->id);
        fprintf(f, "\"name\":\"%s\",", o->name);
        fvec3(f, "position", o->position); fprintf(f, ",");
        fvec3(f, "rotation", o->rotation); fprintf(f, ",");
        fvec3(f, "scale", o->scale);
        fprintf(f, "}");
    }
    fprintf(f, "]}");
    fclose(f);
    return true;
}

static float readFloat(const char** s) {
    float v = 0.0f; int sign = 1;
    while (**s && (**s==' ' || **s=='\n' || **s=='\t' || **s==',' || **s==':' )) (*s)++;
    if (**s=='-'){ sign=-1; (*s)++; }
    sscanf(*s, "%f", &v);
    while (**s && **s!='}' && **s!=',' && **s!=' ') (*s)++;
    return v * sign;
}

static Vector3 readVec3(const char* key, const char** s) {
    Vector3 v = {0,0,0};
    const char* p = *s;
    const char* k = strstr(p, key);
    if (!k) return v;
    p = strchr(k, '{'); if (!p) return v; p++;
    v.x = readFloat(&p);
    v.y = readFloat(&p);
    v.z = readFloat(&p);
    *s = p;
    return v;
}

bool Level_Load(LevelObjects* lo, const char* path) {
    if (!FileExists(path)) return false;
    int len = 0;
    unsigned char* data = LoadFileData(path, &len);
    if (!data) return false;
    const char* s = (const char*)data;

    LO_Init(lo);
    const char* objs = strstr(s, "\"objects\"");
    if (!objs) { UnloadFileData(data); return false; }
    const char* p = strchr(objs, '[');
    if (!p) { UnloadFileData(data); return false; }
    p++;

    while (*p && *p != ']') {
        const char* e = strchr(p, '}');
        if (!e) break;
        const char* blockStart = p;

        // name
        char name[LO_NAME_MAX] = "Mac";
        const char* nk = strstr(blockStart, "\"name\"");
        if (nk) {
            const char* q1 = strchr(nk, '\"'); if (q1) q1 = strchr(q1+1,'\"');
            const char* q2 = q1 ? strchr(q1+1,'\"') : NULL;
            if (q1 && q2) {
                int n = (int)(q2 - (q1+1));
                if (n >= LO_NAME_MAX) n = LO_NAME_MAX-1;
                strncpy(name, q1+1, n);
                name[n] = '\0';
            }
        }

        Vector3 pos = readVec3("\"position\"", &blockStart);
        Vector3 rot = readVec3("\"rotation\"", &blockStart);
        Vector3 scl = readVec3("\"scale\"", &blockStart);

        LO_Add(lo, name, pos, rot, scl);

        p = e + 1;
        const char* comma = strchr(p, ',');
        if (comma && comma < strchr(objs, ']')) p = comma + 1;
        while (*p && (*p==' '||*p=='\n'||*p=='\t'||*p==',')) p++;
    }

    UnloadFileData(data);
    return true;
}