#version 450 core

layout(location = 0) out vec4 fbCloudBlurred;

in vec2 ScreenCoord;

uniform sampler2D blueNoiseSampler;

uniform sampler2D fbCloudBlurX;

layout(std140, binding = 4) uniform SceneData {
    vec3 camPos;
    float camFov;

    vec3 camDir;
    float camRatio;

    mat4 viewMat;

    mat4 projMat;

    mat4 combMat;

    vec2 mousePos;
    ivec2 windowRes;

    vec3 bkgColor;
    float timepoint;

    float vdbScale;
    float aabbScale;
    float vdbDensityMultipier;
    float backgroundDensity;

    float primaryRayLength;
    float primaryRayLengthMultipier;
    float secondaryRayLength;
    float secondaryRayLengthMultipier;

    vec3 aabbPosition;
    float extinction_A;

    vec3 aabbSize;
    float extinction_B;

    vec3 cameraLookDirCrossX;
    float lm_mod_A;

    vec3 cameraLookDirCrossY;
    float lm_mod_B;

    vec3 sunDir;
    float sunPower;

    vec3 sunColor;
    float sunFocus;

    vec3 backgroundColorTop;
    float topPower;

    vec3 backgroundColorBottom;
    float bottomPower;

    vec3 backgroundColorMid;
    float midPower;

    vec4 randomData[8];

    float alphaBlendIn;
    float radianceMultiplier;
    float ambientMultiplier;
    float cloudHeight;

    float cloudHeightSensitivity;
    float densityMultiplier;
    float _padding_1;
    float _padding_2;

    vec4 gaussian[32];

    int gaussianRadius;
    float _padding_3;
    float _padding_4;
    float _padding_5;

} u_SceneData;

float getGaussian(int x) {
    x += 64;

    int minor = x % 4;
    int major = x / 4;

    return u_SceneData.gaussian[major][minor];
}

void main() {
    fbCloudBlurred = vec4(0);
    int radius = u_SceneData.gaussianRadius;
    vec2 coord = (ScreenCoord + 1) / 2;
    float dotsize = 1.0f / float(u_SceneData.windowRes.y);

    for (int i = -radius; i <= radius; i++) {
        fbCloudBlurred += getGaussian(i) * texture(fbCloudBlurX, coord - vec2(0, i * dotsize));
    }

    fbCloudBlurred.a = 1;
}
