//
// Created by pekopeko on 25.07.2021.
//

#ifndef MB_CLOUDS_SHADERUBO_H
#define MB_CLOUDS_SHADERUBO_H

#include <GL/glew.h>

#include <string>
#include <unordered_map>

class shaderUBO {
public:

    shaderUBO(GLuint index, GLsizei size);

    ~shaderUBO();

    shaderUBO(const shaderUBO &) = delete;

    shaderUBO(shaderUBO &&) = delete;

    shaderUBO &operator=(const shaderUBO &) = delete;

    shaderUBO &operator=(shaderUBO &&) = delete;

    void update(void* data);

private:
    GLuint _ubo{};
    GLuint _index{};
    GLsizei _size{};

};


#endif //MB_CLOUDS_SHADERUBO_H
