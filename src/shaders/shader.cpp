//
// Created by pekopeko on 25.07.2021.
//

#include "shader.h"

#include <util/misc.h>
#include <util/file/binaryFile.h>
#include <config.h>

#include <sstream>
#include <iostream>

shader::shader(std::initializer_list<const char *> sources) {
    _compileSources(sources);
    _initBindings();
}

shader::~shader() {
    if (programID != 0) {
        glDeleteProgram(programID);
    }
}

void shader::_compileSources(const std::initializer_list<const char *> &sources) {
    for (auto &source : sources) {
        std::string_view path(source);

        GLenum shaderType = GL_INVALID_ENUM;

        if (path.find(filePaths::GLSL_FRAG_SUFFIX) != std::string::npos) {
            shaderType = GL_FRAGMENT_SHADER;
        } else if (path.find(filePaths::GLSL_VERT_SUFFIX) != std::string::npos) {
            shaderType = GL_VERTEX_SHADER;
        }

        if (shaderType == GL_INVALID_ENUM) {
            std::stringstream ss;
            ss << "Unrecognizable shader file extension: " << source;
            mb::misc::exception(ss.str().c_str());
        }

        describedSources.emplace_back(source, shaderType);
    }

    _compileSource(GL_VERTEX_SHADER);
    _compileSource(GL_FRAGMENT_SHADER);

    _linkProgram();
    _postLinkCleanup();
}

void shader::_compileSource(GLenum requestedType) {
    std::string typeStr;

    switch (requestedType) {
        case GL_FRAGMENT_SHADER:
            typeStr = "Fragment shader";
            break;
        case GL_VERTEX_SHADER:
            typeStr = "Vertex shader";
            break;
        default:
            mb::misc::exception("Unknown shader type");
            break;
    }

    std::vector<std::string_view> paths;

    for (auto &[path, type] : describedSources) {
        if (type == requestedType) {
            paths.emplace_back(path);
        }
    }

    if (paths.empty()) {
        std::cout << "No sources for: " << typeStr << "\n";
        return;
    }

    std::cout << "Compiling shader: " << typeStr << "\n";

    GLuint shader;

    shader = glCreateShader(requestedType);

    for (auto &path : paths) {
        std::cout << "Source path: " << path << "\n";

        mb::binaryFile file(path.data());

        const auto *shaderSource = reinterpret_cast<const GLchar *>(file.data());
        auto shaderSourceSize = static_cast<GLint>(file.size());

        glShaderSource(shader, 1, &shaderSource, &shaderSourceSize);
    }

    glCompileShader(shader);

    {
        GLint shaderCompileStatus{};

        glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderCompileStatus);

        if (shaderCompileStatus == GL_TRUE) {
            std::cout << "Successfully compiled shader: " << typeStr << "\n";
            compiledShaders.push_back(shader);
        } else {
            std::cout << "Error during shader compilation: " << typeStr << "\n";
        }
    }

    GLint infoLogLength{};

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

    if (infoLogLength > 0) {
        std::cout << "Shader info log: " << typeStr << "\n";

        std::vector<char> buffer(infoLogLength);

        glGetShaderInfoLog(shader, static_cast<GLsizei>(buffer.size()), nullptr, buffer.data());
        std::cout << buffer.data() << "End of shader info log: " << typeStr << "\n";
    }
}

void shader::_linkProgram() {
    GLuint program = glCreateProgram();

    for (auto &shader : compiledShaders) {
        glAttachShader(program, shader);
    }

    glLinkProgram(program);

    {
        GLint linkCompileStatus{};

        glGetProgramiv(program, GL_LINK_STATUS, &linkCompileStatus);

        if (linkCompileStatus == GL_TRUE) {
            std::cout << "Successfully linked program:\n";
            programID = program;
        } else {
            std::cout << "Error during program linking:\n";
        }
    }

    GLint infoLogLength{};

    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

    if (infoLogLength > 0) {
        std::cout << "Program info log:\n";

        std::vector<char> buffer(infoLogLength);

        glGetProgramInfoLog(program, static_cast<GLsizei>(buffer.size()), nullptr, buffer.data());
        std::cout << buffer.data() << "End of program info log:\n";
    }
}

void shader::_postLinkCleanup() {
    for (auto &shader : compiledShaders) {
        glDeleteShader(shader);
    }
}

void shader::use() const {
    glUseProgram(programID);
}

void shader::_initBindings() {
    auto bindUbo = [this](GLuint index, const char *name) {
        auto glIndex = glGetProgramResourceIndex(programID, GL_UNIFORM_BLOCK, name);

        if (glIndex == GL_INVALID_INDEX) return;

        std::cout << "Shader has UBO: " << name << "\n";
        glUniformBlockBinding(programID, glIndex, index);
    };

    auto bindSsbo = [this](GLuint index, const char *name) {
        auto glIndex = glGetProgramResourceIndex(programID, GL_SHADER_STORAGE_BLOCK, name);

        if (glIndex == GL_INVALID_INDEX) return;

        std::cout << "Shader has SSBO: " << name << "\n";
        glShaderStorageBlockBinding(programID, glIndex, index);
    };

    bindSsbo(bindings::vdbDesc, "VdbDesc");
    bindSsbo(bindings::vdbRoots, "VdbRoots");
    bindSsbo(bindings::vdbNodes, "VdbNodes");
    bindSsbo(bindings::vdbLeaves, "VdbLeaves");

    bindUbo(bindings::sceneData, "SceneData");
}
