//
// Created by pekopeko on 25.07.2021.
//

#ifndef MB_CLOUDS_UIFUNCTIONS_H
#define MB_CLOUDS_UIFUNCTIONS_H

#include <shaders/sceneData.h>
#include <imgui.h>
#include <string>

class vdbClouds;

struct applicationData {
    bool isRunning = true;
    bool vsync = false;

    bool showMainMenuBar = true;
    bool showSceneDataWindow = false;

    float currentFps{};
    float currentFrameTime{0.1};

    bool rotateCamera = false;
    bool relativeMode = false;

    glm::vec3 cameraRotation = {M_PI / 4, M_PI / 8, 0};
    glm::vec3 cameraSpeed{};
    float cameraSlowDownSpeed = 1;
    float cameraAcceleration = 0.3;

    float cameraRotationSpeed = 1;
    float cameraMoveSpeed = 1;

    bool wantsRecompileShaders = false;
    bool wantsToggleFullScreen = false;
    bool wantsToggleImGui = false;

    bool rightMouseButton = false;

    bool wKey{};
    bool sKey{};
    bool aKey{};
    bool dKey{};
    bool qKey{};
    bool eKey{};
    bool rKey{};

    bool f11Key{};
};

class uiFunctions {
public:
    explicit uiFunctions(sceneData *sceneDataPtr, applicationData *appData);

    void doUi();

    void setVdbCloudsPtr(vdbClouds *vdbCloudsPtr);

    void _initValues();

private:

    // main menu bar

    void _uiMainMenuBarFile();

    void _uiMainMenuBarWindows();

    void _uiMainMenuBarOptions();

    void _uiMainMenuBarFpsCounter();

    void _uiMainMenuBar();

    // windows

    void _uiSceneDataWindow();

    sceneData *_sceneDataPtr;
    applicationData *_appData;
    vdbClouds *_vdbCloudsPtr{};

    // variables

    std::string currentShaderStr;
    std::string currentVdbFileStr;

    // strings

    std::string _strVdbShaderOptions;
};

#endif //MB_CLOUDS_UIFUNCTIONS_H
