#version 100

attribute vec3 vertexPosition;
attribute vec3 vertexNormal;

uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matNormal;
uniform float maxDepth;

varying vec3 worldNormal;
varying float viewDepth;

void main()
{
    worldNormal = normalize((matNormal * vec4(vertexNormal, 0.0)).xyz);
    vec4 viewPos = matView * matModel * vec4(vertexPosition, 1.0);
    viewDepth = -viewPos.z / maxDepth;
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
