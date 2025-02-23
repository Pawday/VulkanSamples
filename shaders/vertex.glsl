#version 450

vec2 positions[] = vec2[](
    vec2(-0.5, 0.5),
    vec2(0.5, 1),
    vec2(0,  -1)
);

vec3 colors[] = vec3[](
    vec3(1, 0, 0),
    vec3(0, 1, 0),
    vec3(0, 0, 1)
);

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 pos;

void main()
{
    gl_Position = vec4(positions[gl_VertexIndex % 3], 0.0, 1.0);

    fragColor = colors[gl_VertexIndex % 3];
}
