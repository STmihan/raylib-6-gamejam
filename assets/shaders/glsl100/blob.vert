#version 100

attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;

uniform mat4 mvp;
uniform mat4 matModel;

varying vec3 fragWorldPos;
varying vec2 fragTexCoord;

void main()
{
    fragWorldPos = (matModel * vec4(vertexPosition, 1.0)).xyz;
    fragTexCoord = vertexTexCoord;
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
