#version 450 core

uniform sampler2D fbColor;
uniform sampler2D fbBackgroundColor;

layout(location = 0) out vec4 finalColor;

in vec2 ScreenCoord;

uniform sampler2D blueNoiseSampler;

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

} u_SceneData;

float getRandomData(in int n) {
    vec4 packedFloats = u_SceneData.randomData[n >> 2];
    switch (n & 3) {
        case 0:
        return packedFloats.x;
        case 1:
        return packedFloats.y;
        case 2:
        return packedFloats.z;
        case 3:
        return packedFloats.w;
    }
    return 0;// Unreachable, it's here just to please GLSL compiler
}

float MapValue(float as, float bs, float ad, float bd, float value) {
    float ratio = (value - as) / (bs - as);
    value = ratio * (bd - ad) + ad;
    return value;
}

vec4 SampleBlueNoise(in ivec2 pixIndex) {
    vec2 randomOffset = vec2(getRandomData(0), getRandomData(1)) * 1000;
    pixIndex += ivec2(randomOffset);
    pixIndex %= textureSize(blueNoiseSampler, 0);
    return texelFetch(blueNoiseSampler, pixIndex, 0);
}

vec4 SampleBlueNoiseLinear(in vec2 pos) {
    vec2 randomOffset = vec2(getRandomData(0), getRandomData(1)) * 1000;
    return texture(blueNoiseSampler, pos + randomOffset);
}

vec4 NoiseShaping() {
    const float noiseValue = 2.0f / 256;
    const float offset = 0.5f * noiseValue;
    return vec4(SampleBlueNoise(ivec2(gl_FragCoord.xy)).rgb * noiseValue, 0) - offset;
}

vec3 LinearToHDR(in vec3 inColor, in float exposure) {
    const float invGamma = 1.0 / 2.2;

    vec3 tempCol = inColor;

    inColor = vec3(1.0) - exp(-inColor * exposure);
    inColor = pow(inColor, vec3(invGamma));

    return inColor;
}

void main() {
    vec2 coord = (ScreenCoord + 1) / 2;

    vec3 backgroundColor = texture(fbBackgroundColor, coord).rgb;
    vec4 cloudColor = texture(fbColor, (ScreenCoord + 1) / 2);
    float T = cloudColor.a;

    finalColor = cloudColor;

    finalColor.rgb = cloudColor.rgb + backgroundColor * (1 - pow(T, 2.2));

    finalColor.rgb = LinearToHDR(finalColor.rgb, 1.0);
    finalColor += NoiseShaping();
    finalColor.a = u_SceneData.alphaBlendIn;
}
