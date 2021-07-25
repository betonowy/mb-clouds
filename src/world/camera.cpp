//
// Created by pekopeko on 25.07.2021.
//

#include "camera.h"

#include <glm/gtc/matrix_transform.hpp>

glm::mat4 camera::GetCombined() {
    if (dirtyFullMatrix || dirtyViewMatrix || dirtyProjectionMatrix) {
        fullMatrix = GetProjection() * GetView();
        dirtyFullMatrix = false;
    }

    return fullMatrix;
}

glm::mat4 camera::GetView() {
    if (dirtyViewMatrix) {
        viewMatrix = glm::lookAt(position, position + lookAtVector, up);
        dirtyViewMatrix = false;
    }

    return viewMatrix;
}

void camera::SetPosition(const glm::vec3 &pos) {
    dirtyViewMatrix = dirtyFullMatrix = true;
    position = pos;
}

void camera::SetDirection(const glm::vec3 &dir) {
    dirtyViewMatrix = dirtyFullMatrix = true;
    lookAtVector = dir;
}

void camera::SetPositionAndLookAtPoint(const glm::vec3 &pos, const glm::vec3 &lookAt) {
    SetPosition(pos);
    SetLookAtPoint(lookAt);
}

void camera::SetLookAtPoint(const glm::vec3 &lookAt) {
    SetDirection(lookAt - position);
}

void camera::SetPositionAndDirection(const glm::vec3 &pos, const glm::vec3 &dir) {
    SetPosition(pos);
    SetDirection(dir);
}
