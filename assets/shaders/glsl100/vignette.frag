#version 100

precision mediump float;

varying vec2 fragTexCoord;
varying vec4 fragColor;

uniform vec3 vColor;
uniform float vIntensity;
uniform float vRadius;
uniform float vSoftness;

void main()
{
    vec2 d = fragTexCoord - vec2(0.5);
    float dist = length(d) * 1.41421356;
    float a = smoothstep(vRadius, vRadius + vSoftness, dist) * vIntensity;
    gl_FragColor = vec4(vColor, a);
}
