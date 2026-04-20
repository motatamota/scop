#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
flat in vec3 vColor;  // 面ごとに割り当てられたグレー (scop)

uniform sampler2D texture1;
uniform float blend;    // 0.0 = 色のみ, 1.0 = テクスチャのみ

void main()
{
	vec4 tex = texture(texture1, TexCoord);
	FragColor = vec4(mix(vColor, tex.rgb, blend), 1.0);
}
