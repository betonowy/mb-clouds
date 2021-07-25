//
// Created by pekopeko on 25.07.2021.
//

#ifndef MB_CLOUDS_CAMERA_H
#define MB_CLOUDS_CAMERA_H

#include <glm/glm.hpp>

class camera {
public:
    glm::mat4 GetCombined();

    glm::mat4 GetView();

    virtual glm::mat4 GetProjection() = 0;

    void SetCurrentCameraMatrix();

    void SetPosition(const glm::vec3 &pos);

    void SetDirection(const glm::vec3 &dir);

    void SetPositionAndLookAtPoint(const glm::vec3 &pos, const glm::vec3 &lookAt);

    void SetLookAtPoint(const glm::vec3 &lookAt);

    void SetPositionAndDirection(const glm::vec3 &pos, const glm::vec3 &dir);

    [[nodiscard]] inline float GetFarPlane() const { return zFar; }

    static float GetScreenAspectRatio();

    inline static int visibleObjs = 0;
private:

    glm::mat4 viewMatrix = glm::mat4(1.0);
    bool dirtyViewMatrix = true;

    glm::mat4 projectionMatrix = glm::mat4(1.0);
    bool dirtyProjectionMatrix = true;

    float zNear = 0.1f, zFar = 1000.f;

    glm::mat4 worldTransform = glm::mat4(1.0);

    static constexpr const glm::vec3 up = {0, 0, 1};
    static constexpr const glm::vec3 forward = {0, 1, 0};

    glm::vec3 lookAtVector = {0, 1, 0};
    glm::vec3 position = {0, 0, 0};

    glm::mat4 fullMatrix = glm::mat4(1.0);
    bool dirtyFullMatrix = true;

};


#endif //MB_CLOUDS_CAMERA_H
