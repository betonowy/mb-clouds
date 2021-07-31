//
// Created by pekopeko on 30.07.2021.
//

#include "texture.h"

#include <util/file/binaryFile.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_STDIO

#include <stb_image.h>

texture::texture(std::string_view path, int index) {
    glGenTextures(1, &texID);

    int iX, iY, iC;
    uint8_t *imageData;

    {
        mb::binaryFile file(path.data());
        imageData = stbi_load_from_memory(file.data(), int(file.size()), &iX, &iY, &iC, 4);
    }

    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, iX, iY, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);

    stbi_image_free(imageData);

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

}

texture::~texture() {
    glDeleteTextures(1, &texID);
}
