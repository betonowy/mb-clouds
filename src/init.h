//
// Created by pekopeko on 20.05.2021.
//

#ifndef MB_CLOUDS_INIT_H
#define MB_CLOUDS_INIT_H

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <backends/imgui_impl_opengl3.h>
#include <memory>
#include <shaders/sceneData.h>
#include <ui/uiFunctions.h>
#include <world/camera.h>

class vdbClouds;

namespace mb {
    class init {
    public:
        init();

        ~init();

        void littleLoop();

        static init *GetInstance();

    private:

        // init / quit

        void _initEverything();

        void _quitEverything();

        void _initSdl();

        void _quitSdl();

        void _initImGui();

        void _quitImGui();

        // operations

        void _beginFrame();

        void _endFrame();

        void _windowTitleFps();

        void _updateSceneData();

        void _userInterface();

        void _render();

        void _processEvents();

        void _cameraHandle();

        // events

        void _scheduledEvents();

        void _leftMouseButtonHandle(bool value);

        void _rightMouseButtonHandle(bool value);

        void _mouseMotionHandle(glm::vec2 mAbs, glm::vec2 mRel, glm::vec2 pAbs, glm::vec2 pRel);

        void _wKeyAction();

        void _sKeyAction();

        void _aKeyAction();

        void _dKeyAction();

        void _qKeyAction();

        void _eKeyAction();

        void _rKeyAction();

        // constants

        static constexpr const char *_appWindowName = "Clouds";
        static constexpr int _appDefaultWindowSizeX = 800;
        static constexpr int _appDefaultWindowSizeY = 600;

        // static

        inline static init *_currentInitInstance = nullptr;

        // sdl variables

        SDL_Window *_mainWindow{};
        SDL_GLContext _glContext{};

        // variables

        sceneData _sceneData;
        applicationData _appData;

        // synchronisation

        // ImGui

        ImGuiIO* _imGuiIO{};

        uiFunctions _uiFunctions;

        camera _camera;

        // vdbClouds

        std::unique_ptr<vdbClouds> _vdbClouds;
    };
}

#endif //MB_CLOUDS_INIT_H
