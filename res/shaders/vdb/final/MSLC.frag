#version 450 core

uniform sampler2D blueNoiseSampler;
uniform sampler2D adaptiveNoiseSampler;

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
    float _padding_5;
    float _padding_6;

    vec4 gaussian[32];

    int gaussianRadius;
    int ms_skip;
    float ms_ratio;
    float ms_cloudRadius;

    vec4 ms_samplePoints[256];

    float ms_integral_mult;
    float ms_intensity_mult;
    float ms_distance_pow;
    float ms_lorenz_mie_mult;

    float ss_integral_mult;
    float ss_intensity_mult;
    float ss_unused_1;
    float ss_lorenz_mie_mult;

    vec4 ms_cloud_ambient_col;

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

//layout(std430, binding = 0) restrict readonly buffer VdbDesc {
//    ivec3 lowDimBB;
//    uint rootCount;
//
//    ivec3 highDimBB;
//    uint nodeCount;
//
//    uint leafCount;
//} s_VdbDesc;
//
//struct VdbRoot {
//    ivec3 lowDimBB;
//    uint index;
//
//    ivec3 highDimBB;
//    uint _padding_1;
//
//    uint bitsets[1024];
//    uint indices[32768];
//};
//
//layout(std430, binding = 1) restrict readonly buffer VdbRoots {
//    VdbRoot root[];
//} s_VdbRoots;
//
//struct VdbNode {
//    ivec3 lowDimBB;
//    uint index;
//
//    ivec3 highDimBB;
//    uint _padding_1;
//
//    uint bitsets[128];
//    uint indices[4096];
//};
//
//layout(std430, binding = 2) restrict readonly buffer VdbNodes {
//    VdbNode node[];
//} s_VdbNodes;
//
//struct VdbLeaf {
//    ivec3 lowDimBB;
//    uint index;
//
//    ivec3 highDimBB;
//    uint _padding_1;
//
//    uint bitsets[16];
//    float values[512];
//};
//
//layout(std430, binding = 3) restrict readonly buffer VdbLeaves {
//    VdbLeaf leaf[];
//} s_VdbLeaves;

struct VdbValue {
    float density;
    float integral;
};

layout(std430, binding = 0) restrict readonly buffer CachedDesc {
    ivec3 lowDimBB;
    uint rootCount;

    ivec3 highDimBB;
    uint nodeCount;

    uint leafCount;
} s_CachedDesc;

struct CachedRoot {
    ivec3 lowDimBB;
    uint index;

    ivec3 highDimBB;
    uint _padding_1;

    uint bitsets[1024];
    uint indices[32768];
};

layout(std430, binding = 1) restrict readonly buffer CachedRoots {
    CachedRoot root[];
} s_CachedRoots;

struct CachedNode {
    ivec3 lowDimBB;
    uint index;

    ivec3 highDimBB;
    uint _padding_1;

    uint bitsets[128];
    uint indices[4096];
};

layout(std430, binding = 2) restrict readonly buffer CachedNodes {
    CachedNode node[];
} s_CachedNodes;

struct CachedLeaf {
    ivec3 lowDimBB;
    uint index;

    ivec3 highDimBB;
    uint _padding_1;

    uint bitsets[16];
    VdbValue values[512];

    uint _padding_2[1024];
};

layout(std430, binding = 3) restrict readonly buffer CachedLeaves {
    CachedLeaf leaf[];
} s_CachedLeaves;

layout(location = 0) out vec4 fbCloudColor;

in vec2 ScreenCoord;

// *************************************************** begin utilities

float MapValue(float as, float bs, float ad, float bd, float value) {
    float ratio = (value - as) / (bs - as);
    value = ratio * (bd - ad) + ad;
    return value;
}

// *************************************************** end utilities

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

float SampleAdaptiveLayer(in ivec2 pixIndex) {
    return texelFetch(adaptiveNoiseSampler, pixIndex, 0).r;
}

vec4 SampleBlueNoiseLinear(in vec2 pos) {
    vec2 randomOffset = vec2(getRandomData(0), getRandomData(1)) * 1000;
    return texture(blueNoiseSampler, pos + randomOffset);
}

// *************************************************** end background generator

// *************************************************** begin VDB accessor

struct VDB_Accessor {
    ivec3 rootPos;
    uint root;

    ivec3 nodePos;
    uint node;

    ivec3 leafPos;
    uint leaf;

    ivec3 voxelPos;
    VdbValue voxelValue;
};

struct VDB_AccessorLinear {
    ivec3 rootPos[8];
    uint root[8];

    ivec3 nodePos[8];
    uint node[8];

    ivec3 leafPos[8];
    uint leaf[8];

    ivec3 voxelPos[8];
    VdbValue voxelValue[8];

    vec3 ratio;
    vec3 invRatio;
};

VDB_Accessor VDB_AccessorFromLinear(in VDB_AccessorLinear accL, in int index) {
    VDB_Accessor acc;

    acc.rootPos = accL.rootPos[index];
    acc.root = accL.root[index];
    acc.nodePos = accL.nodePos[index];
    acc.node = accL.node[index];
    acc.leafPos = accL.leafPos[index];
    acc.leaf = accL.leaf[index];
    acc.voxelPos = accL.voxelPos[index];
    acc.voxelValue = accL.voxelValue[index];

    return acc;
}

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
    const ivec3 VDB_Size = s_CachedDesc.highDimBB - s_CachedDesc.lowDimBB;
    _VDB_DescCenter = (s_CachedDesc.highDimBB + s_CachedDesc.lowDimBB) / 2;

    const vec3 AABB_Size = u_SceneData.aabbSize;
    _VDB_AABB_Center = u_SceneData.aabbPosition;

    const vec3 sizeRatio = vec3(VDB_Size) / (AABB_Size * u_SceneData.vdbScale);
    _VDB_SizeRatioMax = max(max(sizeRatio.x, sizeRatio.y), sizeRatio.z);
}

ivec3 VDB_WorldToVDB(in vec3 pos) {
    return ivec3((pos - _VDB_AABB_Center) * _VDB_SizeRatioMax) + _VDB_DescCenter;
}

vec3 VDB_GetInterpolationRatio(in vec3 pos) {
    vec3 vPos = (pos - _VDB_AABB_Center) * _VDB_SizeRatioMax;
    return vPos - floor(vPos);
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

VDB_AccessorLinear VDB_GetAccessorLinear(in vec3 pos) {
    VDB_AccessorLinear acc;

    for (int i = 0; i < 8; i++) {
        acc.root[i] = acc.node[i] = acc.leaf[i] = VDB_BAD_INDEX;
    }

    const ivec3 vdb_basePos = VDB_WorldToVDB(pos);

    acc.ratio = VDB_GetInterpolationRatio(pos);
    acc.invRatio = 1 - acc.ratio;

//    acc.ratio = vec3(0);
//    acc.invRatio = vec3(1);

    const ivec3 vdb_pos[8] = { vdb_basePos, vdb_basePos + ivec3(1, 0, 0), vdb_basePos + ivec3(0, 1, 0), vdb_basePos + ivec3(1, 1, 0), vdb_basePos + ivec3(0, 0, 1), vdb_basePos + ivec3(1, 0, 1), vdb_basePos + ivec3(0, 1, 1), vdb_basePos + ivec3(1, 1, 1) };
//    const ivec3 vdb_pos[8] = { vdb_basePos, vdb_basePos + ivec3(0, 0, 0), vdb_basePos + ivec3(0, 0, 0), vdb_basePos + ivec3(0, 0, 0), vdb_basePos + ivec3(0, 0, 0), vdb_basePos + ivec3(0, 0, 0), vdb_basePos + ivec3(0, 0, 0), vdb_basePos + ivec3(0, 0, 0) };

    for (int i = 0; i < 8; i++) {
        acc.rootPos[i] =  VDB_ReduceDim(vdb_pos[i], 12, 0);
        acc.nodePos[i] =  VDB_ReduceDim(vdb_pos[i], 7, 12);
        acc.leafPos[i] =  VDB_ReduceDim(vdb_pos[i], 3, 7);
        acc.voxelPos[i] = VDB_ReduceDim(vdb_pos[i], 0, 3);

        if (acc.rootPos[i] != _VDB_CachedRootPos) { continue; }
        acc.root[i] = _VDB_CachedRootIndex;

        if (acc.nodePos[i] != _VDB_CachedNodePos) { continue; }
        acc.node[i] = _VDB_CachedNodeIndex;

        if (acc.leafPos[i] != _VDB_CachedLeafPos) { continue; }
        acc.leaf[i] = _VDB_CachedLeafIndex;
    }

    return acc;
}

bool VDB_IsRootHit(in uint index, in VDB_Accessor acc) {
    return s_CachedRoots.root[index].lowDimBB / 4096 == acc.rootPos;
}

bool VDB_FindRoot(inout VDB_Accessor acc) {
    for (uint i = 0; i < s_CachedDesc.rootCount; i++) {
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

    if (!bool(s_CachedRoots.root[acc.root].bitsets[index] & (1 << bit))) { return false; }

    acc.node = s_CachedRoots.root[acc.root].indices[mainIndex];
    return true;
}

bool VDB_FindLeaf(inout VDB_Accessor acc) {
    if (acc.node == VDB_BAD_INDEX && !VDB_FindNode(acc)) { return false; }

    _VDB_CachedLeafPos = acc.leafPos;
    uint index, bit, mainIndex = VDB_PosToIndex(acc.leafPos, 4);
    VDB_Bitset32Access(mainIndex, index, bit);

    if (!bool(s_CachedNodes.node[acc.node].bitsets[index] & (1 << bit))) { return false; }

    acc.leaf = s_CachedNodes.node[acc.node].indices[mainIndex];
    return true;
}

bool VDB_GetVoxel(inout VDB_Accessor acc) {
    if (acc.leaf == VDB_BAD_INDEX && !VDB_FindLeaf(acc)) { return false; }

    uint index, bit, mainIndex = VDB_PosToIndex(acc.voxelPos, 3);
    VDB_Bitset32Access(mainIndex, index, bit);

    if (!bool(s_CachedLeaves.leaf[acc.leaf].bitsets[index] & (1 << bit))) { return false; }

    acc.voxelValue = s_CachedLeaves.leaf[acc.leaf].values[mainIndex];
    return true;
}

float _VDB_BackgroundValue;
float _VDB_VoxelValueMultiplier;

void _INIT_VDB_GetValue() {
    _VDB_BackgroundValue = u_SceneData.backgroundDensity;
    _VDB_VoxelValueMultiplier = u_SceneData.vdbDensityMultipier;
}

//float VDB_GetValue(in vec3 pos) {
//    // randomize position per voxel
//    VDB_Accessor acc = VDB_GetAccessor(pos);
//
//    if (!VDB_GetVoxel(acc)) { return _VDB_BackgroundValue; }
//
//    return acc.voxelValue * _VDB_VoxelValueMultiplier;
//}

VdbValue VDB_GetVoxelAcc(in vec3 pos) {
    VDB_Accessor acc = VDB_GetAccessor(pos);

    VdbValue bkg;
    bkg.density = 0;
    bkg.integral = 0;

    if (!VDB_GetVoxel(acc)) { return bkg; }

    acc.voxelValue.density *= _VDB_VoxelValueMultiplier;
    acc.voxelValue.integral *= _VDB_VoxelValueMultiplier;

    return acc.voxelValue;
}

VdbValue VDB_GetVoxelAccLinear(in vec3 pos) {
    VDB_AccessorLinear accL = VDB_GetAccessorLinear(pos);

    VdbValue bkg;
    bkg.density = 0;
    bkg.integral = 10;

    for (int i = 0; i < 8; i++) {
        VDB_Accessor acc = VDB_AccessorFromLinear(accL, i);

        accL.voxelValue[i] = bkg;
        if (!VDB_GetVoxel(acc)) { continue; }

        acc.voxelValue.density *= _VDB_VoxelValueMultiplier;
        acc.voxelValue.integral *= _VDB_VoxelValueMultiplier;

        accL.voxelValue[i] = acc.voxelValue;
    }

    // 3D linear interpolation

    // first dimension

    VdbValue planes[4];

    // you can do it simpler
//    for (int y = 0; y <= 1; y++) {
//        for (int z = 0; z <= 1; z++) {
//            planes[y + 2 * z].density = accL.invRatio.x * accL.voxelValue[2 * y + 4 * z].density + accL.ratio.x * accL.voxelValue[1 + 2 * y + 4 * z].density;
//            planes[0].integral = accL.invRatio.x * accL.voxelValue[2 * y + 4 * z].integral + accL.ratio.x * accL.voxelValue[1 + 2 * y + 4 * z].integral;
//        }
//    }

    planes[0].density = accL.invRatio.x * accL.voxelValue[1 * 0 + 2 * 0 + 4 * 0].density + accL.ratio.x * accL.voxelValue[1 * 1 + 2 * 0 + 4 * 0].density;
    planes[0].integral = accL.invRatio.x * accL.voxelValue[1 * 0 + 2 * 0 + 4 * 0].integral + accL.ratio.x * accL.voxelValue[1 * 1 + 2 * 0 + 4 * 0].integral;

    planes[1].density = accL.invRatio.x * accL.voxelValue[1 * 0 + 2 * 1 + 4 * 0].density + accL.ratio.x * accL.voxelValue[1 * 1 + 2 * 1 + 4 * 0].density;
    planes[1].integral = accL.invRatio.x * accL.voxelValue[1 * 0 + 2 * 1 + 4 * 0].integral + accL.ratio.x * accL.voxelValue[1 * 1 + 2 * 1 + 4 * 0].integral;

    planes[2].density = accL.invRatio.x * accL.voxelValue[1 * 0 + 2 * 0 + 4 * 1].density + accL.ratio.x * accL.voxelValue[1 * 1 + 2 * 0 + 4 * 1].density;
    planes[2].integral = accL.invRatio.x * accL.voxelValue[1 * 0 + 2 * 0 + 4 * 1].integral + accL.ratio.x * accL.voxelValue[1 * 1 + 2 * 0 + 4 * 1].integral;

    planes[3].density = accL.invRatio.x * accL.voxelValue[1 * 0 + 2 * 1 + 4 * 1].density + accL.ratio.x * accL.voxelValue[1 * 1 + 2 * 1 + 4 * 1].density;
    planes[3].integral = accL.invRatio.x * accL.voxelValue[1 * 0 + 2 * 1 + 4 * 1].integral + accL.ratio.x * accL.voxelValue[1 * 1 + 2 * 1 + 4 * 1].integral;

    // second dimension

    VdbValue lines[2];

    lines[0].density = accL.invRatio.y * planes[1 * 0 + 2 * 0].density + accL.ratio.y * planes[1 * 1 + 2 * 0].density;
    lines[0].integral = accL.invRatio.y * planes[1 * 0 + 2 * 0].integral + accL.ratio.y * planes[1 * 1 + 2 * 0].integral;

    lines[1].density = accL.invRatio.y * planes[1 * 0 + 2 * 1].density + accL.ratio.y * planes[1 * 1 + 2 * 1].density;
    lines[1].integral = accL.invRatio.y * planes[1 * 0 + 2 * 1].integral + accL.ratio.y * planes[1 * 1 + 2 * 1].integral;

    // third dimension

    VdbValue point;

    point.density = accL.invRatio.z * lines[0].density + accL.ratio.z * lines[1].density;
    point.integral = accL.invRatio.z * lines[0].integral + accL.ratio.z * lines[1].integral;

    return point;
}

VdbValue VDB_GetVoxelAccNoisy(in vec3 pos) {
    vec3 offset = (SampleBlueNoise(ivec2(gl_FragCoord)).yzw - 0.5) / _VDB_SizeRatioMax;
    VDB_Accessor acc = VDB_GetAccessor(pos + offset);

    VdbValue bkg;
    bkg.density = 0;
    bkg.integral = 0;

    if (!VDB_GetVoxel(acc)) { return bkg; }

    acc.voxelValue.density *= _VDB_VoxelValueMultiplier;
    acc.voxelValue.integral *= _VDB_VoxelValueMultiplier;

    acc.voxelValue.density = max(0, acc.voxelValue.density);
    acc.voxelValue.integral = max(0, acc.voxelValue.integral);

    return acc.voxelValue;
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

vec3 BkgFunction(in float integral, in float dotVal, in vec3 col) {
    return vec3(0);
}

vec3 SunFunction(in float integral, in float dotVal, in vec3 col) {
    return vec3(0);
}

vec3 SecondaryRayEnergyFunction(in float integral, in float dotVal, in vec3 sunCol, in vec3 bkgCol) {
    return vec3(0.1);
}

//float BeersLawPre;
//
//void _INIT_SecondaryRay() {
//
//}

float LorenzMie(in float dotVal, in float p) {
    return (1 * p) / 2 * pow((1 + dotVal) / 2, p) / (4 * 3.141593);
}

float FittedLorenzMie(float dotVal) {
    float lm[10] = {
    9.805233e-06,
    -6.500000e+01,
    -5.500000e+01,
    8.194068e-01,
    1.388198e-01,
    -8.370334e+01,
    7.810083e+00,
    2.054747e-03,
    2.600563e-02,
    -4.552125e-12,
    };

    float p1 = dotVal + lm[3];
    vec4 expValues = exp(vec4(lm[1] *dotVal+lm[2], lm[5] *p1*p1, lm[6] *dotVal, lm[9] *dotVal));
    vec4 expValWeight = vec4(lm[0], lm[4], lm[7], lm[8]);
    return dot(expValues, expValWeight);
}

float SecondaryRay(in vec3 ro, in float localDensity, in float phaseFunc, in float dotVal, float cloudHeight) {
    const vec3 rd = u_SceneData.sunDir;
    const float stepL = 1;
    float integral = 0;
    float firstIntegral = 0;

    { // cached integral
        firstIntegral = VDB_GetVoxelAccNoisy(ro).integral;
        if (firstIntegral <= 0) firstIntegral = 0.001;
    }

    integral += firstIntegral;
    integral *= u_SceneData.densityMultiplier * u_SceneData.ss_integral_mult;

    float depthParam = 10 / (integral * 1);

    float beersLaw = exp(-stepL * integral);

    float light = beersLaw;
    float backLightFix = 0;
    return LorenzMie(dotVal, depthParam) * 1 + light + backLightFix;
    //    return light;
}

float MultiscatterRay(in vec3 ro, in float localDensity, in float distance, in float phaseFunc, in float dotVal, float cloudHeight) {
    const vec3 rd = u_SceneData.sunDir;
    const float stepL = 1;
    float integral = 0;
    float firstIntegral = 0;

    const VdbValue value = VDB_GetVoxelAccNoisy(ro);

    { // cached integral
        firstIntegral = value.integral;
        if (firstIntegral < 0.001) firstIntegral = 0.001;
    }

    integral += firstIntegral;
    integral += pow(distance, 2.0) * (localDensity + value.density) * 0.5;
    integral *= u_SceneData.densityMultiplier * u_SceneData.ms_integral_mult;

    float depthParam = 10 / (integral * 1);

    float scatter = mix(0.008, 1.0, smoothstep(0.96, 0.0, 0.4));
    float beersLaw = exp(-stepL * integral) + 0.5 * scatter * exp(-0.1 * stepL * integral) + scatter * 0.4 * exp(-0.02 * stepL * integral);

    float light = beersLaw * phaseFunc * mix(0.05 + 1.5 * pow(min(1.0, localDensity * 8.5), 0.3 + 5.5 * cloudHeight), 1.0, clamp(integral * 0.4, 0.0, 1.0));
    return LorenzMie(dotVal, depthParam) * u_SceneData.ms_lorenz_mie_mult + light;
}

// *************************************************** begin multiscattering

vec4 _ms_samplePoints[256];

void _INIT_MultiScaterring() {
    for (int i = 0; i < 256; i++) {
        _ms_samplePoints[i] = u_SceneData.ms_samplePoints[i];
    }
}

float MultiScattering(in vec3 ro, in float localDensity, in float phaseFunc, in float dotVal, float cloudHeight) {
    const int skip = u_SceneData.ms_skip;
    const float radius = u_SceneData.ms_cloudRadius;
    float ss_Sum = 0;

    if (skip == 32){ return ss_Sum; }

    float ms_Sum = 0;
    float pointsCount = 0;

    // multiscattering sample points
    for (int i = 0; i < 256; i += skip) {
        vec3 samplePoint = _ms_samplePoints[i].xyz * radius * _ms_samplePoints[i].w + ro;

        // isotropic
        ms_Sum += MultiscatterRay(samplePoint, localDensity, length(ro - samplePoint), phaseFunc, dotVal, cloudHeight);
        // fitted lorenz mie
//                ms_Sum += MultiscatterRay(samplePoint, localDensity, length(ro - samplePoint), phaseFunc, dotVal, cloudHeight) * FittedLorenzMie(dot(-normalize(ro - samplePoint), u_SceneData.sunDir)) * 3.1415;
        // simpler lorenz mie
//                ms_Sum += MultiscatterRay(samplePoint, localDensity, length(ro - samplePoint), phaseFunc, dotVal, cloudHeight) * LorenzMie(dot(-normalize(ro - samplePoint), u_SceneData.sunDir), 5) * 31.415;
        pointsCount += 1;
    }

    ms_Sum *= 1.0 / pointsCount;

    float ms_ss_ratio = u_SceneData.ms_ratio;

    return ss_Sum * ms_ss_ratio + ms_Sum * (1.0 - ms_ss_ratio);
}

// *************************************************** end multiscaterring

float FakeLM(float x, float density) {
    const float sunFocus = 0.04;
    const float peakBrightness = 2.0;
    const float peakThreshold = 0.00005;

    x = MapValue(-1.0, 1.0, 0.5, 1.002, x);
    x = pow(x, sunFocus / (pow(density, peakBrightness) + peakThreshold));
    x = MapValue(0.0, 1.0, 0.5, 1.5, x);
    return x;
}

float SimpleLorenzMie(float dot, float integral) {
    const float pa = 50;
    const float pb = 1.0 / 33;
    //    const float m = 8;
    const float m = 1 / (integral * pa + pb);
    const float mltp = 2 / (4 * 3.141593);
    return (1 + (m + 1) * pow(1 + dot * 0.5, m)) * mltp + 2.1;
}

float HenyeyGreenstein(float g, float costh) {
    return (1.0 - g * g) / (4.0 * 3.141593 * pow(1.0 + g*g - 2.0*g*costh, 1.5));
}

float Extinction(float integral) {
    const float pa = 100;
    const float pb = 1;
    return 1.0f / (pa * integral + pb);
}

vec4 RayMarching(in vec3 ro, in vec3 rd) {
    const float distAABB = GetDistAABB(ro, rd);// calculate hit distance for AABB domain

    if (distAABB > 0.0f) { // outside -> jump to AABB boundary
        RayAdvance(ro, rd, distAABB);
    } else if (distAABB < 0.0f) { // no hit -> bailing out
        return vec4(0, 0, 0, 0);
    }// if inside -> don't do anything

    vec4 accumulatedColor = vec4(0);

    // get adaptive sample value (noisiness) tiled
    float value = SampleAdaptiveLayer((ivec2(gl_FragCoord) / 16) * 16);
    // get adaptive sample value (noisiness) untiled
    //    float value = SampleAdaptiveLayer(ivec2(gl_FragCoord));

    float stepMultiplier = 1;

    //    if (value > 0.8) {
    //        stepMultiplier *= 0.2;
    //    }
    //
    //    if (value > 0.9) {
    //        stepMultiplier *= 0.2;
    //    }
    //
    //    if (value > 0.95) {
    //        stepMultiplier *= 0.2;
    //    }

    // first ray has random length (monte carlo sampling)
    {
        const float rayLength = SampleBlueNoise(ivec2(gl_FragCoord)).x * u_SceneData.primaryRayLength;
        float value = VDB_GetVoxelAccNoisy(ro).density;
        RayAdvance(ro, rd, rayLength);
    }

    // precalculate values

    const float rayDot = dot(u_SceneData.sunDir, rd);
    //    const float phaseFunc = sqrt(FittedLorenzMie(rayDot));
    //    const float phaseFunc = 1.0;
    const float phaseFunc = HenyeyGreenstein(0.0, rayDot);
    float step = u_SceneData.primaryRayLength;

    float accAlpha = 0.0;
    float prevValue = 0.0;
    float T = 1.0;

    const float alphaThreshold = 0.05;
    float baseIntegral = 0.0;
    const float albedo = 0.01;

    vec3 color = vec3(0);

    float cloudHeight = u_SceneData.cloudHeight;

    float stepLength = u_SceneData.primaryRayLength;
    const float ar = 1.00;

    while (InsideAABB(ro)) {
        float localDensity = VDB_GetVoxelAccNoisy(ro).density;

        float cloudHeight = (u_SceneData.cloudHeight + ro.z) * u_SceneData.cloudHeightSensitivity;
        cloudHeight = min(max(0.0, cloudHeight + 0.5), 1.0);

        if (localDensity > 0.0f) {
            //            float intensity = SecondaryRay(ro, localDensity, phaseFunc, rayDot, cloudHeight);
            float intensityMS = MultiScattering(ro, localDensity, phaseFunc, rayDot, cloudHeight);
            float intensitySS = SecondaryRay(ro, localDensity, phaseFunc, rayDot, cloudHeight);
            intensitySS *= u_SceneData.radianceMultiplier;
            intensityMS *= u_SceneData.radianceMultiplier;

//            vec3 ambient = (0.5 + 0.6 * cloudHeight) * vec3(0.3, 0.3, 0.8) * 6.5 + vec3(0.8) * max(0.0, 1.0 - 2.0 * cloudHeight);
            vec3 colCHigh = u_SceneData.sunColor;
            vec3 colCLow = vec3(0.35, 0.3, 1.3);

            vec3 colorMS = (1.0 - intensityMS) * colCLow + (intensityMS) * colCHigh;
            vec3 colorSS = u_SceneData.sunColor;

            vec3 radiance = colorMS * intensityMS * u_SceneData.ms_intensity_mult + intensitySS * u_SceneData.sunColor * u_SceneData.ss_intensity_mult;
            color += T * (radiance - radiance * exp(-localDensity * step));

            T *= exp(-localDensity * step);

            if (T < alphaThreshold) {
                return vec4(color, 1);
            }
        }

        RayAdvance(ro, rd, step);
    }

    T = MapValue(alphaThreshold, 1.0, 0.0, 1.0, T);

    return vec4(color, 1 - clamp(1.0, 0.0, T));
}

// *************************************************** end ray marching primary ray

// Initialize a few "almost constant" variables.
// Compiler has no knowledge needed to evaluate
// them at compile time, but we have it.
void _INIT_() {
    _INIT_VDB_GetValue();
    _INIT_VDB_WorldToVDB();
    _INIT_MultiScaterring();
}

void main() {
    _INIT_();

    vec3 ro, rd;
    GetStartingRay(ro, rd);

    fbCloudColor = RayMarching(ro, rd);
}
