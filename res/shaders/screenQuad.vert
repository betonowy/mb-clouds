#version 450 core

const vec2 TRIANGLE[3] = { { -3, -2 }, { 0, 3 }, { 3, -2 } };

out vec2 ScreenCoord;

void main() {
    gl_Position = vec4(TRIANGLE[gl_VertexID], 0.0, 1.0);
    ScreenCoord = gl_Position.xy;
}
