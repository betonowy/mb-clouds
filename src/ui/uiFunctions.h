//
// Created by pekopeko on 25.07.2021.
//

#ifndef MB_CLOUDS_UIFUNCTIONS_H
#define MB_CLOUDS_UIFUNCTIONS_H

#include <shaders/sceneData.h>
#include <imgui.h>

struct applicationData {
    bool isRunning = true;
    bool showMainMenuBar = true;
    float currentFps{};
    float currentFrameTime{0.1};

    bool rotateCamera = false;
    bool moveCamera = false;

    glm::vec3 cameraRotation = {};
    float cameraRotationSpeed = 1;
    float cameraMoveSpeed = 4;

    bool wantsRecompileShaders = false;

    bool wKey{};
    bool sKey{};
    bool aKey{};
    bool dKey{};
    bool qKey{};
    bool eKey{};
};

class uiFunctions {
public:
    explicit uiFunctions(sceneData* sceneDataPtr, applicationData* appData);

    void doUi();

private:

    void _uiMainMenuBarFile();

    void _uiMainMenuBarFpsCounter();

    void _uiMainMenuBar();

    sceneData* _sceneDataPtr;
    applicationData* _appData;

};

#endif //MB_CLOUDS_UIFUNCTIONS_H
