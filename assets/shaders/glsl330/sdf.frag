#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

out vec4 finalColor;

void main()
{
    float dist = texture(texture0, fragTexCoord).a - 0.5;
    float change = length(vec2(dFdx(dist), dFdy(dist)));
    float alpha = smoothstep(-change, change, dist);
    finalColor = vec4(fragColor.rgb, fragColor.a * alpha) * vec4(1.0, 1.0, 1.0, colDiffuse.a);
}
