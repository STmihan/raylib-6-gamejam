#version 100

attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;
attribute vec3 vertexNormal;

uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

varying vec2 fragTexCoord;
varying vec3 fragNormal;
varying vec3 fragWorldPos;

void main()
{
    fragTexCoord = vertexTexCoord;
    fragNormal = normalize((matNormal * vec4(vertexNormal, 0.0)).xyz);
    fragWorldPos = (matModel * vec4(vertexPosition, 1.0)).xyz;
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
