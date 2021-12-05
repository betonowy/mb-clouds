//
// Created by pekopeko on 26.11.2021.
//

#include "CloudPassSSNC.h"

#include <config.h>

CloudPassSSNC::CloudPassSSNC(const std::string &output)
        : RenderPass({
                             filePaths::GLSL_SCREEN_QUAD_VERT,
                             filePaths::GLSL_VDB_SSNC
                     }, {}),
          _blueNoiseTex(filePaths::TEX_BLUENOISE) {
    _frameBuffer.attach(GL_RGBA, GL_RGBA16F, GL_FLOAT, GL_COLOR_ATTACHMENT0, output);
    _frameBuffer.complete();
}

void CloudPassSSNC::execute() {
    const auto blueNoiseTexBinding = _blueNoiseTex.getBinding();
    BindSampler(blueNoiseTexBinding.getBindingPoint(), "blueNoiseSampler");

    RenderQuad(CLEAR);
}