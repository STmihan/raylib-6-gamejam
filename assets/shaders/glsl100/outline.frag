#version 100

precision mediump float;

varying vec2 fragTexCoord;

uniform sampler2D texture0;
uniform sampler2D normalDepthTex;
uniform sampler2D unitMaskTex;
uniform vec2 texelSize;
uniform float outlineWidth;
uniform float creaseCos;
uniform float depthThreshold;
uniform vec4 outlineColor;
uniform vec3 screenRight;
uniform vec3 screenUp;
uniform float cavityRadius;
uniform float cavityValley;
uniform float cavityRidge;
uniform float unitOutlineScale;
uniform float unitCavityScale;

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

    vec2 off = texelSize * outlineWidth;
    float edge = 0.0;
    edge = max(edge, edgeAt(centerNormal, centerDepth, fragTexCoord + vec2(off.x, 0.0)));
    edge = max(edge, edgeAt(centerNormal, centerDepth, fragTexCoord + vec2(-off.x, 0.0)));
    edge = max(edge, edgeAt(centerNormal, centerDepth, fragTexCoord + vec2(0.0, off.y)));
    edge = max(edge, edgeAt(centerNormal, centerDepth, fragTexCoord + vec2(0.0, -off.y)));

    vec3 color = texture2D(texture0, fragTexCoord).rgb;

    float unitMask = texture2D(unitMaskTex, fragTexCoord).r;
    float outlineFactor = mix(1.0, unitOutlineScale, unitMask);
    float cavityFactor = mix(1.0, unitCavityScale, unitMask);
    edge *= outlineFactor;

    vec2 cr = texelSize * cavityRadius;
    vec4 sR = texture2D(normalDepthTex, fragTexCoord + vec2(cr.x, 0.0));
    vec4 sL = texture2D(normalDepthTex, fragTexCoord - vec2(cr.x, 0.0));
    vec4 sU = texture2D(normalDepthTex, fragTexCoord + vec2(0.0, cr.y));
    vec4 sD = texture2D(normalDepthTex, fragTexCoord - vec2(0.0, cr.y));
    float valid = 1.0;
    if (center.a <= 0.001 || sR.a <= 0.001 || sL.a <= 0.001 || sU.a <= 0.001 || sD.a <= 0.001) valid = 0.0;
    vec3 nR = normalize(sR.rgb * 2.0 - 1.0);
    vec3 nL = normalize(sL.rgb * 2.0 - 1.0);
    vec3 nU = normalize(sU.rgb * 2.0 - 1.0);
    vec3 nD = normalize(sD.rgb * 2.0 - 1.0);
    float convex = dot(nR - nL, screenRight) + dot(nU - nD, screenUp);
    float valley = max(-convex, 0.0) * cavityValley * valid * cavityFactor;
    float ridge = max(convex, 0.0) * cavityRidge * valid * cavityFactor;
    color *= 1.0 - clamp(valley, 0.0, 0.9);
    color += clamp(ridge, 0.0, 0.9) * (1.0 - color);

    color = mix(color, outlineColor.rgb, edge);
    gl_FragColor = vec4(color, 1.0);
}
