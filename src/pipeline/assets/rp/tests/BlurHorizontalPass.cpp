//
// Created by pekopeko on 08.09.2021.
//

#include "BlurHorizontalPass.h"

#include <config.h>
#include <shaders/texture.h>

BlurHorizontalPass::BlurHorizontalPass(std::string input, const std::string &output)
        : RenderPass({
                             filePaths::GLSL_SCREEN_QUAD_VERT,
                             filePaths::GLSL_BLUR_HORIZONTAL_FRAG
                     }, {
                             {input}
                     }),
          _input(std::move(input)) {
    _frameBuffer.attach(GL_RGBA, GL_RGBA16F, GL_FLOAT, GL_COLOR_ATTACHMENT0, output);
    _frameBuffer.complete();
}

void BlurHorizontalPass::execute() {
    auto bindingCloud = texture::TextureBinding(getResponse(_input));
    BindSampler(bindingCloud.getBindingPoint(), "fbCloudColor");

    RenderQuad(CLEAR);
}
