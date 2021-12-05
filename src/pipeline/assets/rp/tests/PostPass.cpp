//
// Created by pekopeko on 06.09.2021.
//

#include "PostPass.h"
#include <config.h>
#include <shaders/texture.h>

PostPass::PostPass(std::string backgroundTargetName, std::string cloudTargetName)
        : RenderPass({
                             filePaths::GLSL_SCREEN_QUAD_VERT,
                             filePaths::GLSL_POST_PASS_FRAG,
                     }, {
                             {backgroundTargetName},
                             {cloudTargetName},
                     }),
          _backgroundTargetName(std::move(backgroundTargetName)),
          _cloudTargetName(std::move(cloudTargetName)),
          _hasCustomFb(false),
          _blueNoiseTex(filePaths::TEX_BLUENOISE) {}

PostPass::PostPass(std::string backgroundTargetName, std::string cloudTargetName, const std::string &output)
        : RenderPass({
                             filePaths::GLSL_SCREEN_QUAD_VERT,
                             filePaths::GLSL_POST_PASS_FRAG,
                     }, {
                             {backgroundTargetName},
                             {cloudTargetName},
                     }),
          _backgroundTargetName(std::move(backgroundTargetName)),
          _cloudTargetName(std::move(cloudTargetName)),
          _hasCustomFb(true),
          _blueNoiseTex(filePaths::TEX_BLUENOISE) {
    _frameBuffer.attach(GL_RGBA, GL_RGBA32F, GL_FLOAT, GL_COLOR_ATTACHMENT0, output);
    _frameBuffer.complete();
}

void PostPass::execute() {
    auto bindingCloud = texture::TextureBinding(getResponse(_cloudTargetName));
    BindSampler(bindingCloud.getBindingPoint(), "fbColor");

    auto bindingBackground = texture::TextureBinding(getResponse(_backgroundTargetName));
    BindSampler(bindingBackground.getBindingPoint(), "fbBackgroundColor");

    const auto blueNoiseTexBinding = _blueNoiseTex.getBinding();
    BindSampler(blueNoiseTexBinding.getBindingPoint(), "blueNoiseSampler");

    if (_hasCustomFb) {
        RenderQuad();
    } else {
        RenderQuad(static_cast<RenderFlags>(DEFAULT_FB));
    }
}
