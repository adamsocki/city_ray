#ifndef EDITOR_H
#define EDITOR_H

#include "raylib.h"
#include <stdbool.h>
#include "../level_object.h"

// Manipulation mode
typedef enum {
    EDIT_MODE_SELECT = 0,
    EDIT_MODE_TRANSLATE,
    EDIT_MODE_ROTATE,
    EDIT_MODE_SCALE
} EditMode;

typedef struct {
    bool enabled;                 // Editor mode on/off
    Camera3D camera;              // Free-fly editor camera
    float cameraPitch;            // Camera pitch angle
    float cameraYaw;              // Camera yaw angle
    bool showUI;                  // Toggle UI visibility
    bool showHelp;                // Show keyboard shortcuts help
    EditMode mode;                // Active tool
    int selectedId;               // Selected object id or -1
    bool bboxEdit;                // Toggle bounding box editing
    bool snap;                    // Grid snapping
    float snapStep;               // Snap step for translate/rotate/scale
    Vector3 gizmoAxis;            // Current axis for manipulation
    Vector2 dragStartMouse;       // Mouse position when drag started
    Vector3 dragStartPos;         // Object position when drag started
    Vector3 dragStartRot;         // Object rotation when drag started
    Vector3 dragStartScale;       // Object scale when drag started
    bool isDragging;              // Currently dragging an object
    // Text input buffers for property editing
    char editPosX[32];
    char editPosY[32];
    char editPosZ[32];
    char editRotX[32];
    char editRotY[32];
    char editRotZ[32];
    char editScaleX[32];
    char editScaleY[32];
    char editScaleZ[32];
    bool editTextActive;          // Whether any text field is being edited
} EditorState;

void Editor_Init(EditorState* ed, int screenWidth, int screenHeight);
void Editor_Toggle(EditorState* ed);
void Editor_Update(EditorState* ed, LevelObjects* lo);
void Editor_Render3D(EditorState* ed, const LevelObjects* lo);
void Editor_RenderUI(EditorState* ed, LevelObjects* lo, int screenWidth, int screenHeight);

#endif // EDITOR_H