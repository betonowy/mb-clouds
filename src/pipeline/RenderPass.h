//
// Created by pekopeko on 05.09.2021.
//

#ifndef MB_CLOUDS_RENDERPASS_H
#define MB_CLOUDS_RENDERPASS_H

#include "FrameBuffer.h"

class RenderPass {
public:
    struct SrcRequest {
        std::string name;
    };

    struct SrcResponse {
        std::string name;
        GLint id;
    };

    struct SrcExpose {
        std::string_view name;
        GLint id;
    };

    RenderPass(std::initializer_list<const char*> shaderSources, std::initializer_list<SrcRequest> requests);

    void resize(int x, int y);

    [[nodiscard]] inline const std::vector<SrcRequest>& getRequests() const {
        return _requests;
    }

    void provideRequests(const std::vector<SrcExpose> &srcExposes);

    std::vector<SrcExpose> provideExposes();

    virtual void execute() = 0;

    void useShader();

    GLint getResponse(std::string_view name);

protected:

    enum RenderFlags {
        NULL_FLAG = 0,
        DEFAULT_FB = 1,
        CLEAR = 2,
    };

    void RenderQuad(RenderFlags renderFlags = NULL_FLAG);

    void BindSampler(int binding, std::string_view name);

    FrameBuffer _frameBuffer;
    std::vector<SrcRequest> _requests;
    std::vector<SrcResponse> _fulfilledResponses;
    shader _shader;
};


#endif //MB_CLOUDS_RENDERPASS_H
