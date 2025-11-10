#ifndef ROADWAY_H
#define ROADWAY_H

#include "raylib.h"
#include <vector>

struct RoadIntersection {
    Vector3 position;
    std::vector<int> connectedSegments;
};

struct RoadSegment {
    int startIntersection;
    int endIntersection;
    float width;
};

struct Roadway {
    std::vector<RoadIntersection> intersections;
    std::vector<RoadSegment> segments;
    float segmentHeight;
};

void Init_Roadway(Roadway* roadway, int gridSize, float spacing, float roadWidth);
void Render_Roadway(const Roadway& roadway, Color roadColor);

#endif // ROADWAY_H
