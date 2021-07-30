#version 450 core

// scene data uniform buffer objects

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
} u_SceneData;

// vdb shader storage buffer objects

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

// end of vdb shader storage buffer objects

// begin constants

const float c_PrimaryRayStepLength = 0.01;
const float c_SecondaryRayStepLength = 0.01;

const uint c_BadIndex = 0xFFFFFFFF;

// end constants

// input/output data

out vec4 FragColor;

in vec2 ScreenCoord;

// end of input output data

// begin first bounce handle functions

void getStartingRay(out vec3 ro, out vec3 rd) {
    rd = u_SceneData.camDir;
    ro = u_SceneData.camPos;

    vec3 xPosVec = normalize(cross(rd, vec3(0, 0, 1)));

    vec3 yPosVec = cross(rd, -xPosVec) * u_SceneData.camRatio;

    vec2 offset = ScreenCoord;
    rd += xPosVec * offset.x + yPosVec * offset.y;

    rd = normalize(rd);
}

const vec3 C_P = vec3(0, 0, 0);// 0,1,3
const vec3 C_S = vec3(1, 1, 1);// 8,8,4
const float AABB_TOL = 0.99999;

float GetDistAABB(in vec3 pos, in vec3 nor) {
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
    if (dO == 0.0f) return vec3(1.0, 0.0, 0.0);// You're inside
    else if (dO < -0.0f) return vec3(1.0, 0.0, 1.0);// no intersection
    return vec3(0.0, dO, 0.0);
}

// end first bounce handle functions

// begin misc ray functions

void RayAdvance(inout vec3 ro, in vec3 rd, in float dist) {
    ro += rd * dist;
}

bool InsideAABB(in vec3 pos) {
    vec3 diff = abs(C_P - pos) * AABB_TOL;
    int cond = int(diff.x < C_S.x) + int(diff.y < C_S.y) + int(diff.z < C_S.z);
    return cond == 3;
}

// end misc ray functions

// begin VDB accessor

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

void Bitset_uint32_Access(in uint index, out uint dwordIndex, out uint bitIndex) {
    dwordIndex = index >> 5;
    bitIndex = index & 31u;
}

ivec3 VDB_WorldToVDB(vec3 pos) {
    ivec3 VDB_Size = s_VdbDesc.highDimBB - s_VdbDesc.lowDimBB;
    ivec3 VDB_center = (s_VdbDesc.highDimBB + s_VdbDesc.lowDimBB) / 2;

    vec3 AABB_Size = C_S * 2;
    vec3 AABB_Center = C_P;

    vec3 sizeRatio = vec3(VDB_Size) / (AABB_Size * u_SceneData.vdbScale);
    float sizeRatioMax = max(max(sizeRatio.x, sizeRatio.y), sizeRatio.z);

    return ivec3((pos - AABB_Center) * sizeRatioMax);
}

VDB_Accessor VDB_GetAccessor(in vec3 pos) {
    VDB_Accessor acc;

    ivec3 vdb_pos = VDB_WorldToVDB(pos);

    acc.rootPos =  VDB_ReduceDim(vdb_pos, 12, 0);
    acc.nodePos =  VDB_ReduceDim(vdb_pos, 7, 12);
    acc.leafPos =  VDB_ReduceDim(vdb_pos, 3, 7);
    acc.voxelPos = VDB_ReduceDim(vdb_pos, 0, 3);

    return acc;
}

uint _VDB_CachedRootIndex = 0;

bool VDB_IsRootHit(uint index, VDB_Accessor acc) {
    if (s_VdbRoots.root[index].lowDimBB / 4096 == acc.rootPos) {
        return true;
    }
    return false;
}

bool VDB_FindRoot(inout VDB_Accessor acc) {
    if (VDB_IsRootHit(_VDB_CachedRootIndex, acc)) {
        acc.root = _VDB_CachedRootIndex;
        return true;
    }

    for (uint i = 0; i < s_VdbDesc.rootCount; i++) {
        if (VDB_IsRootHit(i, acc)) {
            acc.root = i;
            _VDB_CachedRootIndex = i;
            return true;
        }
    }

    return false;
}

bool VDB_FindNode(inout VDB_Accessor acc) {
    uint bit, index;
    uint mainIndex = VDB_PosToIndex(acc.nodePos, 5);
    Bitset_uint32_Access(mainIndex, index, bit);

    if ((s_VdbRoots.root[acc.root].bitsets[index] & (1 << bit)) != 0) {
        acc.node = s_VdbRoots.root[acc.root].indices[mainIndex];
        return true;
    }

    return false;
}

bool VDB_FindLeaf(inout VDB_Accessor acc) {
    uint bit, index;
    uint mainIndex = VDB_PosToIndex(acc.leafPos, 4);
    Bitset_uint32_Access(mainIndex, index, bit);

    if ((s_VdbNodes.node[acc.node].bitsets[index] & (1 << bit)) != 0) {
        acc.leaf = s_VdbNodes.node[acc.node].indices[mainIndex];
        return true;
    }

    return false;
}

bool VDB_GetVoxel(inout VDB_Accessor acc) {
    uint bit, index;
    uint mainIndex = VDB_PosToIndex(acc.voxelPos, 3);
    Bitset_uint32_Access(VDB_PosToIndex(acc.voxelPos, 3), index, bit);

    if ((s_VdbLeaves.leaf[acc.leaf].bitsets[index] & (1 << bit)) != 0) {
        acc.voxelValue = s_VdbLeaves.leaf[acc.leaf].values[mainIndex];
        return true;
    }

    return false;
}

float VDB_GetValue(in vec3 pos) {
    VDB_Accessor acc = VDB_GetAccessor(pos);

    const float backGroundValue = 0.0001f;

    if (!VDB_FindRoot(acc)) {
        return backGroundValue;
    }

    if (!VDB_FindNode(acc)) {
        return backGroundValue;
    }

    if (!VDB_FindLeaf(acc)) {
        return backGroundValue;
    }

    if (!VDB_GetVoxel(acc)) {
        return backGroundValue;
    }

    return acc.voxelValue;
}

// end VDB accessor

// begin ray marching

vec4 RayMarching(in vec3 ro, in vec3 rd) {
    float distAABB = GetDistAABB(ro, rd);// calculate hit distance for AABB domain

    if (distAABB > 0.0f) { // jump to AABB boundary
        RayAdvance(ro, rd, distAABB);
    } else if (distAABB < 0.0f) { // no hit - bailing out
        return vec4(0, 0, 0, 0);
    }

    vec4 accumulatedColor = vec4(vec3(0), 1);

    while (InsideAABB(ro)) {
        float value = VDB_GetValue(ro);

        accumulatedColor += vec4(vec3(value), 0);

        RayAdvance(ro, rd, c_PrimaryRayStepLength);
    }

    return accumulatedColor;
}

// end ray marching

// begin color filtering

vec3 LinearToHDR(vec3 inColor, float exposure) {
    const float invGamma = 1.0 / 2.2;

    // color bleed
    vec3 tempCol = inColor;
    // inColor.r += tempCol.g * 0.15 + tempCol.b * 0.01;
    // inColor.g += tempCol.r * 0.15 + tempCol.b * 0.1;
    // inColor.b += tempCol.g * 0.1 + tempCol.r * 0.01;

    inColor = vec3(1.0) - exp(-inColor * exposure);
    inColor = pow(inColor, vec3(invGamma));

    return inColor;
}

// end color filtering

void main() {
    vec3 ro, rd;

    getStartingRay(ro, rd);

    vec4 color = RayMarching(ro, rd);

    color.rgb = LinearToHDR(color.rgb, 0.03);

    FragColor = color;
}
