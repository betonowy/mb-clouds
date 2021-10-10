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

    explicit shader(std::initializer_list<const char*> sources);

    explicit shader(const std::vector<std::string>& sources);

    ~shader();

    void use() const;

    void bindTextureUnit(int tex, std::string_view name);

private:
    template<class iterator>
    void _compileSources(iterator begin, iterator end);

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
