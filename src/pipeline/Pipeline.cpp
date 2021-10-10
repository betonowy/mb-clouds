//
// Created by pekopeko on 05.09.2021.
//

#include "Pipeline.h"

Pipeline::Pipeline(std::initializer_list<std::shared_ptr<RenderPass>> passes) : _passes(passes) {
    std::vector<RenderPass::SrcExpose> exposes;

    for (auto& pass : _passes) {
        const auto& expose = pass->provideExposes();
        exposes.insert(exposes.end(), expose.begin(), expose.end());
    }

    for (auto& pass : _passes) {
        pass->provideRequests(exposes);
    }
}

void Pipeline::execute() {
    for (auto& pass : _passes) {
        pass->useShader();
        pass->execute();
    }
}

void Pipeline::resize(glm::ivec2 resolution) {
    if (resolution == _lastResolution) {
        return;
    }

    for (auto& pass : _passes) {
        pass->resize(resolution.x, resolution.y);
    }
}
