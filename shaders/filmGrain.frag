#ifdef GL_ES
//
precision mediump float;

#endif


uniform sampler2D uTexture;

varying float vTime;
varying vec2 vUvs;
varying vec4 vColor;

void main(void)
{

    vec4 original = texture2D(uTexture, vUvs) * vColor;

    float noise = (fract(sin(dot(vUvs * vTime, vec2(12.9898, 78.233))) * 43758.5453) - 0.5) * 2.0;

    original.rgb += noise * 0.05 * 2.0;

    gl_FragColor = clamp(original, 0.0, 1.0);
}
