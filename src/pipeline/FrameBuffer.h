//
// Created by pekopeko on 05.09.2021.
//

#ifndef MB_CLOUDS_FRAMEBUFFER_H
#define MB_CLOUDS_FRAMEBUFFER_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string_view>
#include <string>
#include <shaders/shader.h>
#include <memory>

class FrameBuffer {
public:
    struct Attachment {
        GLuint id;
        GLenum type;
        std::string name;
        GLenum internalFormat;
        GLenum format;
        GLenum formatType;
        glm::ivec2 size;
    };

    class FramebufferBinding {
    public:
        FramebufferBinding() : _type(0) {}

        inline explicit FramebufferBinding(GLuint id, GLenum type) : _type(type) {
            GLenum derivedType;

            switch (type) {
                case GL_FRAMEBUFFER:
                    derivedType = GL_FRAMEBUFFER_BINDING;
                    break;
            }

            glGetIntegerv(derivedType, &_previousId);
            glBindFramebuffer(type, id);
        }

        inline ~FramebufferBinding() {
            if (_type == 0) { return; }
            glBindFramebuffer(_type, _previousId);
        }

        FramebufferBinding(FramebufferBinding &) = delete;

        FramebufferBinding &operator=(FramebufferBinding &) = delete;

    private:
        GLint _previousId{};
        GLenum _type;
    };

    class TextureBinding {
    public:
        inline explicit TextureBinding(GLuint id, GLenum type) : _type(type) {
            glGetIntegerv(type, &_previousId);
            glBindTexture(type, id);
        }

        inline ~TextureBinding() {
            glBindTexture(_type, _previousId);
        }

        TextureBinding(TextureBinding &) = delete;

        TextureBinding &operator=(TextureBinding &) = delete;

    private:
        GLint _previousId{};
        GLenum _type;
    };

    FrameBuffer();

    ~FrameBuffer();

    void resize(int x, int y);

    void complete();

    void attach(GLenum format, GLenum internalFormat, GLenum formatType, GLenum attachmentType, std::string_view name);

    void save(std::string filename);

    [[nodiscard]] FramebufferBinding getBinding(GLenum type) const;

    [[nodiscard]] const std::vector<Attachment> &getAttachments() const;

private:
    GLuint _fboId{};
    std::vector<Attachment> _attachments;
    glm::ivec2 _size{};
};


#endif //MB_CLOUDS_FRAMEBUFFER_H
