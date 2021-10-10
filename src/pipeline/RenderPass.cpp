//
// Created by pekopeko on 05.09.2021.
//

#include "RenderPass.h"
#include <algorithm>

void RenderPass::resize(int x, int y) {
    _frameBuffer.resize(x, y);
}

void RenderPass::provideRequests(const std::vector<SrcExpose> &srcExposes) {
    _fulfilledResponses.clear();

    std::for_each(_requests.begin(), _requests.end(), [this, &srcExposes](auto &req) {
        std::for_each(srcExposes.begin(), srcExposes.end(), [this, &req](auto &res) {
            if (res.name == req.name) {
                _fulfilledResponses.emplace_back(SrcResponse{.name = req.name, .id = res.id});
            }
        });
    });
}

std::vector<RenderPass::SrcExpose> RenderPass::provideExposes() {
    const auto &atts = _frameBuffer.getAttachments();

    std::vector<SrcExpose> exposes;
    exposes.reserve(atts.size());

    for (const auto &att : atts) {
        exposes.emplace_back(SrcExpose{.name = att.name, .id = static_cast<GLint>(att.id)});
    }

    return exposes;
}

RenderPass::RenderPass(std::initializer_list<const char *> shaderSources, std::initializer_list<SrcRequest> requests)
        : _requests(requests),
          _shader(shaderSources) {}

void RenderPass::useShader() {
    _shader.use();
}

GLint RenderPass::getResponse(std::string_view name) {
    for (auto &res : _fulfilledResponses) {
        if (res.name == name) {
            return res.id;
        }
    }
    return -1;
}

void RenderPass::RenderQuad(RenderPass::RenderFlags renderFlags) {
    auto drawBinding = (renderFlags & DEFAULT_FB) ? FrameBuffer::FramebufferBinding() : (_frameBuffer.getBinding(GL_FRAMEBUFFER));

    if (renderFlags & CLEAR) {
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void RenderPass::BindSampler(int binding, std::string_view name) {
    _shader.bindTextureUnit(binding, name);
}
