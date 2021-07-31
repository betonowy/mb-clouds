#version 450 core

uniform sampler2D blueNoiseSampler;

const uint VDB_BAD_INDEX = 0xFFFFFFFF;
const float AABB_TOL = 0.99999;

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
    float _padding_1;

    vec3 aabbSize;
    float _padding_2;

    vec3 cameraLookDirCrossX;
    float _padding_3;

    vec3 cameraLookDirCrossY;
    float _padding_4;

    vec4 randomData[8];
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
    return 0; // Unreachable, it's here just to please GLSL compiler
}

out vec4 FragColor;

in vec2 ScreenCoord;

// *************************************************** begin post processing

vec3 LinearToHDR(vec3 inColor, float exposure) {
    const float invGamma = 1.0 / 2.2;

    vec3 tempCol = inColor;

    inColor = vec3(1.0) - exp(-inColor * exposure);
    inColor = pow(inColor, vec3(invGamma));

    return inColor;
}

vec4 SampleBlueNoise(ivec2 pixIndex) {
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

// *************************************************** end post processing

// *************************************************** begin first bounce handle functions

void getStartingRay(out vec3 ro, out vec3 rd) {
    rd = u_SceneData.camDir;
    ro = u_SceneData.camPos;

    vec3 xPosVec = normalize(cross(rd, vec3(0, 0, 1)));

    vec3 yPosVec = cross(rd, -xPosVec) * u_SceneData.camRatio;

    vec2 offset = ScreenCoord;
    rd += xPosVec * offset.x + yPosVec * offset.y;

    rd = normalize(rd);
}

float GetDistAABB(in vec3 pos, in vec3 nor) {
    const vec3 C_P = u_SceneData.aabbPosition;
    const vec3 C_S = u_SceneData.aabbSize;

    vec3 diff = C_P - pos;
    vec3 absDiff = abs(diff);

    // if inside AABB domain -> distance equals zero
    if (absDiff.x <= C_S.x && absDiff.y <= C_S.y && absDiff.z <= C_S.z) return 0.0f;

    // check which AABB side should be tested
    ivec3 sideDesc;
    sideDesc.x = (1 - 2 * int(diff.x > 0.)) * int(absDiff.x >= C_S.x);
    sideDesc.y = (1 - 2 * int(diff.y > 0.)) * int(absDiff.y >= C_S.y);
    sideDesc.z = (1 - 2 * int(diff.z > 0.)) * int(absDiff.z >= C_S.z);

    // AABB test planes coordinates
    vec3 coordAABB = C_P + C_S * vec3(sideDesc);

    // AABB coord difference
    vec3 diffAABB = coordAABB - pos;

    // X plane test
    if (sideDesc.x != 0 && nor.x != 0.0f && nor.x * diff.x > 0.) {
        vec3 pInt = pos + nor * diffAABB.x / nor.x;
        vec3 intDiff = abs(C_P - pInt) * AABB_TOL;
        if (intDiff.x <= C_S.x && intDiff.y <= C_S.y && intDiff.z <= C_S.z) return length(pInt - pos);
    }

    // Y plane test
    if (sideDesc.y != 0 && nor.y != 0.0f && nor.y * diff.y > 0.) {
        vec3 pInt = pos + nor * diffAABB.y / nor.y;
        vec3 intDiff = abs(C_P - pInt) * AABB_TOL;
        if (intDiff.x <= C_S.x && intDiff.y <= C_S.y && intDiff.z <= C_S.z) return length(pInt - pos);
    }

    // Z plane test
    if (sideDesc.z != 0 && nor.z != 0.0f && nor.z * diff.z > 0.) {
        vec3 pInt = pos + nor * diffAABB.z / nor.z;
        vec3 intDiff = abs(C_P - pInt) * AABB_TOL;
        if (intDiff.x <= C_S.x && intDiff.y <= C_S.y && intDiff.z <= C_S.z) return length(pInt - pos);
    }

    // no intersection -> distance equals minus one
    return -1.0f;
}

vec3 RayTraceDistToCol(in vec3 ro, in vec3 rd) {
    float dO = GetDistAABB(ro, rd);
    if (dO == 0.0f) return vec3(0.01, 0.02, 0.05);// You're inside
    else if (dO < -0.0f) return vec3(0.75, 1.5, 4.0);// no intersection
    dO = pow(dO, 2.0);
    return vec3(dO / 16, dO / 4, dO);
}

// *************************************************** end first bounce handle functions

void main() {
    vec3 ro, rd;
    getStartingRay(ro, rd);

    vec4 color = vec4(RayTraceDistToCol(ro, rd), 1.0);

    color.rgb = LinearToHDR(color.rgb, 0.1);
    color += NoiseShaping();

    FragColor = color;
}
