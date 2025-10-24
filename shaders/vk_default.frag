#version 450

layout(location = 0) in vec2 vUvs;
layout(location = 1) in vec4 vColor;

layout(binding = 0) uniform sampler2D uTexture;

layout(location = 0) out vec4 outColor;

void main(void)
{
   outColor = vColor * texture(uTexture, vUvs);
}
