#version 330

in vec3 fragWorldPos;

uniform sampler2D sdfMap;
uniform vec2 sdfOrigin;
uniform float sdfWorldSize;
uniform float sdfMaxDist;
uniform float time;
uniform float colorRange;
uniform float outlineWidth;
uniform float lineThick;
uniform float lineGap;
uniform float lineThin;
uniform float lineTravel;
uniform float lineSpeed;
uniform float lineInterval;
uniform float lineWobble;
uniform float lineWobbleScale;
uniform float lineWobbleSpeed;
uniform float detailAmount;
uniform float detailScale;
uniform float detailSpeed;
uniform float detailReach;
uniform vec3 deepColor;
uniform vec3 shallowColor;
uniform vec3 foamColor;
uniform vec3 outlineColor;

out vec4 finalColor;

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
    return texture(sdfMap, (world - sdfOrigin) / sdfWorldSize).r * sdfMaxDist;
}

void main()
{
    vec2 worldXZ = fragWorldPos.xz;
    float dist = coastDistance(worldXZ);

    float depth = clamp(dist / colorRange, 0.0, 1.0);
    vec3 baseColor = mix(shallowColor, deepColor, depth);

    float aa = 0.035;
    vec2 wobOrbit = vec2(sin(time * lineWobbleSpeed), cos(time * lineWobbleSpeed * 0.8)) * 1.5;
    float wob = (fbm(worldXZ * lineWobbleScale + wobOrbit) - 0.5) * 2.0 * lineWobble;
    float d = dist + wob;

    float thick = 1.0 - smoothstep(lineThick, lineThick + aa, d);

    float travelLine = 0.0;
    float baseIdx = floor(time / lineInterval);
    for (int k = 0; k < 3; k++)
    {
        float idx = baseIdx - float(k);
        float phase = (time - idx * lineInterval) * lineSpeed;
        if (phase < 1.0)
        {
            float center = lineThick + lineGap + phase * lineTravel;
            float ring = 1.0 - smoothstep(lineThin * 0.5, lineThin * 0.5 + aa, abs(d - center));
            float fade = (1.0 - phase) * smoothstep(0.0, 0.18, phase);
            travelLine = max(travelLine, ring * fade);
        }
    }

    vec2 center = sdfOrigin + vec2(sdfWorldSize * 0.5);
    vec2 outward = normalize(worldXZ - center + vec2(1e-5));
    vec2 speckUV = (worldXZ - outward * time * detailSpeed) * detailScale;
    float speckNoise = valueNoise(speckUV);
    float speckThreshold = 1.0 - detailAmount * 0.6;
    float speckBand = smoothstep(lineThick, lineThick + lineThin, d) *
                      (1.0 - smoothstep(lineThick + detailReach * 0.65, lineThick + detailReach, d));
    float specks = smoothstep(speckThreshold, speckThreshold + 0.04, speckNoise) * speckBand;

    float foam = clamp(max(max(thick, travelLine), specks), 0.0, 1.0);

    float outline = 1.0 - smoothstep(outlineWidth * 0.6, outlineWidth, dist);

    vec3 color = mix(baseColor, foamColor, foam);
    color = mix(color, outlineColor, outline);
    finalColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
