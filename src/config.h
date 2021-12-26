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

    static constexpr const char* GLSL_VDB_COLOR_ADD_FRAG = "../res/shaders/vdbSimpleColorAdditive.frag";
    static constexpr const char* GLSL_VDB_SECONDARY_SUB_FRAG = "../res/shaders/vdbSecondaryRaySubtract.frag";
    static constexpr const char* GLSL_VDB_SECONDARY_SUB_V2_FRAG = "../res/shaders/vdbSecondaryRaySubtract-v2.frag";
    static constexpr const char* GLSL_VDB_SECONDARY_SUB_V3_FRAG = "../res/shaders/vdbSecondaryRaySubtract-v3.frag";
    static constexpr const char* GLSL_VDB_DEBUG_AABB_FRAG = "../res/shaders/vdbDebugAABB.frag";
    static constexpr const char* GLSL_SCREEN_QUAD_VERT = "../res/shaders/screenQuad.vert";
    static constexpr const char* GLSL_POST_PASS_FRAG = "../res/shaders/postPass.frag";
    static constexpr const char* GLSL_POST_PASS_EXPORT_FRAG = "../res/shaders/postPassExport.frag";
    static constexpr const char* GLSL_BACKGROUND_FRAG = "../res/shaders/background.frag";
    static constexpr const char* GLSL_BACKGROUND_HDRI_FRAG = "../res/shaders/backgroundHDRI.frag";
    static constexpr const char* GLSL_BLUR_HORIZONTAL_FRAG = "../res/shaders/blurHorizontal.frag";
    static constexpr const char* GLSL_BLUR_VERTICAL_FRAG = "../res/shaders/blurVertical.frag";
    static constexpr const char* GLSL_COPY_FRAG = "../res/shaders/copy.frag";
    static constexpr const char* GLSL_ABS_DIFF_FRAG = "../res/shaders/absDiff.frag";

    static constexpr const char* GLSL_VDB_SSNC = "../res/shaders/vdb/final/SSNC.frag";
    static constexpr const char* GLSL_VDB_SSLC = "../res/shaders/vdb/final/SSLC.frag";
    static constexpr const char* GLSL_VDB_SSNBF = "../res/shaders/vdb/final/SSNBF.frag";
    static constexpr const char* GLSL_VDB_SSLBF = "../res/shaders/vdb/final/SSLBF.frag";
    static constexpr const char* GLSL_VDB_MSNC = "../res/shaders/vdb/final/MSNC.frag";
    static constexpr const char* GLSL_VDB_MSLC = "../res/shaders/vdb/final/MSLC.frag";

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
    static constexpr GLuint cachedDesc = 5;
    static constexpr GLuint cachedRoots = 6;
    static constexpr GLuint cachedNodes = 7;
    static constexpr GLuint cachedLeaves = 8;
}

namespace texIndex {
    static constexpr GLint texBlueNoise = 15;
}

namespace ui {
    static constexpr const char* titleBarTitle = "titleBar";
}

#endif // MB_CLOUDS_CONFIG_H
