//
// Created by pekopeko on 29.11.2021.
//

#include "CloudPassMSNC.h"

#include <config.h>

CloudPassMSNC::CloudPassMSNC(const std::string &output)
        : RenderPass({
                             filePaths::GLSL_SCREEN_QUAD_VERT,
                             filePaths::GLSL_VDB_MSNC
                     }, {}),
          _blueNoiseTex(filePaths::TEX_BLUENOISE) {
    _frameBuffer.attach(GL_RGBA, GL_RGBA16F, GL_FLOAT, GL_COLOR_ATTACHMENT0, output);
    _frameBuffer.complete();
}

void CloudPassMSNC::execute() {
    const auto blueNoiseTexBinding = _blueNoiseTex.getBinding();
    BindSampler(blueNoiseTexBinding.getBindingPoint(), "blueNoiseSampler");

    RenderQuad(CLEAR);
}
