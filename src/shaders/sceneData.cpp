//
// Created by pekopeko on 25.07.2021.
//

#include "sceneData.h"

#include "shaderUBO.h"
#include <config.h>
#include <cmath>

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

