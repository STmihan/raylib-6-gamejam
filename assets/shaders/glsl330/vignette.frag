#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform vec3 vColor;
uniform float vIntensity;
uniform float vRadius;
uniform float vSoftness;

out vec4 finalColor;

void main()
{
    vec2 d = fragTexCoord - vec2(0.5);
    float dist = length(d) * 1.41421356;
    float a = smoothstep(vRadius, vRadius + vSoftness, dist) * vIntensity;
    finalColor = vec4(vColor, a);
}
