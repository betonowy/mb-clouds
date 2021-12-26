//
// Created by pekopeko on 25.07.2021.
//

#ifndef MB_CLOUDS_UIFUNCTIONS_H
#define MB_CLOUDS_UIFUNCTIONS_H

#include <shaders/sceneData.h>
#include <imgui.h>
#include <string>
#include <pipeline/PipeMan.h>
#include <clouds/vdbIntegrationStatus.h>

class vdbClouds;
class Pipeline;

enum class processingStatus {
    START,
    RUNNING,
    FINISHED,
};

struct applicationData {
    bool isRunning = true;
    bool vsync = false;

    bool showMainMenuBar = true;
    bool showSceneDataWindow = false;

    float currentFps{};
    float currentFrameTime{0.1};

    bool rotateCamera = false;
    bool relativeMode = false;

    static constexpr float startingXRot = 75 * (-1.0 / 180.0 * M_PI) + (M_PI / 2);
    static constexpr float startingYRot = 0 * (1.0 / 180.0 * M_PI);
    static constexpr float startingZRot = 135.0 * (1.0 / 180.0 * M_PI) - (M_PI);

//    static constexpr float startingXRot = 120 * (-1.0 / 180.0 * M_PI) + (M_PI / 2);
//    static constexpr float startingYRot = 0 * (1.0 / 180.0 * M_PI);
//    static constexpr float startingZRot = 200.0 * (1.0 / 180.0 * M_PI) - (M_PI);

//    static constexpr float startingXRot = 60 * (-1.0 / 180.0 * M_PI) + (M_PI / 2);
//    static constexpr float startingYRot = 0 * (1.0 / 180.0 * M_PI);
//    static constexpr float startingZRot = 300.0 * (1.0 / 180.0 * M_PI) - (M_PI);

//    static constexpr float startingXRot = 35 * (-1.0 / 180.0 * M_PI) + (M_PI / 2);
//    static constexpr float startingYRot = 0 * (1.0 / 180.0 * M_PI);
//    static constexpr float startingZRot = 30.0 * (1.0 / 180.0 * M_PI) - (M_PI);

//    static constexpr float startingXRot = 150 * (-1.0 / 180.0 * M_PI) + (M_PI / 2);
//    static constexpr float startingYRot = 0 * (1.0 / 180.0 * M_PI);
//    static constexpr float startingZRot = 180.0 * (1.0 / 180.0 * M_PI) - (M_PI);

    glm::vec3 cameraRotation = {startingZRot, startingXRot, startingYRot};
    glm::vec3 cameraSpeed{};
    float cameraSlowDownSpeed = 1;
    float cameraAcceleration = 0.3;

    float cameraRotationSpeed = 1;
    float cameraMoveSpeed = 1;

    int taaFrame = 0;
    int taaMax = 200;
    bool taaEnabled = true;

    bool wantsRecompileShaders = false;
    bool wantsToggleFullScreen = false;
    bool wantsToggleImGui = false;

    bool rightMouseButton = false;

    processingStatus cacheProcessing{processingStatus::FINISHED};
    integrationStatus cacheProcessingStatus{};

    bool msRandomize = true;
    bool msRandomizeOnce = false;

    bool saveBackground = false;

    bool wKey{};
    bool sKey{};
    bool aKey{};
    bool dKey{};
    bool qKey{};
    bool eKey{};
    bool rKey{};
    bool gKey{};

    bool f11Key{};

    bool uiActive{true};

    bool updatePosition{false};
};

class uiFunctions {
public:
    explicit uiFunctions(sceneData *sceneDataPtr, applicationData *appData);

    void doUi();

    void setVdbCloudsPtr(vdbClouds *vdbCloudsPtr);

    void setPipelinePtr(std::shared_ptr<Pipeline> pipelinePtr);

    void _initValues();

private:

    // actions

    void _taaReset();

    void _taaSet(bool value);

    // main menu bar

    void _uiMainMenuBarFile();

    void _uiMainMenuBarWindows();

    void _uiMainMenuBarOptions();

    void _uiMainMenuBarFpsCounter();

    void _uiMainMenuBar();

    void _uiPoseSelect();

    // windows

    void _uiSceneDataWindow();

    void _cacheProcessing();

    sceneData *_sceneDataPtr;
    applicationData *_appData;
    vdbClouds *_vdbCloudsPtr{};
    std::shared_ptr<Pipeline> _pipelinePtr{};

    // variables

    std::string currentPipelineStr;
    std::string currentVdbFileStr;
    std::string currentPoseStr = "Pose 1";

    // strings

    std::string _strVdbShaderOptions;

    // other

    PipeMan _pipeMan;
};

#endif //MB_CLOUDS_UIFUNCTIONS_H
