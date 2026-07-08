#version 100

attribute vec3 vertexPosition;

uniform mat4 mvp;
uniform mat4 matModel;

varying vec3 fragWorldPos;

void main()
{
    fragWorldPos = (matModel * vec4(vertexPosition, 1.0)).xyz;
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
