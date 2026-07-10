#version 100

precision mediump float;

varying vec2 fragTexCoord;
varying vec4 fragColor;

uniform sampler2D texture0;

void main()
{
    float dist = texture2D(texture0, fragTexCoord).a;
    float alpha = smoothstep(0.5 - 0.06, 0.5 + 0.06, dist);
    gl_FragColor = vec4(fragColor.rgb, fragColor.a * alpha);
}
