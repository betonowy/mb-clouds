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
