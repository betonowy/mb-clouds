//
// Created by pekopeko on 25.07.2021.
//

#ifndef MB_CLOUDS_SCENEDATA_H
#define MB_CLOUDS_SCENEDATA_H

#include <glm/glm.hpp>
#include <random>

struct sceneData {
    glm::vec3 cameraPosition{-1, -1, 0.4};
//    glm::vec3 cameraPosition{0.4, -1.32, -0.35};
//    glm::vec3 cameraPosition{1, 0.75, 0.4};
//    glm::vec3 cameraPosition{-0.45, 0.8, 0.85};
//    glm::vec3 cameraPosition{0, -0.7, -1};
    float fov{};

    glm::vec3 cameraLookDir{1, 1, 1};
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
    float vdbDensityMultiplier = 1620; // /1 /5 /25 /125
    float backgroundDensity = 0.0;

    float primaryRayLength = 0.005f;
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
    float sunPower = 3.573f;

    glm::vec3 sunColor{1, 1, 1};
    float sunFocus = 40000.0f;

    glm::vec3 backgroundColorTop{0.0125, 0.05, 1.0};
    float topPower = 8;

    glm::vec3 backgroundColorBottom{0.05, 0.08, 0.19};
    float bottomPower = 1;

    glm::vec3 backgroundColorMid{1.0, 0.875, 0.75};
    float midPower = 2;

    float randomData[8 * 4];

    float alphaBlendIn = 1;
    float radianceMultiplier = 0.175;
    float ambientMultiplier = 6.414;
    float cloudHeight = 0.096;

    float cloudHeightSensitivity = 4.0;
    float densityMultiplier = 0.04;
    float _padding_5;
    float _padding_6;

    float gaussian[32 * 4];

    int32_t gaussianRadius = 0;
    int32_t ms_skip = 22;
    float ms_ratio = 0;
    float ms_cloudRadius = 0.097f;

    glm::vec4 ms_samplePoints[256];

    float ms_integral_mult = 5;
    float ms_intensity_mult = 6.414;
    float ms_distance_pow = 2;
    float ms_lorenz_mie_mult = 0;

    float ss_integral_mult = 22;
    float ss_intensity_mult = 3.573;
    float ss_depth_param_mult = 10;
    float ss_lorenz_mie_mult = 1;

    glm::vec4 ms_cloud_ambient_col;

    void update();

    void reset();

    void calculateGaussian(int radius);

    void calculateMsPoints(std::uniform_real_distribution<float>& dist, std::ranlux24_base& eRand);
};

#endif //MB_CLOUDS_SCENEDATA_H
