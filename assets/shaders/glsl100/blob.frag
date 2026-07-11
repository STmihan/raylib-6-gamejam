#version 100

precision mediump float;

varying vec3 fragWorldPos;
varying vec2 fragTexCoord;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform sampler2D sdfMap;
uniform vec2 sdfOrigin;
uniform float sdfWorldSize;
uniform float sdfMaxDist;
uniform float waterCutoff;

void main()
{
    float coast = texture2D(sdfMap, (fragWorldPos.xz - sdfOrigin) / sdfWorldSize).r * sdfMaxDist;
    if (coast > waterCutoff) discard;
    float a = texture2D(texture0, fragTexCoord).a;
    gl_FragColor = vec4(colDiffuse.rgb, a * colDiffuse.a);
}
