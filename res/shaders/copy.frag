#version 450 core

uniform sampler2D fbColor;

layout(location = 0) out vec4 copyColor;

in vec2 ScreenCoord;

void main() {
    copyColor = texture(fbColor, (ScreenCoord + 1) / 2);
}
