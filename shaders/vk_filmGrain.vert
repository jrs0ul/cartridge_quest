#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inUvs;
layout(location = 2) in vec4 inColor;

layout(location = 0) out vec2 vUvs;
layout(location = 1) out vec4 vColor;
layout(location = 2) out float vTime;

layout(push_constant) uniform PushConstants {
    mat4 ModelViewProjection;
    float time;
} pc;

void main(void)
{
   gl_Position = pc.ModelViewProjection * vec4(inPosition, 0 , 1);
   vUvs      = inUvs;
   vColor    = inColor;
   vTime     = pc.time;
}
