#version 330

in vec3 worldNormal;
in float viewDepth;

out vec4 finalColor;

void main()
{
    finalColor = vec4(normalize(worldNormal) * 0.5 + 0.5, clamp(viewDepth, 0.0, 1.0));
}
