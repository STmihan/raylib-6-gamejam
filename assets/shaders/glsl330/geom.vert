#version 330

in vec3 vertexPosition;
in vec3 vertexNormal;

uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matNormal;
uniform float maxDepth;

out vec3 worldNormal;
out float viewDepth;

void main()
{
    worldNormal = normalize((matNormal * vec4(vertexNormal, 0.0)).xyz);
    vec4 viewPos = matView * matModel * vec4(vertexPosition, 1.0);
    viewDepth = -viewPos.z / maxDepth;
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
