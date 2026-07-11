#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform float progress;
uniform float innerR;
uniform vec4 fgColor;
uniform vec4 bgColor;

out vec4 finalColor;

void main()
{
    vec2 p = fragTexCoord - vec2(0.5);
    float r = length(p);
    if (r > 0.5 || r < innerR) discard;
    float ang = atan(p.x, -p.y);
    float a = (ang < 0.0 ? ang + 6.2831853 : ang) / 6.2831853;
    vec4 col = (a <= progress) ? fgColor : bgColor;
    finalColor = col * fragColor;
}
