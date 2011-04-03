#version 150
uniform sampler2DMS R5_texture0;
uniform vec2 R5_pixelSize;

out vec4 FinalColor;

void main()
{
	vec4 a = texelFetch(R5_texture0, ivec2(int(gl_FragCoord.x), int(gl_FragCoord.y)), 0);
	vec4 b = texelFetch(R5_texture0, ivec2(int(gl_FragCoord.x), int(gl_FragCoord.y)), 1);
	vec4 c = texelFetch(R5_texture0, ivec2(int(gl_FragCoord.x), int(gl_FragCoord.y)), 2);
	vec4 d = texelFetch(R5_texture0, ivec2(int(gl_FragCoord.x), int(gl_FragCoord.y)), 3);

	FinalColor = (a + b + c + d) * 0.25;
}