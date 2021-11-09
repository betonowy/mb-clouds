//
// Created by pekopeko on 25.07.2021.
//

#include "uiFunctions.h"

#include <clouds/vdbClouds.h>
#include <pipeline/Pipeline.h>
#include <pipeline/PipeMan.h>
#include <util/file/binaryFile.h>

#include <SDL_video.h>

#include <sstream>

uiFunctions::uiFunctions(sceneData *sceneDataPtr, applicationData *appData)
        : _sceneDataPtr(sceneDataPtr), _appData(appData) {}

void uiFunctions::doUi() {
    _uiMainMenuBar();
    _uiSceneDataWindow();
    _cacheProcessing();
}

void uiFunctions::_uiMainMenuBar() {
    if (_appData->showMainMenuBar && ImGui::BeginMainMenuBar()) {
        _uiMainMenuBarFile();
        _uiMainMenuBarWindows();
        _uiMainMenuBarOptions();
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

        if (ImGui::MenuItem("Process cached dataset")) {
            _appData->cacheProcessing = processingStatus::START;
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

void uiFunctions::_uiMainMenuBarOptions() {
    if (ImGui::BeginMenu("Options")) {
        if (ImGui::MenuItem("Vsync", nullptr, &_appData->vsync)) {
            SDL_GL_SetSwapInterval((_appData->vsync) ? 1 : 0);
        }
        ImGui::EndMenu();
    }
}

void uiFunctions::_uiSceneDataWindow() {
    if (_appData->showSceneDataWindow) {
        ImGui::SetNextWindowPos(ImVec2(float(_sceneDataPtr->windowResolution.x) - 300, 0));
        ImGui::SetNextWindowSize(ImVec2(300, -19));

        if (!ImGui::Begin("Scene data", &_appData->showSceneDataWindow, ImGuiWindowFlags_NoResize)) {
            ImGui::End();
            return;
        }

        ImGui::TextWrapped("%s", _strVdbShaderOptions.c_str());

        ImGui::PushItemWidth(100);

        ImGui::PushItemWidth(170);

        if (ImGui::BeginCombo("Current VDB file", currentVdbFileStr.c_str())) {

            for (auto &available : _vdbCloudsPtr->getAvailableVdbFiles()) {
                std::string_view filename = available;
                auto lastSlash = filename.find_last_of('/') + 1;
                filename.remove_prefix(lastSlash);

                if (ImGui::Selectable(filename.begin())) {
                    _vdbCloudsPtr->changeDataset(available);
                    currentVdbFileStr = filename.begin();
                }
            }

            ImGui::EndCombo();
        }

        ImGui::Text("Cloud memory size: %f MiB", _vdbCloudsPtr->getMemorySize() / 1024. / 1024.);

        if (ImGui::BeginCombo("Current shader", currentPipelineStr.c_str())) {

            for (auto &[name, maker] : _pipeMan.getMakers()) {
                if (ImGui::Selectable(name.c_str())) {
                    *_pipelinePtr = maker();
                    currentPipelineStr = name;
                }
            }

            ImGui::EndCombo();
        }

        ImGui::PopItemWidth();

        ImGui::InputFloat("VDB Scale", &_sceneDataPtr->vdbScale);

        ImGui::DragFloat("VDB Density", &_sceneDataPtr->vdbDensityMultiplier,
                         0.5f, 0.0f, 10000.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
        ImGui::DragFloat("VDB Background", &_sceneDataPtr->backgroundDensity,
                         0.5f, 0.0f, 1000.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
        ImGui::DragFloat("Ray Primary step length", &_sceneDataPtr->primaryRayLength,
                         0.0004f, 0.001f, 0.1f, "%.3f", ImGuiSliderFlags_Logarithmic);
//        ImGui::DragFloat("Ray Primary step exp", &_sceneDataPtr->primaryRayLengthExp,
//                         0.001f, 1.0f, 1.5f, "%.3f", ImGuiSliderFlags_Logarithmic);
        ImGui::DragFloat("Ray secondary step length", &_sceneDataPtr->secondaryRayLength,
                         0.0004f, 0.001f, 0.1f, "%.3f", ImGuiSliderFlags_Logarithmic);
//        ImGui::DragFloat("Ray secondary step exp", &_sceneDataPtr->secondaryRayLengthExp,
//                         0.001f, 1.0f, 1.5f, "%.3f", ImGuiSliderFlags_Logarithmic);
        if (ImGui::Button("TAA reset")) {
            _taaReset();
        }

        ImGui::DragFloat("Cloud height", &_sceneDataPtr->cloudHeight,
                         0.001, 0.0f, 1.0f);
        ImGui::DragFloat("Cloud height sensitivity", &_sceneDataPtr->cloudHeightSensitivity,
                         0.001, 0.0f, 1.0f);
        ImGui::DragFloat("Density Multiplier", &_sceneDataPtr->densityMultiplier,
                         0.5f, 0.0f, 1000.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
        ImGui::DragFloat("Ambient multiplier", &_sceneDataPtr->ambientMultiplier,
                         0.5f, 0.0f, 1000.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
        ImGui::DragFloat("Intensity multiplier", &_sceneDataPtr->radianceMultiplier,
                         0.5f, 0.0f, 1000.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
        ImGui::SliderInt("Blur radius", &_sceneDataPtr->gaussianRadius, 0, 127);

        ImGui::Checkbox("TAA enabled", &_appData->taaEnabled);

        ImGui::InputInt("TAA max", &_appData->taaMax);

        ImGui::Separator();

        ImGui::Text("Background settings");

        ImGui::DragFloat3("Sun direction", reinterpret_cast<float *>(&(_sceneDataPtr->sunDir)), 0.01, -1, 1);

        ImGui::DragFloat("Sun focus", reinterpret_cast<float *>(&(_sceneDataPtr->sunFocus)),
                         0.1f, 1.0f, 100.0f, "%.3f",
                         ImGuiSliderFlags_Logarithmic);

        ImGui::ColorPicker3("Sun color", reinterpret_cast<float *>(&(_sceneDataPtr->sunColor)));

        ImGui::DragFloat("Sun power", reinterpret_cast<float *>(&(_sceneDataPtr->sunPower)),
                         0.1f, 0.0f, 100.0f, "%.3f",
                         ImGuiSliderFlags_Logarithmic);

        ImGui::ColorPicker3("BHi color", reinterpret_cast<float *>(&(_sceneDataPtr->backgroundColorTop)));

        ImGui::DragFloat("BHi power", reinterpret_cast<float *>(&(_sceneDataPtr->sunPower)),
                         0.1f, 0.0f, 100.0f, "%.3f",
                         ImGuiSliderFlags_Logarithmic);

        ImGui::ColorPicker3("BMi color", reinterpret_cast<float *>(&(_sceneDataPtr->backgroundColorMid)));

        ImGui::DragFloat("BMi power", reinterpret_cast<float *>(&(_sceneDataPtr->sunPower)),
                         0.1f, 0.0f, 100.0f, "%.3f",
                         ImGuiSliderFlags_Logarithmic);

        ImGui::ColorPicker3("BLo color", reinterpret_cast<float *>(&(_sceneDataPtr->backgroundColorBottom)));

        ImGui::DragFloat("BLo power", reinterpret_cast<float *>(&(_sceneDataPtr->sunPower)),
                         0.1f, 0.0f, 100.0f, "%.3f",
                         ImGuiSliderFlags_Logarithmic);

        ImGui::PopItemWidth();

        ImGui::End();
    }
}

void uiFunctions::setVdbCloudsPtr(vdbClouds *vdbCloudsPtr) {
    _vdbCloudsPtr = vdbCloudsPtr;
}

void uiFunctions::_initValues() {
    currentVdbFileStr = _vdbCloudsPtr->getAvailableVdbFiles().front();
    _vdbCloudsPtr->changeDataset(currentVdbFileStr);
    currentVdbFileStr = currentVdbFileStr.substr(currentVdbFileStr.find_last_of('/') + 1);

//    currentPipelineStr = _vdbCloudsPtr->getAvailableShaders().front().first;
//    _vdbCloudsPtr->recompileShaders(currentPipelineStr);

    // strings

    auto loadStr = [](std::string &str, const char *path) {
        mb::binaryFile file(path);
        const auto &data = file.vector();
        str.insert(str.begin(), data.begin(), data.end());
    };

    loadStr(_strVdbShaderOptions, filePaths::STRING_VDB_SHADER_OPTIONS);
}

void uiFunctions::_taaSet(bool value) {
    _appData->taaEnabled = value;
    if (!value) _taaReset();
}

void uiFunctions::_taaReset() {
    _appData->taaFrame = 0;
}

void uiFunctions::setPipelinePtr(std::shared_ptr<Pipeline> pipelinePtr) {
    _pipelinePtr = std::move(pipelinePtr);
}

void uiFunctions::_cacheProcessing() {
    if (_appData->cacheProcessing == processingStatus::RUNNING) {
        ImGui::SetNextWindowPos(ImVec2(float(_sceneDataPtr->windowResolution.x) / 2 - 150, float(_sceneDataPtr->windowResolution.y) / 2 - 35));
        ImGui::SetNextWindowSize(ImVec2(300, 70));

        if (!ImGui::Begin("Processing status", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar)) {
            ImGui::End();
            return;
        }
        const auto &info = _appData->cacheProcessingStatus;

        ImGui::Text("Processed voxels: %llu out of %llu", info.processed, info.total);
        ImGui::Text("Active jobs: %llu", info.active);
        ImGui::Text("Progress %.1f%%", _appData->cacheProcessingStatus.processedPercentage);

        ImGui::End();
    }
}
