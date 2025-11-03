attribute vec2 aPosition;
attribute vec2 aUvs;
attribute vec4 aColor;

varying vec2 vUvs;
varying vec4 vColor;
varying float vTime;
uniform mat4 ModelViewProjection;
uniform float uTime;

void main(void)
{
   gl_Position = ModelViewProjection * vec4(aPosition, 0 , 1);
   vUvs = aUvs;
   vColor = aColor;
   vTime = uTime;
}
