//
// Created by pekopeko on 20.05.2021.
//

#include "init.h"

#include <util/misc.h>
#include <clouds/vdbClouds.h>

#include <imgui.h>
#include <backends/imgui_impl_sdl.h>
#include <ui/uiFunctions.h>

#include <iostream>

#ifdef _WIN32
extern "C"
{
__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}

extern "C"
{
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

void MessageCallback([[maybe_unused]] GLenum source, GLenum type,
                     [[maybe_unused]] GLuint id, [[maybe_unused]] GLenum severity,
                     [[maybe_unused]] GLsizei length,
                     const GLchar *message, [[maybe_unused]] const void *userParam) {
    std::string info(message);
    std::string openglPrefix = "OpenGL";

    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            std::cout << openglPrefix + " Error: " + info;
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            std::cout << openglPrefix + " Deprecated: " + info;
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            std::cout << openglPrefix + " Undefined Behavior: " + info;
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            std::cout << openglPrefix + " Performance: " + info;
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            std::cout << openglPrefix + " Portability: " + info;
            break;
        case GL_DEBUG_TYPE_MARKER:
            std::cout << openglPrefix + " Marker: " + info;
            break;
        case GL_DEBUG_TYPE_PUSH_GROUP:
            std::cout << openglPrefix + " Push Group: " + info;
            break;
        case GL_DEBUG_TYPE_POP_GROUP:
            std::cout << openglPrefix + " PoP Group: " + info;
            break;
        default:
            std::cout << openglPrefix + ": " + info;
            break;
    }
    std::cout << std::endl;
}

mb::init::init() : _uiFunctions(&_sceneData, &_appData) {
    if (_currentInitInstance) {
        misc::exception("Cannot invoke more than one init instance");
    }

    _currentInitInstance = this;
    _initEverything();
}

mb::init::~init() {
    _quitEverything();
    _currentInitInstance = nullptr;
}

void mb::init::_initEverything() {
    std::cout << "Init everything start\n";

    _initSdl();
    _initImGui();

    _vdbClouds = std::make_unique<vdbClouds>(filePaths::VDB_CLOUD_LD);

    std::cout << "Init everything done\n";
}

void mb::init::_quitEverything() {
    std::cout << "Quit everything start\n";

    _quitImGui();
    _quitSdl();

    std::cout << "Quit everything done\n";
}

void mb::init::_initSdl() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    _mainWindow = SDL_CreateWindow(_appWindowName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                   _appDefaultWindowSizeX, _appDefaultWindowSizeY,
                                   SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    _glContext = SDL_GL_CreateContext(_mainWindow);
    SDL_GL_MakeCurrent(_mainWindow, _glContext);
    SDL_GL_SetSwapInterval(0);

    glewInit();

#ifndef NDEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, nullptr);
#endif

    std::cout << "   GL_VENDOR: " << glGetString(GL_VENDOR) << "\n"
              << " GL_RENDERER: " << glGetString(GL_RENDERER) << "\n"
              << "  GL_VERSION: " << glGetString(GL_VERSION) << "\n"
              << "GLSL_VERSION: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
}

void mb::init::_quitSdl() {
    SDL_GL_DeleteContext(_glContext);
    SDL_DestroyWindow(_mainWindow);
    SDL_Quit();
}

void mb::init::_initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(_mainWindow, _glContext);
    ImGui_ImplOpenGL3_Init("#version 130");
    _imGuiIO = &(ImGui::GetIO());
}

void mb::init::_quitImGui() {
    _imGuiIO = nullptr;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

mb::init *mb::init::GetInstance() { return _currentInitInstance; }

void mb::init::littleLoop() {
    while (_appData.isRunning) {
        _beginFrame();
        _processEvents();
        _userInterface();
        _updateSceneData();
        _render();
        _endFrame();
        _windowTitleFps();
    }
}

void mb::init::_beginFrame() {
    glClearColor(0.1, 0.2, 0.4, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(_mainWindow);
    ImGui::NewFrame();
}

void mb::init::_endFrame() {
    ImGui::Render();

    glViewport(0, 0, (GLsizei) _imGuiIO->DisplaySize.x, (GLsizei) _imGuiIO->DisplaySize.y);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(_mainWindow);
}

void mb::init::_windowTitleFps() {
    static auto t1 = SDL_GetTicks();
    static auto t2 = t1;
    static int frameCount = 0;

    t2 = SDL_GetTicks();
    frameCount++;

    if (t2 - t1 >= 1000) {
        _appData.currentFps = round(float(frameCount) / float(t2 - t1) * 100000) / 100;

        t1 = t2;
        frameCount = 0;
    }
}

void mb::init::_updateSceneData() {
    {
        int x, y;
        SDL_GetWindowSize(_mainWindow, &x, &y);

        _sceneData.aspectRatio = float(y) / float(x);
    }

    _sceneData.update();
}

void mb::init::_userInterface() {
    _uiFunctions.doUi();
}

void mb::init::_render() {
    _vdbClouds->render();
}

void mb::init::_processEvents() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT) {
            _appData.isRunning = false;
        }
    }
}
