//
// Created by pekopeko on 25.07.2021.
//

#ifndef MB_CLOUDS_SHADER_H
#define MB_CLOUDS_SHADER_H

#include <gl/glew.h>

#include <initializer_list>
#include <vector>
#include <string>

class shader {
public:
    shader(std::initializer_list<const char*> sources);

    ~shader();

    void use() const;

private:
    void _compileSources(const std::initializer_list<const char*> &sources);

    void _compileSource(GLenum requestedType);

    void _linkProgram();

    void _postLinkCleanup();

    void _initBindings();

    std::vector<std::pair<std::string, GLenum>> describedSources;
    std::vector<GLuint> compiledShaders;

    std::string shaderPath;

    GLuint programID = 0;

};

#endif //MB_CLOUDS_SHADER_H
