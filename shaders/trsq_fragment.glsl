#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragPos;

layout(location = 0) out vec4 outColor;

void main()
{
    bool outside_square = false;
    outside_square = outside_square || (fragPos.x + 1) / 2 > 0.5;
    outside_square = outside_square || (fragPos.y + 1) / 2 > 0.5;
    if(outside_square) {
        discard;
        return;
    }
    outColor = vec4(fragColor, 0.5);
}

