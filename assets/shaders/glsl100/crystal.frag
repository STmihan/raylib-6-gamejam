#version 100

precision mediump float;

varying vec2 fragTexCoord;
varying vec4 fragColor;

uniform sampler2D texture0;
uniform vec3 colorTop;
uniform vec3 colorBottom;
uniform float gloss;
uniform float colorSplit;
uniform float colorEdge;

void main()
{
    float mask = texture2D(texture0, fragTexCoord).a;
    float v = fragTexCoord.y;
    float m = smoothstep(colorSplit - colorEdge, colorSplit + colorEdge, v);
    vec3 col = mix(colorTop, colorBottom, m);
    float g = smoothstep(0.35, 0.0, v) * gloss;
    col = mix(col, vec3(1.0), g);
    gl_FragColor = vec4(col, mask * fragColor.a);
}
