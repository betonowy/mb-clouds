//
// Created by pekopeko on 25.07.2021.
//

#include "shaderUBO.h"

shaderUBO::shaderUBO(GLuint index, GLsizei size)
        : _index(index), _size(size) {
    glGenBuffers(1, &_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, _ubo);
    glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, index, _ubo, 0, size);
}

shaderUBO::~shaderUBO() {
    glDeleteBuffers(1, &_ubo);
}

void shaderUBO::update(void *data) {
    glBufferSubData(GL_UNIFORM_BUFFER, 0, _size, data);
}
