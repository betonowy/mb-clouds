#version 450 core

layout(location = 0) out vec4 fbBackgroundColor;

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

vec4 NoiseShaping() {
    const float noiseValue = 1.0f / 256;
    const float offset = 0.5f * noiseValue;
    return vec4(SampleBlueNoise(ivec2(gl_FragCoord.xy)).rgb * noiseValue, 0) - offset;
}

vec3 BackgroundSun(vec3 rd) {
    vec3 color = vec3(0);

    float sunInfluence = 0.5 * (dot(rd, u_SceneData.sunDir) + 1.0);
    sunInfluence = pow(sunInfluence, u_SceneData.sunFocus);

    color += u_SceneData.sunColor * sunInfluence * u_SceneData.sunPower;

    return color;
}

vec3 BackgroundSky(vec3 rd) {
    const float sunFocus = 2.0;
    const float sunFocusB = 2.0;

    vec3 color = vec3(0);

    float sunInfluence = 0.5 * (dot(rd, u_SceneData.sunDir) + 1.0);
    sunInfluence = pow(sunInfluence, sunFocus);

    sunInfluence = MapValue(0.0, 1.0, 0.5, 1.0, sunInfluence);

    color += u_SceneData.backgroundColorTop * sunInfluence * u_SceneData.topPower;

    sunInfluence = pow(sunInfluence, sunFocusB);

    color += u_SceneData.backgroundColorMid * sunInfluence * u_SceneData.midPower;

    return color;
}

vec3 BackgroundColor(vec3 rd) {
    vec3 color = vec3(0);
    const float zenithBlackout = 0.95;
    const float groundInfluence = 32.0;
    vec3 skyColor = BackgroundSky(rd);

    float zenith = (rd.z + 1.0) * 0.5;
    float zenithBis = clamp(2.0 * zenith - 1.0, 0.0, 1.0);

    if (zenith < 0.5) zenith = pow(zenith, 1.0 - groundInfluence * (zenith - 0.5));
    color += skyColor * zenith + u_SceneData.backgroundColorBottom * (1.0 - zenith) * u_SceneData.bottomPower;

    color -= zenithBlackout * pow(zenithBis, 0.8) * skyColor;

    color += BackgroundSun(rd);

    return color;
}

void GetStartingRay(out vec3 ro, out vec3 rd) {
    ro = vec3(0);

    const float PI = 3.141593;
    const vec2 coord = ScreenCoord * PI * vec2(1, 0.5);

    float x = sin(coord.x) * cos(coord.y);
    float y = cos(coord.x) * cos(coord.y);
    float z = -sin(coord.y);

    rd = vec3(x, y, z);
}

void main() {
    vec3 ro, rd;
    GetStartingRay(ro, rd);
    fbBackgroundColor = vec4(BackgroundColor(rd), 1);
}
