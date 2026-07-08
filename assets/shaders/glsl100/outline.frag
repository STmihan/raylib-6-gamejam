#version 100

precision mediump float;

varying vec2 fragTexCoord;

uniform sampler2D texture0;
uniform sampler2D normalDepthTex;
uniform vec2 texelSize;
uniform float outlineWidth;
uniform float creaseCos;
uniform float depthThreshold;
uniform vec4 outlineColor;

float edgeAt(vec3 centerNormal, float centerDepth, vec2 uv)
{
    vec4 sampled = texture2D(normalDepthTex, uv);
    vec3 sampledNormal = normalize(sampled.rgb * 2.0 - 1.0);
    float normalEdge = 0.0;
    if (dot(centerNormal, sampledNormal) < creaseCos) normalEdge = 1.0;
    float depthEdge = 0.0;
    if (abs(centerDepth - sampled.a) > depthThreshold) depthEdge = 1.0;
    return max(normalEdge, depthEdge);
}

void main()
{
    vec4 center = texture2D(normalDepthTex, fragTexCoord);
    vec3 centerNormal = normalize(center.rgb * 2.0 - 1.0);
    float centerDepth = center.a;

    vec2 step = texelSize * outlineWidth;
    float edge = 0.0;
    edge = max(edge, edgeAt(centerNormal, centerDepth, fragTexCoord + vec2(step.x, 0.0)));
    edge = max(edge, edgeAt(centerNormal, centerDepth, fragTexCoord + vec2(-step.x, 0.0)));
    edge = max(edge, edgeAt(centerNormal, centerDepth, fragTexCoord + vec2(0.0, step.y)));
    edge = max(edge, edgeAt(centerNormal, centerDepth, fragTexCoord + vec2(0.0, -step.y)));

    vec3 color = texture2D(texture0, fragTexCoord).rgb;
    color = mix(color, outlineColor.rgb, edge);
    gl_FragColor = vec4(color, 1.0);
}
