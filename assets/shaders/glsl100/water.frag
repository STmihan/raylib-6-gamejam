#version 100

precision highp float;

varying vec3 fragWorldPos;

uniform sampler2D sdfMap;
uniform vec2 sdfOrigin;
uniform float sdfWorldSize;
uniform float sdfMaxDist;
uniform float time;
uniform float colorRange;
uniform float foamDistance;
uniform float foamCutoff;
uniform float noiseScale;
uniform float distortAmount;
uniform float scrollSpeed;
uniform float outlineWidth;
uniform float flowSpeed;
uniform float flowAmount;
uniform vec3 deepColor;
uniform vec3 shallowColor;
uniform vec3 foamColor;
uniform vec3 outlineColor;

float hash21(vec2 p)
{
    p = fract(p * vec2(123.34, 345.45));
    p += dot(p, p + 34.345);
    return fract(p.x * p.y);
}

float valueNoise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    float a = hash21(i);
    float b = hash21(i + vec2(1.0, 0.0));
    float c = hash21(i + vec2(0.0, 1.0));
    float d = hash21(i + vec2(1.0, 1.0));
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

float fbm(vec2 p)
{
    return 0.62 * valueNoise(p) + 0.38 * valueNoise(p * 2.03 + vec2(5.1, 1.7));
}

float coastDistance(vec2 world)
{
    return texture2D(sdfMap, (world - sdfOrigin) / sdfWorldSize).r * sdfMaxDist;
}

void main()
{
    vec2 worldXZ = fragWorldPos.xz;
    float dist = coastDistance(worldXZ);

    float depth = clamp(dist / colorRange, 0.0, 1.0);
    vec3 baseColor = mix(shallowColor, deepColor, depth);

    vec2 warp = (vec2(
        valueNoise(worldXZ * noiseScale * 0.5),
        valueNoise(worldXZ * noiseScale * 0.5 + 13.1)) - 0.5) * distortAmount;

    float t = clamp(dist / foamDistance, 0.0, 1.0);
    vec2 coastScroll = vec2(time * 0.02, time * -0.015);
    float coastNoise = fbm(worldXZ * noiseScale + warp + coastScroll);
    float band = 0.5 + 0.5 * sin(dist * (12.566 / foamDistance) - time * flowSpeed);
    float coastCutoff = mix(foamCutoff, 1.0, t * t) - band * flowAmount;
    float coastFoam = smoothstep(coastCutoff - 0.04, coastCutoff + 0.04, coastNoise);
    coastFoam *= 1.0 - smoothstep(0.85, 1.0, t);

    vec2 drift = vec2(-0.94, -0.34);
    float oceanNoise = fbm(worldXZ * noiseScale + warp - drift * time * scrollSpeed);
    float oceanFade = clamp(dist / colorRange, 0.0, 1.0);
    float oceanCutoff = mix(0.86, 0.98, oceanFade);
    float oceanFoam = smoothstep(oceanCutoff - 0.025, oceanCutoff + 0.025, oceanNoise);
    oceanFoam *= smoothstep(foamDistance * 0.9, foamDistance * 1.6, dist);

    float foam = max(coastFoam, oceanFoam);

    float outline = 1.0 - smoothstep(outlineWidth * 0.6, outlineWidth, dist);

    vec3 color = mix(baseColor, foamColor, foam);
    color = mix(color, outlineColor, outline);
    gl_FragColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
