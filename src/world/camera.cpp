//
// Created by pekopeko on 25.07.2021.
//

#include "camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

camera::camera() {
    _recalculateRelativeAxis();
}

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
    _recalculateRelativeAxis();
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

glm::mat4 camera::GetProjection() {
    if (dirtyProjectionMatrix) {
        projectionMatrix = glm::perspective(_fov, _aspectRatio, zNear, zFar);
        dirtyProjectionMatrix = false;
    }

    return projectionMatrix;
}

void camera::SetFovAndAspectRatio(float fov, float ratio) {
    _fov = fov;
    _aspectRatio = ratio;
}

void camera::MoveRelative(glm::vec3 vector) {
    position += xPlus * vector.x + yPlus * vector.y + zPlus * vector.z;
}

void camera::RotateAbs(glm::vec3 vector) {
    auto mat = glm::yawPitchRoll(-vector.x, vector.y, vector.z);

    auto rotateMat = glm::mat3(mat);

    lookAtVector = rotateMat * glm::vec3(0, 0, 1);
    lookAtVector = glm::normalize(lookAtVector);
    std::swap(lookAtVector.y, lookAtVector.z);
}

void camera::_recalculateRelativeAxis() {
    yPlus = lookAtVector;
    xPlus = glm::normalize(glm::cross(yPlus, glm::vec3(0, 0, 1)));
    zPlus = glm::cross(lookAtVector, xPlus);
}

void camera::MoveRelativeAbs(glm::vec3 vector) {
    position += vector;
}
