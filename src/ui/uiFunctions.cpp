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

namespace {
    using PoseType = std::tuple<std::string_view, glm::vec3, glm::vec3>;
    std::array<PoseType, 5> _compiledPoses{
            {
                    {
                            "Pose 1",
                            {-1, -1, 0.4},
                            {
                                    135.0 * (1.0 / 180.0 * M_PI) - (M_PI),
                                    75 * (-1.0 / 180.0 * M_PI) + (M_PI / 2),
                                    0 * (1.0 / 180.0 * M_PI),
                            }
                    },
                    {
                            "Pose 2",
                            {0.4, -1.32, -0.35},
                            {
                                    200.0 * (1.0 / 180.0 * M_PI) - (M_PI),
                                    120 * (-1.0 / 180.0 * M_PI) + (M_PI / 2),
                                    0 * (1.0 / 180.0 * M_PI),
                            }
                    },
                    {
                            "Pose 3",
                            {1, 0.75, 0.4},
                            {
                                    300.0 * (1.0 / 180.0 * M_PI) - (M_PI),
                                    60 * (-1.0 / 180.0 * M_PI) + (M_PI / 2),
                                    0 * (1.0 / 180.0 * M_PI),
                            }
                    },
                    {
                            "Pose 4",
                            {-0.45, 0.8, 0.85},
                            {
                                    30.0 * (1.0 / 180.0 * M_PI) - (M_PI),
                                    35 * (-1.0 / 180.0 * M_PI) + (M_PI / 2),
                                    0 * (1.0 / 180.0 * M_PI),
                            }
                    },
                    {
                            "Pose 5",
                            {0, -0.7, -1},
                            {
                                    180.0 * (1.0 / 180.0 * M_PI) - (M_PI),
                                    150 * (-1.0 / 180.0 * M_PI) + (M_PI / 2),
                                    0 * (1.0 / 180.0 * M_PI),
                            }
                    },
            }
    };
}

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

        if (ImGui::BeginCombo("Current VDB", currentVdbFileStr.c_str())) {

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

        ImGui::Text("Cached memory size: %f MiB", _vdbCloudsPtr->getMemorySize() / 1024. / 1024.);
        ImGui::Text("Original memory size: %f MiB", _vdbCloudsPtr->getOriginalMemorySize() / 1024. / 1024.);

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

        ImGui::Separator();

        _uiPoseSelect();

        ImGui::Separator();

        ImGui::Text("General VDB settings");

        ImGui::InputFloat("VDB Scale", &_sceneDataPtr->vdbScale);

        ImGui::DragFloat("VDB Density", &_sceneDataPtr->vdbDensityMultiplier,
                         0.5f, 0.0f, 10000.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
        ImGui::DragFloat("VDB Background", &_sceneDataPtr->backgroundDensity,
                         0.5f, 0.0f, 1000.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
        ImGui::Separator();

        ImGui::Text("Single Scattering settings");

        ImGui::DragFloat("SS ray step length", &_sceneDataPtr->primaryRayLength,
                         0.0004f, 0.001f, 0.1f, "%.3f", ImGuiSliderFlags_Logarithmic);
        ImGui::DragFloat("SS integral multiplier", &_sceneDataPtr->ss_integral_mult);

        ImGui::DragFloat("SS intensity multiplier", &_sceneDataPtr->ss_intensity_mult);

        ImGui::DragFloat("SS Lorenz-Mie multiplier", &_sceneDataPtr->ss_lorenz_mie_mult);

        ImGui::Separator();

        ImGui::Text("Multi Scattering settings");

        ImGui::DragFloat("MS integral multiplier", &_sceneDataPtr->ms_integral_mult);

        ImGui::DragFloat("MS intensity multiplier", &_sceneDataPtr->ms_intensity_mult);

        ImGui::DragFloat("MS Lorenz-Mie multiplier", &_sceneDataPtr->ms_lorenz_mie_mult);

        ImGui::DragFloat("MS distance power", &_sceneDataPtr->ms_distance_pow);

        ImGui::DragFloat("MS SoI radius", &_sceneDataPtr->ms_cloudRadius, 0.0004f, 0.00001f, 1.0f, "%.5f",
                         ImGuiSliderFlags_Logarithmic);

        ImGui::DragFloat("Cloud height", &_sceneDataPtr->cloudHeight,
                         0.001, -1.0f, 1.0f);
        ImGui::DragFloat("Cloud height sensitivity", &_sceneDataPtr->cloudHeightSensitivity,
                         0.001, 0.0f, 1.0f);

        ImGui::SliderInt("MS point skip", &_sceneDataPtr->ms_skip, 1, 32, "%d", ImGuiSliderFlags_AlwaysClamp);

        ImGui::Checkbox("MS rand on", &_appData->msRandomize);

        _appData->msRandomizeOnce = ImGui::Button("MS rand once");

        ImGui::Separator();

        ImGui::Text("General ray marching settings");

        ImGui::DragFloat("Ray Primary step length", &_sceneDataPtr->primaryRayLength,
                         0.0004f, 0.001f, 0.1f, "%.3f", ImGuiSliderFlags_Logarithmic);
        ImGui::DragFloat("Ray secondary step length", &_sceneDataPtr->secondaryRayLength,
                         0.0004f, 0.001f, 0.1f, "%.3f", ImGuiSliderFlags_Logarithmic);

        ImGui::Separator();

        ImGui::Text("Post-processing settings");

        ImGui::Checkbox("TAA enabled", &_appData->taaEnabled);

        ImGui::InputInt("TAA max", &_appData->taaMax);

        if (ImGui::Button("TAA reset")) {
            _taaReset();
        }

        ImGui::SliderInt("Blur radius", &_sceneDataPtr->gaussianRadius, 0, 66);

        _appData->saveBackground = ImGui::Button("Export background");

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

        ImGui::Separator();

        ImGui::Text("Fine tune debug settings");

        ImGui::DragFloat("Density Multiplier", &_sceneDataPtr->densityMultiplier,
                         0.5f, 0.0f, 1000.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
        ImGui::DragFloat("Ambient multiplier", &_sceneDataPtr->ambientMultiplier,
                         0.5f, 0.0f, 1000.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
        ImGui::DragFloat("Intensity multiplier", &_sceneDataPtr->radianceMultiplier,
                         0.5f, 0.0f, 1000.0f, "%.3f", ImGuiSliderFlags_Logarithmic);

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
        ImGui::SetNextWindowPos(ImVec2(float(_sceneDataPtr->windowResolution.x) / 2 - 150,
                                       float(_sceneDataPtr->windowResolution.y) / 2 - 35));
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

void uiFunctions::_uiPoseSelect() {
    if (ImGui::BeginCombo("Pose select", currentPoseStr.c_str())) {

        for (auto &[name, pos, rot] : _compiledPoses) {
            if (ImGui::Selectable(name.data())) {
                currentPoseStr = name;
                _sceneDataPtr->cameraPosition = pos;
                _appData->cameraRotation = rot;
                _appData->updatePosition = true;
            }
        }

        ImGui::EndCombo();
    }
}
