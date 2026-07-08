#version 330

in vec2 fragTexCoord;

uniform sampler2D texture0;
uniform sampler2D normalDepthTex;
uniform vec2 texelSize;
uniform float outlineWidth;
uniform float creaseCos;
uniform float depthThreshold;
uniform vec4 outlineColor;

out vec4 finalColor;

float EdgeAt(vec3 centerNormal, float centerDepth, vec2 uv)
{
    vec4 sampled = texture(normalDepthTex, uv);
    vec3 sampledNormal = normalize(sampled.rgb * 2.0 - 1.0);
    float normalEdge = (dot(centerNormal, sampledNormal) < creaseCos) ? 1.0 : 0.0;
    float depthEdge = (abs(centerDepth - sampled.a) > depthThreshold) ? 1.0 : 0.0;
    return max(normalEdge, depthEdge);
}

void main()
{
    vec4 center = texture(normalDepthTex, fragTexCoord);
    vec3 centerNormal = normalize(center.rgb * 2.0 - 1.0);
    float centerDepth = center.a;

    vec2 step = texelSize * outlineWidth;
    float edge = 0.0;
    edge = max(edge, EdgeAt(centerNormal, centerDepth, fragTexCoord + vec2(step.x, 0.0)));
    edge = max(edge, EdgeAt(centerNormal, centerDepth, fragTexCoord + vec2(-step.x, 0.0)));
    edge = max(edge, EdgeAt(centerNormal, centerDepth, fragTexCoord + vec2(0.0, step.y)));
    edge = max(edge, EdgeAt(centerNormal, centerDepth, fragTexCoord + vec2(0.0, -step.y)));

    vec3 color = texture(texture0, fragTexCoord).rgb;
    color = mix(color, outlineColor.rgb, edge);
    finalColor = vec4(color, 1.0);
}
