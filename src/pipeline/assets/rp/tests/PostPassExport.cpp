//
// Created by pekopeko on 02.12.2021.
//

#include "PostPassExport.h"
#include <config.h>

static int runtimeImgCounter = 0;

PostPassExport::PostPassExport(std::string backgroundTargetName)
        : RenderPass({
                             filePaths::GLSL_SCREEN_QUAD_VERT,
                             filePaths::GLSL_POST_PASS_EXPORT_FRAG,
                     }, {
                             {backgroundTargetName}
                     }),
          _backgroundTargetName(std::move(backgroundTargetName)),
          _blueNoiseTex(filePaths::TEX_BLUENOISE) {
    _frameBuffer.attach(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT0, "output");
    _frameBuffer.complete();
}

void PostPassExport::execute() {
    auto bindingBackground = texture::TextureBinding(getResponse(_backgroundTargetName));
    BindSampler(bindingBackground.getBindingPoint(), "fbBackgroundColor");

    const auto blueNoiseTexBinding = _blueNoiseTex.getBinding();
    BindSampler(blueNoiseTexBinding.getBindingPoint(), "blueNoiseSampler");

    RenderQuad();

    _frameBuffer.save("background" + std::to_string(runtimeImgCounter++));
}
