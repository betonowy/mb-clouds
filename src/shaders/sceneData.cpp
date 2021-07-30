//
// Created by pekopeko on 25.07.2021.
//

#include "sceneData.h"

#include "shaderUBO.h"
#include <config.h>

#include <cstring>

namespace {
    sceneData lastUpdated;

    void updateUBO(void* data) {
        static shaderUBO ubo(bindings::sceneData, sizeof(sceneData));

        ubo.update(data);
    }
}

void sceneData::update() {
    updateUBO(this);
}
