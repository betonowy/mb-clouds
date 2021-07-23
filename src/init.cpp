//
// Created by pekopeko on 20.05.2021.
//

#include <iostream>
#include <glm/glm.hpp>
#include <imgui.h>
#include <backends/imgui_impl_sdl.h>
#include <util/misc.h>
#include <util/file/binaryFile.h>
#include "init.h"
#include <sstream>
#include <openvdb/openvdb.h>
#include <filePaths.h>

//#include "tree/vdbTree.h"

mb::init::init() {
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
    _initSdl();
    _initImGui();
    openvdb::initialize();
}

void mb::init::_quitEverything() {
    openvdb::uninitialize();
    _quitImGui();
    _quitSdl();
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
    bool run = true;

    openvdb::io::File cloudFile(filePaths::VDB_CLOUD_LD);

    cloudFile.open(false);

    while (run) {
        _beginFrame();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) {
                run = false;
            }
        }

        ImGui::ShowDemoWindow();

        _endFrame();
        _windowTitleFps();
    }

    cloudFile.close();
}

void mb::init::_beginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(_mainWindow);
    ImGui::NewFrame();
}

void mb::init::_endFrame() {
    ImGui::Render();
    glViewport(0, 0, (GLsizei) _imGuiIO->DisplaySize.x, (GLsizei) _imGuiIO->DisplaySize.y);
    glClearColor(0.1, 0.2, 0.4, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
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
        int fps = int(round(float(frameCount) / float(t2 - t1) * 1000));

        std::stringstream ss;
        ss << _appWindowName << " FPS: " << fps;

        SDL_SetWindowTitle(_mainWindow, ss.str().c_str());

        t1 = t2;
        frameCount = 0;
    }
}
