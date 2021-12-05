//
// Created by pekopeko on 07.10.2021.
//

#include "AbsDiffPass.h"

#include <config.h>
#include <shaders/texture.h>

AbsDiffPass::AbsDiffPass(std::string input_1, std::string input_2)
        : RenderPass(
        {
                filePaths::GLSL_SCREEN_QUAD_VERT,
                filePaths::GLSL_ABS_DIFF_FRAG,
        }, {
                {input_1},
                {input_2},
        }),
          _input_1(std::move(input_1)),
          _input_2(std::move(input_2)),
          _hasCustomFb(false) {}

AbsDiffPass::AbsDiffPass(std::string input_1, std::string input_2, const std::string &output)
        : RenderPass(
        {
                filePaths::GLSL_SCREEN_QUAD_VERT,
                filePaths::GLSL_ABS_DIFF_FRAG,
        }, {
                {input_1},
                {input_2},
        }),
          _input_1(std::move(input_1)),
          _input_2(std::move(input_2)),
          _hasCustomFb(true) {
    _frameBuffer.attach(GL_RGBA, GL_RGBA16F, GL_FLOAT, GL_COLOR_ATTACHMENT0, output);
    _frameBuffer.complete();
}

void AbsDiffPass::execute() {
    auto bindingInput_1 = texture::TextureBinding(getResponse(_input_1));
    BindSampler(bindingInput_1.getBindingPoint(), "absDiffInput_1");

    auto bindingInput_2 = texture::TextureBinding(getResponse(_input_2));
    BindSampler(bindingInput_2.getBindingPoint(), "absDiffInput_2");

    glDisable(GL_BLEND);
    if (_hasCustomFb) {
        RenderQuad(CLEAR);
    } else {
        RenderQuad(static_cast<RenderFlags>(CLEAR | DEFAULT_FB));
    }
    glEnable(GL_BLEND);
}
