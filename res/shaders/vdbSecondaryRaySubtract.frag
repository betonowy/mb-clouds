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

layout(std430, binding = 0) restrict readonly buffer VdbDesc {
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

layout(std430, binding = 1) restrict readonly buffer VdbRoots {
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

layout(std430, binding = 2) restrict readonly buffer VdbNodes {
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

layout(std430, binding = 3) restrict readonly buffer VdbLeaves {
    VdbLeaf leaf[];
} s_VdbLeaves;

out vec4 FragColor;

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

vec4 SampleBlueNoiseLinear(in vec2 pos) {
    vec2 randomOffset = vec2(getRandomData(0), getRandomData(1)) * 1000;
    return texture(blueNoiseSampler, pos + randomOffset);
}

vec4 NoiseShaping() {
    const float noiseValue = 1.0f / 256;
    const float offset = 0.5f * noiseValue;
    return vec4(SampleBlueNoise(ivec2(gl_FragCoord.xy)).rgb * noiseValue, 0) - offset;
}

// *************************************************** end post processing

// *************************************************** begin background generator

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
    _VDB_BackgroundValue = u_SceneData.backgroundDensity;
    _VDB_VoxelValueMultiplier = u_SceneData.vdbDensityMultipier;
}

float VDB_GetValue(in vec3 pos) {
    // randomize position per voxel
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

vec3 BkgFunction(in float integral, in float dotVal, in vec3 col) {
    return vec3(0);
}

vec3 SunFunction(in float integral, in float dotVal, in vec3 col) {
    return vec3(0);
}

vec3 SecondaryRayEnergyFunction(in float integral, in float dotVal, in vec3 sunCol, in vec3 bkgCol) {
    return vec3(0.1);
}

void SecondaryRay(in vec3 ro, in vec3 rd, out float integral, out float dotVal) {
    dotVal = dot(u_SceneData.sunDir, rd);

    rd = u_SceneData.sunDir;
    integral = VDB_GetValue(ro) * u_SceneData.secondaryRayLength * 0.0;

    while (InsideAABB(ro)) {
        RayAdvance(ro, rd, u_SceneData.secondaryRayLength);
        integral += VDB_GetValue(ro);
    }

    integral *= u_SceneData.secondaryRayLength;
}

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

float Extinction(float integral) {
    const float pa = 100;
    const float pb = 1;
    return 1.0f / (pa * integral + pb);
}

vec4 RayMarching(in vec3 ro, in vec3 rd, in vec3 bkgCol) {
    //    const vec4 randomVector = (SampleBlueNoise(ivec2(gl_FragCoord) + 10) - 0.5) * 0.2;
    //    RayAdvance(ro, randomVector.xyz, randomVector.w);

    const float distAABB = GetDistAABB(ro, rd);// calculate hit distance for AABB domain

    if (distAABB > 0.0f) { // outside -> jump to AABB boundary
        RayAdvance(ro, rd, distAABB);
    } else if (distAABB < 0.0f) { // no hit -> bailing out
        return vec4(0, 0, 0, 0);
    }// if inside -> don't do anything

    vec4 accumulatedColor = vec4(0);

    // first ray has random length (monte carlo sampling)
    {
        const float rayLength = SampleBlueNoise(ivec2(gl_FragCoord)).x * u_SceneData.primaryRayLength;
        float value = VDB_GetValue(ro);
        // TURNED OFF FOR NOW!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //        accumulatedColor += vec4(vec3(value) * SampleBlueNoise(ivec2(gl_FragCoord)).x, 0);
        RayAdvance(ro, rd, rayLength);
    }

    float accAlpha = 0.0;
    float prevValue = 0.0;

    const float alphaThreshold = 0.99;
    float baseIntegral = 0.0;
    const float albedo = 0.01;

    float stepLength = u_SceneData.primaryRayLength;

    // every other ray has constant length
    while (InsideAABB(ro)) {
        float value = VDB_GetValue(ro);

        if (value > 0.0f) {
            float lastAlpha = accAlpha;
            float sampledValue = value * stepLength;
            baseIntegral += sampledValue;
            accAlpha = 1.0 - exp(-baseIntegral * 100);
//            accAlpha = 1.0 - 1 / (baseIntegral * 10000 + 1.0);

            float sampleImportance = (accAlpha - lastAlpha) / accAlpha;

            float integral, dot;
            SecondaryRay(ro, rd, integral, dot);
            integral += baseIntegral;

            //            if (integral < 0.00001) continue;

            //            float phaseFunc = FakeLM(dot, integral/ 100) / ((integral * integral) + 0.001);
            float phaseFunc = SimpleLorenzMie(dot, integral);
            float extinction = Extinction(integral * 1) * 100;

            accumulatedColor.rgb += sampleImportance * phaseFunc * extinction * sampledValue;

            //            float integral, dot;
            //            SecondaryRay(ro, rd, integral, dot);
            //
            //            float addAlpha = value / 6;
            //
            //            float addFactor = (1.0 - accAlpha);
            //            addFactor *= (1.0 - 2 * u_SceneData.primaryRayLength);
            //
            //            prevValue = accAlpha;
            //            accAlpha += addAlpha * addFactor;
            //
            //            accumulatedColor += vec4(vec3(1 / (integral + 0.05)) * u_SceneData.primaryRayLength * 10, 0);
            //

//            stepLength = u_SceneData.primaryRayLength * (1 / (1 - accAlpha + 0.1));

            if (accAlpha > alphaThreshold) {
                break;
            }
        }

        RayAdvance(ro, rd, u_SceneData.primaryRayLength);
    }

    accAlpha /= alphaThreshold;
    accAlpha = min(1.0, accAlpha);

    bkgCol *= pow(Extinction(baseIntegral), 2.2);

    return vec4(accumulatedColor.rgb + bkgCol, 1.0);
//        return vec4(0, 0, 0, accAlpha);
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

    vec4 color = RayMarching(ro, rd, BackgroundColor(rd).rgb);

    color = vec4(vec3(mix(BackgroundColor(rd).rgb, color.rgb, pow(color.a, 2.2))), 1.0);

    color.rgb = LinearToHDR(color.rgb, 1.0);
    color += NoiseShaping();

    FragColor = color;
    FragColor.a = u_SceneData.alphaBlendIn;
}
