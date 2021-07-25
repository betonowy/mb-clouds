//
// Created by pekopeko on 25.07.2021.
//

#ifndef MB_CLOUDS_SCENEDATA_H
#define MB_CLOUDS_SCENEDATA_H

#include <glm/glm.hpp>

struct sceneData {
    glm::vec3 cameraPosition{0, 0, 0};
    float fov{};

    glm::vec3 cameraLookDir{0 , 0, 1};
    float aspectRatio{};

    glm::mat4 viewMatrix{};

    glm::mat4 projectionMatrix{};

    glm::mat4 combinedMatrix{};

    glm::vec2 mousePosition{};
    glm::ivec2 windowResolution{};

    glm::vec3 backgroundColor{};
    float time{};

    void update();
};

#endif //MB_CLOUDS_SCENEDATA_H
