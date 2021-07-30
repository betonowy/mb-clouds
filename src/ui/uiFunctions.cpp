//
// Created by pekopeko on 25.07.2021.
//

#include "uiFunctions.h"

#include <sstream>

uiFunctions::uiFunctions(sceneData *sceneDataPtr, applicationData *appData)
        : _sceneDataPtr(sceneDataPtr), _appData(appData) {}

void uiFunctions::doUi() {
    _uiMainMenuBar();
    _uiSceneDataWindow();
}

void uiFunctions::_uiMainMenuBar() {
    if (_appData->showMainMenuBar && ImGui::BeginMainMenuBar()) {
        _uiMainMenuBarFile();
        _uiMainMenuBarWindows();
        _uiMainMenuBarFpsCounter();

        ImGui::EndMainMenuBar();
    }
}

void uiFunctions::_uiMainMenuBarFpsCounter() {
    std::stringstream ss;
    ss << "\tFPS: " << _appData->currentFps;

    if (ImGui::BeginMenu(ss.str().c_str(), false)) {
        ImGui::EndMenu();
    }
}

void uiFunctions::_uiMainMenuBarFile() {
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Recompile shaders", "R")) {
            _appData->wantsRecompileShaders = true;
        }

        if (ImGui::MenuItem("Exit application", "ALT+F4")) {
            _appData->isRunning = false;
        }

        ImGui::EndMenu();
    }
}

void uiFunctions::_uiMainMenuBarWindows() {
    if (ImGui::BeginMenu("Windows")) {
        ImGui::MenuItem("Scene data", nullptr, &_appData->showSceneDataWindow);
        ImGui::EndMenu();
    }
}

void uiFunctions::_uiSceneDataWindow() {
    if (_appData->showSceneDataWindow) {
        if (!ImGui::Begin("Scene data", &_appData->showSceneDataWindow)) {
            ImGui::End();
            return;
        }

        ImGui::InputFloat("VDB Scale", &_sceneDataPtr->vdbScale);
        ImGui::InputFloat("AABB Scale", &_sceneDataPtr->aabbScale);

        ImGui::End();
    }
}
