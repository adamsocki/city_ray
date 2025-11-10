#include "roadway.h"
#include "raymath.h"
#include <cmath>

static int GetIntersectionIndex(int x, int z, int gridSize)
{
    return z * gridSize + x;
}

static unsigned char ClampToByte(int value)
{
    if (value < 0) return 0;
    if (value > 255) return 255;
    return (unsigned char)value;
}

void Init_Roadway(Roadway* roadway, int gridSize, float spacing, float roadWidth)
{
    if (gridSize < 2)
    {
        gridSize = 2;
    }

    roadway->intersections.clear();
    roadway->segments.clear();
    roadway->segmentHeight = 0.05f;

    roadway->intersections.reserve(gridSize * gridSize);

    float halfSize = (gridSize - 1) * spacing * 0.5f;
    for (int z = 0; z < gridSize; ++z)
    {
        for (int x = 0; x < gridSize; ++x)
        {
            Vector3 position = {
                (float)x * spacing - halfSize,
                0.0f,
                (float)z * spacing - halfSize
            };

            RoadIntersection intersection;
            intersection.position = position;
            intersection.connectedSegments.clear();

            roadway->intersections.push_back(intersection);
        }
    }

    for (int z = 0; z < gridSize; ++z)
    {
        for (int x = 0; x < gridSize; ++x)
        {
            int currentIndex = GetIntersectionIndex(x, z, gridSize);

            if (x + 1 < gridSize)
            {
                int rightIndex = GetIntersectionIndex(x + 1, z, gridSize);
                RoadSegment segment{currentIndex, rightIndex, roadWidth};
                roadway->intersections[currentIndex].connectedSegments.push_back((int)roadway->segments.size());
                roadway->intersections[rightIndex].connectedSegments.push_back((int)roadway->segments.size());
                roadway->segments.push_back(segment);
            }

            if (z + 1 < gridSize)
            {
                int downIndex = GetIntersectionIndex(x, z + 1, gridSize);
                RoadSegment segment{currentIndex, downIndex, roadWidth};
                roadway->intersections[currentIndex].connectedSegments.push_back((int)roadway->segments.size());
                roadway->intersections[downIndex].connectedSegments.push_back((int)roadway->segments.size());
                roadway->segments.push_back(segment);
            }
        }
    }
}

void Render_Roadway(const Roadway& roadway, Color roadColor)
{
    for (const RoadSegment& segment : roadway.segments)
    {
        const Vector3& startPos = roadway.intersections[segment.startIntersection].position;
        const Vector3& endPos = roadway.intersections[segment.endIntersection].position;

        Vector3 center = Vector3Scale(Vector3Add(startPos, endPos), 0.5f);
        center.y = roadway.segmentHeight * 0.5f;

        float roadWidth = segment.width;

        if (fabsf(startPos.x - endPos.x) < 0.001f)
        {
            float length = fabsf(endPos.z - startPos.z);
            DrawCube(center, roadWidth, roadway.segmentHeight, length + roadWidth, roadColor);
        }
        else
        {
            float length = fabsf(endPos.x - startPos.x);
            DrawCube(center, length + roadWidth, roadway.segmentHeight, roadWidth, roadColor);
        }
    }

    int lightenAmount = 35;
    Color intersectionColor = {
        ClampToByte(roadColor.r + lightenAmount),
        ClampToByte(roadColor.g + lightenAmount),
        ClampToByte(roadColor.b + lightenAmount),
        roadColor.a
    };

    float intersectionHeight = roadway.segmentHeight + 0.02f;

    for (const RoadIntersection& intersection : roadway.intersections)
    {
        Vector3 position = intersection.position;
        position.y = intersectionHeight * 0.5f;
        DrawCylinder(position, intersectionHeight * 0.5f, intersectionHeight * 0.5f, intersectionHeight, 12, intersectionColor);
    }
}
