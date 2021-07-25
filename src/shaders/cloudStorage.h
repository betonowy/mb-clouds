//
// Created by pekopeko on 25.07.2021.
//

#ifndef MB_CLOUDS_CLOUDSTORAGE_H
#define MB_CLOUDS_CLOUDSTORAGE_H

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <tree/vdbDataExtract.h>
#include <config.h>

template<typename valueType, int rootLevel, int nodeLevel, int leafLevel>
class cloudStorage {
    using vdbGlType = vdbGl<valueType, rootLevel, nodeLevel, leafLevel>;

public:
    explicit cloudStorage(vdbGlType &extract);

    ~cloudStorage();

    void bind();

private:
    GLuint descSSBO{};
    GLuint rootsSSBO{};
    GLuint nodesSSBO{};
    GLuint leavesSSBO{};

    GLuint VAO{};
//    GLuint VBO{};
};

template<typename valueType, int rootLevel, int nodeLevel, int leafLevel>
cloudStorage<valueType, rootLevel, nodeLevel, leafLevel>::cloudStorage(cloudStorage::vdbGlType &extract) {
    glGenVertexArrays(1, &VAO);

    glGenBuffers(1, &descSSBO);
    glGenBuffers(1, &rootsSSBO);
    glGenBuffers(1, &nodesSSBO);
    glGenBuffers(1, &leavesSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, descSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, extract.getDescriptionSize(), extract.getDescriptionPtr(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindings::vdbDesc, descSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, rootsSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, extract.getRootsSize(), extract.getRootsPtr(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindings::vdbDesc, rootsSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, nodesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, extract.getNodesSize(), extract.getNodesPtr(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindings::vdbDesc, nodesSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, leavesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, extract.getLeavesSize(), extract.getLeavesPtr(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindings::vdbDesc, leavesSSBO);
}

template<typename valueType, int rootLevel, int nodeLevel, int leafLevel>
cloudStorage<valueType, rootLevel, nodeLevel, leafLevel>::~cloudStorage() {
    glDeleteBuffers(1, &descSSBO);
    glDeleteBuffers(1, &rootsSSBO);
    glDeleteBuffers(1, &nodesSSBO);
    glDeleteBuffers(1, &leavesSSBO);

    glDeleteVertexArrays(1, &VAO);
}

template<typename valueType, int rootLevel, int nodeLevel, int leafLevel>
void cloudStorage<valueType, rootLevel, nodeLevel, leafLevel>::bind() {
    glBindVertexArray(VAO);
}


#endif //MB_CLOUDS_CLOUDSTORAGE_H
