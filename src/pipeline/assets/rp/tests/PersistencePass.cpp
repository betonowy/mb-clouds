//
// Created by pekopeko on 02.10.2021.
//

#include "PersistencePass.h"
#include <config.h>
#include <shaders/texture.h>

PersistencePass::PersistencePass(std::string input, const std::string &output)
        : RenderPass(
        {
                filePaths::GLSL_SCREEN_QUAD_VERT,
                filePaths::GLSL_COPY_FRAG,
        }, {
                {input},
        }),
        _input(std::move(input)) {
    _frameBuffer.attach(GL_RGBA, GL_RGBA16F, GL_FLOAT, GL_COLOR_ATTACHMENT0, output);
    _frameBuffer.complete();
}

void PersistencePass::execute() {
    auto bindingCloud = texture::TextureBinding(getResponse(_input));
    BindSampler(bindingCloud.getBindingPoint(), "fbColor");

    RenderQuad(CLEAR);
}
