#version 450 core

uniform sampler2D absDiffInput_1;
uniform sampler2D absDiffInput_2;

layout(location = 0) out vec4 absDiffColor;

in vec2 ScreenCoord;

void main() {
    vec4 input_1 = texture(absDiffInput_1, (ScreenCoord + 1) / 2);
    vec4 input_2 = texture(absDiffInput_2, (ScreenCoord + 1) / 2);

    vec4 diff = abs(input_1 - input_2);

    float sum = diff.r + diff.g + diff.b + diff.a;

    absDiffColor = vec4(vec3(sum), 1);
}
