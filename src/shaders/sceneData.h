//
// Created by pekopeko on 25.07.2021.
//

#ifndef MB_CLOUDS_SCENEDATA_H
#define MB_CLOUDS_SCENEDATA_H

#include <glm/glm.hpp>

struct sceneData {
    glm::vec3 cameraPosition{};
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

    float vdbScale = 3;
    float aabbScale = 1;

    void update();
};

#endif //MB_CLOUDS_SCENEDATA_H
