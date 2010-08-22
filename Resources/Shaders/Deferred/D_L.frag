uniform sampler2D R5_texture0;
uniform sampler2D R5_texture1;

varying vec2 _texCoord0;
varying vec2 _texCoord1;
varying vec3 _normal;

void main()
{
	vec4 diffuse = texture2D(R5_texture0, _texCoord0);
	float ao 	 = texture2D(R5_texture1, _texCoord1).a;

	gl_FragData[0] = gl_FrontMaterial.diffuse * gl_Color * diffuse;
	gl_FragData[1] = vec4(
		R5_MATERIAL_SPECULARITY,
		R5_MATERIAL_SPECULAR_HUE,
		R5_MATERIAL_GLOW,
		R5_MATERIAL_OCCLUSION * ao);
	gl_FragData[2] = vec4(normalize(_normal) * 0.5 + 0.5, gl_FrontMaterial.specular.a);
}
