//
// Created by pekopeko on 05.09.2021.
//

#include "CloudPass.h"

#include <config.h>

CloudPass::CloudPass(const std::string &output)
        : RenderPass({
                             filePaths::GLSL_SCREEN_QUAD_VERT,
                             filePaths::GLSL_VDB_SECONDARY_SUB_V2_FRAG
                     }, {}),
          _blueNoiseTex(filePaths::TEX_BLUENOISE) {
    _frameBuffer.attach(GL_RGBA, GL_RGBA16F, GL_FLOAT, GL_COLOR_ATTACHMENT0, output);
    _frameBuffer.complete();
}

CloudPass::CloudPass(const std::string &output, const std::string &noiseLayer)
        : RenderPass({
                             filePaths::GLSL_SCREEN_QUAD_VERT,
                             filePaths::GLSL_VDB_SECONDARY_SUB_V3_FRAG
                     }, {{noiseLayer}}),
          _blueNoiseTex(filePaths::TEX_BLUENOISE),
          _noiseLayerInput(noiseLayer) {
    _frameBuffer.attach(GL_RGBA, GL_RGBA16F, GL_FLOAT, GL_COLOR_ATTACHMENT0, output);
    _frameBuffer.complete();
}

void CloudPass::execute() {
    const auto blueNoiseTexBinding = _blueNoiseTex.getBinding();
    BindSampler(blueNoiseTexBinding.getBindingPoint(), "blueNoiseSampler");

    if (_noiseLayerInput) {
        const auto noiseLayerBinding = texture::TextureBinding(getResponse(*_noiseLayerInput));
        BindSampler(noiseLayerBinding.getBindingPoint(), "adaptiveNoiseSampler");
        RenderQuad(CLEAR);
    } else {
        RenderQuad(CLEAR);
    }
}
