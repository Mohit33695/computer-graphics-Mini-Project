#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <GL/gl.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>

#include "app.h"

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------

static HDC   g_hDC = NULL;
static HGLRC g_hRC = NULL;
static bool  g_running = true;
static App*  g_app = NULL;
static int   g_width = 1280;
static int   g_height = 720;

static const char* kWindowClass = "CGProductShowroomWindow";
static const char* kWindowTitle = "3D Product Showroom and Configurator - Computer Graphics Project";

// Menu command IDs.
enum {
    ID_FILE_SAVE = 1001,
    ID_FILE_LOAD,
    ID_FILE_RESET,
    ID_FILE_EXIT,

    ID_VIEW_TURNTABLE,
    ID_VIEW_RESET_CAMERA,
    ID_VIEW_GRID,
    ID_VIEW_WIREFRAME,
    ID_VIEW_NORMALS,
    ID_VIEW_ORTHO,

    ID_MAT_PLASTIC,
    ID_MAT_METAL,
    ID_MAT_MATTE,
    ID_MAT_GLOSSY,
    ID_MAT_GOLD,
    ID_MAT_COLOR,

    ID_LIGHT_STUDIO,
    ID_LIGHT_WARM,
    ID_LIGHT_COOL,
    ID_LIGHT_ANIMATE,

    ID_ANIM_EXPLODE,
    ID_ANIM_TOUR,
    ID_ANIM_RECORD,
    ID_ANIM_CLEAR_TOUR,

    ID_HELP_CONTROLS,
    ID_HELP_ABOUT
};

// -----------------------------------------------------------------------------
// Forward declarations
// -----------------------------------------------------------------------------

static void updateMenuChecks(HWND hwnd);
static void showControls(HWND hwnd);
static void showAbout(HWND hwnd);
static void saveDialog(HWND hwnd);
static void loadDialog(HWND hwnd);
static void chooseColor(HWND hwnd);
static void handleCommand(HWND hwnd, int id);

// -----------------------------------------------------------------------------
// Menus
// -----------------------------------------------------------------------------

static HMENU createMenuBar() {
    HMENU menu = CreateMenu();

    HMENU fileMenu = CreatePopupMenu();
    AppendMenuA(fileMenu, MF_STRING, ID_FILE_SAVE, "&Save Configuration...\tCtrl+S");
    AppendMenuA(fileMenu, MF_STRING, ID_FILE_LOAD, "&Load Configuration...\tCtrl+O");
    AppendMenuA(fileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuA(fileMenu, MF_STRING, ID_FILE_RESET, "&Reset Showroom\tX");
    AppendMenuA(fileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuA(fileMenu, MF_STRING, ID_FILE_EXIT, "E&xit\tEsc");
    AppendMenuA(menu, MF_POPUP, (UINT_PTR)fileMenu, "&File");

    HMENU viewMenu = CreatePopupMenu();
    AppendMenuA(viewMenu, MF_STRING | MF_CHECKED, ID_VIEW_TURNTABLE, "&Turntable\tT");
    AppendMenuA(viewMenu, MF_STRING, ID_VIEW_RESET_CAMERA, "&Reset Camera\tR");
    AppendMenuA(viewMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuA(viewMenu, MF_STRING | MF_CHECKED, ID_VIEW_GRID, "Show &Grid\tG");
    AppendMenuA(viewMenu, MF_STRING, ID_VIEW_WIREFRAME, "&Wireframe\tW");
    AppendMenuA(viewMenu, MF_STRING, ID_VIEW_NORMALS, "Show &Normals\tN");
    AppendMenuA(viewMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuA(viewMenu, MF_STRING, ID_VIEW_ORTHO, "&Orthographic Projection\tP");
    AppendMenuA(menu, MF_POPUP, (UINT_PTR)viewMenu, "&View");

    HMENU materialMenu = CreatePopupMenu();
    AppendMenuA(materialMenu, MF_STRING, ID_MAT_PLASTIC, "&Plastic");
    AppendMenuA(materialMenu, MF_STRING, ID_MAT_METAL, "&Metal");
    AppendMenuA(materialMenu, MF_STRING, ID_MAT_MATTE, "&Matte");
    AppendMenuA(materialMenu, MF_STRING, ID_MAT_GLOSSY, "&Glossy");
    AppendMenuA(materialMenu, MF_STRING, ID_MAT_GOLD, "&Gold");
    AppendMenuA(materialMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuA(materialMenu, MF_STRING, ID_MAT_COLOR, "Change &Colour...");

    HMENU lightMenu = CreatePopupMenu();
    AppendMenuA(lightMenu, MF_STRING, ID_LIGHT_STUDIO, "&Studio");
    AppendMenuA(lightMenu, MF_STRING, ID_LIGHT_WARM, "&Warm");
    AppendMenuA(lightMenu, MF_STRING, ID_LIGHT_COOL, "C&ool");
    AppendMenuA(lightMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuA(lightMenu, MF_STRING, ID_LIGHT_ANIMATE, "&Orbit Light\tF");

    AppendMenuA(materialMenu, MF_POPUP, (UINT_PTR)lightMenu, "&Light");
    AppendMenuA(menu, MF_POPUP, (UINT_PTR)materialMenu, "&Material");

    HMENU animateMenu = CreatePopupMenu();
    AppendMenuA(animateMenu, MF_STRING, ID_ANIM_EXPLODE, "&Exploded View\tE");
    AppendMenuA(animateMenu, MF_STRING, ID_ANIM_TOUR, "&Camera Tour\tC");
    AppendMenuA(animateMenu, MF_STRING, ID_ANIM_RECORD, "&Record Camera Keyframe");
    AppendMenuA(animateMenu, MF_STRING, ID_ANIM_CLEAR_TOUR, "Reset Camera &Tour");
    AppendMenuA(menu, MF_POPUP, (UINT_PTR)animateMenu, "&Animate");

    HMENU helpMenu = CreatePopupMenu();
    AppendMenuA(helpMenu, MF_STRING, ID_HELP_CONTROLS, "&Controls\tF1");
    AppendMenuA(helpMenu, MF_STRING, ID_HELP_ABOUT, "&About");
    AppendMenuA(menu, MF_POPUP, (UINT_PTR)helpMenu, "&Help");

    return menu;
}

static void updateMenuChecks(HWND hwnd) {
    if (!g_app) return;

    HMENU menu = GetMenu(hwnd);
    if (!menu) return;

    CheckMenuItem(menu, ID_VIEW_TURNTABLE,
                  MF_BYCOMMAND | (g_app->turntableActive ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, ID_VIEW_GRID,
                  MF_BYCOMMAND | (g_app->gridVisible ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, ID_VIEW_WIREFRAME,
                  MF_BYCOMMAND | (g_app->wireframe ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, ID_VIEW_NORMALS,
                  MF_BYCOMMAND | (g_app->showNormals ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, ID_VIEW_ORTHO,
                  MF_BYCOMMAND | (g_app->camera.orthographic ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, ID_ANIM_EXPLODE,
                  MF_BYCOMMAND | (g_app->explodeActive ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, ID_ANIM_TOUR,
                  MF_BYCOMMAND | (g_app->cameraTourActive ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(menu, ID_LIGHT_ANIMATE,
                  MF_BYCOMMAND | (g_app->lightAnimate ? MF_CHECKED : MF_UNCHECKED));

    CheckMenuRadioItem(menu, ID_LIGHT_STUDIO, ID_LIGHT_COOL,
                       ID_LIGHT_STUDIO + g_app->lightPreset, MF_BYCOMMAND);
}

// -----------------------------------------------------------------------------
// File dialogs and colour picker
// -----------------------------------------------------------------------------

static void saveDialog(HWND hwnd) {
    if (!g_app) return;

    char fileName[MAX_PATH] = "showroom.cfg";

    OPENFILENAMEA ofn;
    std::memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "Showroom Config (*.cfg)\0*.cfg\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrDefExt = "cfg";
    ofn.lpstrTitle = "Save Showroom Configuration";
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

    if (GetSaveFileNameA(&ofn)) {
        g_app->saveConfig(fileName);
        MessageBoxA(hwnd, "Configuration saved successfully.",
                    "Save Configuration", MB_OK | MB_ICONINFORMATION);
    }
}

static void loadDialog(HWND hwnd) {
    if (!g_app) return;

    char fileName[MAX_PATH] = "";

    OPENFILENAMEA ofn;
    std::memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "Showroom Config (*.cfg)\0*.cfg\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrDefExt = "cfg";
    ofn.lpstrTitle = "Load Showroom Configuration";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileNameA(&ofn)) {
        if (g_app->loadConfig(fileName)) {
            MessageBoxA(hwnd, "Configuration loaded successfully.",
                        "Load Configuration", MB_OK | MB_ICONINFORMATION);
        } else {
            MessageBoxA(hwnd, "Could not load the configuration file.",
                        "Load Configuration", MB_OK | MB_ICONERROR);
        }
    }
}

static void chooseColor(HWND hwnd) {
    if (!g_app) return;

    static COLORREF customColors[16] = {0};

    Vec3 c = g_app->selectedDiffuse();
    CHOOSECOLORA cc;
    std::memset(&cc, 0, sizeof(cc));
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = hwnd;
    cc.lpCustColors = customColors;
    cc.rgbResult = RGB(
        (BYTE)(clampf(c.x, 0.0f, 1.0f) * 255.0f),
        (BYTE)(clampf(c.y, 0.0f, 1.0f) * 255.0f),
        (BYTE)(clampf(c.z, 0.0f, 1.0f) * 255.0f)
    );
    cc.Flags = CC_FULLOPEN | CC_RGBINIT;

    if (ChooseColorA(&cc)) {
        g_app->setSelectedDiffuse(Vec3(
            GetRValue(cc.rgbResult) / 255.0f,
            GetGValue(cc.rgbResult) / 255.0f,
            GetBValue(cc.rgbResult) / 255.0f
        ));
    }
}

// -----------------------------------------------------------------------------
// Help dialogs
// -----------------------------------------------------------------------------

static void showControls(HWND hwnd) {
    MessageBoxA(
        hwnd,
        "MOUSE\n"
        "  Left drag  : orbit camera\n"
        "  Mouse wheel: zoom in / out\n"
        "  Left click : select a product part\n\n"
        "KEYBOARD\n"
        "  E : exploded view\n"
        "  T : turntable\n"
        "  C : camera tour\n"
        "  R : reset camera\n"
        "  G : show / hide grid\n"
        "  W : wireframe\n"
        "  N : show normals\n"
        "  P : perspective / orthographic\n"
        "  L : cycle light preset\n"
        "  F : orbit light\n"
        "  M : cycle material preset\n"
        "  X : reset showroom\n"
        "  1 : studio light\n"
        "  2 : warm light\n"
        "  3 : cool light\n"
        "  Ctrl+S / Ctrl+O : save / load configuration\n"
        "  F1 : this help screen\n"
        "  Esc: exit\n\n"
        "MENUS\n"
        "  File     : save / load configuration\n"
        "  View     : camera and display options\n"
        "  Material : presets and custom colour\n"
        "  Animate  : exploded view and camera tour\n"
        "  Help     : this screen\n\n"
        "GRAPHICS\n"
        "  DDA / Bresenham, 3D transforms, Phong lighting,\n"
        "  ray-AABB picking, keyframe interpolation.",
        "Controls",
        MB_OK | MB_ICONINFORMATION
    );
}

static void showAbout(HWND hwnd) {
    MessageBoxA(
        hwnd,
        "3D Product Showroom and Configurator\n\n"
        "Computer Graphics Mini Project\n"
        "C++ and OpenGL\n\n"
        "Topics demonstrated:\n"
        "  - 3D transformations and homogeneous coordinates\n"
        "  - Perspective and orthographic projection\n"
        "  - RGB material colours and colour selection\n"
        "  - Ambient, diffuse and specular Phong lighting\n"
        "  - Interactive input, picking and menus\n"
        "  - Turntable, exploded-view and camera-tour animation\n"
        "  - DDA / Bresenham grid and pedestal ring\n",
        "About",
        MB_OK | MB_ICONINFORMATION
    );
}

// -----------------------------------------------------------------------------
// Command handling
// -----------------------------------------------------------------------------

static void handleCommand(HWND hwnd, int id) {
    if (!g_app) return;

    switch (id) {
    case ID_FILE_SAVE:
        saveDialog(hwnd);
        break;
    case ID_FILE_LOAD:
        loadDialog(hwnd);
        break;
    case ID_FILE_RESET:
        if (MessageBoxA(hwnd, "Reset the showroom to its default state?",
                        "Reset Showroom", MB_YESNO | MB_ICONQUESTION) == IDYES) {
            g_app->resetAll();
        }
        break;
    case ID_FILE_EXIT:
        PostMessageA(hwnd, WM_CLOSE, 0, 0);
        break;

    case ID_VIEW_TURNTABLE:
        g_app->toggleTurntable();
        break;
    case ID_VIEW_RESET_CAMERA:
        g_app->resetCamera();
        break;
    case ID_VIEW_GRID:
        g_app->toggleGrid();
        break;
    case ID_VIEW_WIREFRAME:
        g_app->toggleWireframe();
        break;
    case ID_VIEW_NORMALS:
        g_app->toggleNormals();
        break;
    case ID_VIEW_ORTHO:
        g_app->toggleProjection();
        break;

    case ID_MAT_PLASTIC:
        g_app->applyMaterialPreset(0);
        break;
    case ID_MAT_METAL:
        g_app->applyMaterialPreset(1);
        break;
    case ID_MAT_MATTE:
        g_app->applyMaterialPreset(2);
        break;
    case ID_MAT_GLOSSY:
        g_app->applyMaterialPreset(3);
        break;
    case ID_MAT_GOLD:
        g_app->applyMaterialPreset(4);
        break;
    case ID_MAT_COLOR:
        chooseColor(hwnd);
        break;

    case ID_LIGHT_STUDIO:
        g_app->setLightPreset(0);
        break;
    case ID_LIGHT_WARM:
        g_app->setLightPreset(1);
        break;
    case ID_LIGHT_COOL:
        g_app->setLightPreset(2);
        break;
    case ID_LIGHT_ANIMATE:
        g_app->toggleLightAnimation();
        break;

    case ID_ANIM_EXPLODE:
        g_app->toggleExplode();
        break;
    case ID_ANIM_TOUR:
        if (g_app->cameraTourActive) {
            g_app->stopCameraTour();
        } else {
            g_app->playCameraTour();
        }
        break;
    case ID_ANIM_RECORD:
        g_app->recordCameraKeyframe();
        MessageBoxA(hwnd, "Camera keyframe recorded.", "Camera Tour",
                    MB_OK | MB_ICONINFORMATION);
        break;
    case ID_ANIM_CLEAR_TOUR:
        g_app->clearCameraTour();
        break;

    case ID_HELP_CONTROLS:
        showControls(hwnd);
        break;
    case ID_HELP_ABOUT:
        showAbout(hwnd);
        break;

    default:
        break;
    }

    updateMenuChecks(hwnd);
}

// -----------------------------------------------------------------------------
// Window procedure
// -----------------------------------------------------------------------------

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_SIZE:
        g_width = LOWORD(lParam);
        g_height = HIWORD(lParam);
        if (g_hRC) {
            glViewport(0, 0, g_width, g_height);
        }
        return 0;

    case WM_ERASEBKGND:
        return 1;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        BeginPaint(hwnd, &ps);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_LBUTTONDOWN:
        if (g_app) {
            SetCapture(hwnd);
            g_app->onMouseDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        }
        return 0;

    case WM_MOUSEMOVE:
        if (g_app) {
            g_app->onMouseMove(
                GET_X_LPARAM(lParam),
                GET_Y_LPARAM(lParam),
                (wParam & MK_LBUTTON) != 0
            );
        }
        return 0;

    case WM_LBUTTONUP:
        if (g_app) {
            g_app->onMouseUp();
        }
        ReleaseCapture();
        return 0;

    case WM_MOUSEWHEEL:
        if (g_app) {
            g_app->onMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam));
        }
        return 0;

    case WM_KEYDOWN:
        if ((GetKeyState(VK_CONTROL) & 0x8000) != 0) {
            if (wParam == 'S') {
                saveDialog(hwnd);
                return 0;
            }
            if (wParam == 'O') {
                loadDialog(hwnd);
                return 0;
            }
        }

        if (wParam == VK_F1) {
            showControls(hwnd);
            return 0;
        }

        if (wParam == VK_ESCAPE) {
            PostMessageA(hwnd, WM_CLOSE, 0, 0);
            return 0;
        }

        if (g_app) {
            g_app->onKeyDown((int)wParam);
            updateMenuChecks(hwnd);
        }
        return 0;

    case WM_COMMAND:
        handleCommand(hwnd, LOWORD(wParam));
        return 0;

    case WM_INITMENUPOPUP:
        updateMenuChecks(hwnd);
        return 0;

    case WM_CLOSE:
        if (MessageBoxA(hwnd, "Exit the 3D Product Showroom?",
                        "Confirm Exit", MB_YESNO | MB_ICONQUESTION) == IDYES) {
            DestroyWindow(hwnd);
        }
        return 0;

    case WM_DESTROY:
        g_running = false;
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
}

// -----------------------------------------------------------------------------
// OpenGL initialisation
// -----------------------------------------------------------------------------

static bool initOpenGL(HWND hwnd) {
    g_hDC = GetDC(hwnd);
    if (!g_hDC) return false;

    PIXELFORMATDESCRIPTOR pfd;
    std::memset(&pfd, 0, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pixelFormat = ChoosePixelFormat(g_hDC, &pfd);
    if (pixelFormat == 0) return false;

    if (!SetPixelFormat(g_hDC, pixelFormat, &pfd)) return false;

    g_hRC = wglCreateContext(g_hDC);
    if (!g_hRC) return false;

    if (!wglMakeCurrent(g_hDC, g_hRC)) return false;

    // Enable vsync if the driver exposes the extension.
    typedef BOOL (WINAPI *PFNWGLSWAPINTERVALEXTPROC)(int);
    PFNWGLSWAPINTERVALEXTPROC swapInterval =
        (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
    if (swapInterval) {
        swapInterval(1);
    }

    return true;
}

// -----------------------------------------------------------------------------
// Entry point
// -----------------------------------------------------------------------------

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    WNDCLASSA wc;
    std::memset(&wc, 0, sizeof(wc));
    wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszClassName = kWindowClass;
    wc.lpszMenuName = NULL;

    if (!RegisterClassA(&wc)) {
        MessageBoxA(NULL, "Failed to register the window class.",
                    "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    HMENU menu = createMenuBar();

    RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = 1280;
    rect.bottom = 720;
    AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, TRUE, 0);

    HWND hwnd = CreateWindowA(
        kWindowClass,
        kWindowTitle,
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        NULL,
        menu,
        hInstance,
        NULL
    );

    if (!hwnd) {
        MessageBoxA(NULL, "Failed to create the application window.",
                    "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    if (!initOpenGL(hwnd)) {
        MessageBoxA(hwnd, "Failed to initialise OpenGL.",
                    "Error", MB_OK | MB_ICONERROR);
        if (g_hRC) {
            wglMakeCurrent(NULL, NULL);
            wglDeleteContext(g_hRC);
            g_hRC = NULL;
        }
        if (g_hDC) {
            ReleaseDC(hwnd, g_hDC);
            g_hDC = NULL;
        }
        DestroyWindow(hwnd);
        return 1;
    }

    g_app = new App();

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    updateMenuChecks(hwnd);

    LARGE_INTEGER frequency;
    LARGE_INTEGER previous;
    LARGE_INTEGER current;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&previous);

    while (g_running) {
        MSG msg;
        while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                g_running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }

        if (!g_running) break;

        QueryPerformanceCounter(&current);
        double elapsed = (double)(current.QuadPart - previous.QuadPart)
                         / (double)frequency.QuadPart;
        previous = current;

        if (elapsed > 0.1) elapsed = 0.1;

        g_app->update((float)elapsed);

        if (g_width > 0 && g_height > 0) {
            g_app->renderScene(g_width, g_height);
            g_app->renderHud(g_width, g_height);
            SwapBuffers(g_hDC);
        }

        Sleep(1);
    }

    if (g_app) {
        delete g_app;
        g_app = NULL;
    }

    if (g_hRC) {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(g_hRC);
        g_hRC = NULL;
    }

    if (g_hDC) {
        ReleaseDC(hwnd, g_hDC);
        g_hDC = NULL;
    }

    UnregisterClassA(kWindowClass, hInstance);
    return 0;
}
