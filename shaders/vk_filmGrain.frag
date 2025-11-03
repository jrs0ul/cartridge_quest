#version 450

layout(location = 0) in vec2 vUvs;
layout(location = 1) in vec4 vColor;


layout(binding = 0) uniform sampler2D uTexture;

layout(location = 0) out vec4 outColor;

void main(void)
{
    vec4 original = texture(uTexture, vUvs) * vColor;
    float noise = (fract(sin(dot(vUvs /** time*/, vec2(12.9898, 78.233))) * 43758.5453) - 0.5) * 2.0;
    original.rgb += noise * 0.05 * 2.0;

    outColor = clamp(original, 0.0, 1.0);
}
