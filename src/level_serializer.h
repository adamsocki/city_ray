#ifndef LEVEL_SERIALIZER_H
#define LEVEL_SERIALIZER_H

#include "level_object.h"
#include <stdbool.h>

bool Level_Save(const LevelObjects* lo, const char* path);
bool Level_Load(LevelObjects* lo, const char* path);

#endif // LEVEL_SERIALIZER_H