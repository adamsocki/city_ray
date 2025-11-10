# Light Level Editor Implementation

## Overview

Ensure that all code is written in a functional data orented style and do not use OOP.

Add a toggleable editor mode to the existing raylib application that allows placing and manipulating 3D objects with a model library system, bounding box editing, and JSON-based level persistence.

## Core Components

### 1. Editor State Management

Create `src/editor.h` and `src/editor.cpp` to manage:

- Editor mode toggle (Tab key to switch between play/editor mode)
- Selected object tracking
- Editor camera (free-flying, no collision in editor mode)
- Object manipulation modes (translate, rotate, scale)

### 2. Object System

Create `src/level_object.h` and `src/level_object.cpp`:

- `LevelObject` struct with: position, rotation, scale, model reference, bounding box, unique ID
- Object list management (add, remove, select, duplicate)
- Model library loading from `resources/models/` directory
- Support for multiple model types with string identifiers

### 3. Editor UI (raygui)

Add UI panels in editor mode:

- **Top toolbar**: Mode selector (Select/Translate/Rotate/Scale), Save/Load buttons
- **Side panel**: Model library browser, selected object properties (position, rotation, scale inputs)
- **Object list**: Hierarchical list of all objects in scene with select/delete options
- **Gizmos**: Visual manipulation handles for selected objects (simple axis arrows)

### 4. Input Handling in Editor Mode

- Left click: Select object (raycast from mouse)
- Right click + drag: Rotate camera view
- Mouse wheel: Zoom camera
- Delete key: Remove selected object
- Ctrl+D: Duplicate selected object
- G/R/S keys: Switch to translate/rotate/scale mode
- Grid snapping (optional, with Ctrl held)

### 5. Bounding Box Editor

- Visual editing of bounding boxes per object
- Toggle between automatic (model mesh bounds) and manual editing
- Draggable handles on bounding box corners/faces
- Display dimensions in UI panel

### 6. JSON Serialization

Create `src/level_serializer.h` and `src/level_serializer.cpp`:

- Save level data: objects array with all properties, camera position
- Load level data: parse JSON and instantiate objects
- File format: `resources/levels/level_name.json`
- Include metadata (version, creation date)

## File Structure

```
src/
  editor.h, editor.cpp          - Main editor state and logic
  level_object.h, level_object.cpp - Object management
  level_serializer.h, level_serializer.cpp - JSON save/load
resources/
  models/                        - Model library directory
  levels/                        - Saved level JSON files
```

## Integration with Existing Code

- Modify `main.cpp`: Add editor mode toggle, conditional rendering
- Update `zayn.h/cpp`: Separate play mode vs editor mode logic
- Keep existing camera controls for play mode, add new editor camera
- Existing collision system only active in play mode

## Key Features

- Intuitive mode switching (Tab key)
- Visual feedback for selected objects (highlight/outline)
- Undo/redo support (optional, but recommended)
- Model preview thumbnails in library browser
- Collision bounds visualization and editing