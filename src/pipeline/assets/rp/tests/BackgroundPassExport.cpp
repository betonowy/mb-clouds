//
// Created by pekopeko on 04.12.2021.
//

#include "BackgroundPassExport.h"

#include <config.h>
#include <shaders/texture.h>

BackgroundPassExport::BackgroundPassExport(const std::string &output)
        : RenderPass({
                             filePaths::GLSL_SCREEN_QUAD_VERT,
                             filePaths::GLSL_BACKGROUND_HDRI_FRAG
                     }, {}) {
    _frameBuffer.attach(GL_RGBA, GL_RGBA16F, GL_FLOAT, GL_COLOR_ATTACHMENT0, output);
    _frameBuffer.complete();
}

void BackgroundPassExport::execute() {
    RenderQuad();
}
