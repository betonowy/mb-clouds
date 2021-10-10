//
// Created by pekopeko on 08.09.2021.
//

#include "PostPassForBlur.h"

#include <config.h>
#include <shaders/texture.h>

PostPassForBlur::PostPassForBlur(std::string backgroundTargetName, std::string cloudTargetName, const std::string& outputName)
        : RenderPass({
                             filePaths::GLSL_SCREEN_QUAD_VERT,
                             filePaths::GLSL_POST_PASS_FRAG
                     }, {
                             {backgroundTargetName},
                             {cloudTargetName}
                     }),
          _backgroundTargetName(std::move(backgroundTargetName)),
          _cloudTargetName(std::move(cloudTargetName)) {
    _frameBuffer.attach(GL_RGB, GL_RGB16F, GL_FLOAT, GL_COLOR_ATTACHMENT0, outputName);
    _frameBuffer.complete();
}

void PostPassForBlur::execute() {
    auto bindingCloud = texture::TextureBinding(getResponse(_cloudTargetName));
    BindSampler(bindingCloud.getBindingPoint(), "fbColor");

    auto bindingBackground = texture::TextureBinding(getResponse(_backgroundTargetName));
    BindSampler(bindingBackground.getBindingPoint(), "fbBackgroundColor");

    RenderQuad();
}
