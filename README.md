# 3D Product Showroom and Configurator

A Computer Graphics mini-project built with **C++ and OpenGL** for a 5th-semester DSATM CG syllabus. It presents a 3D smartphone on a showroom pedestal and lets the user inspect, select, customize, and animate it.

## Features

- 3D smartphone assembled from primitive meshes (boxes and cylinders), including a front camera, side buttons, battery, main board, and rear camera module.
- Orbit camera with mouse drag and wheel zoom.
- Three-point studio lighting (key, fill, and rim) with per-vertex Phong illumination: ambient, diffuse, specular, and shininess.
- Material presets: plastic, metal, matte, glossy, gold, and custom colour.
- Light presets: studio, warm, cool, and an animated orbiting light.
- Part picking with ray-AABB intersection and a selection box.
- Turntable animation.
- Exploded-view animation.
- Camera keyframes and a camera-tour animation.
- Wireframe, normals, grid, and perspective/orthographic toggles.
- Native Win32 menus and a custom bitmap-font status HUD.
- Save/load showroom configuration (camera, view flags, animation state, selected part, and materials) to a text file.
- DDA-generated floor grid and Bresenham-generated pedestal ring.

## Build

Requirements:

- Windows
- MinGW g++ (C++11)
- OpenGL, GDI, User32, and Comdlg32 libraries (included with MinGW/Windows)

Run:

```bat
build.bat
```

The executable is created at:

```text
build\ProductShowroom.exe
```

Manual build command:

```bat
g++ -std=c++11 -O2 -mwindows -static -static-libgcc -static-libstdc++ -Isrc src\main.cpp src\app.cpp -o build\ProductShowroom.exe -lopengl32 -lgdi32 -luser32 -lcomdlg32
```

## Controls

### Mouse

| Input | Action |
|---|---|
| Left drag | Orbit camera |
| Mouse wheel | Zoom in / out |
| Left click | Select a product part |

### Keyboard

| Key | Action |
|---|---|
| `E` | Toggle exploded view |
| `T` | Toggle turntable |
| `C` | Start/stop camera tour |
| `R` | Reset camera |
| `G` | Toggle grid |
| `W` | Toggle wireframe |
| `N` | Toggle normals |
| `P` | Perspective / orthographic |
| `L` | Cycle light preset |
| `F` | Toggle orbiting light |
| `M` | Cycle material preset |
| `X` | Reset showroom |
| `1` / `2` / `3` | Studio / warm / cool light |
| `Ctrl+S` / `Ctrl+O` | Save / load configuration |
| `F1` | Controls help |
| `Esc` | Exit |

### Menus

- **File:** Save Configuration, Load Configuration, Reset Showroom, Exit
- **View:** Turntable, Reset Camera, Show Grid, Wireframe, Show Normals, Orthographic Projection
- **Material:** Plastic, Metal, Matte, Glossy, Gold, Change Colour, Light submenu
- **Animate:** Exploded View, Camera Tour, Record Camera Keyframe, Reset Camera Tour
- **Help:** Controls, About

## Configuration file

**File → Save Configuration** writes a plain-text file, and **File → Load Configuration** restores it. The file stores the light preset, turntable speed, camera orbit/projection, display flags, animation states, selected part, per-part material values, and camera-tour keyframes.

Example structure (one `PART` and one `CAMERA_KEYFRAME` line shown):

```text
SHOWROOM_CONFIG 1
LIGHT_PRESET 0
TURNTABLE_SPEED 0.65
CAMERA 0.5585 0.2094 7.8 0 2.2 0 0
VIEW 1 0 0
ANIM 0 0 0 0
SELECTED -1
PART 0 0.18 0.19 0.21 0.67 0.69 0.73 0.88 0.90 0.94 0 0 0 72
CAMERA_KEYFRAME 4.1 3.8 6.4 0 2.2 0
```

## Project structure

```text
CG/
  build.bat
  README.md
  src/
    main.cpp    Win32 window, OpenGL context, menus, input, file dialogs
    app.h       Application data structures and declarations
    app.cpp     Scene, product, rendering, HUD text, lighting, picking, animation, save/load
    math3d.h    Vec2/Vec3/Vec4, Mat4, AABB, ray-AABB, interpolation helpers
```

## Syllabus mapping

| Module | Coverage |
|---|---|
| Module 1 | DDA floor grid, Bresenham pedestal ring, coordinate systems, OpenGL drawing fundamentals |
| Module 2 | 3D transformations, homogeneous coordinates, composite transforms, view/projection matrices, perspective and orthographic projection |
| Module 3 | RGB material colours, ambient/diffuse/specular reflection, Phong illumination, shininess |
| Module 4 | Mouse orbit, click picking, keyboard shortcuts, native menus, interactive configuration |
| Module 5 | Turntable periodic motion, exploded-view animation, camera keyframes, interpolation, playback |

Cohen-Sutherland clipping and character animation are not central to this project. They are intentionally not forced.

## Algorithms implemented manually

- DDA line rasterization (floor grid).
- Bresenham/midpoint circle rasterization (pedestal ring).
- 3D transformation matrices (translate, rotate, scale, compose).
- Look-at view matrix and perspective/orthographic projection matrices.
- Phong illumination per vertex.
- Ray-AABB picking.
- Keyframe interpolation with smoothstep easing.
- Time-based animation using delta time.
- RGB colour selection via the Windows colour picker.

## Notes for the report/viva

- The vertex transform chain is:
  `clip = projection * view * productRoot * partTranslation * vertex`
- The product root applies the turntable rotation; each part's exploded offset is applied in product-local space.
- Normals are transformed by the model matrix; no non-uniform scaling is used.
- Picking tests a world-space ray against each part's transformed AABB.
- The camera tour stores position and target keyframes and interpolates both with smoothstep.
- The exploded view uses a global `explodeFactor` in `[0, 1]` and eases it with smoothstep.
- Studio lighting uses a key light, cool fill, and rim light; Warm and Cool presets use complementary fills so colours remain readable from all angles.
