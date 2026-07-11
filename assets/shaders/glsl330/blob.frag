#version 330

in vec3 fragWorldPos;
in vec2 fragTexCoord;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform sampler2D sdfMap;
uniform vec2 sdfOrigin;
uniform float sdfWorldSize;
uniform float sdfMaxDist;
uniform float waterCutoff;

out vec4 finalColor;

void main()
{
    float coast = texture(sdfMap, (fragWorldPos.xz - sdfOrigin) / sdfWorldSize).r * sdfMaxDist;
    if (coast > waterCutoff) discard;
    float a = texture(texture0, fragTexCoord).a;
    finalColor = vec4(colDiffuse.rgb, a * colDiffuse.a);
}
