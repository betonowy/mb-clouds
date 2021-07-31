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

layout(std430, binding = 0) buffer VdbDesc {
    ivec3 lowDimBB;
    uint rootCount;

    ivec3 highDimBB;
    uint nodeCount;

    uint leafCount;
} s_VdbDesc;

struct VdbRoot {
    ivec3 lowDimBB;
    uint index;

    ivec3 highDimBB;
    uint _padding_1;

    uint bitsets[1024];
    uint indices[32768];
};

layout(std430, binding = 1) buffer VdbRoots {
    VdbRoot root[];
} s_VdbRoots;

struct VdbNode {
    ivec3 lowDimBB;
    uint index;

    ivec3 highDimBB;
    uint _padding_1;

    uint bitsets[128];
    uint indices[4096];
};

layout(std430, binding = 2) buffer VdbNodes {
    VdbNode node[];
} s_VdbNodes;

struct VdbLeaf {
    ivec3 lowDimBB;
    uint index;

    ivec3 highDimBB;
    uint _padding_1;

    uint bitsets[16];
    float values[512];
};

layout(std430, binding = 3) buffer VdbLeaves {
    VdbLeaf leaf[];
} s_VdbLeaves;

out vec4 FragColor;

in vec2 ScreenCoord;

// *************************************************** begin post processing

vec3 LinearToHDR(in vec3 inColor, in float exposure) {
    const float invGamma = 1.0 / 2.2;

    vec3 tempCol = inColor;

    inColor = vec3(1.0) - exp(-inColor * exposure);
    inColor = pow(inColor, vec3(invGamma));

    return inColor;
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

// *************************************************** end post processing

// *************************************************** begin VDB accessor

struct VDB_Accessor {
    ivec3 rootPos;
    uint root;

    ivec3 nodePos;
    uint node;

    ivec3 leafPos;
    uint leaf;

    ivec3 voxelPos;
    float voxelValue;
};

int VDB_Pattern(in int a, in int b) {
    int p = 0;

    for (int i = a; i < b; i++) {
        p |= 1 << i;
    }

    return p;
}

ivec3 VDB_ReduceDim(in ivec3 pos, in int l, in int h) {
    if (bool(h)) {
        pos &= ivec3(VDB_Pattern(0, h));
    }

    if (bool(l)) {
        pos >>= ivec3(l);
    }

    return pos;
}

uint VDB_PosToIndex(in ivec3 pos, in int level) {
    return (pos.x) | (pos.y << level) | (pos.z << (level * 2));
}

void VDB_Bitset32Access(in uint index, out uint dwordIndex, out uint bitIndex) {
    dwordIndex = index >> 5;
    bitIndex = index & 31u;
}

ivec3 _VDB_DescCenter;
vec3 _VDB_AABB_Center;
float _VDB_SizeRatioMax;

void _INIT_VDB_WorldToVDB() {
    const ivec3 VDB_Size = s_VdbDesc.highDimBB - s_VdbDesc.lowDimBB;
    _VDB_DescCenter = (s_VdbDesc.highDimBB + s_VdbDesc.lowDimBB) / 2;

    const vec3 AABB_Size = u_SceneData.aabbSize;
    _VDB_AABB_Center = u_SceneData.aabbPosition;

    const vec3 sizeRatio = vec3(VDB_Size) / (AABB_Size * u_SceneData.vdbScale);
    _VDB_SizeRatioMax = max(max(sizeRatio.x, sizeRatio.y), sizeRatio.z);
}

ivec3 VDB_WorldToVDB(in vec3 pos) {
    return ivec3((pos - _VDB_AABB_Center) * _VDB_SizeRatioMax) + _VDB_DescCenter;
}

ivec3 _VDB_CachedRootPos = ivec3(VDB_BAD_INDEX);
uint _VDB_CachedRootIndex = VDB_BAD_INDEX;
ivec3 _VDB_CachedNodePos = ivec3(VDB_BAD_INDEX);
uint _VDB_CachedNodeIndex = VDB_BAD_INDEX;
ivec3 _VDB_CachedLeafPos = ivec3(VDB_BAD_INDEX);
uint _VDB_CachedLeafIndex = VDB_BAD_INDEX;
uint _VDB_UsableCacheDepth = VDB_BAD_INDEX;

VDB_Accessor VDB_GetAccessor(in vec3 pos) {
    VDB_Accessor acc;

    acc.root = acc.node = acc.leaf = VDB_BAD_INDEX;

    const ivec3 vdb_pos = VDB_WorldToVDB(pos);

    acc.rootPos =  VDB_ReduceDim(vdb_pos, 12, 0);
    acc.nodePos =  VDB_ReduceDim(vdb_pos, 7, 12);
    acc.leafPos =  VDB_ReduceDim(vdb_pos, 3, 7);
    acc.voxelPos = VDB_ReduceDim(vdb_pos, 0, 3);

    if (acc.rootPos != _VDB_CachedRootPos) { return acc; }
    acc.root = _VDB_CachedRootIndex;

    if (acc.nodePos != _VDB_CachedNodePos) { return acc; }
    acc.node = _VDB_CachedNodeIndex;

    if (acc.leafPos != _VDB_CachedLeafPos) { return acc; }
    acc.leaf = _VDB_CachedLeafIndex;

    return acc;
}

bool VDB_IsRootHit(in uint index, in VDB_Accessor acc) {
    return s_VdbRoots.root[index].lowDimBB / 4096 == acc.rootPos;
}

bool VDB_FindRoot(inout VDB_Accessor acc) {
    for (uint i = 0; i < s_VdbDesc.rootCount; i++) {
        if (VDB_IsRootHit(i, acc)) {
            acc.root = i;
            _VDB_CachedRootIndex = i;
            _VDB_CachedRootPos = acc.rootPos;
            return true;
        }
    }
    return false;
}

bool VDB_FindNode(inout VDB_Accessor acc) {
    if (acc.root == VDB_BAD_INDEX && VDB_FindRoot(acc)) { return false; }

    _VDB_CachedNodePos = acc.nodePos;
    uint index, bit, mainIndex = VDB_PosToIndex(acc.nodePos, 5);
    VDB_Bitset32Access(mainIndex, index, bit);

    if (!bool(s_VdbRoots.root[acc.root].bitsets[index] & (1 << bit))) { return false; }

    acc.node = s_VdbRoots.root[acc.root].indices[mainIndex];
    return true;
}

bool VDB_FindLeaf(inout VDB_Accessor acc) {
    if (acc.node == VDB_BAD_INDEX && !VDB_FindNode(acc)) { return false; }

    _VDB_CachedLeafPos = acc.leafPos;
    uint index, bit, mainIndex = VDB_PosToIndex(acc.leafPos, 4);
    VDB_Bitset32Access(mainIndex, index, bit);

    if (!bool(s_VdbNodes.node[acc.node].bitsets[index] & (1 << bit))) { return false; }

    acc.leaf = s_VdbNodes.node[acc.node].indices[mainIndex];
    return true;
}

bool VDB_GetVoxel(inout VDB_Accessor acc) {
    if (acc.leaf == VDB_BAD_INDEX && !VDB_FindLeaf(acc)) { return false; }

    uint index, bit, mainIndex = VDB_PosToIndex(acc.voxelPos, 3);
    VDB_Bitset32Access(mainIndex, index, bit);

    if (!bool(s_VdbLeaves.leaf[acc.leaf].bitsets[index] & (1 << bit))) { return false; }

    acc.voxelValue = s_VdbLeaves.leaf[acc.leaf].values[mainIndex];
    return true;
}

float _VDB_BackgroundValue;
float _VDB_VoxelValueMultiplier;

void _INIT_VDB_GetValue() {
    _VDB_BackgroundValue = u_SceneData.primaryRayLength * u_SceneData.backgroundDensity;
    _VDB_VoxelValueMultiplier = u_SceneData.primaryRayLength * u_SceneData.vdbDensityMultipier;
}

float VDB_GetValue(in vec3 pos) {
    VDB_Accessor acc = VDB_GetAccessor(pos);

    if (!VDB_GetVoxel(acc)) { return _VDB_BackgroundValue; }

    return acc.voxelValue * _VDB_VoxelValueMultiplier;
}

// *************************************************** end VDB accessor

// *************************************************** begin ray marching primary ray

void GetStartingRay(out vec3 ro, out vec3 rd) {
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

void RayAdvance(inout vec3 ro, in vec3 rd, in float dist) { ro += rd * dist; }

bool InsideAABB(in vec3 pos) {
    const vec3 C_P = u_SceneData.aabbPosition;
    const vec3 C_S = u_SceneData.aabbSize;
    vec3 diff = abs(C_P - pos) * AABB_TOL;
    int cond = int(diff.x < C_S.x) + int(diff.y < C_S.y) + int(diff.z < C_S.z);
    return cond == 3;
}

vec4 RayMarching(in vec3 ro, in vec3 rd) {
    const float distAABB = GetDistAABB(ro, rd);// calculate hit distance for AABB domain

    if (distAABB > 0.0f) { // outside -> jump to AABB boundary
        RayAdvance(ro, rd, distAABB);
    } else if (distAABB < 0.0f) { // no hit -> bailing out
        return vec4(0, 0, 0, 0);
    } // if inside -> don't do anything

    vec4 accumulatedColor = vec4(vec3(0), 1);

    // first ray has random length (monte carlo sampling)
    {
        const float rayLength = SampleBlueNoise(ivec2(gl_FragCoord)).x * u_SceneData.primaryRayLength;
        float value = VDB_GetValue(ro);

        accumulatedColor += vec4(vec3(value) * SampleBlueNoise(ivec2(gl_FragCoord)).x, 0);
        RayAdvance(ro, rd, rayLength);
    }

    // every other ray has constant length
    while (InsideAABB(ro)) {
        float value = VDB_GetValue(ro);

        accumulatedColor += vec4(vec3(value), 0);
        RayAdvance(ro, rd, u_SceneData.primaryRayLength);
    }

    return accumulatedColor;
}

// *************************************************** end ray marching primary ray

// Initialize a few "almost constant" variables.
// Compiler has no knowledge needed to evaluate
// them at compile time, but we have it.
void _INIT_() {
    _INIT_VDB_GetValue();
    _INIT_VDB_WorldToVDB();
}

void main() {
    _INIT_();

    vec3 ro, rd;
    GetStartingRay(ro, rd);

    vec4 color = RayMarching(ro, rd);

    color.rgb = LinearToHDR(color.rgb, 0.03);
    color += NoiseShaping();

    FragColor = color;
}
