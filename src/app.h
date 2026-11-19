#pragma once

#include "math3d.h"

#include <vector>
#include <string>

struct Vertex {
    Vec3 position;
    Vec3 normal;
    Vertex() {}
    Vertex(const Vec3& p, const Vec3& n) : position(p), normal(n) {}
};

struct Mesh {
    std::vector<Vertex> vertices;

    void clear() { vertices.clear(); }
    void addTriangle(const Vec3& a, const Vec3& b, const Vec3& c,
                     const Vec3& na, const Vec3& nb, const Vec3& nc);
    void addBox(const Vec3& center, const Vec3& size);
    void addCylinderY(const Vec3& center, float radius, float height, int segments);
    void addCylinderZ(const Vec3& center, float radius, float height, int segments);
    Aabb computeBounds() const;
};

struct Material {
    Vec3 ambient;
    Vec3 diffuse;
    Vec3 specular;
    Vec3 emissive;
    float shininess;
    std::string name;

    Material();
    Material(const Vec3& a, const Vec3& d, const Vec3& s, float sh, const std::string& n);
};

struct Part {
    Mesh mesh;
    Aabb localBounds;
    Vec3 explodeOffset;
    Material material;
    std::string name;

    Part();
};

struct Light {
    Vec3 position;
    Vec3 color;
    float ambient;

    Light();
};

struct CameraKeyframe {
    Vec3 position;
    Vec3 target;
    CameraKeyframe() {}
    CameraKeyframe(const Vec3& p, const Vec3& t) : position(p), target(t) {}
};

struct CameraState {
    Vec3 target;
    float yaw;       // radians
    float pitch;     // radians
    float distance;
    float fovY;      // radians
    bool orthographic;

    CameraState();
    Vec3 eye() const;
    Mat4 view() const;
    Mat4 projection(float aspect) const;
};

// Custom 5x7 bitmap font, implemented in app.cpp. Requires the 2D ortho
// projection to be active.
void drawText2D(float x, float y, const char* text);

struct App {
    std::vector<Part> parts;
    int selectedPart;

    CameraState camera;
    CameraState preTourCamera;

    Light light;
    int lightPreset;
    bool lightAnimate;
    float lightAngle;
    Vec3 fillLightPosition;
    Vec3 fillLightColor;
    float fillLightStrength;
    Vec3 rimLightColor;
    float rimLightStrength;
    int materialPresetCursor;

    bool gridVisible;
    bool wireframe;
    bool showNormals;

    bool turntableActive;
    float turntableAngle;
    float turntableSpeed;

    bool explodeActive;
    float explodeFactor;
    float explodeSpeed;

    bool cameraTourActive;
    float cameraTourTime;
    float cameraTourSegmentDuration;
    bool cameraTourLoop;
    std::vector<CameraKeyframe> cameraTour;

    float fps;
    float fpsAccumTime;
    int fpsAccumFrames;

    int viewportWidth;
    int viewportHeight;
    int lastMouseX;
    int lastMouseY;
    int dragStartX;
    int dragStartY;
    bool dragging;
    bool dragMoved;

    App();

    void init();
    void update(float dt);
    void renderScene(int width, int height);
    void renderHud(int width, int height);

    void onMouseDown(int x, int y);
    void onMouseMove(int x, int y, bool leftDown);
    void onMouseUp();
    void onMouseWheel(int delta);
    void onKeyDown(int vk);

    void toggleTurntable();
    void toggleExplode();
    void playCameraTour();
    void stopCameraTour();
    void recordCameraKeyframe();
    void clearCameraTour();
    void resetCamera();
    void resetAll();
    void setLightPreset(int index);
    void toggleLightAnimation();
    void applyMaterialPreset(int preset);
    void setSelectedDiffuse(const Vec3& color);
    void toggleGrid();
    void toggleWireframe();
    void toggleNormals();
    void toggleProjection();

    void saveConfig(const std::string& path) const;
    bool loadConfig(const std::string& path);

    Vec3 cameraEye() const;
    Vec3 selectedDiffuse() const;
    int pickPart(int mouseX, int mouseY, int width, int height) const;
    bool isExploded() const { return explodeFactor > 0.01f; }

private:
    void buildScene();
    void buildDefaultTour();
    void setPartMaterialPreset(Part& part, int preset);

    void drawBackground(int width, int height);
    void drawGrid();
    void drawPedestal();
    void drawProduct(const Mat4& productRoot);
    void drawMesh(const Mesh& mesh, const Mat4& model, const Material& material,
                  bool selected, bool useMaterial = true);
    void drawNormals(const Part& part, const Mat4& model);
    void drawSelectionBox(const Aabb& worldBounds);
    void drawHudPanel(float x, float y, float w, float h);

    Vec3 shadePhong(const Vec3& worldPos, const Vec3& normal,
                    const Material& material) const;

    float explodeEased() const;
    Mat4 productRootMatrix() const;
    Mat4 partMatrix(const Part& part) const;
    Aabb partWorldBounds(const Part& part) const;

    void updateCameraTour(float dt);
    Vec3 currentLightPosition() const;
};
