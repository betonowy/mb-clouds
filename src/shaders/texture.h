//
// Created by pekopeko on 30.07.2021.
//

#ifndef MB_CLOUDS_TEXTURE_H
#define MB_CLOUDS_TEXTURE_H

#include <gl/glew.h>

#include <string_view>

class texture {
public:
    explicit texture(std::string_view path);

    ~texture();

    [[nodiscard]] inline GLuint get() const { return texID; }

    class TextureBinding {
    public:
        explicit TextureBinding(const texture &tex) : TextureBinding(tex.get()) {}

        explicit TextureBinding(int id) {
            for (int i = 0; i < 16; i++) {
                if (bindings[i] <= 0) {
                    glActiveTexture(GL_TEXTURE0 + i);
                    glBindTexture(GL_TEXTURE_2D, id);
                    bindings[i] = id;
                    bindingPoint = i;
                    return;
                }
            }
        }

        static inline int findUnit(int tex) {
            for (int i = 0; i < 16; i++) {
                if (bindings[i] == tex) return i;
            }
            return -1;
        }

        [[nodiscard]] int getBindingPoint() const {
            return bindingPoint;
        }

        ~TextureBinding() {
            glActiveTexture(GL_TEXTURE0 + bindingPoint);
            glBindTexture(GL_TEXTURE_2D, 0);
            bindings[bindingPoint] = -1;
        }

    private:
        inline static GLint bindings[16]{-1};
        int bindingPoint = 0;
    };

    TextureBinding getBinding();

private:
    GLuint texID{};
};


#endif //MB_CLOUDS_TEXTURE_H
