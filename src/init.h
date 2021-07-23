//
// Created by pekopeko on 20.05.2021.
//

#ifndef MB_CLOUDS_INIT_H
#define MB_CLOUDS_INIT_H

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <backends/imgui_impl_opengl3.h>

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

        // constants

        static constexpr const char *_appWindowName = "Clouds";
        static constexpr int _appDefaultWindowSizeX = 800;
        static constexpr int _appDefaultWindowSizeY = 600;

        // static

        inline static init *_currentInitInstance = nullptr;

        // variables

        SDL_Window *_mainWindow{};
        SDL_GLContext _glContext{};

        // synchronisation

        // ImGui

        ImGuiIO* _imGuiIO{};
    };
}

#endif //MB_CLOUDS_INIT_H
