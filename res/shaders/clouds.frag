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

// input/output data

out vec4 FragColor;

in vec2 ScreenCoord;

// end of input output data

// begin first bounce handle functions

void getStartingRay(out vec3 ro, out vec3 rd) {
    rd = u_SceneData.camDir;
    ro = u_SceneData.camPos;

    vec3 xPosVec = normalize(cross(rd, vec3(0, 0, 1)));

    vec3 yPosVec = cross(rd, xPosVec) * u_SceneData.camRatio; //* u_SceneData.camRatio;

    vec2 offset = ScreenCoord;
    rd += xPosVec * offset.x + yPosVec * offset.y;

    rd = normalize(rd);
}

const vec3 C_P = vec3(0, 0, 0); // 0,1,3
const vec3 C_S = vec3(1, 1, 1); // 8,8,4
const float AABB_TOL = 0.99999;

float GetDistAABB(vec3 pos, vec3 nor) {
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

vec3 RayTraceDistToCol(vec3 ro, vec3 rd) {
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

// end misc ray functions

// begin ray marching

// end ray marching

void main() {
    vec3 ro, rd;

    ro.y += s_VdbDesc.rootCount;

    getStartingRay(ro, rd);

    ro.y += s_VdbRoots.root[0].index;
    ro.y += s_VdbNodes.node[0].index;
    ro.y += s_VdbLeaves.leaf[0].index;

    vec3 debugColor = RayTraceDistToCol(ro, rd);

    vec2 color = (ScreenCoord + 1) * 0.5;
    FragColor = vec4(debugColor, 1.0);
}
