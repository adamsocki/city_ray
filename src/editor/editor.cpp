#include "editor.h"
#include "raymath.h"
#include "raygui.h"
#include "../level_serializer.h"
#include "rlgl.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static Ray Editor_GetMouseRay3D(const EditorState* ed) {
    Vector2 mouse = GetMousePosition();
    return GetMouseRay(mouse, ed->camera);
}

// Helper function to draw a model with full XYZ rotation support
static void DrawModelWithRotation(Model model, Vector3 position, Vector3 rotation, Vector3 scale, Color tint) {
    // Convert rotation from degrees to a rotation matrix
    Matrix matScale = MatrixScale(scale.x, scale.y, scale.z);
    Matrix matRotation = MatrixRotateXYZ((Vector3){
        rotation.x * DEG2RAD,
        rotation.y * DEG2RAD,
        rotation.z * DEG2RAD
    });
    Matrix matTranslation = MatrixTranslate(position.x, position.y, position.z);
    Matrix matTransform = MatrixMultiply(MatrixMultiply(matScale, matRotation), matTranslation);

    // Apply transformation and draw
    rlPushMatrix();
    rlMultMatrixf(MatrixToFloat(matTransform));
    DrawModel(model, (Vector3){0, 0, 0}, 1.0f, tint);
    rlPopMatrix();
}

static int Editor_RaycastPick(const EditorState* ed, const LevelObjects* lo) {
    Ray ray = Editor_GetMouseRay3D(ed);
    float bestDist = 1e9f;
    int bestId = -1;
    for (int i = 0; i < lo->count; i++) {
        const LevelObject* o = &lo->items[i];
        // AABB test in object space is skipped; approximate with world AABB
        BoundingBox bb = o->bounds;
        RayCollision hit = GetRayCollisionBox(ray, bb);
        if (hit.hit && hit.distance < bestDist) {
            bestDist = hit.distance;
            bestId = o->id;
        }
    }
    return bestId;
}

static void Editor_UpdateCamera(EditorState* ed) {
    // Simple free-fly camera with improved controls
    float mouseSensitivity = 0.12f;
    float speed = 8.0f * GetFrameTime();

    // Increase speed when shift is held
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
        speed *= 3.0f;
    }

    if (!IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) return;

    Vector2 delta = GetMouseDelta();
    ed->cameraYaw += delta.x * mouseSensitivity;
    ed->cameraPitch -= delta.y * mouseSensitivity;
    if (ed->cameraPitch > 89.0f) ed->cameraPitch = 89.0f;
    if (ed->cameraPitch < -89.0f) ed->cameraPitch = -89.0f;

    Vector3 dir = {
        cosf(DEG2RAD * ed->cameraYaw) * cosf(DEG2RAD * ed->cameraPitch),
        sinf(DEG2RAD * ed->cameraPitch),
        sinf(DEG2RAD * ed->cameraYaw) * cosf(DEG2RAD * ed->cameraPitch)
    };
    dir = Vector3Normalize(dir);
    Vector3 forward = { dir.x, 0.0f, dir.z };
    forward = Vector3Normalize(forward);
    Vector3 right = Vector3CrossProduct(forward, ed->camera.up);

    if (IsKeyDown(KEY_W)) ed->camera.position = Vector3Add(ed->camera.position, Vector3Scale(forward, speed));
    if (IsKeyDown(KEY_S)) ed->camera.position = Vector3Subtract(ed->camera.position, Vector3Scale(forward, speed));
    if (IsKeyDown(KEY_A)) ed->camera.position = Vector3Subtract(ed->camera.position, Vector3Scale(right, speed));
    if (IsKeyDown(KEY_D)) ed->camera.position = Vector3Add(ed->camera.position, Vector3Scale(right, speed));
    if (IsKeyDown(KEY_Q)) ed->camera.position.y -= speed;
    if (IsKeyDown(KEY_E)) ed->camera.position.y += speed;

    ed->camera.target = Vector3Add(ed->camera.position, dir);
}

void Editor_Init(EditorState* ed, int screenWidth, int screenHeight) {
    ed->enabled = false;
    ed->showUI = true;
    ed->showHelp = false;
    ed->mode = EDIT_MODE_SELECT;
    ed->selectedId = -1;
    ed->bboxEdit = false;
    ed->snap = false;
    ed->snapStep = 0.5f;
    ed->gizmoAxis = (Vector3){1,0,0};
    ed->dragStartMouse = (Vector2){0,0};
    ed->dragStartPos = (Vector3){0,0,0};
    ed->dragStartRot = (Vector3){0,0,0};
    ed->dragStartScale = (Vector3){1,1,1};
    ed->isDragging = false;
    ed->editTextActive = false;

    // Initialize camera angles
    ed->cameraPitch = 0.0f;
    ed->cameraYaw = -90.0f;

    // Initialize text buffers
    memset(ed->editPosX, 0, sizeof(ed->editPosX));
    memset(ed->editPosY, 0, sizeof(ed->editPosY));
    memset(ed->editPosZ, 0, sizeof(ed->editPosZ));
    memset(ed->editRotX, 0, sizeof(ed->editRotX));
    memset(ed->editRotY, 0, sizeof(ed->editRotY));
    memset(ed->editRotZ, 0, sizeof(ed->editRotZ));
    memset(ed->editScaleX, 0, sizeof(ed->editScaleX));
    memset(ed->editScaleY, 0, sizeof(ed->editScaleY));
    memset(ed->editScaleZ, 0, sizeof(ed->editScaleZ));

    ed->camera.position = (Vector3){ 3.0f, 2.0f, 6.0f };
    ed->camera.target = (Vector3){ 0.0f, 1.0f, 0.0f };
    ed->camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    ed->camera.fovy = 45.0f;
    ed->camera.projection = CAMERA_PERSPECTIVE;

    (void)screenWidth; (void)screenHeight;
}

void Editor_Toggle(EditorState* ed) {
    ed->enabled = !ed->enabled;
    if (ed->enabled) {
        EnableCursor(); // UI + free look uses RMB
        printf("Editor enabled\n");
    } else {
        DisableCursor(); // return to play mode capture
        printf("Editor disabled\n");
    }
}

static void Editor_HandleShortcuts(EditorState* ed, LevelObjects* lo) {
    // Toggle help with F1
    if (IsKeyPressed(KEY_F1)) {
        ed->showHelp = !ed->showHelp;
    }

    // Mode shortcuts (only when not dragging to avoid conflicts)
    if (!ed->isDragging) {
        if (IsKeyPressed(KEY_G)) ed->mode = EDIT_MODE_TRANSLATE;
        if (IsKeyPressed(KEY_R)) ed->mode = EDIT_MODE_ROTATE;
        if (IsKeyPressed(KEY_C)) ed->mode = EDIT_MODE_SCALE; // Changed from S to avoid camera conflict
        if (IsKeyPressed(KEY_ESCAPE)) {
            ed->mode = EDIT_MODE_SELECT;
            ed->selectedId = -1; // Deselect on ESC
        }

        if (IsKeyPressed(KEY_DELETE) && ed->selectedId != -1) {
            LO_Remove(lo, ed->selectedId);
            ed->selectedId = -1;
        }
        if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_D)) {
            if (ed->selectedId != -1) {
                int newId = LO_Duplicate(lo, ed->selectedId);
                ed->selectedId = newId;
            }
        }
    }

    // Snap can be toggled anytime with Ctrl
    ed->snap = (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL));
}

static float SnapF(float v, float step, bool snap) {
    if (!snap) return v;
    return roundf(v / step) * step;
}

static void Editor_Manipulate(EditorState* ed, LevelObjects* lo) {
    if (ed->selectedId == -1 || ed->mode == EDIT_MODE_SELECT) return;
    LevelObject* o = LO_GetById(lo, ed->selectedId);
    if (!o) return;

    // Axis selection by keys X/Y/Z (only when not already dragging)
    if (!ed->isDragging) {
        if (IsKeyPressed(KEY_X)) ed->gizmoAxis = (Vector3){1,0,0};
        if (IsKeyPressed(KEY_Y)) ed->gizmoAxis = (Vector3){0,1,0};
        if (IsKeyPressed(KEY_Z)) ed->gizmoAxis = (Vector3){0,0,1};
    }

    // Start drag on left mouse press
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !ed->isDragging) {
        ed->isDragging = true;
        ed->dragStartMouse = GetMousePosition();
        ed->dragStartPos = o->position;
        ed->dragStartRot = o->rotation;
        ed->dragStartScale = o->scale;
    }

    // Active dragging
    if (ed->isDragging && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        Vector2 mouseDelta = Vector2Subtract(GetMousePosition(), ed->dragStartMouse);

        // Use combined mouse movement for more intuitive control
        float movement = (mouseDelta.x + mouseDelta.y) * 0.02f;

        if (ed->mode == EDIT_MODE_TRANSLATE) {
            // Apply movement along the selected axis from start position
            Vector3 offset = Vector3Scale(ed->gizmoAxis, movement);
            Vector3 newPos = Vector3Add(ed->dragStartPos, offset);

            o->position.x = SnapF(newPos.x, ed->snapStep, ed->snap);
            o->position.y = SnapF(newPos.y, ed->snapStep, ed->snap);
            o->position.z = SnapF(newPos.z, ed->snapStep, ed->snap);
            LO_RecalcBounds(o);

        } else if (ed->mode == EDIT_MODE_ROTATE) {
            // Rotation: apply delta from start rotation
            float rotAmount = movement * 100.0f; // degrees (increased sensitivity)
            Vector3 rotOffset = Vector3Scale(ed->gizmoAxis, rotAmount);
            Vector3 newRot = Vector3Add(ed->dragStartRot, rotOffset);

            o->rotation.x = ed->snap ? SnapF(newRot.x, 15.0f, true) : newRot.x;
            o->rotation.y = ed->snap ? SnapF(newRot.y, 15.0f, true) : newRot.y;
            o->rotation.z = ed->snap ? SnapF(newRot.z, 15.0f, true) : newRot.z;

        } else if (ed->mode == EDIT_MODE_SCALE) {
            // Scale: apply delta from start scale
            float scaleChange = movement * 2.0f; // Increased sensitivity
            Vector3 scaleOffset = Vector3Scale(ed->gizmoAxis, scaleChange);
            Vector3 newScale = Vector3Add(ed->dragStartScale, scaleOffset);

            // Clamp to minimum scale
            o->scale.x = fmaxf(0.1f, ed->snap ? SnapF(newScale.x, 0.25f, true) : newScale.x);
            o->scale.y = fmaxf(0.1f, ed->snap ? SnapF(newScale.y, 0.25f, true) : newScale.y);
            o->scale.z = fmaxf(0.1f, ed->snap ? SnapF(newScale.z, 0.25f, true) : newScale.z);
            LO_RecalcBounds(o);
        }
    }

    // End drag on mouse release
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        ed->isDragging = false;
    }
}

void Editor_Update(EditorState* ed, LevelObjects* lo) {
    if (!ed->enabled) return;
    Editor_HandleShortcuts(ed, lo);
    Editor_UpdateCamera(ed);

    // Picking: only in select mode, not while camera moving, and not while dragging
    if (ed->mode == EDIT_MODE_SELECT &&
        !ed->isDragging &&
        !IsMouseButtonDown(MOUSE_RIGHT_BUTTON) &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        int hit = Editor_RaycastPick(ed, lo);
        ed->selectedId = hit;
    }

    // Manipulate after selection (handles its own drag state)
    Editor_Manipulate(ed, lo);
}

static void DrawAxisGizmo(const Vector3 pos, const Vector3 activeAxis, float len, EditMode mode) {
    // Highlight the active axis with thicker/brighter rendering
    float thickActive = 0.06f;
    float thickInactive = 0.03f;

    // X axis (RED)
    Color xCol = (activeAxis.x > 0.5f) ? RED : Fade(RED, 0.5f);
    float xThick = (activeAxis.x > 0.5f) ? thickActive : thickInactive;
    DrawCylinderEx(pos, Vector3Add(pos, (Vector3){len,0,0}), xThick, xThick, 8, xCol);

    // Different tip shapes based on mode
    if (mode == EDIT_MODE_TRANSLATE) {
        DrawCone(Vector3Add(pos, (Vector3){len,0,0}), 0.12f, 0.2f, 8, xCol);
    } else if (mode == EDIT_MODE_ROTATE) {
        DrawSphere(Vector3Add(pos, (Vector3){len,0,0}), 0.1f, xCol);
    } else if (mode == EDIT_MODE_SCALE) {
        DrawCube(Vector3Add(pos, (Vector3){len,0,0}), 0.15f, 0.15f, 0.15f, xCol);
    }

    // Y axis (GREEN)
    Color yCol = (activeAxis.y > 0.5f) ? GREEN : Fade(GREEN, 0.5f);
    float yThick = (activeAxis.y > 0.5f) ? thickActive : thickInactive;
    DrawCylinderEx(pos, Vector3Add(pos, (Vector3){0,len,0}), yThick, yThick, 8, yCol);

    if (mode == EDIT_MODE_TRANSLATE) {
        DrawCone(Vector3Add(pos, (Vector3){0,len,0}), 0.12f, 0.2f, 8, yCol);
    } else if (mode == EDIT_MODE_ROTATE) {
        DrawSphere(Vector3Add(pos, (Vector3){0,len,0}), 0.1f, yCol);
    } else if (mode == EDIT_MODE_SCALE) {
        DrawCube(Vector3Add(pos, (Vector3){0,len,0}), 0.15f, 0.15f, 0.15f, yCol);
    }

    // Z axis (BLUE)
    Color zCol = (activeAxis.z > 0.5f) ? BLUE : Fade(BLUE, 0.5f);
    float zThick = (activeAxis.z > 0.5f) ? thickActive : thickInactive;
    DrawCylinderEx(pos, Vector3Add(pos, (Vector3){0,0,len}), zThick, zThick, 8, zCol);

    if (mode == EDIT_MODE_TRANSLATE) {
        DrawCone(Vector3Add(pos, (Vector3){0,0,len}), 0.12f, 0.2f, 8, zCol);
    } else if (mode == EDIT_MODE_ROTATE) {
        DrawSphere(Vector3Add(pos, (Vector3){0,0,len}), 0.1f, zCol);
    } else if (mode == EDIT_MODE_SCALE) {
        DrawCube(Vector3Add(pos, (Vector3){0,0,len}), 0.15f, 0.15f, 0.15f, zCol);
    }

    // Center sphere
    DrawSphere(pos, 0.08f, WHITE);
}

void Editor_Render3D(EditorState* ed, const LevelObjects* lo) {
    if (!ed->enabled) return;

    BeginMode3D(ed->camera);
    DrawGrid(20, 1.0f);

    for (int i = 0; i < lo->count; i++) {
        const LevelObject* o = &lo->items[i];
        // Use full XYZ rotation rendering
        DrawModelWithRotation(o->model, o->position, o->rotation, o->scale, WHITE);

        if (ed->selectedId == o->id) {
            DrawBoundingBox(o->bounds, YELLOW);
            // Show gizmo with active axis highlighted
            if (ed->mode != EDIT_MODE_SELECT) {
                DrawAxisGizmo(o->position, ed->gizmoAxis, 1.0f, ed->mode);
            }
        } else {
            if (lo->showBounds) DrawBoundingBox(o->bounds, Fade(BLUE, 0.3f));
        }
    }

    EndMode3D();
}

void Editor_RenderUI(EditorState* ed, LevelObjects* lo, int screenWidth, int screenHeight) {
    if (!ed->enabled) return;

    const float panelW = 280.0f;
    const float toolbarH = 36.0f;

    // Top toolbar with mode highlighting
    bool isSel = (ed->mode == EDIT_MODE_SELECT);
    bool isMov = (ed->mode == EDIT_MODE_TRANSLATE);
    bool isRot = (ed->mode == EDIT_MODE_ROTATE);
    bool isScl = (ed->mode == EDIT_MODE_SCALE);

    GuiSetState(isSel ? STATE_PRESSED : STATE_NORMAL);
    if (GuiButton((Rectangle){ 8.0f, 4.0f, 90.0f, toolbarH - 8.0f }, "#83#Select")) ed->mode = EDIT_MODE_SELECT;
    GuiSetState(isMov ? STATE_PRESSED : STATE_NORMAL);
    if (GuiButton((Rectangle){ 102.0f, 4.0f, 90.0f, toolbarH - 8.0f }, "#84#Move(G)")) ed->mode = EDIT_MODE_TRANSLATE;
    GuiSetState(isRot ? STATE_PRESSED : STATE_NORMAL);
    if (GuiButton((Rectangle){ 196.0f, 4.0f, 90.0f, toolbarH - 8.0f }, "#85#Rotate(R)")) ed->mode = EDIT_MODE_ROTATE;
    GuiSetState(isScl ? STATE_PRESSED : STATE_NORMAL);
    if (GuiButton((Rectangle){ 290.0f, 4.0f, 90.0f, toolbarH - 8.0f }, "#86#Scale(C)")) ed->mode = EDIT_MODE_SCALE;
    GuiSetState(STATE_NORMAL);

    if (GuiButton((Rectangle){ (float)screenWidth - 200.0f, 4.0f, 92.0f, toolbarH - 8.0f }, "#138#Save")) {
        Level_Save(lo, "resources/levels/level_001.json");
    }
    if (GuiButton((Rectangle){ (float)screenWidth - 100.0f, 4.0f, 92.0f, toolbarH - 8.0f }, "#139#Load")) {
        Level_Load(lo, "resources/levels/level_001.json");
    }

    // Side panel
    Rectangle side = { 8.0f, toolbarH + 8.0f, panelW, (float)screenHeight - toolbarH - 16.0f };
    GuiPanel(side, "Editor");

    float y = side.y + 28.0f;

    // Model library (very minimal: add one known model)
    if (GuiButton((Rectangle){ side.x + 8.0f, y, panelW - 16.0f, 28.0f }, "#31# Add MacintoshClassic")) {
        int id = LO_Add(lo, "Mac", (Vector3){0,0,0}, (Vector3){0,0,0}, (Vector3){1,1,1});
        ed->selectedId = id;
    }
    y += 36.0f;

    // Objects list
    GuiLabel((Rectangle){ side.x + 8.0f, y, 100.0f, 20.0f }, "Objects");
    y += 22.0f;

    for (int i = 0; i < lo->count; i++) {
        const LevelObject* o = &lo->items[i];
        bool sel = (o->id == ed->selectedId);
        bool active = sel;
        GuiToggle((Rectangle){ side.x + 8.0f, y, panelW - 16.0f, 24.0f }, o->name, &active);
        if (active) {
            ed->selectedId = o->id;
        }
        y += 26.0f;
    }

    // Selected properties
    y += 8.0f;
    GuiLabel((Rectangle){ side.x + 8.0f, y, 120.0f, 22.0f }, "Properties");
    y += 24.0f;

    LevelObject* cur = LO_GetById(lo, ed->selectedId);
    if (cur) {
        const float labelW = 30.0f;
        const float inputW = panelW - labelW - 24.0f;
        const float inputH = 22.0f;

        // Sync text buffers with current values
        snprintf(ed->editPosX, sizeof(ed->editPosX), "%.2f", cur->position.x);
        snprintf(ed->editPosY, sizeof(ed->editPosY), "%.2f", cur->position.y);
        snprintf(ed->editPosZ, sizeof(ed->editPosZ), "%.2f", cur->position.z);
        snprintf(ed->editRotX, sizeof(ed->editRotX), "%.1f", cur->rotation.x);
        snprintf(ed->editRotY, sizeof(ed->editRotY), "%.1f", cur->rotation.y);
        snprintf(ed->editRotZ, sizeof(ed->editRotZ), "%.1f", cur->rotation.z);
        snprintf(ed->editScaleX, sizeof(ed->editScaleX), "%.2f", cur->scale.x);
        snprintf(ed->editScaleY, sizeof(ed->editScaleY), "%.2f", cur->scale.y);
        snprintf(ed->editScaleZ, sizeof(ed->editScaleZ), "%.2f", cur->scale.z);

        // Position
        GuiGroupBox((Rectangle){ side.x + 8.0f, y, panelW - 16.0f, 100.0f }, "Position");
        y += 22.0f;

        GuiLabel((Rectangle){ side.x + 16.0f, y, labelW, inputH }, "X");
        if (GuiTextBox((Rectangle){ side.x + 46.0f, y, inputW, inputH }, ed->editPosX, 32, ed->editTextActive)) {
            cur->position.x = (float)atof(ed->editPosX);
            LO_RecalcBounds(cur);
        }
        y += 24.0f;

        GuiLabel((Rectangle){ side.x + 16.0f, y, labelW, inputH }, "Y");
        if (GuiTextBox((Rectangle){ side.x + 46.0f, y, inputW, inputH }, ed->editPosY, 32, ed->editTextActive)) {
            cur->position.y = (float)atof(ed->editPosY);
            LO_RecalcBounds(cur);
        }
        y += 24.0f;

        GuiLabel((Rectangle){ side.x + 16.0f, y, labelW, inputH }, "Z");
        if (GuiTextBox((Rectangle){ side.x + 46.0f, y, inputW, inputH }, ed->editPosZ, 32, ed->editTextActive)) {
            cur->position.z = (float)atof(ed->editPosZ);
            LO_RecalcBounds(cur);
        }
        y += 32.0f;

        // Rotation
        GuiGroupBox((Rectangle){ side.x + 8.0f, y, panelW - 16.0f, 100.0f }, "Rotation");
        y += 22.0f;

        GuiLabel((Rectangle){ side.x + 16.0f, y, labelW, inputH }, "X");
        if (GuiTextBox((Rectangle){ side.x + 46.0f, y, inputW, inputH }, ed->editRotX, 32, ed->editTextActive)) {
            cur->rotation.x = (float)atof(ed->editRotX);
        }
        y += 24.0f;

        GuiLabel((Rectangle){ side.x + 16.0f, y, labelW, inputH }, "Y");
        if (GuiTextBox((Rectangle){ side.x + 46.0f, y, inputW, inputH }, ed->editRotY, 32, ed->editTextActive)) {
            cur->rotation.y = (float)atof(ed->editRotY);
        }
        y += 24.0f;

        GuiLabel((Rectangle){ side.x + 16.0f, y, labelW, inputH }, "Z");
        if (GuiTextBox((Rectangle){ side.x + 46.0f, y, inputW, inputH }, ed->editRotZ, 32, ed->editTextActive)) {
            cur->rotation.z = (float)atof(ed->editRotZ);
        }
        y += 32.0f;

        // Scale
        GuiGroupBox((Rectangle){ side.x + 8.0f, y, panelW - 16.0f, 100.0f }, "Scale");
        y += 22.0f;

        GuiLabel((Rectangle){ side.x + 16.0f, y, labelW, inputH }, "X");
        if (GuiTextBox((Rectangle){ side.x + 46.0f, y, inputW, inputH }, ed->editScaleX, 32, ed->editTextActive)) {
            cur->scale.x = fmaxf(0.01f, (float)atof(ed->editScaleX));
            LO_RecalcBounds(cur);
        }
        y += 24.0f;

        GuiLabel((Rectangle){ side.x + 16.0f, y, labelW, inputH }, "Y");
        if (GuiTextBox((Rectangle){ side.x + 46.0f, y, inputW, inputH }, ed->editScaleY, 32, ed->editTextActive)) {
            cur->scale.y = fmaxf(0.01f, (float)atof(ed->editScaleY));
            LO_RecalcBounds(cur);
        }
        y += 24.0f;

        GuiLabel((Rectangle){ side.x + 16.0f, y, labelW, inputH }, "Z");
        if (GuiTextBox((Rectangle){ side.x + 46.0f, y, inputW, inputH }, ed->editScaleZ, 32, ed->editTextActive)) {
            cur->scale.z = fmaxf(0.01f, (float)atof(ed->editScaleZ));
            LO_RecalcBounds(cur);
        }
        y += 26.0f;
    }

    // Help text at bottom of screen
    const char* modeText = "Select Mode";
    if (ed->mode == EDIT_MODE_TRANSLATE) modeText = "Move Mode - Press X/Y/Z then drag";
    else if (ed->mode == EDIT_MODE_ROTATE) modeText = "Rotate Mode - Press X/Y/Z then drag";
    else if (ed->mode == EDIT_MODE_SCALE) modeText = "Scale Mode - Press X/Y/Z then drag";

    DrawText(modeText, 8, screenHeight - 44, 14, WHITE);
    DrawText("F1: Help | RMB+WASD: Camera | G: Move | R: Rotate | C: Scale | ESC: Deselect",
             8, screenHeight - 24, 14, LIGHTGRAY);

    if (ed->snap) {
        DrawText("SNAP: ON (Ctrl)", screenWidth - 180, screenHeight - 24, 14, YELLOW);
    }

    // Keyboard shortcuts help panel
    if (ed->showHelp) {
        float helpW = 400.0f;
        float helpH = 480.0f;
        float helpX = (screenWidth - helpW) / 2.0f;
        float helpY = (screenHeight - helpH) / 2.0f;

        // Semi-transparent background overlay
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.6f));

        // Help panel
        GuiPanel((Rectangle){helpX, helpY, helpW, helpH}, "Keyboard Shortcuts (F1 to close)");

        float textY = helpY + 30.0f;
        float textX = helpX + 16.0f;
        int fontSize = 14;
        int lineHeight = 20;

        DrawText("EDITOR CONTROLS:", textX, textY, fontSize, YELLOW);
        textY += lineHeight + 8;

        DrawText("TAB           - Toggle Editor Mode", textX, textY, fontSize, WHITE);
        textY += lineHeight;
        DrawText("F1            - Toggle this help", textX, textY, fontSize, WHITE);
        textY += lineHeight + 8;

        DrawText("MODES:", textX, textY, fontSize, YELLOW);
        textY += lineHeight + 8;

        DrawText("G             - Move/Translate Mode", textX, textY, fontSize, WHITE);
        textY += lineHeight;
        DrawText("R             - Rotate Mode", textX, textY, fontSize, WHITE);
        textY += lineHeight;
        DrawText("C             - Scale Mode", textX, textY, fontSize, WHITE);
        textY += lineHeight;
        DrawText("ESC           - Select Mode / Deselect", textX, textY, fontSize, WHITE);
        textY += lineHeight + 8;

        DrawText("MANIPULATION:", textX, textY, fontSize, YELLOW);
        textY += lineHeight + 8;

        DrawText("X / Y / Z     - Select axis (Red/Green/Blue)", textX, textY, fontSize, WHITE);
        textY += lineHeight;
        DrawText("Left Click    - Start drag operation", textX, textY, fontSize, WHITE);
        textY += lineHeight;
        DrawText("Ctrl (hold)   - Enable grid snapping", textX, textY, fontSize, WHITE);
        textY += lineHeight + 8;

        DrawText("CAMERA:", textX, textY, fontSize, YELLOW);
        textY += lineHeight + 8;

        DrawText("RMB + Mouse   - Look around", textX, textY, fontSize, WHITE);
        textY += lineHeight;
        DrawText("RMB + WASD    - Move camera", textX, textY, fontSize, WHITE);
        textY += lineHeight;
        DrawText("RMB + Q/E     - Move down/up", textX, textY, fontSize, WHITE);
        textY += lineHeight;
        DrawText("Shift (hold)  - Faster camera movement", textX, textY, fontSize, WHITE);
        textY += lineHeight + 8;

        DrawText("OBJECTS:", textX, textY, fontSize, YELLOW);
        textY += lineHeight + 8;

        DrawText("Ctrl+D        - Duplicate selected object", textX, textY, fontSize, WHITE);
        textY += lineHeight;
        DrawText("Delete        - Remove selected object", textX, textY, fontSize, WHITE);
        textY += lineHeight;
    }
}