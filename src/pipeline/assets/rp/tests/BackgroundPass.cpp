//
// Created by pekopeko on 08.09.2021.
//

#include "BackgroundPass.h"

#include <config.h>


BackgroundPass::BackgroundPass(const std::string &output)
        : RenderPass({
                             filePaths::GLSL_SCREEN_QUAD_VERT,
                             filePaths::GLSL_BACKGROUND_FRAG
                     }, {}) {
    _frameBuffer.attach(GL_RGBA, GL_RGBA16F, GL_FLOAT, GL_COLOR_ATTACHMENT0, output);
    _frameBuffer.complete();
}

void BackgroundPass::execute() {
    RenderQuad();
}
