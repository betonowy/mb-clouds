//
// Created by pekopeko on 05.09.2021.
//

#include "FrameBuffer.h"
#include <util/misc.h>

FrameBuffer::FrameBuffer() {
    glGenFramebuffers(1, &_fboId);
}

FrameBuffer::~FrameBuffer() {
    glDeleteFramebuffers(1, &_fboId);
}

void FrameBuffer::attach(GLenum format, GLenum internalFormat, GLenum formatType,
                         GLenum attachmentType, std::string_view name) {
    FramebufferBinding bind(_fboId, GL_FRAMEBUFFER);

    Attachment att{
            .id = 0,
            .type = attachmentType,
            .name = std::string(name),
            .internalFormat = internalFormat,
            .format = format,
            .formatType = formatType,
            .size = {800, 600},
    };

    glGenTextures(1, &att.id);
    glBindTexture(GL_TEXTURE_2D, att.id);

    glTexImage2D(GL_TEXTURE_2D, 0, (GLint) att.internalFormat,
                 att.size.x, att.size.y, 0, att.format, formatType, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, att.type, GL_TEXTURE_2D, att.id, 0);

    glBindTexture(GL_TEXTURE_2D, 0);

    _attachments.push_back(std::move(att));
}

void FrameBuffer::resize(int x, int y) {
    if (_size == glm::ivec2{x, y}) return;

    for (auto &att : _attachments) {
        att.size = {x, y};
        glBindTexture(GL_TEXTURE_2D, att.id);
        glTexImage2D(GL_TEXTURE_2D, 0, (GLint) att.internalFormat,
                     att.size.x, att.size.y, 0, att.format, att.formatType, nullptr);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    _size = {x, y};
}

FrameBuffer::FramebufferBinding FrameBuffer::getBinding(GLenum type) const {
    return FrameBuffer::FramebufferBinding(_fboId, type);
}

void FrameBuffer::complete() {
    std::vector<GLenum> buffers;

    for (auto &att : _attachments) {
        buffers.push_back(att.type);
    }

    glNamedFramebufferDrawBuffers(_fboId, (GLsizei) buffers.size(), buffers.data());

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        mb::misc::exception("Assert frame buffer completeness failed");
    }
}

const std::vector<FrameBuffer::Attachment> &FrameBuffer::getAttachments() const {
    return _attachments;
}
