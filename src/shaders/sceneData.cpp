//
// Created by pekopeko on 25.07.2021.
//

#include "sceneData.h"

#include "shaderUBO.h"
#include <config.h>
#include <cmath>
#include <iostream>

namespace {
    void updateUBO(void *data) {
        static shaderUBO ubo(bindings::sceneData, sizeof(sceneData));

        ubo.update(data);
    }
}

void sceneData::update() {
    calculateGaussian(gaussianRadius);
    updateUBO(this);
}

void sceneData::reset() {
    *this = sceneData();
}

void sceneData::calculateGaussian(int radius) {
    if (radius == 0) {
        gaussian[std::size(gaussian) / 2] = 1;
        return;
    }

    int centerCoord = int(std::size(gaussian) / 2);

    if (radius + 1 >= centerCoord) { return; }

    auto function = [](float x, float o) -> float {
        o *= 0.33f;
        const auto xsq = x * x;
        const auto osq = o * o;

        return std::exp(-xsq / (2.f * osq)) / (std::sqrt(2.f * float(M_PI) * osq));
    };

    for (int i{}; i < int(std::size(gaussian)); i++) {
        gaussian[i] = function(float(i - centerCoord), float(radius));
    }

    float sum{};

    for (int i{-radius}; i <= radius; i++) {
        sum += gaussian[i + centerCoord];
    }

    float inv = 1.f / sum;

    for (int i{}; i < int(std::size(gaussian)); i++) {
        gaussian[i] *= inv;
    }
}

void sceneData::calculateMsPoints(std::uniform_real_distribution<float>& dist, std::ranlux24_base& eRand) {
    std::size_t index{0};
    float multiplier = 1.f / static_cast<float>(std::size(ms_samplePoints));

    auto offsetFunc = [](float x, float a) {
        return x + exp2f(x) * a;
    };

    for (auto &point : ms_samplePoints) {
        glm::vec3 dir{glm::normalize(glm::vec3{dist(eRand) - 0.5f, dist(eRand) - 0.5f, dist(eRand) - 0.5f})};

        glm::float32 length{static_cast<float>(++index) * multiplier};
        // square distribution
//        length *= length;
        // cubic distribution
//        length *= length * length;
        point = {dir, length * static_cast<float>(ms_sphereSize)};
        point.z = offsetFunc(point.z, 1.f);
    }
}

