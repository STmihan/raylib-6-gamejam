#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec3 colorTop;
uniform vec3 colorBottom;
uniform float gloss;
uniform float colorSplit;
uniform float colorEdge;

out vec4 finalColor;

void main()
{
    float mask = texture(texture0, fragTexCoord).a;
    float v = fragTexCoord.y;
    float m = smoothstep(colorSplit - colorEdge, colorSplit + colorEdge, v);
    vec3 col = mix(colorTop, colorBottom, m);
    float g = smoothstep(0.35, 0.0, v) * gloss;
    col = mix(col, vec3(1.0), g);
    finalColor = vec4(col, mask * fragColor.a);
}
