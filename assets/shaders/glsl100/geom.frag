#version 100

precision mediump float;

varying vec3 worldNormal;
varying float viewDepth;

void main()
{
    gl_FragColor = vec4(normalize(worldNormal) * 0.5 + 0.5, clamp(viewDepth, 0.0, 1.0));
}
