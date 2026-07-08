#version 330

in vec2 fragTexCoord;
in vec3 fragNormal;
in vec3 fragWorldPos;

uniform sampler2D texture0;
uniform sampler2D shadowMap;
uniform vec4 colDiffuse;
uniform vec3 sunDir;
uniform float ambient;
uniform float bands;
uniform mat4 lightViewProj;
uniform float shadowStrength;

out vec4 finalColor;

float computeShadow(vec3 normal, vec3 lightVec)
{
    vec4 lightClip = lightViewProj * vec4(fragWorldPos, 1.0);
    vec3 proj = lightClip.xyz / lightClip.w;
    proj = proj * 0.5 + 0.5;
    if (proj.x < 0.0 || proj.x > 1.0 || proj.y < 0.0 || proj.y > 1.0 || proj.z > 1.0) return 1.0;
    float bias = max(0.0016 * (1.0 - dot(normal, lightVec)), 0.0004);
    float stored = texture(shadowMap, proj.xy).r;
    return (proj.z - bias > stored) ? 0.0 : 1.0;
}

void main()
{
    vec4 texel = texture(texture0, fragTexCoord) * colDiffuse;
    vec3 normal = normalize(fragNormal);
    vec3 lightVec = normalize(sunDir);
    float ndl = max(dot(normal, lightVec), 0.0);
    float step = clamp(floor(ndl * bands) / (bands - 1.0), 0.0, 1.0);
    float litSun = ambient + (1.0 - ambient) * step;
    float shadow = computeShadow(normal, lightVec);
    float lit = mix(ambient * shadowStrength, litSun, shadow);
    finalColor = vec4(texel.rgb * lit, texel.a);
}
