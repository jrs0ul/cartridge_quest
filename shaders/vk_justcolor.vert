#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 2) in vec4 inColor;

layout(location = 0) out vec4 vColor;

layout(push_constant) uniform PushConstants {
    mat4 ModelViewProjection;
} pc;

void main(void)
{
   gl_Position = pc.ModelViewProjection * vec4(inPosition, 0, 1);
   vColor = inColor;
}
