//
// Created by pekopeko on 30.07.2021.
//

#ifndef MB_CLOUDS_TEXTURE_H
#define MB_CLOUDS_TEXTURE_H

#include <gl/glew.h>

#include <string_view>

class texture {
public:
    explicit texture(std::string_view path, int index);

    ~texture();

    [[nodiscard]] inline GLuint get() const { return texID; }

private:
    GLuint texID{};
};


#endif //MB_CLOUDS_TEXTURE_H
