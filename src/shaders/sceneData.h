//
// Created by pekopeko on 25.07.2021.
//

#ifndef MB_CLOUDS_SCENEDATA_H
#define MB_CLOUDS_SCENEDATA_H

#include <glm/glm.hpp>

struct sceneData {
    glm::vec3 cameraPosition{1.1, -1.1, 0.4};
    float fov{};

    glm::vec3 cameraLookDir{};
    float aspectRatio{};

    glm::mat4 viewMatrix{};

    glm::mat4 projectionMatrix{};

    glm::mat4 combinedMatrix{};

    glm::vec2 mousePosition{};
    glm::ivec2 windowResolution{};

    glm::vec3 backgroundColor{};
    float time{};

    float vdbScale = 2;
    float aabbScale = 1;
    float vdbDensityMultiplier = 500;
    float backgroundDensity = 0.0;

    float primaryRayLength = 0.020f;
    float primaryRayLengthExp = 1.0f;
    float secondaryRayLength = 0.060f;
    float secondaryRayLengthExp = 1.0f;

    glm::vec3 aabbPosition{0, 0, 0};
    float _padding_1;

    glm::vec3 aabbSize{1, 1, 1};
    float _padding_2;

    glm::vec3 cameraLookDirCrossX{1, 1, 1};
    float _padding_3;

    glm::vec3 cameraLookDirCrossY{1, 1, 1};
    float _padding_4;

    glm::vec3 sunDir = glm::normalize(glm::vec3(0, 1, 0.5));
    float sunPower = 80.0f;

    glm::vec3 sunColor{1, 0.9, 0.8};
    float sunFocus = 40000.0f;

    glm::vec3 backgroundColorTop{0.0125, 0.05, 1.0};
    float topPower = 8;

    glm::vec3 backgroundColorBottom{0.05, 0.08, 0.19};
    float bottomPower = 1;

    glm::vec3 backgroundColorMid{1.0, 0.875, 0.75};
    float midPower = 2;

    float randomData[8 * 4];

    float alphaBlendIn = 1;
    float radianceMultiplier = 0.1;
    float ambientMultiplier = 0.05;
    float cloudHeight = 0.5;

    float cloudHeightSensitivity = 2.0;
    float densityMultiplier = 0.1;
    float _padding_5;
    float _padding_6;

    float gaussian[32 * 4];

    int32_t gaussianRadius = 3;

    void update();

    void reset();

    void calculateGaussian(int radius);
};

#endif //MB_CLOUDS_SCENEDATA_H
