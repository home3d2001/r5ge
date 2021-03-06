#if Vertex

#pragma skinned

void main()
{
	R5_vertexPosition 	= R5_position;
	R5_vertexNormal 	= R5_normal;
	R5_vertexColor 		= R5_color;
	R5_vertexTexCoord0 	= R5_texCoord0;
}

#else if Fragment

void main()
{
	R5_surfaceColor = R5_vertexColor * R5_materialColor * Sample2D(0, R5_vertexTexCoord0);
}

#endif