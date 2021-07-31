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
    float vdbDensityMultiplier = 22;
    float backgroundDensity = 0.0;

    float primaryRayLength = 0.029f;
    float primaryRayLengthExp = 1.0f;
    float secondaryRayLength = 0.01f;
    float secondaryRayLengthExp = 1.0f;

    glm::vec3 aabbPosition{0, 0, 0};
    float _padding_1;

    glm::vec3 aabbSize{1, 1, 1};
    float _padding_2;

    glm::vec3 cameraLookDirCrossX{1, 1, 1};
    float _padding_3;

    glm::vec3 cameraLookDirCrossY{1, 1, 1};
    float _padding_4;

    float randomData[32];

    void update();

    void reset();
};

#endif //MB_CLOUDS_SCENEDATA_H
