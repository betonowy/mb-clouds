//
// Created by pekopeko on 10.07.2021.
//

#ifndef MB_CLOUDS_CONFIG_H
#define MB_CLOUDS_CONFIG_H

#include <GL/gl.h>

namespace filePaths {
    static constexpr const char* VDB_CLOUD_HD = "../res/vdb/cloud-hd.vdb";
    static constexpr const char* VDB_CLOUD_MD = "../res/vdb/cloud-md.vdb";
    static constexpr const char* VDB_CLOUD_LD = "../res/vdb/cloud-ld.vdb";

    static constexpr const char* GLSL_VDB_LATEST_FRAG = "../res/shaders/vdbSimpleColorAdditive.frag";
    static constexpr const char* GLSL_VDB_DEBUG_AABB_FRAG = "../res/shaders/vdbDebugAABB.frag";
    static constexpr const char* GLSL_SCREEN_QUAD_VERT = "../res/shaders/screenQuad.vert";

    static constexpr const char* GLSL_FRAG_SUFFIX = ".frag";
    static constexpr const char* GLSL_VERT_SUFFIX = ".vert";

    static constexpr const char* TEX_BLUENOISE = "../res/tex/LDR_RGBA_0.png";

    static constexpr const char* STRING_VDB_SHADER_OPTIONS = "../res/strings/stringVdbShaderOptions.txt";
}

namespace bindings {
    static constexpr GLuint vdbDesc = 0;
    static constexpr GLuint vdbRoots = 1;
    static constexpr GLuint vdbNodes = 2;
    static constexpr GLuint vdbLeaves = 3;
    static constexpr GLuint sceneData = 4;
}

namespace texIndex {
    static constexpr GLint texBlueNoise = 0;
}

namespace ui {
    static constexpr const char* titleBarTitle = "titleBar";
}

#endif // MB_CLOUDS_CONFIG_H
