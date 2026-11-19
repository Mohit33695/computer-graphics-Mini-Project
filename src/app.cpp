#include "app.h"

#include <windows.h>
#include <GL/gl.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>

// -----------------------------------------------------------------------------
// Tiny 5x7 bitmap font for the HUD. This avoids depending on GDI font bitmaps.
// -----------------------------------------------------------------------------

struct Glyph {
    char ch;
    unsigned char rows[7];
};

static const Glyph kGlyphs[] = {
    {'A', {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}},
    {'B', {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E}},
    {'C', {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E}},
    {'D', {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E}},
    {'E', {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F}},
    {'F', {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10}},
    {'G', {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0E}},
    {'H', {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}},
    {'I', {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E}},
    {'J', {0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C}},
    {'K', {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11}},
    {'L', {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F}},
    {'M', {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11}},
    {'N', {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11}},
    {'O', {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}},
    {'P', {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10}},
    {'Q', {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D}},
    {'R', {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11}},
    {'S', {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E}},
    {'T', {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}},
    {'U', {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}},
    {'V', {0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04}},
    {'W', {0x11, 0x11, 0x11, 0x15, 0x15, 0x1B, 0x11}},
    {'X', {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11}},
    {'Y', {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04}},
    {'Z', {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F}},
    {'0', {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E}},
    {'1', {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E}},
    {'2', {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F}},
    {'3', {0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E}},
    {'4', {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}},
    {'5', {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E}},
    {'6', {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E}},
    {'7', {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08}},
    {'8', {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}},
    {'9', {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C}},
    {':', {0x00, 0x04, 0x04, 0x00, 0x04, 0x04, 0x00}},
    {'|', {0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}},
    {'%', {0x18, 0x19, 0x02, 0x04, 0x08, 0x13, 0x03}},
    {'.', {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C}},
    {'-', {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00}},
    {'_', {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F}},
    {'/', {0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x00}},
    {'(', {0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02}},
    {')', {0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08}},
    {',', {0x00, 0x00, 0x00, 0x00, 0x0C, 0x04, 0x08}},
    {'+', {0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00}},
    {'=', {0x00, 0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00}},
    {'*', {0x00, 0x0A, 0x04, 0x1F, 0x04, 0x0A, 0x00}}
};

static const unsigned char* glyphRows(char ch) {
    static const unsigned char blank[7] = {0, 0, 0, 0, 0, 0, 0};
    char c = (char)std::toupper((unsigned char)ch);
    for (size_t i = 0; i < sizeof(kGlyphs) / sizeof(kGlyphs[0]); ++i) {
        if (kGlyphs[i].ch == c) return kGlyphs[i].rows;
    }
    return blank;
}

void drawText2D(float x, float y, const char* text) {
    if (!text) return;

    const float pixel = 1.6f;
    const float advance = 6.0f * pixel;
    const float lineStep = 8.0f * pixel;

    glBegin(GL_QUADS);
    float cx = x;
    float cy = y;

    for (const char* p = text; *p; ++p) {
        if (*p == '\n') {
            cx = x;
            cy += lineStep;
            continue;
        }

        const unsigned char* rows = glyphRows(*p);
        for (int row = 0; row < 7; ++row) {
            for (int col = 0; col < 5; ++col) {
                if (rows[row] & (1 << (4 - col))) {
                    float x0 = cx + col * pixel;
                    float y0 = cy + row * pixel;
                    float x1 = x0 + pixel;
                    float y1 = y0 + pixel;
                    glVertex2f(x0, y0);
                    glVertex2f(x1, y0);
                    glVertex2f(x1, y1);
                    glVertex2f(x0, y1);
                }
            }
        }

        cx += advance;
    }

    glEnd();
}

// -----------------------------------------------------------------------------
// Small 2D raster helpers used for the pedestal ring and floor grid.
// -----------------------------------------------------------------------------

static void ddaLinePoints(int x0, int y0, int x1, int y1, std::vector<Vec2>& out) {
    int dx = x1 - x0;
    int dy = y1 - y0;
    int steps = std::max(std::abs(dx), std::abs(dy));
    if (steps == 0) {
        out.push_back(Vec2((float)x0, (float)y0));
        return;
    }
    float xInc = (float)dx / (float)steps;
    float yInc = (float)dy / (float)steps;
    float x = (float)x0;
    float y = (float)y0;
    for (int i = 0; i <= steps; ++i) {
        out.push_back(Vec2(x, y));
        x += xInc;
        y += yInc;
    }
}

static void bresenhamCirclePoints(int cx, int cy, int r, std::vector<Vec2>& out) {
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;

    while (x <= y) {
        out.push_back(Vec2((float)(cx + x), (float)(cy + y)));
        out.push_back(Vec2((float)(cx - x), (float)(cy + y)));
        out.push_back(Vec2((float)(cx + x), (float)(cy - y)));
        out.push_back(Vec2((float)(cx - x), (float)(cy - y)));
        out.push_back(Vec2((float)(cx + y), (float)(cy + x)));
        out.push_back(Vec2((float)(cx - y), (float)(cy + x)));
        out.push_back(Vec2((float)(cx + y), (float)(cy - x)));
        out.push_back(Vec2((float)(cx - y), (float)(cy - x)));

        if (d < 0) {
            d += 4 * x + 6;
        } else {
            d += 4 * (x - y) + 10;
            --y;
        }
        ++x;
    }
}

// -----------------------------------------------------------------------------
// Mesh
// -----------------------------------------------------------------------------

void Mesh::addTriangle(const Vec3& a, const Vec3& b, const Vec3& c,
                       const Vec3& na, const Vec3& nb, const Vec3& nc) {
    vertices.push_back(Vertex(a, na));
    vertices.push_back(Vertex(b, nb));
    vertices.push_back(Vertex(c, nc));
}

void Mesh::addBox(const Vec3& center, const Vec3& size) {
    Vec3 h = size * 0.5f;
    float x0 = center.x - h.x;
    float x1 = center.x + h.x;
    float y0 = center.y - h.y;
    float y1 = center.y + h.y;
    float z0 = center.z - h.z;
    float z1 = center.z + h.z;

    auto quad = [&](const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d,
                    const Vec3& n) {
        addTriangle(a, b, c, n, n, n);
        addTriangle(a, c, d, n, n, n);
    };

    // +Z
    quad(Vec3(x0, y0, z1), Vec3(x1, y0, z1), Vec3(x1, y1, z1), Vec3(x0, y1, z1),
         Vec3(0, 0, 1));
    // -Z
    quad(Vec3(x1, y0, z0), Vec3(x0, y0, z0), Vec3(x0, y1, z0), Vec3(x1, y1, z0),
         Vec3(0, 0, -1));
    // +X
    quad(Vec3(x1, y0, z1), Vec3(x1, y0, z0), Vec3(x1, y1, z0), Vec3(x1, y1, z1),
         Vec3(1, 0, 0));
    // -X
    quad(Vec3(x0, y0, z0), Vec3(x0, y0, z1), Vec3(x0, y1, z1), Vec3(x0, y1, z0),
         Vec3(-1, 0, 0));
    // +Y
    quad(Vec3(x0, y1, z1), Vec3(x1, y1, z1), Vec3(x1, y1, z0), Vec3(x0, y1, z0),
         Vec3(0, 1, 0));
    // -Y
    quad(Vec3(x0, y0, z0), Vec3(x1, y0, z0), Vec3(x1, y0, z1), Vec3(x0, y0, z1),
         Vec3(0, -1, 0));
}

void Mesh::addCylinderY(const Vec3& center, float radius, float height, int segments) {
    if (segments < 3) segments = 3;
    float y0 = center.y - height * 0.5f;
    float y1 = center.y + height * 0.5f;

    for (int i = 0; i < segments; ++i) {
        float a0 = (float)(2.0 * M_PI * i / segments);
        float a1 = (float)(2.0 * M_PI * (i + 1) / segments);

        float c0 = std::cos(a0), s0 = std::sin(a0);
        float c1 = std::cos(a1), s1 = std::sin(a1);

        Vec3 p0(center.x + radius * c0, y0, center.z + radius * s0);
        Vec3 p1(center.x + radius * c1, y0, center.z + radius * s1);
        Vec3 p2(center.x + radius * c1, y1, center.z + radius * s1);
        Vec3 p3(center.x + radius * c0, y1, center.z + radius * s0);

        Vec3 n0(c0, 0, s0);
        Vec3 n1(c1, 0, s1);

        addTriangle(p0, p1, p2, n0, n1, n1);
        addTriangle(p0, p2, p3, n0, n1, n0);
    }

    Vec3 top(center.x, y1, center.z);
    Vec3 bottom(center.x, y0, center.z);
    Vec3 nTop(0, 1, 0);
    Vec3 nBottom(0, -1, 0);

    for (int i = 0; i < segments; ++i) {
        float a0 = (float)(2.0 * M_PI * i / segments);
        float a1 = (float)(2.0 * M_PI * (i + 1) / segments);

        Vec3 p0(center.x + radius * std::cos(a0), y1, center.z + radius * std::sin(a0));
        Vec3 p1(center.x + radius * std::cos(a1), y1, center.z + radius * std::sin(a1));
        addTriangle(top, p0, p1, nTop, nTop, nTop);

        Vec3 b0(center.x + radius * std::cos(a0), y0, center.z + radius * std::sin(a0));
        Vec3 b1(center.x + radius * std::cos(a1), y0, center.z + radius * std::sin(a1));
        addTriangle(bottom, b1, b0, nBottom, nBottom, nBottom);
    }
}

void Mesh::addCylinderZ(const Vec3& center, float radius, float height, int segments) {
    if (segments < 3) segments = 3;
    float z0 = center.z - height * 0.5f;
    float z1 = center.z + height * 0.5f;

    for (int i = 0; i < segments; ++i) {
        float a0 = (float)(2.0 * M_PI * i / segments);
        float a1 = (float)(2.0 * M_PI * (i + 1) / segments);

        float c0 = std::cos(a0), s0 = std::sin(a0);
        float c1 = std::cos(a1), s1 = std::sin(a1);

        Vec3 p0(center.x + radius * c0, center.y + radius * s0, z0);
        Vec3 p1(center.x + radius * c1, center.y + radius * s1, z0);
        Vec3 p2(center.x + radius * c1, center.y + radius * s1, z1);
        Vec3 p3(center.x + radius * c0, center.y + radius * s0, z1);

        Vec3 n0(c0, s0, 0);
        Vec3 n1(c1, s1, 0);

        addTriangle(p0, p1, p2, n0, n1, n1);
        addTriangle(p0, p2, p3, n0, n1, n0);
    }

    Vec3 front(center.x, center.y, z1);
    Vec3 back(center.x, center.y, z0);
    Vec3 nFront(0, 0, 1);
    Vec3 nBack(0, 0, -1);

    for (int i = 0; i < segments; ++i) {
        float a0 = (float)(2.0 * M_PI * i / segments);
        float a1 = (float)(2.0 * M_PI * (i + 1) / segments);

        Vec3 f0(center.x + radius * std::cos(a0), center.y + radius * std::sin(a0), z1);
        Vec3 f1(center.x + radius * std::cos(a1), center.y + radius * std::sin(a1), z1);
        addTriangle(front, f0, f1, nFront, nFront, nFront);

        Vec3 b0(center.x + radius * std::cos(a0), center.y + radius * std::sin(a0), z0);
        Vec3 b1(center.x + radius * std::cos(a1), center.y + radius * std::sin(a1), z0);
        addTriangle(back, b1, b0, nBack, nBack, nBack);
    }
}

Aabb Mesh::computeBounds() const {
    Aabb b;
    for (size_t i = 0; i < vertices.size(); ++i) {
        b.expand(vertices[i].position);
    }
    return b;
}

// -----------------------------------------------------------------------------
// Materials and simple types
// -----------------------------------------------------------------------------

Material::Material()
    : ambient(0.10f, 0.10f, 0.10f),
      diffuse(0.65f, 0.65f, 0.65f),
      specular(0.40f, 0.40f, 0.40f),
      emissive(0.0f, 0.0f, 0.0f),
      shininess(32.0f),
      name("Default") {}

Material::Material(const Vec3& a, const Vec3& d, const Vec3& s, float sh,
                   const std::string& n)
    : ambient(a), diffuse(d), specular(s), emissive(0, 0, 0),
      shininess(sh), name(n) {}

Part::Part()
    : explodeOffset(0, 0, 0), material(), name("Part") {}

Light::Light()
    : position(4.5f, 7.0f, 6.0f),
      color(1.0f, 0.98f, 0.95f),
      ambient(0.18f) {}

CameraState::CameraState()
    : target(0.0f, 2.2f, 0.0f),
      yaw(radiansf(32.0f)),
      pitch(radiansf(12.0f)),
      distance(7.8f),
      fovY(radiansf(45.0f)),
      orthographic(false) {}

Vec3 CameraState::eye() const {
    float cp = std::cos(pitch);
    float sp = std::sin(pitch);
    Vec3 offset(
        distance * cp * std::sin(yaw),
        distance * sp,
        distance * cp * std::cos(yaw)
    );
    return target + offset;
}

Mat4 CameraState::view() const {
    return Mat4::lookAt(eye(), target, Vec3(0, 1, 0));
}

Mat4 CameraState::projection(float aspect) const {
    if (orthographic) {
        float halfH = 4.0f;
        float halfW = halfH * aspect;
        return Mat4::orthographic(-halfW, halfW, -halfH, halfH, 0.1f, 100.0f);
    }
    return Mat4::perspective(fovY, aspect, 0.1f, 100.0f);
}

// -----------------------------------------------------------------------------
// App
// -----------------------------------------------------------------------------

App::App() {
    init();
}

void App::init() {
    camera = CameraState();
    preTourCamera = camera;

    light = Light();
    lightPreset = 0;
    lightAnimate = false;
    lightAngle = 0.0f;
    materialPresetCursor = 0;

    gridVisible = true;
    wireframe = false;
    showNormals = false;

    turntableActive = false;
    turntableAngle = 0.0f;
    turntableSpeed = 0.65f;

    explodeActive = false;
    explodeFactor = 0.0f;
    explodeSpeed = 1.15f;

    cameraTourActive = false;
    cameraTourTime = 0.0f;
    cameraTourSegmentDuration = 2.1f;
    cameraTourLoop = true;

    fps = 0.0f;
    fpsAccumTime = 0.0f;
    fpsAccumFrames = 0;

    viewportWidth = 1280;
    viewportHeight = 720;
    lastMouseX = 0;
    lastMouseY = 0;
    dragStartX = 0;
    dragStartY = 0;
    dragging = false;
    dragMoved = false;
    selectedPart = -1;

    buildScene();
    buildDefaultTour();
    setLightPreset(0);
}

void App::buildScene() {
    parts.clear();

    auto addPart = [&](const std::string& name, const Mesh& mesh,
                       const Material& material, const Vec3& explodeOffset) {
        Part p;
        p.name = name;
        p.mesh = mesh;
        p.material = material;
        p.explodeOffset = explodeOffset;
        p.localBounds = mesh.computeBounds();
        parts.push_back(p);
    };

    Material aluminum(
        Vec3(0.18f, 0.19f, 0.21f),
        Vec3(0.67f, 0.69f, 0.73f),
        Vec3(0.88f, 0.90f, 0.94f),
        72.0f, "Aluminium"
    );

    Material screenGlass(
        Vec3(0.030f, 0.050f, 0.090f),
        Vec3(0.080f, 0.160f, 0.320f),
        Vec3(0.90f, 0.95f, 1.00f),
        128.0f, "Screen Glass"
    );
    screenGlass.emissive = Vec3(0.025f, 0.065f, 0.135f);

    Material backGlass(
        Vec3(0.085f, 0.090f, 0.115f),
        Vec3(0.270f, 0.290f, 0.350f),
        Vec3(0.50f, 0.52f, 0.58f),
        36.0f, "Matte Glass"
    );

    Material batteryMat(
        Vec3(0.045f, 0.055f, 0.070f),
        Vec3(0.230f, 0.270f, 0.330f),
        Vec3(0.34f, 0.36f, 0.40f),
        18.0f, "Battery"
    );

    Material pcbMat(
        Vec3(0.035f, 0.085f, 0.045f),
        Vec3(0.100f, 0.340f, 0.170f),
        Vec3(0.25f, 0.36f, 0.25f),
        12.0f, "PCB"
    );

    Material cameraHousing(
        Vec3(0.035f, 0.035f, 0.045f),
        Vec3(0.115f, 0.120f, 0.145f),
        Vec3(0.62f, 0.64f, 0.70f),
        72.0f, "Camera Housing"
    );

    Material lensMat(
        Vec3(0.010f, 0.010f, 0.015f),
        Vec3(0.035f, 0.040f, 0.065f),
        Vec3(1.00f, 1.00f, 1.00f),
        128.0f, "Lens"
    );

    Material buttonMat(
        Vec3(0.16f, 0.17f, 0.19f),
        Vec3(0.55f, 0.57f, 0.60f),
        Vec3(0.78f, 0.80f, 0.84f),
        80.0f, "Metal Button"
    );

    Material portMat(
        Vec3(0.018f, 0.018f, 0.022f),
        Vec3(0.075f, 0.075f, 0.095f),
        Vec3(0.28f, 0.28f, 0.32f),
        20.0f, "Port"
    );

    // Frame: four bars forming a rectangular shell.
    Mesh frame;
    frame.addBox(Vec3(0.0f,  1.46f, 0.0f), Vec3(1.60f, 0.08f, 0.12f));
    frame.addBox(Vec3(0.0f, -1.46f, 0.0f), Vec3(1.60f, 0.08f, 0.12f));
    frame.addBox(Vec3(-0.76f, 0.0f, 0.0f), Vec3(0.08f, 2.84f, 0.12f));
    frame.addBox(Vec3( 0.76f, 0.0f, 0.0f), Vec3(0.08f, 2.84f, 0.12f));
    addPart("Frame", frame, aluminum, Vec3(0, 0, 0));

    Mesh screen;
    screen.addBox(Vec3(0.0f, 0.0f, 0.078f), Vec3(1.48f, 2.84f, 0.020f));
    addPart("Screen", screen, screenGlass, Vec3(-2.10f, 0.0f, 0.75f));

    Mesh frontCamera;
    frontCamera.addCylinderZ(Vec3(0.50f, 1.15f, 0.091f), 0.035f, 0.006f, 20);
    addPart("Front Camera", frontCamera, lensMat, Vec3(-2.10f, 0.0f, 0.77f));

    Mesh back;
    back.addBox(Vec3(0.0f, 0.0f, -0.078f), Vec3(1.52f, 2.88f, 0.030f));
    addPart("Back Cover", back, backGlass, Vec3(2.10f, 0.0f, -0.75f));

    Mesh battery;
    battery.addBox(Vec3(0.0f, -0.40f, -0.02f), Vec3(1.18f, 1.75f, 0.050f));
    addPart("Battery", battery, batteryMat, Vec3(-0.75f, -0.55f, -1.35f));

    Mesh pcb;
    pcb.addBox(Vec3(0.0f, 0.92f, -0.02f), Vec3(1.28f, 0.72f, 0.035f));
    addPart("Main Board", pcb, pcbMat, Vec3(0.75f, 0.55f, -1.35f));

    Mesh cameraModule;
    cameraModule.addBox(Vec3(0.44f, 1.02f, -0.115f), Vec3(0.38f, 0.38f, 0.055f));
    addPart("Camera Module", cameraModule, cameraHousing, Vec3(1.15f, 0.95f, -1.70f));

    Mesh lens;
    lens.addCylinderZ(Vec3(0.44f, 1.02f, -0.155f), 0.085f, 0.045f, 28);
    addPart("Camera Lens", lens, lensMat, Vec3(1.15f, 0.95f, -2.05f));

    Mesh powerButton;
    powerButton.addBox(Vec3(0.835f, 0.62f, 0.0f), Vec3(0.060f, 0.27f, 0.080f));
    addPart("Power Button", powerButton, buttonMat, Vec3(0.85f, 0.0f, 0.0f));

    Mesh volumeButtons;
    volumeButtons.addBox(Vec3(-0.835f, 0.82f, 0.0f), Vec3(0.060f, 0.20f, 0.080f));
    volumeButtons.addBox(Vec3(-0.835f, 0.48f, 0.0f), Vec3(0.060f, 0.16f, 0.080f));
    addPart("Volume Buttons", volumeButtons, buttonMat, Vec3(-0.85f, 0.0f, 0.0f));

    Mesh chargingPort;
    chargingPort.addBox(Vec3(0.0f, -1.505f, 0.0f), Vec3(0.34f, 0.045f, 0.065f));
    addPart("Charging Port", chargingPort, portMat, Vec3(0.0f, 0.0f, 1.20f));

    selectedPart = -1;
}

void App::buildDefaultTour() {
    cameraTour.clear();
    cameraTour.push_back(CameraKeyframe(Vec3(0.0f, 3.2f, 9.0f),  Vec3(0.0f, 2.0f, 0.0f)));
    cameraTour.push_back(CameraKeyframe(Vec3(7.0f, 4.4f, 6.0f),  Vec3(0.0f, 2.0f, 0.0f)));
    cameraTour.push_back(CameraKeyframe(Vec3(6.0f, 2.4f, -6.5f), Vec3(0.0f, 2.0f, 0.0f)));
    cameraTour.push_back(CameraKeyframe(Vec3(0.0f, 4.6f, -9.0f), Vec3(0.0f, 2.0f, 0.0f)));
    cameraTour.push_back(CameraKeyframe(Vec3(-6.0f, 3.0f, 6.5f), Vec3(0.0f, 2.0f, 0.0f)));
    cameraTour.push_back(CameraKeyframe(Vec3(0.0f, 3.2f, 9.0f),  Vec3(0.0f, 2.0f, 0.0f)));
}

void App::update(float dt) {
    if (dt > 0.1f) dt = 0.1f;

    fpsAccumTime += dt;
    ++fpsAccumFrames;
    if (fpsAccumTime >= 0.45f) {
        fps = (float)fpsAccumFrames / fpsAccumTime;
        fpsAccumTime = 0.0f;
        fpsAccumFrames = 0;
    }

    if (turntableActive) {
        turntableAngle += turntableSpeed * dt;
        if (turntableAngle > (float)(2.0 * M_PI)) turntableAngle -= (float)(2.0 * M_PI);
    }

    float target = explodeActive ? 1.0f : 0.0f;
    if (explodeFactor < target) {
        explodeFactor += explodeSpeed * dt;
        if (explodeFactor > target) explodeFactor = target;
    } else if (explodeFactor > target) {
        explodeFactor -= explodeSpeed * dt;
        if (explodeFactor < target) explodeFactor = target;
    }

    if (lightAnimate) {
        lightAngle += 0.75f * dt;
        if (lightAngle > (float)(2.0 * M_PI)) lightAngle -= (float)(2.0 * M_PI);
    }

    if (cameraTourActive) {
        updateCameraTour(dt);
    }
}

void App::updateCameraTour(float dt) {
    if (cameraTour.size() < 2) {
        stopCameraTour();
        return;
    }

    cameraTourTime += dt;
    int segments = (int)cameraTour.size() - 1;
    float totalDuration = cameraTourSegmentDuration * (float)segments;

    if (cameraTourTime >= totalDuration) {
        if (cameraTourLoop) {
            cameraTourTime = std::fmod(cameraTourTime, totalDuration);
        } else {
            stopCameraTour();
            return;
        }
    }

    int segment = (int)(cameraTourTime / cameraTourSegmentDuration);
    if (segment >= segments) segment = segments - 1;
    float localT = (cameraTourTime - cameraTourSegmentDuration * (float)segment)
                   / cameraTourSegmentDuration;
    float t = smoothstepf(localT);

    const CameraKeyframe& a = cameraTour[segment];
    const CameraKeyframe& b = cameraTour[segment + 1];

    Vec3 eye = a.position + (b.position - a.position) * t;
    Vec3 target = a.target + (b.target - a.target) * t;

    Vec3 d = eye - target;
    float dist = length(d);
    if (dist < 0.001f) dist = 0.001f;

    camera.target = target;
    camera.distance = dist;
    camera.pitch = std::asin(clampf(d.y / dist, -1.0f, 1.0f));
    camera.yaw = std::atan2(d.x, d.z);
}

void App::renderScene(int width, int height) {
    viewportWidth = width;
    viewportHeight = height;

    glViewport(0, 0, width, height);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);

    glClearColor(0.035f, 0.045f, 0.075f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawBackground(width, height);
    glClear(GL_DEPTH_BUFFER_BIT);

    float aspect = (float)width / (float)(height > 0 ? height : 1);
    Mat4 proj = camera.projection(aspect);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(proj.m);

    Mat4 view = camera.view();
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(view.m);

    glEnable(GL_DEPTH_TEST);

    if (gridVisible) drawGrid();
    drawPedestal();

    drawProduct(productRootMatrix());

    if (selectedPart >= 0 && selectedPart < (int)parts.size()) {
        drawSelectionBox(partWorldBounds(parts[selectedPart]));
    }

    if (showNormals) {
        for (size_t i = 0; i < parts.size(); ++i) {
            drawNormals(parts[i], partMatrix(parts[i]));
        }
    }

    glDisable(GL_DEPTH_TEST);
}

void App::drawBackground(int width, int height) {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, (double)width, 0.0, (double)height, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBegin(GL_QUADS);
    glColor3f(0.140f, 0.160f, 0.210f);
    glVertex2f(0.0f, 0.0f);
    glVertex2f((float)width, 0.0f);
    glColor3f(0.040f, 0.050f, 0.080f);
    glVertex2f((float)width, (float)height);
    glVertex2f(0.0f, (float)height);
    glEnd();
}

void App::drawGrid() {
    glLineWidth(1.0f);
    glColor3f(0.205f, 0.225f, 0.275f);

    std::vector<Vec2> points;

    // Lines parallel to X.
    for (int i = -10; i <= 10; ++i) {
        points.clear();
        ddaLinePoints(-10, i, 10, i, points);
        glBegin(GL_LINE_STRIP);
        for (size_t j = 0; j < points.size(); ++j) {
            glVertex3f(points[j].x, 0.001f, points[j].y);
        }
        glEnd();
    }

    // Lines parallel to Z.
    for (int i = -10; i <= 10; ++i) {
        points.clear();
        ddaLinePoints(i, -10, i, 10, points);
        glBegin(GL_LINE_STRIP);
        for (size_t j = 0; j < points.size(); ++j) {
            glVertex3f(points[j].x, 0.001f, points[j].y);
        }
        glEnd();
    }
}

void App::drawPedestal() {
    static Mesh pedestalMesh;
    static Material pedestalMat(
        Vec3(0.035f, 0.040f, 0.055f),
        Vec3(0.200f, 0.225f, 0.285f),
        Vec3(0.45f, 0.48f, 0.55f),
        48.0f, "Pedestal"
    );

    if (pedestalMesh.vertices.empty()) {
        pedestalMesh.addCylinderY(Vec3(0.0f, 0.35f, 0.0f), 1.75f, 0.70f, 72);
    }

    drawMesh(pedestalMesh, Mat4::identityMat(), pedestalMat, false, true);

    // A ring generated with the Bresenham circle algorithm.
    std::vector<Vec2> ring;
    bresenhamCirclePoints(0, 0, 100, ring);

    const float ringRadius = 1.55f;
    const float scale = ringRadius / 100.0f;

    glColor3f(0.52f, 0.57f, 0.66f);
    glLineWidth(1.6f);
    glBegin(GL_LINE_LOOP);
    for (size_t i = 0; i < ring.size(); ++i) {
        glVertex3f(ring[i].x * scale, 0.705f, ring[i].y * scale);
    }
    glEnd();
    glLineWidth(1.0f);
}

Mat4 App::productRootMatrix() const {
    return Mat4::translation(Vec3(0.0f, 2.2f, 0.0f)) * Mat4::rotationY(turntableAngle);
}

Mat4 App::partMatrix(const Part& part) const {
    float e = explodeEased();
    return productRootMatrix() * Mat4::translation(part.explodeOffset * e);
}

Aabb App::partWorldBounds(const Part& part) const {
    return transformAabb(part.localBounds, partMatrix(part));
}

float App::explodeEased() const {
    return smoothstepf(explodeFactor);
}

void App::drawProduct(const Mat4& productRoot) {
    (void)productRoot;

    if (wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    for (size_t i = 0; i < parts.size(); ++i) {
        Mat4 model = partMatrix(parts[i]);
        bool selected = ((int)i == selectedPart);
        drawMesh(parts[i].mesh, model, parts[i].material, selected, !wireframe);
    }

    if (wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void App::drawMesh(const Mesh& mesh, const Mat4& model, const Material& material,
                   bool selected, bool useMaterial) {
    glBegin(GL_TRIANGLES);
    for (size_t i = 0; i < mesh.vertices.size(); ++i) {
        const Vertex& v = mesh.vertices[i];
        Vec3 worldPos = model.transformPoint(v.position);
        Vec3 worldNormal = normalize(model.transformDirection(v.normal));

        Vec3 color;
        if (useMaterial) {
            color = shadePhong(worldPos, worldNormal, material);
            if (selected) {
                color = clamp01(color + Vec3(0.035f, 0.030f, 0.0f));
            }
        } else {
            color = selected ? Vec3(1.0f, 0.85f, 0.25f) : Vec3(0.55f, 0.72f, 0.95f);
        }

        glColor3f(color.x, color.y, color.z);
        glVertex3f(worldPos.x, worldPos.y, worldPos.z);
    }
    glEnd();
}

void App::drawNormals(const Part& part, const Mat4& model) {
    glColor3f(1.0f, 0.72f, 0.18f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for (size_t i = 0; i < part.mesh.vertices.size(); ++i) {
        const Vertex& v = part.mesh.vertices[i];
        Vec3 p = model.transformPoint(v.position);
        Vec3 n = normalize(model.transformDirection(v.normal));
        Vec3 q = p + n * 0.10f;
        glVertex3f(p.x, p.y, p.z);
        glVertex3f(q.x, q.y, q.z);
    }
    glEnd();
}

void App::drawSelectionBox(const Aabb& b) {
    Vec3 p[8] = {
        Vec3(b.mn.x, b.mn.y, b.mn.z),
        Vec3(b.mx.x, b.mn.y, b.mn.z),
        Vec3(b.mn.x, b.mx.y, b.mn.z),
        Vec3(b.mx.x, b.mx.y, b.mn.z),
        Vec3(b.mn.x, b.mn.y, b.mx.z),
        Vec3(b.mx.x, b.mn.y, b.mx.z),
        Vec3(b.mn.x, b.mx.y, b.mx.z),
        Vec3(b.mx.x, b.mx.y, b.mx.z)
    };

    const int edges[12][2] = {
        {0,1},{1,3},{3,2},{2,0},
        {4,5},{5,7},{7,6},{6,4},
        {0,4},{1,5},{2,6},{3,7}
    };

    glDisable(GL_DEPTH_TEST);
    glColor3f(1.0f, 0.84f, 0.20f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    for (int i = 0; i < 12; ++i) {
        const Vec3& a = p[edges[i][0]];
        const Vec3& c = p[edges[i][1]];
        glVertex3f(a.x, a.y, a.z);
        glVertex3f(c.x, c.y, c.z);
    }
    glEnd();
    glLineWidth(1.0f);
    glEnable(GL_DEPTH_TEST);
}

Vec3 App::currentLightPosition() const {
    if (lightAnimate) {
        return Vec3(
            std::cos(lightAngle) * 6.0f,
            7.0f,
            std::sin(lightAngle) * 6.0f
        );
    }
    return light.position;
}

Vec3 App::cameraEye() const {
    return camera.eye();
}

Vec3 App::selectedDiffuse() const {
    if (selectedPart >= 0 && selectedPart < (int)parts.size()) {
        return parts[selectedPart].material.diffuse;
    }
    if (!parts.empty()) {
        return parts[0].material.diffuse;
    }
    return Vec3(0.6f, 0.6f, 0.65f);
}

Vec3 App::shadePhong(const Vec3& worldPos, const Vec3& normal,
                     const Material& material) const {
    Vec3 N = normalize(normal);
    Vec3 lightPos = currentLightPosition();
    Vec3 toLight = lightPos - worldPos;
    float dist = length(toLight);
    Vec3 L = normalize(toLight);
    Vec3 V = normalize(cameraEye() - worldPos);
    Vec3 R = reflectVec(-L, N);

    float ndotl = std::max(dot(N, L), 0.0f);
    float rdotv = std::max(dot(R, V), 0.0f);

    // Soft inverse-square-ish falloff, tuned for the scene scale.
    float attenuation = 1.0f / (1.0f + 0.0035f * dist * dist);

    Vec3 ambient = material.ambient * light.ambient;
    Vec3 diffuse = material.diffuse * light.color * (ndotl * attenuation);
    Vec3 specular = material.specular * light.color
                    * (std::pow(rdotv, material.shininess) * attenuation);

    // Soft fill from the opposite side so faces turned away from the key light
    // are still readable.
    Vec3 toFill = fillLightPosition - worldPos;
    float fillDist = length(toFill);
    Vec3 fillDir = normalize(toFill);
    float ndotf = std::max(dot(N, fillDir), 0.0f);
    float fillAttenuation = 1.0f / (1.0f + 0.0045f * fillDist * fillDist);
    Vec3 fill = material.diffuse * fillLightColor * (ndotf * fillAttenuation * fillLightStrength);

    // Cool rim light separates the product from the dark background.
    float rim = std::pow(std::max(1.0f - std::max(dot(N, V), 0.0f), 0.0f), 3.0f);
    Vec3 rimLight = rimLightColor * rim * rimLightStrength * material.diffuse;

    return clamp01(ambient + diffuse + specular + fill + rimLight + material.emissive);
}

void App::renderHud(int width, int height) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, (double)width, (double)height, 0.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    drawHudPanel(12.0f, (float)height - 92.0f,
                 (float)width - 24.0f, 80.0f);

    const Part* selected = NULL;
    if (selectedPart >= 0 && selectedPart < (int)parts.size()) {
        selected = &parts[selectedPart];
    }

    char line[512];

    glColor3f(0.96f, 0.97f, 1.0f);
    if (selected) {
        std::sprintf(line, "3D PRODUCT SHOWROOM   |   Selected: %s   |   Material: %s",
                     selected->name.c_str(), selected->material.name.c_str());
    } else {
        std::sprintf(line, "3D PRODUCT SHOWROOM   |   Selected: None   |   Click the product to select a part");
    }
    drawText2D(22.0f, (float)height - 68.0f, line);

    glColor3f(0.78f, 0.82f, 0.90f);
    std::sprintf(line,
                 "LMB drag: Orbit   Wheel: Zoom   Click: Select   E: Explode   T: Turntable   C: Camera Tour   R: Reset");
    drawText2D(22.0f, (float)height - 46.0f, line);

    glColor3f(0.66f, 0.72f, 0.82f);
    std::sprintf(line,
                 "Turntable: %s   |   Exploded: %d%%   |   Camera Tour: %s   |   Light: %s   |   FPS: %.0f",
                 turntableActive ? "ON" : "OFF",
                 (int)(explodeFactor * 100.0f),
                 cameraTourActive ? "PLAYING" : "OFF",
                 lightAnimate ? "ORBIT" : "STATIC",
                 fps);
    drawText2D(22.0f, (float)height - 24.0f, line);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void App::drawHudPanel(float x, float y, float w, float h) {
    glColor4f(0.015f, 0.025f, 0.050f, 0.82f);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();

    glColor4f(0.30f, 0.40f, 0.60f, 0.35f);
    glLineWidth(1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void App::toggleTurntable() {
    turntableActive = !turntableActive;
}

void App::toggleExplode() {
    explodeActive = !explodeActive;
}

void App::playCameraTour() {
    if (cameraTour.size() < 2) {
        buildDefaultTour();
    }
    preTourCamera = camera;
    cameraTourTime = 0.0f;
    cameraTourActive = true;
}

void App::stopCameraTour() {
    if (cameraTourActive) {
        cameraTourActive = false;
        camera = preTourCamera;
    }
}

void App::recordCameraKeyframe() {
    cameraTour.push_back(CameraKeyframe(camera.eye(), camera.target));
    if (cameraTour.size() > 16) {
        cameraTour.erase(cameraTour.begin());
    }
}

void App::clearCameraTour() {
    buildDefaultTour();
    cameraTourActive = false;
    camera = preTourCamera;
}

void App::resetCamera() {
    camera = CameraState();
    preTourCamera = camera;
    cameraTourActive = false;
}

void App::resetAll() {
    init();
}

void App::setLightPreset(int index) {
    if (index < 0) index = 0;
    if (index > 2) index = 2;
    lightPreset = index;

    if (lightPreset == 0) {
        light.position = Vec3(4.5f, 7.0f, 6.0f);
        light.color = Vec3(1.00f, 1.00f, 0.98f);
        light.ambient = 0.46f;
        fillLightPosition = Vec3(-5.0f, 5.5f, -5.5f);
        fillLightColor = Vec3(0.58f, 0.68f, 0.88f);
        fillLightStrength = 0.52f;
        rimLightColor = Vec3(0.72f, 0.82f, 1.00f);
        rimLightStrength = 0.20f;
    } else if (lightPreset == 1) {
        light.position = Vec3(5.0f, 5.5f, 5.5f);
        light.color = Vec3(1.00f, 0.82f, 0.62f);
        light.ambient = 0.40f;
        fillLightPosition = Vec3(-4.5f, 5.0f, -5.0f);
        fillLightColor = Vec3(0.55f, 0.68f, 0.88f);
        fillLightStrength = 0.48f;
        rimLightColor = Vec3(1.00f, 0.74f, 0.46f);
        rimLightStrength = 0.16f;
    } else {
        light.position = Vec3(-4.5f, 6.0f, 5.5f);
        light.color = Vec3(0.72f, 0.85f, 1.00f);
        light.ambient = 0.44f;
        fillLightPosition = Vec3(4.5f, 5.0f, -5.0f);
        fillLightColor = Vec3(1.00f, 0.78f, 0.56f);
        fillLightStrength = 0.48f;
        rimLightColor = Vec3(0.72f, 0.88f, 1.00f);
        rimLightStrength = 0.20f;
    }
}

void App::toggleLightAnimation() {
    lightAnimate = !lightAnimate;
    if (lightAnimate) lightAngle = 0.0f;
}

void App::setPartMaterialPreset(Part& part, int preset) {
    if (preset == 0) {
        part.material = Material(
            Vec3(0.08f, 0.08f, 0.08f),
            Vec3(0.52f, 0.53f, 0.56f),
            Vec3(0.35f, 0.36f, 0.38f),
            32.0f, "Plastic"
        );
    } else if (preset == 1) {
        part.material = Material(
            Vec3(0.18f, 0.19f, 0.21f),
            Vec3(0.66f, 0.68f, 0.72f),
            Vec3(0.90f, 0.92f, 0.96f),
            96.0f, "Metal"
        );
    } else if (preset == 2) {
        part.material = Material(
            Vec3(0.10f, 0.10f, 0.10f),
            Vec3(0.46f, 0.47f, 0.50f),
            Vec3(0.12f, 0.12f, 0.12f),
            8.0f, "Matte"
        );
    } else if (preset == 3) {
        part.material = Material(
            Vec3(0.06f, 0.06f, 0.065f),
            Vec3(0.20f, 0.22f, 0.26f),
            Vec3(0.92f, 0.93f, 0.97f),
            128.0f, "Glossy"
        );
    } else {
        part.material = Material(
            Vec3(0.22f, 0.15f, 0.05f),
            Vec3(0.83f, 0.62f, 0.20f),
            Vec3(1.00f, 0.86f, 0.45f),
            88.0f, "Gold"
        );
    }
}

void App::applyMaterialPreset(int preset) {
    if (parts.empty()) return;

    int index = selectedPart;
    if (index < 0 || index >= (int)parts.size()) {
        index = 0;
    }

    setPartMaterialPreset(parts[index], preset);
    if (parts[index].name == "Screen") {
        parts[index].material.emissive = parts[index].material.diffuse * 0.08f;
    }
    selectedPart = index;
}

void App::setSelectedDiffuse(const Vec3& color) {
    if (parts.empty()) return;

    int index = selectedPart;
    if (index < 0 || index >= (int)parts.size()) {
        index = 0;
    }

    Vec3 c = clamp01(color);
    parts[index].material.diffuse = c;
    parts[index].material.ambient = c * 0.20f;
    parts[index].material.emissive = (parts[index].name == "Screen") ? c * 0.10f : Vec3(0.0f, 0.0f, 0.0f);
    parts[index].material.name = "Custom";
    selectedPart = index;
}

void App::toggleGrid() {
    gridVisible = !gridVisible;
}

void App::toggleWireframe() {
    wireframe = !wireframe;
}

void App::toggleNormals() {
    showNormals = !showNormals;
}

void App::toggleProjection() {
    camera.orthographic = !camera.orthographic;
}

void App::onMouseDown(int x, int y) {
    dragStartX = x;
    dragStartY = y;
    lastMouseX = x;
    lastMouseY = y;
    dragging = true;
    dragMoved = false;
}

void App::onMouseMove(int x, int y, bool leftDown) {
    if (!leftDown || !dragging) {
        lastMouseX = x;
        lastMouseY = y;
        return;
    }

    if (cameraTourActive) {
        lastMouseX = x;
        lastMouseY = y;
        return;
    }

    int dx = x - lastMouseX;
    int dy = y - lastMouseY;
    lastMouseX = x;
    lastMouseY = y;

    if (std::abs(x - dragStartX) + std::abs(y - dragStartY) > 4) {
        dragMoved = true;
    }

    camera.yaw += (float)dx * 0.008f;
    camera.pitch -= (float)dy * 0.008f;
    camera.pitch = clampf(camera.pitch, radiansf(-78.0f), radiansf(78.0f));
}

void App::onMouseUp() {
    if (!dragging) return;
    dragging = false;

    if (!dragMoved) {
        selectedPart = pickPart(lastMouseX, lastMouseY, viewportWidth, viewportHeight);
    }
}

void App::onMouseWheel(int delta) {
    if (cameraTourActive) return;
    float factor = (delta > 0) ? 0.90f : 1.10f;
    camera.distance *= factor;
    camera.distance = clampf(camera.distance, 3.2f, 16.0f);
}

void App::onKeyDown(int vk) {
    switch (vk) {
    case 'E':
        toggleExplode();
        break;
    case 'T':
        toggleTurntable();
        break;
    case 'C':
        if (cameraTourActive) stopCameraTour();
        else playCameraTour();
        break;
    case 'R':
        resetCamera();
        break;
    case 'G':
        toggleGrid();
        break;
    case 'W':
        toggleWireframe();
        break;
    case 'N':
        toggleNormals();
        break;
    case 'P':
        toggleProjection();
        break;
    case 'L':
        setLightPreset((lightPreset + 1) % 3);
        break;
    case 'F':
        toggleLightAnimation();
        break;
    case 'M':
        applyMaterialPreset(materialPresetCursor);
        materialPresetCursor = (materialPresetCursor + 1) % 5;
        break;
    case 'X':
        resetAll();
        break;
    case '1':
        setLightPreset(0);
        break;
    case '2':
        setLightPreset(1);
        break;
    case '3':
        setLightPreset(2);
        break;
    default:
        break;
    }
}

int App::pickPart(int mouseX, int mouseY, int width, int height) const {
    if (parts.empty() || width <= 0 || height <= 0) return -1;

    float ndcX = 2.0f * (float)mouseX / (float)width - 1.0f;
    float ndcY = 1.0f - 2.0f * (float)mouseY / (float)height;

    Vec3 eye = camera.eye();
    Vec3 forward = normalize(camera.target - eye);
    Vec3 right = normalize(cross(forward, Vec3(0, 1, 0)));
    Vec3 up = normalize(cross(right, forward));

    float aspect = (float)width / (float)height;
    Vec3 origin;
    Vec3 dir;

    if (camera.orthographic) {
        float halfH = 4.0f;
        float halfW = halfH * aspect;
        origin = eye + right * (ndcX * halfW) + up * (ndcY * halfH);
        dir = forward;
    } else {
        float halfH = std::tan(camera.fovY * 0.5f);
        float halfW = halfH * aspect;
        dir = normalize(forward + right * (ndcX * halfW) + up * (ndcY * halfH));
        origin = eye;
    }

    int bestIndex = -1;
    float bestT = 1e30f;

    for (size_t i = 0; i < parts.size(); ++i) {
        Aabb world = partWorldBounds(parts[i]);
        float t = 0.0f;
        if (rayAabb(origin, dir, world, t)) {
            if (t < bestT) {
                bestT = t;
                bestIndex = (int)i;
            }
        }
    }

    return bestIndex;
}

void App::saveConfig(const std::string& path) const {
    std::ofstream out(path.c_str());
    if (!out) return;

    out << std::fixed << std::setprecision(5);
    out << "SHOWROOM_CONFIG 1\n";
    out << "LIGHT_PRESET " << lightPreset << "\n";
    out << "TURNTABLE_SPEED " << turntableSpeed << "\n";
    out << "CAMERA " << camera.yaw << " " << camera.pitch << " " << camera.distance
        << " " << camera.target.x << " " << camera.target.y << " " << camera.target.z
        << " " << (camera.orthographic ? 1 : 0) << "\n";
    out << "VIEW " << (gridVisible ? 1 : 0) << " " << (wireframe ? 1 : 0)
        << " " << (showNormals ? 1 : 0) << "\n";
    out << "ANIM " << (turntableActive ? 1 : 0) << " " << (explodeActive ? 1 : 0)
        << " " << (lightAnimate ? 1 : 0) << " " << (cameraTourActive ? 1 : 0) << "\n";
    out << "SELECTED " << selectedPart << "\n";

    for (size_t i = 0; i < parts.size(); ++i) {
        const Material& m = parts[i].material;
        out << "PART " << i << " "
            << m.ambient.x << " " << m.ambient.y << " " << m.ambient.z << " "
            << m.diffuse.x << " " << m.diffuse.y << " " << m.diffuse.z << " "
            << m.specular.x << " " << m.specular.y << " " << m.specular.z << " "
            << m.emissive.x << " " << m.emissive.y << " " << m.emissive.z << " "
            << m.shininess << "\n";
    }

    for (size_t i = 0; i < cameraTour.size(); ++i) {
        out << "CAMERA_KEYFRAME "
            << cameraTour[i].position.x << " "
            << cameraTour[i].position.y << " "
            << cameraTour[i].position.z << " "
            << cameraTour[i].target.x << " "
            << cameraTour[i].target.y << " "
            << cameraTour[i].target.z << "\n";
    }

    out << "END\n";
}

bool App::loadConfig(const std::string& path) {
    std::ifstream in(path.c_str());
    if (!in) return false;

    std::vector<CameraKeyframe> loadedTour;
    int loadedPreset = -1;
    float loadedSpeed = turntableSpeed;
    std::string line;

    while (std::getline(in, line)) {
        std::stringstream ss(line);
        std::string tag;
        ss >> tag;
        if (tag.empty()) continue;

        if (tag == "LIGHT_PRESET") {
            ss >> loadedPreset;
        } else if (tag == "TURNTABLE_SPEED") {
            ss >> loadedSpeed;
        } else if (tag == "CAMERA") {
            int ortho = 0;
            ss >> camera.yaw >> camera.pitch >> camera.distance
               >> camera.target.x >> camera.target.y >> camera.target.z >> ortho;
            camera.pitch = clampf(camera.pitch, radiansf(-78.0f), radiansf(78.0f));
            camera.distance = clampf(camera.distance, 3.2f, 16.0f);
            camera.orthographic = (ortho != 0);
            preTourCamera = camera;
        } else if (tag == "VIEW") {
            int g = 1, w = 0, n = 0;
            ss >> g >> w >> n;
            gridVisible = (g != 0);
            wireframe = (w != 0);
            showNormals = (n != 0);
        } else if (tag == "ANIM") {
            int t = 0, e = 0, l = 0, c = 0;
            ss >> t >> e >> l >> c;
            turntableActive = (t != 0);
            explodeActive = (e != 0);
            explodeFactor = explodeActive ? 1.0f : 0.0f;
            lightAnimate = (l != 0);
            if (c != 0 && cameraTour.size() >= 2) {
                preTourCamera = camera;
                cameraTourActive = true;
                cameraTourTime = 0.0f;
            } else {
                cameraTourActive = false;
            }
        } else if (tag == "SELECTED") {
            int index = -1;
            ss >> index;
            if (index >= -1 && index < (int)parts.size()) {
                selectedPart = index;
            }
        } else if (tag == "PART") {
            int index = -1;
            float ar, ag, ab;
            float dr, dg, db;
            float sr, sg, sb;
            float er, eg, eb;
            float sh;
            ss >> index >> ar >> ag >> ab >> dr >> dg >> db
               >> sr >> sg >> sb >> er >> eg >> eb >> sh;
            if (index >= 0 && index < (int)parts.size()) {
                Material& m = parts[index].material;
                m.ambient = Vec3(ar, ag, ab);
                m.diffuse = Vec3(dr, dg, db);
                m.specular = Vec3(sr, sg, sb);
                m.emissive = Vec3(er, eg, eb);
                m.shininess = sh;
                m.name = "Loaded";
            }
        } else if (tag == "CAMERA_KEYFRAME") {
            Vec3 p, t;
            ss >> p.x >> p.y >> p.z >> t.x >> t.y >> t.z;
            loadedTour.push_back(CameraKeyframe(p, t));
        }
    }

    if (loadedPreset >= 0) {
        setLightPreset(loadedPreset);
    }
    turntableSpeed = clampf(loadedSpeed, 0.05f, 3.0f);

    if (loadedTour.size() >= 2) {
        cameraTour = loadedTour;
    }

    return true;
}
