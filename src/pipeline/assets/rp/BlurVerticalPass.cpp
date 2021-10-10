//
// Created by pekopeko on 08.09.2021.
//

#include "BlurVerticalPass.h"

#include <config.h>
#include <shaders/texture.h>

BlurVerticalPass::BlurVerticalPass(std::string input)
        : RenderPass({
                             filePaths::GLSL_SCREEN_QUAD_VERT,
                             filePaths::GLSL_BLUR_VERTICAL_FRAG
                     }, {
                             {input}
                     }),
          _input(std::move(input)),
          _hasCustomFb(false) {}

BlurVerticalPass::BlurVerticalPass(std::string input, const std::string &output)
        : RenderPass({
                             filePaths::GLSL_SCREEN_QUAD_VERT,
                             filePaths::GLSL_BLUR_VERTICAL_FRAG
                     }, {
                             {input}
                     }),
          _input(std::move(input)),
          _hasCustomFb(true) {
    _frameBuffer.attach(GL_RGBA, GL_RGBA16F, GL_FLOAT, GL_COLOR_ATTACHMENT0, output);
    _frameBuffer.complete();
}

void BlurVerticalPass::execute() {
    auto bindingCloudBlurX = texture::TextureBinding(getResponse(_input));
    BindSampler(bindingCloudBlurX.getBindingPoint(), "fbCloudBlurX");

    if (_hasCustomFb) {
        RenderQuad();
    } else {
        RenderQuad(DEFAULT_FB);
    }
}
