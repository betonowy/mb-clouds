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

mb::init::init()
        : _uiFunctions(&_sceneData, &_appData),
          _random(std::random_device()()),
          _uniformDist(_uniformRandomRangeLow, _uniformRandomRangeHigh) {
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

    _vdbClouds = std::make_unique<vdbClouds>(filePaths::VDB_CLOUD_HD);

    _blueNoiseTexture = std::make_unique<texture>(filePaths::TEX_BLUENOISE, texIndex::texBlueNoise);

    _uiFunctions.setVdbCloudsPtr(_vdbClouds.get());
    _uiFunctions._initValues();

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

    ImGui::GetStyle().Alpha = 0.8;

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
        _cameraHandle();
        _userInterface();
        _scheduledEvents();
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
    static auto t3 = t1;
    static int frameCount = 0;

    t3 = t2;
    t2 = SDL_GetTicks();
    frameCount++;

    _appData.currentFrameTime = float(t2 - t3) / 1000;

    if (t2 - t1 >= 1000) {
        _appData.currentFps = round(float(frameCount) / float(t2 - t1) * 100000) / 100;

        t1 = t2;
        frameCount = 0;
    }
}

void mb::init::_updateSceneData() {
    _sceneData.cameraPosition = _camera.GetPosition();
    _sceneData.fov = _camera.GetFov();
    _sceneData.cameraLookDir = _camera.GetLookDir();
    _sceneData.viewMatrix = _camera.GetView();
    _sceneData.projectionMatrix = _camera.GetProjection();
    _sceneData.combinedMatrix = _camera.GetCombined();

    {
        int x, y;
        SDL_GetWindowSize(_mainWindow, &x, &y);

        _sceneData.aspectRatio = float(y) / float(x);
        _sceneData.windowResolution = {x, y};
    }

    int a = 0;

    for (auto &n : _sceneData.randomData) {
        n = _uniformDist(_random);
    }

    dimType cloudSize = _vdbClouds->getSize();
    int maxCoord = 0;
    for (int i = 0; i < dimType::length(); i++) {
        maxCoord = std::max(cloudSize[i], maxCoord);
    }

    _sceneData.aabbSize = glm::vec3(cloudSize) / float(maxCoord);
    _sceneData.aabbPosition = {0.004, 0.002, 0.003};

    // sanitize values
    _sceneData.primaryRayLength = std::max(_sceneData.primaryRayLength, 0.001f);
    _sceneData.secondaryRayLength = std::max(_sceneData.secondaryRayLength, 0.001f);
    _sceneData.sunDir /= glm::length(_sceneData.sunDir);

    _sceneData.update();
}

void mb::init::_userInterface() {
    _uiFunctions.doUi();

    _wKeyAction();
    _sKeyAction();
    _aKeyAction();
    _dKeyAction();
    _qKeyAction();
    _eKeyAction();
    _rKeyAction();
    _f11KeyAction();
    _rightMouseButtonAction();
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

        if (!_imGuiIO->WantCaptureMouse) {
            constexpr bool BOOL_VAL_ON_DOWN = true;
            constexpr bool BOOL_VAL_ON_UP = false;

            switch (event.type) {
                case SDL_MOUSEBUTTONDOWN: {
                    switch (event.button.button) {
                        case SDL_BUTTON_LEFT:
                            _leftMouseButtonHandle(BOOL_VAL_ON_DOWN);
                            break;
                        case SDL_BUTTON_RIGHT:
                            _rightMouseButtonHandle(BOOL_VAL_ON_DOWN);
                            break;
                    }
                    break;
                }
                case SDL_MOUSEBUTTONUP: {
                    switch (event.button.button) {
                        case SDL_BUTTON_LEFT:
                            _leftMouseButtonHandle(BOOL_VAL_ON_UP);
                            break;
                        case SDL_BUTTON_RIGHT:
                            _rightMouseButtonHandle(BOOL_VAL_ON_UP);
                            break;
                    }
                    break;
                }
                case SDL_MOUSEMOTION: {
                    _mouseMotionHandle(
                            glm::vec2(event.motion.x, event.motion.y) * 2.f /
                            glm::vec2(_sceneData.windowResolution) - glm::vec2(1.0, 1.0),
                            glm::vec2(event.motion.xrel, event.motion.yrel) * 2.f /
                            glm::vec2(_sceneData.windowResolution),
                            glm::vec2(event.motion.x, event.motion.y),
                            glm::vec2(event.motion.xrel, event.motion.yrel)
                    );
                    break;
                }
            }
        }

        if (!_imGuiIO->WantCaptureKeyboard) {
            switch (event.type) {
                case SDL_KEYUP:
                case SDL_KEYDOWN: {
                    bool value = event.type == SDL_KEYDOWN;

                    switch (event.key.keysym.sym) {
                        case SDLK_w:
                            _appData.wKey = value;
                            break;
                        case SDLK_s:
                            _appData.sKey = value;
                            break;
                        case SDLK_a:
                            _appData.aKey = value;
                            break;
                        case SDLK_d:
                            _appData.dKey = value;
                            break;
                        case SDLK_q:
                            _appData.qKey = value;
                            break;
                        case SDLK_e:
                            _appData.eKey = value;
                            break;
                        case SDLK_r:
                            _appData.rKey = value;
                            break;
                        case SDLK_F11:
                            _appData.f11Key = value;
                    }
                    break;
                }
            }
        }
    }
}

void mb::init::_leftMouseButtonHandle(bool value) {
    // for now only one app state

    _appData.rotateCamera = value;
}

void mb::init::_rightMouseButtonHandle(bool value) {
    // for now only one app state

    _appData.rightMouseButton = value;
}

void mb::init::_cameraHandle() {
    _sceneData.cameraPosition += _appData.cameraSpeed * _appData.currentFrameTime;
    _appData.cameraSpeed -= _appData.cameraSpeed * std::min(_appData.cameraSlowDownSpeed * _appData.currentFrameTime, 0.99f);

    _camera.SetFovAndAspectRatio(_sceneData.fov, _sceneData.aspectRatio);
    _camera.SetPositionAndDirection(_sceneData.cameraPosition, _sceneData.cameraLookDir);
    _camera.RotateAbs(_appData.cameraRotation);

    _appData.cameraRotation.y = std::min(_appData.cameraRotation.y, float(M_PI / 2 - 0.01));
    _appData.cameraRotation.y = std::max(_appData.cameraRotation.y, float(-M_PI / 2 + 0.01));

    auto &vec = _appData.cameraRotation;

    if (vec.x > M_PI) vec.x -= 2 * M_PI;
    else if (vec.x < M_PI) vec.x += 2 * M_PI;

    if (vec.y > M_PI) vec.y -= 2 * M_PI;
    else if (vec.x < M_PI) vec.x += 2 * M_PI;

    if (vec.z > M_PI) vec.z -= 2 * M_PI;
    else if (vec.z < M_PI) vec.z += 2 * M_PI;
}

void mb::init::_mouseMotionHandle(glm::vec2 mAbs, glm::vec2 mRel, glm::vec2 pAbs, glm::vec2 pRel) {
    if (_appData.rotateCamera || _appData.relativeMode) {
        if (_appData.relativeMode) {
            _appData.cameraRotation += glm::vec3(-pRel.x, pRel.y, 0) / 500.0f;
        } else {
            _appData.cameraRotation += glm::vec3(mRel.x, -mRel.y * _sceneData.aspectRatio, 0);
        }
        _sceneData.mousePosition = pAbs;
    }
}

void mb::init::_wKeyAction() {
    if (_appData.wKey) {
        _appData.cameraSpeed += _camera.getRelY() * _appData.currentFrameTime * _appData.cameraAcceleration;
//        _camera.MoveRelative(glm::vec3(0, 1, 0) * _appData.currentFrameTime * _appData.cameraMoveSpeed);
    }
}

void mb::init::_sKeyAction() {
    if (_appData.sKey) {
        _appData.cameraSpeed -= _camera.getRelY() * _appData.currentFrameTime * _appData.cameraAcceleration;
//        _camera.MoveRelative(glm::vec3(0, -1, 0) * _appData.currentFrameTime * _appData.cameraMoveSpeed);
    }
}

void mb::init::_aKeyAction() {
    if (_appData.aKey) {
        _appData.cameraSpeed -= _camera.getRelX() * _appData.currentFrameTime * _appData.cameraAcceleration;
//        _camera.MoveRelative(glm::vec3(-1, 0, 0) * _appData.currentFrameTime * _appData.cameraMoveSpeed);
    }
}

void mb::init::_dKeyAction() {
    if (_appData.dKey) {
        _appData.cameraSpeed += _camera.getRelX() * _appData.currentFrameTime * _appData.cameraAcceleration;
//        _camera.MoveRelative(glm::vec3(1, 0, 0) * _appData.currentFrameTime * _appData.cameraMoveSpeed);
    }
}

void mb::init::_qKeyAction() {
    if (_appData.qKey) {
        _appData.cameraSpeed += _camera.getRelZ() * _appData.currentFrameTime * _appData.cameraAcceleration;
//        _camera.MoveRelative(glm::vec3(0, 0, 1) * _appData.currentFrameTime * _appData.cameraMoveSpeed);
    }
}

void mb::init::_eKeyAction() {
    if (_appData.eKey) {
        _appData.cameraSpeed -= _camera.getRelZ() * _appData.currentFrameTime * _appData.cameraAcceleration;
//        _camera.MoveRelative(glm::vec3(0, 0, -1) * _appData.currentFrameTime * _appData.cameraMoveSpeed);
    }
}

void mb::init::_rKeyAction() {
    if (_appData.rKey) {
        _appData.rKey = false;
        _appData.wantsRecompileShaders = true;
    }
}

void mb::init::_f11KeyAction() {
    if (_appData.f11Key) {
        _appData.f11Key = false;
        bool isFullscreen = SDL_GetWindowFlags(_mainWindow) & SDL_WINDOW_FULLSCREEN;
        SDL_SetWindowFullscreen(_mainWindow, isFullscreen ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
}

void mb::init::_rightMouseButtonAction() {
    if (_appData.rightMouseButton) {
        _appData.rightMouseButton = false;
        _appData.relativeMode = !_appData.relativeMode;
        SDL_SetRelativeMouseMode(_appData.relativeMode ? SDL_TRUE : SDL_FALSE);
    }
}

void mb::init::_scheduledEvents() {
    if (_appData.wantsRecompileShaders) {
        _appData.wantsRecompileShaders = false;
        _vdbClouds->recompileShaders();
    }
}
