#include "../Include/_All.h"
#include "../Include/_OpenGL.h"
using namespace R5;

//============================================================================================================
// Keep a record of built-in uniforms so the shader post-processing is able ot reference them automatically
//============================================================================================================

struct Record
{
	String name;
	uint elements;
};

Array<Record> g_uniforms;

//============================================================================================================
// Insert the specified uniform
//============================================================================================================

inline void InsertRecord (const String& name, uint elements)
{
	Record& r = g_uniforms.Expand();
	r.name = name;
	r.elements = elements;
}

//============================================================================================================
// Initialize uniforms -- should only be called once
//============================================================================================================

void RegisterBuiltInUniforms()
{
	InsertRecord("R5_time",							3);
	InsertRecord("R5_eyePosition",					3);
	InsertRecord("R5_pixelSize",					2);
	InsertRecord("R5_clipRange",					4);
	InsertRecord("R5_fogRange",						2);
	InsertRecord("R5_fogColor",						4);
	InsertRecord("R5_materialColor",				4);
	InsertRecord("R5_materialParams0",				4);
	InsertRecord("R5_materialParams1",				2);
	InsertRecord("R5_lightAmbient",					3);
	InsertRecord("R5_lightDiffuse",					3);
	InsertRecord("R5_lightPosition",				3);
	InsertRecord("R5_lightDirection",				3);
	InsertRecord("R5_lightParams",					3);
	InsertRecord("R5_modelScale",					3);
	InsertRecord("R5_modelMatrix",					16);
	InsertRecord("R5_viewMatrix",					16);
	InsertRecord("R5_projMatrix",					16);
	InsertRecord("R5_modelViewMatrix",				16);
	InsertRecord("R5_modelViewProjMatrix",			16);
	InsertRecord("R5_inverseViewMatrix",			16);
	InsertRecord("R5_inverseProjMatrix",			16);
	InsertRecord("R5_inverseMVPMatrix",				16);
	InsertRecord("R5_inverseViewRotationMatrix",	9);
}

//============================================================================================================
// Convenience (fake) uniforms
//============================================================================================================

void ProcessMaterials (String& code)
{
	code.Replace("R5_materialSpecularity",		"R5_materialParams0.x", true);
	code.Replace("R5_materialSpecularHue",		"R5_materialParams0.y", true);
	code.Replace("R5_materialGlow",				"R5_materialParams0.z", true);
	code.Replace("R5_materialOcclusion",		"R5_materialParams0.w", true);
	code.Replace("R5_materialShininess",		"R5_materialParams1.x", true);
	code.Replace("R5_materialReflectiveness",	"R5_materialParams1.y", true);
}

//============================================================================================================
// Helper function
//============================================================================================================

inline void Replace (String& code, const char* match, const char* replacementText, const char* absentText)
{
	if (!code.Replace(match, replacementText, true))
	{
		code << "\t";
		code << replacementText;
		code << " = ";
		code << absentText;
		code << ";\n";
	}
}

//============================================================================================================
// Helper function that extracts the value of the specified variable
//============================================================================================================

void ExtractValue (String& code, String& out, const String& var, const char* def = 0)
{
	uint start = code.Find(var);
	uint length = code.GetLength();

	if (start < length)
	{
		uint offset = start + var.GetLength();

		// Skip up to the equals sign
		while (offset < length && code[offset] != '=') ++offset;

		if (offset < length)
		{
			++offset;
			uint end = offset;
			while (end < length && code[end] != ';') ++end;

			if (end < length)
			{
				// Get the value of this variable
				code.GetTrimmed(out, offset, end);

				// Skip the ';' character
				++end;

				// Trim the line so it can be removed cleanly
				//while (start > 0 && code[start-1] == '\t') --start;
				while (end < length && code[end] < 33) ++end;

				// Erase this segment
				code.Erase(start, end);
				return;
			}
		}
	}
	out = def;
}

//============================================================================================================
// Adds appropriate surface shader functionality
//============================================================================================================

bool ProcessSurfaceShader (String& code, const Flags& desired, Flags& final)
{
	// Standard Surface Color tag
	// Example: R5_surfaceColor = R5_vertexColor * R5_materialColor;
	ASSERT(code.Contains("R5_surfaceColor", true), "Surface shader must always contain R5_surfaceColor output");

	// Shadows and lights are automatically turned off when deferred rendering is requested
	bool depthOnly	= desired.Get(IShader::Flag::DepthOnly);
	bool deferred	= desired.Get(IShader::Flag::Deferred);
	bool shadowed	= desired.Get(IShader::Flag::Shadowed);
	bool lightsOff	= code.Contains("#pragma lighting off", true);
	bool lit		= desired.Get(IShader::Flag::Lit) && !lightsOff;
	bool fog		= desired.Get(IShader::Flag::Fog) && !code.Contains("#pragma fog off", true);

	if (depthOnly)
	{
		final.Set(IShader::Flag::DepthOnly, true);
		deferred = false;
		shadowed = false;
		lit = false;
	}
	else if (deferred)
	{
		shadowed = false;
		lit = false;
	}

	// Remove the last closing bracket
	uint lastBracket = code.Find("}", true, 0, 0xFFFFFFFF, true);
	if (lastBracket >= code.GetSize()) return false;

	// Skip trailing spaces
	while (lastBracket > 0 && code[lastBracket-1] < 33) --lastBracket;
	code.Resize(lastBracket+1);

	if (deferred)
	{
		final.Set(IShader::Flag::Deferred, true);

		// Deferred steps are extremely simple: simply store the values in the 3 output buffers
		code.Replace("R5_surfaceColor", "R5_finalColor[0]", true);

		String spec, hue, glow, occ, norm, shin;

		ExtractValue(code, norm, "R5_surfaceNormal",		"normalize(R5_vertexNormal)");
		ExtractValue(code, spec, "R5_surfaceSpecularity",	"R5_materialSpecularity");
		ExtractValue(code, hue,  "R5_surfaceSpecularHue",	"R5_materialSpecularHue");
		ExtractValue(code, glow, "R5_surfaceGlow",			lightsOff ? "1.0" : "R5_materialGlow");
		ExtractValue(code, occ,  "R5_surfaceOcclusion",		"R5_materialOcclusion");
		ExtractValue(code, shin, "R5_surfaceShininess",		"R5_materialShininess");

		code << "	R5_finalColor[1] = vec4(";
		code << spec;
		code << ", ";
		code << hue;
		code << ", ";
		code << glow;
		code << ", ";
		code << occ;
		code << ");\n\tR5_finalColor[2] = vec4(";
		code << norm;
		code << " * 0.5 + 0.5, ";
		code << shin;
		code << ");\n";
	}
	else if (!lit)
	{
		if (fog)
		{
			final.Set(IShader::Flag::Fog, true);

			code.Replace("R5_surfaceColor", "vec4 R5_surfaceColor", true);
			code <<
			"	R5_surfaceColor.rgb = mix(R5_surfaceColor.rgb, R5_fogColor.rgb, R5_vertexFog);\n"
			"	R5_finalColor[0] = R5_surfaceColor;\n";
		}
		else
		{
			code.Replace("R5_surfaceColor", "R5_finalColor[0]", true);
		}
	}
	else // Lit forward rendering shader
	{
		// Lit forward rendering involves lighting calculations
		code.Replace("R5_surfaceColor", "vec4 R5_surfaceColor", true);

		// Create temporary variables in place of surface properties
		Replace(code, "R5_surfaceNormal",		"vec3 R5_surfaceNormal",		"normalize(R5_vertexNormal)");
		Replace(code, "R5_surfaceSpecularity",	"float R5_surfaceSpecularity",	"R5_materialSpecularity");
		Replace(code, "R5_surfaceSpecularHue",	"float R5_surfaceSpecularHue",	"R5_materialSpecularHue");
		Replace(code, "R5_surfaceGlow",			"float R5_surfaceGlow",			"R5_materialGlow");
		Replace(code, "R5_surfaceOcclusion",	"float R5_surfaceOcclusion",	"R5_materialOcclusion");
		Replace(code, "R5_surfaceShininess",	"float R5_surfaceShininess",	"R5_materialShininess");

		// Direction from the camera to the fragment
		code << "	vec3 eyeDir = normalize(R5_vertexEye);\n";

		if (desired.Get(IShader::Flag::DirLight))
		{
			final.Set(IShader::Flag::DirLight, true);
			code << "	vec3 lightDir = normalize(R5_lightDirection);\n";
		}
		else
		{
			final.Set( desired.Get(IShader::Flag::PointLight) ?
				IShader::Flag::PointLight : IShader::Flag::SpotLight, true);

			// Point lights and spot lights need to take attenuation into account
			code <<
			"	vec3 eyeToLight = R5_lightPosition - R5_vertexEye;\n"
			"	float dist = length(eyeToLight);\n"
			"	float atten = 1.0 - clamp(dist / R5_lightParams.x, 0.0, 1.0);\n"
			"	vec3 lightDir = normalize(eyeToLight);\n"
			"	atten = pow(atten, R5_lightParams.y);\n";
		}

		code <<
		"	float diffuseFactor = max(0.0, dot(lightDir, R5_surfaceNormal));\n"
		"	float reflectiveFactor = max(0.0, dot(reflect(lightDir, R5_surfaceNormal), eyeDir));\n"
		"	float specularFactor = pow(reflectiveFactor, (R5_surfaceShininess * R5_surfaceShininess) * 250.0 + 4.0);\n";

		if (shadowed)
		{
			final.Set(IShader::Flag::Shadowed, true);
			code <<
			"	float shadowFactor = SampleShadow(GetPixelTexCoords()).a;\n"
			"	diffuseFactor  = min(diffuseFactor, shadowFactor);\n"
			"	specularFactor = min(diffuseFactor, specularFactor);\n";
		}

		code <<
		"	vec3 final = R5_lightDiffuse * (diffuseFactor * (R5_surfaceOcclusion * 0.25 + 0.75));\n"
		"	final = (R5_lightAmbient * R5_surfaceOcclusion + final) * R5_surfaceColor.rgb";

		code << (desired.Get(IShader::Flag::DirLight) ? ";\n" : " * atten;\n");

		code <<
		"	final = mix(final, R5_surfaceColor.rgb, R5_surfaceGlow);\n"
		"	vec3 specular = vec3(R5_surfaceSpecularity * specularFactor);\n"
		"	final += mix(specular, specular * R5_surfaceColor.rgb, R5_surfaceSpecularHue)";

		code << (desired.Get(IShader::Flag::DirLight) ? ";\n" : " * atten;\n");

		if (fog)
		{
			final.Set(IShader::Flag::Fog, true);
			code << "	final = mix(final, R5_fogColor.rgb, R5_vertexFog);\n";
		}
		code << "	R5_finalColor[0] = vec4(final, R5_surfaceColor.a);\n";
	}

	uint lastChar = code.GetLength() - 1;

	// For readability purposes
	if (code[lastChar] == '\t')
	{
		code[lastChar] = '}';
		code << "\n";
	}
	else
	{
		// Restore the final bracket
		code << "}\n";
	}
	return true;
}

//============================================================================================================
// Adds appropriate vertex shader functionality
//============================================================================================================

bool AddVertexFunctions (const String& full, String& code, const Flags& desired, Flags& final)
{
	// Remove the last closing bracket
	uint lastBracket = code.Find("}", true, 0, 0xFFFFFFFF, true);
	if (lastBracket >= code.GetSize()) return false;

	bool depthOnly	= desired.Get(IShader::Flag::DepthOnly);
	bool deferred	= desired.Get(IShader::Flag::Deferred);
	bool lit		= desired.Get(IShader::Flag::Lit) && !code.Contains("#pragma lighting off", true);
	bool fog		= desired.Get(IShader::Flag::Fog) && !code.Contains("#pragma fog off", true);
	bool skin		= desired.Get(IShader::Flag::Skinned)	|| code.Contains("#pragma skinned", true);
	bool billboard	= desired.Get(IShader::Flag::Billboard) || code.Contains("#pragma billboard", true);

	// Skip trailing spaces
	while (lastBracket > 0 && code[lastBracket-1] < 33) --lastBracket;
	code.Resize(lastBracket+1);
	String temp;

	// Depth-only rendering should simplify the shader quite a bit
	if (depthOnly)
	{
		deferred = false;
		lit		 = false;
		fog		 = false;
		final.Set(IShader::Flag::DepthOnly, true);
	}

	// Deferred rendering doesn't need lighting or fog
	if (deferred)
	{
		lit = false;
		fog = false;
	}

	// If final position is present then the vertex shader has taken care of applying the model and view transforms
	if (code.Contains("R5_viewPosition", true))
	{
		String vertexName;
		ExtractValue(code, vertexName, "R5_viewPosition");
		code << "	vec4 R5_viewPosition = ";
		code << vertexName;
		code << ";\n";
	}
	else
	{
		String vertexName;
		ExtractValue(code, vertexName, "R5_vertexPosition", "R5_position");

		// Vertex position we'll be working with is a vec4
		code << "	vec4 R5_viewPosition = vec4(";
		code << vertexName;
		code << ", 1.0);\n";

		// Skinned shaders use an additional set of matrices
		if (skin)
		{
			final.Set(IShader::Flag::Skinned, true);

			code <<
			"	mat4 R5_skinMat = R5_boneTransforms[int(R5_boneIndex.x)] * R5_boneWeight.x +\n"
			"		R5_boneTransforms[int(R5_boneIndex.y)] * R5_boneWeight.y +\n"
			"		R5_boneTransforms[int(R5_boneIndex.z)] * R5_boneWeight.z +\n"
			"		R5_boneTransforms[int(R5_boneIndex.w)] * R5_boneWeight.w;\n"
			"	R5_viewPosition = R5_skinMat * R5_viewPosition;\n";
		}

		// Calculate the vertex position
		code <<
		"	R5_viewPosition = R5_modelMatrix * R5_viewPosition;\n"
		"	R5_viewPosition = R5_viewMatrix * R5_viewPosition;\n";

		// Billboarding section
		if (billboard)
		{
			final.Set(IShader::Flag::Billboard, true);

			// Billboard calculations are done in view space
			code <<
			"	vec3 R5_offset = R5_texCoord0.xyz;\n"
			"	R5_offset.xy = (R5_offset.xy * 2.0 - 1.0) * R5_offset.z;\n"
			"	R5_offset.z *= 0.25;\n"
			"	R5_viewPosition.xyz = R5_offset * R5_modelScale + R5_viewPosition.xyz;\n";

			if (lit || deferred)
			{
				code <<
				"	R5_vertexNormal = R5_vertexPosition.xyz - R5_origin.xyz;\n"
				"	R5_vertexTangent = vec3(R5_vertexNormal.y, -R5_vertexNormal.z, 0.0);\n";
			}
		}
	}

	// Final transformed vertex position
	code << "	gl_Position = R5_projMatrix * R5_viewPosition;\n";

	// Earlier versions of GLSL may not like the vertex position being an attribute
	if (g_caps.mVersion < 210) code.Replace("R5_position", "gl_Vertex.xyz", true);

	// Vertex eye is needed for forward rendering or if it's referenced
	bool vertexEye = lit || full.Contains("R5_vertexEye", true);
	if (vertexEye) code << "	R5_vertexEye = R5_viewPosition.xyz;\n";
	if (lit) final.Set(desired.Get() & IShader::Flag::Lit, true);

	// Calculate per-vertex fog
	if (fog)
	{
		final.Set(IShader::Flag::Fog, true);
		code <<
		"	R5_vertexFog = 1.0 - (R5_clipRange.y + R5_viewPosition.z) / R5_clipRange.w;\n"
		"	R5_vertexFog = clamp((R5_vertexFog - R5_fogRange.x) / R5_fogRange.y, 0.0, 1.0);\n"
		"	R5_vertexFog = 0.5 * (R5_vertexFog * R5_vertexFog + R5_vertexFog);\n";
	}

	// Transform the normal
	if (code.Contains("R5_vertexNormal", true))
	{
		// Tangent shouldn't exist without the normal
		if (code.Contains("R5_vertexTangent", true))
		{
			code <<
			"	mat3 R5_rotMat;\n";

			if (skin) code <<
			"	R5_rotMat = mat3(R5_skinMat);\n"
			"	R5_vertexNormal = R5_rotMat * R5_vertexNormal;\n"
			"	R5_vertexTangent = R5_rotMat * R5_vertexTangent;\n";

			code <<
			"	R5_rotMat = mat3(R5_modelMatrix);\n"
			"	R5_vertexNormal = R5_rotMat * R5_vertexNormal;\n"
			"	R5_vertexTangent = R5_rotMat * R5_vertexTangent;\n";

			code <<
			"	R5_rotMat = mat3(R5_viewMatrix);\n"
			"	R5_vertexNormal = R5_rotMat * R5_vertexNormal;\n"
			"	R5_vertexTangent = R5_rotMat * R5_vertexTangent;\n";
		}
		else
		{
			if (skin)
			code << "	R5_vertexNormal = mat3(R5_skinMat) * R5_vertexNormal;\n";
			code << "	R5_vertexNormal = mat3(R5_modelMatrix) * R5_vertexNormal;\n"
					"	R5_vertexNormal = mat3(R5_viewMatrix) * R5_vertexNormal;\n";
		}
	}
	// Restore the final bracket
	code << "}\n";
	return true;
}

//============================================================================================================
// Helper function that attempts to fix legacy shaders (pre-OpenGL 3.0)
//============================================================================================================

void FixLegacyShader (String& code)
{
	// Remove the #version tag as we'll be pre-pending data
	uint vers = code.Find("#version ", true);

	if (vers < code.GetSize())
	{
		code[vers] = '/';
		code[vers+1] = '/';
	}

	// Remove the last closing bracket
	uint lastBracket = code.Find("}", true, 0, 0xFFFFFFFF, true);
	if (lastBracket >= code.GetSize()) return;
	code[lastBracket] = '\n';

	// Skip trailing spaces
	while (lastBracket > 0 && code[lastBracket-1] < 33) --lastBracket;
	code.Resize(lastBracket+1);

	uint lightPos	= code.Replace("gl_LightSource[0].position", "R5_lightPosition", true);
	uint ambient	= code.Replace("gl_LightSource[0].ambient", "R5_lightAmbient", true);
	uint diffuse	= code.Replace("gl_LightSource[0].diffuse", "R5_lightDiffuse", true) |
					  code.Replace("gl_LightSource[0].specular", "R5_lightDiffuse", true);
	uint texCoord0	= code.Replace("gl_MultiTexCoord0", "R5_texCoord0", true);
	uint texCoord1	= code.Replace("gl_MultiTexCoord1", "R5_texCoord1", true);

	String prefix;

	if (texCoord0)	prefix << "attribute vec2 R5_texCoord0;\n";
	if (texCoord1)	prefix << "attribute vec2 R5_texCoord1;\n";
	if (lightPos)	prefix << "uniform vec3 R5_lightPosition;\n";
	if (ambient)	prefix << "uniform vec3 R5_lightAmbient;\n";
	if (diffuse)	prefix << "uniform vec3 R5_lightDiffuse;\n";

	if (prefix.IsValid()) code = prefix + code;

	::ProcessMaterials(code);

	// Restore the final bracket
	code << "}\n";
}

//============================================================================================================
// Adds appropriate common functions if the program uses them
//============================================================================================================

void AddCommonFunctions (String& code)
{
	String prefix;

	if (code.Contains("NormalMapToNormal"))
	{
		prefix <<
		"vec3 NormalMapToNormal (in vec4 val)\n"
		"{\n"
		"	vec3 tangent = normalize(R5_vertexTangent);\n"
		"	vec3 normal = normalize(R5_vertexNormal);\n"
		"	mat3 TBN = mat3(tangent, cross(normal, tangent), normal);\n"
		"	return normalize(TBN * (val.xyz * 2.0 - 1.0));\n"
		"}\n";
	}

	if (code.Contains("GetFogFactor", true))
	{
		prefix <<
		"float GetFogFactor (in float depth)\n"
		"{\n"
		"	float linear = (R5_clipRange.z / (R5_clipRange.y - depth * R5_clipRange.w) - R5_clipRange.x) / R5_clipRange.w;\n"
		"	float fogFactor = clamp((linear - R5_fogRange.x) / R5_fogRange.y, 0.0, 1.0);\n"
		"	return (fogFactor * fogFactor + fogFactor) * 0.5;\n"
		"}\n";
	}

	if (code.Contains("GetLinearDepth", true))
	{
		prefix <<
		"float GetLinearDepth (in float depth)\n"
		"{\n"
		"	return (R5_clipRange.z / (R5_clipRange.y - depth * R5_clipRange.w) - R5_clipRange.x) / R5_clipRange.w;\n"
		"}\n";
	}

	if (code.Contains("GetViewPosition", true))
	{
		prefix <<
		"vec3 GetViewPosition (in vec2 texCoord, in float depth)\n"
		"{\n"
		"	vec4 view = vec4(texCoord.x, texCoord.y, depth, 1.0);\n"
		"	view.xyz = view.xyz * 2.0 - 1.0;\n"
		"	view = R5_inverseProjMatrix * view;\n"
		"	return view.xyz / view.w;\n"
		"}\n";
	}

	if (prefix.IsValid()) code = prefix + code;
}

//============================================================================================================
// Helper function used in AddReferencedVariables function below
//============================================================================================================

void ReplaceSample (String& code, String& prefix, const char* type)
{
	String match ("Sample"), test, replace;
	match << type;
	match << "(";

	for (uint i = 0; i < 7; ++i)
	{
		test = match;
		test << i;

		replace = "texture";
		if (g_caps.mVersion < 300) replace << type;
		replace << "(R5_texture";
		replace << i;

		if (code.Replace(test, replace, true))
		{
			prefix << "uniform sampler";
			prefix << type;
			prefix << " R5_texture";
			prefix << i;
			prefix << ";\n";
		}
	}
}

//============================================================================================================
// Add appropriate uniforms
//============================================================================================================

void AddReferencedVariables (String& code, bool isFragmentShader)
{
	// If we have no uniform records to work with, create them
	if (g_uniforms.IsEmpty()) RegisterBuiltInUniforms();

	String prefix;

	FOREACH(i, g_uniforms)
	{
		const Record& r = g_uniforms[i];

		if (code.Contains(r.name, true))
		{
			if		(r.elements == 16)	prefix << "uniform mat4 ";
			else if (r.elements == 9)	prefix << "uniform mat3 ";
			else if (r.elements == 4)	prefix << "uniform vec4 ";
			else if (r.elements == 3)	prefix << "uniform vec3 ";
			else if (r.elements == 2)	prefix << "uniform vec2 ";
			else						prefix << "uniform float ";

			prefix << r.name;
			prefix << ";\n";
		}
	}

	// OpenGL 2.1 and earlier versions used an 'attribute' keyword
	const char* inOut = "attribute ";

	if (isFragmentShader)
	{
		ReplaceSample(code, prefix, "2D");
		ReplaceSample(code, prefix, "Cube");

		if (code.Replace("SampleShadow(", (g_caps.mVersion > 210) ? "texture(R5_shadowMap, " : "texture2D(R5_shadowMap, ", true))
		{
			prefix << "uniform sampler2D R5_shadowMap;\n";
		}
	}
	else
	{
		if (g_caps.mVersion >= 300) inOut = "in ";
		if (code.Contains("R5_position",		true)) { prefix << inOut; prefix << "vec3 R5_position;\n"; }
		if (code.Contains("R5_tangent",			true)) { prefix << inOut; prefix << "vec3 R5_tangent;\n"; }
		if (code.Contains("R5_normal",			true)) { prefix << inOut; prefix << "vec3 R5_normal;\n"; }
		if (code.Contains("R5_color",			true)) { prefix << inOut; prefix << "vec4 R5_color;\n"; }
		if (code.Contains("R5_boneWeight",		true)) { prefix << inOut; prefix << "vec4 R5_boneWeight;\n"; }
		if (code.Contains("R5_boneIndex",		true)) { prefix << inOut; prefix << "vec4 R5_boneIndex;\n"; }
		if (code.Contains("R5_boneTransforms",	true)) prefix << "uniform mat4 R5_boneTransforms[32];\n";
		if (code.Contains("R5_texCoord0",		true))
		{
			prefix << inOut;

			if (code.Contains("R5_texCoord0.xyz"))
			{
				prefix << "vec3 R5_texCoord0;\n";
			}
			else
			{
				prefix << "vec2 R5_texCoord0;\n";
			}
		}
		if (code.Contains("R5_texCoord1", true))
		{
			prefix << inOut;

			if (code.Contains("R5_texCoord1.xyz"))
			{
				prefix << "vec3 R5_texCoord1;\n";
			}
			else
			{
				prefix << "vec2 R5_texCoord1;\n";
			}
		}
	}

	if (g_caps.mVersion < 300) inOut = "varying ";
	else inOut = (isFragmentShader ? "in " : "out ");

	if (code.Contains("R5_vertexColor",		true)) { prefix << inOut; prefix << "vec4 R5_vertexColor;\n"; }
	if (code.Contains("R5_vertexEye",		true)) { prefix << inOut; prefix << "vec3 R5_vertexEye;\n"; }
	if (code.Contains("R5_vertexLight",		true)) { prefix << inOut; prefix << "vec4 R5_vertexLight;\n"; }
	if (code.Contains("R5_vertexNormal",	true)) { prefix << inOut; prefix << "vec3 R5_vertexNormal;\n"; }
	if (code.Contains("R5_vertexTangent",	true)) { prefix << inOut; prefix << "vec3 R5_vertexTangent;\n"; }
	if (code.Contains("R5_vertexTexCoord0",	true)) { prefix << inOut; prefix << "vec2 R5_vertexTexCoord0;\n"; }
	if (code.Contains("R5_vertexTexCoord1",	true)) { prefix << inOut; prefix << "vec2 R5_vertexTexCoord1;\n"; }
	if (code.Contains("R5_vertexFog",		true)) { prefix << inOut; prefix << "float R5_vertexFog;\n"; }

	if (prefix.IsValid()) code = prefix + code;
}

//============================================================================================================
// R5 shader format needs to be translated to the appropriate API
//============================================================================================================

void ConvertCommonTypes (String& code)
{
	if (g_caps.mVersion < 300)
	{
		code.Replace("half4", "vec4", true);
		code.Replace("half3", "vec3", true);
		code.Replace("half2", "vec2", true);
	}
	else
	{
		code.Replace("half4", "mediump vec4", true);
		code.Replace("half3", "mediump vec3", true);
		code.Replace("half2", "mediump vec2", true);
	}

	code.Replace("int2", "ivec2", true);
	code.Replace("int3", "ivec3", true);
	code.Replace("int4", "ivec4", true);

	code.Replace("float2", "vec2", true);
	code.Replace("float3", "vec3", true);
	code.Replace("float4", "vec4", true);
}

//============================================================================================================
// NOTE: There is a bug in NVIDIA's OpenGL 2.1 drivers for OSX with mat3(mat4):
// it simply doesn't work. It gives completely unpredictable results.
// It took me a while to track this one down.
//============================================================================================================

void ExpandMat3 (String& code)
{
	uint offset (0);
	String left, val, right;

	for (;;)
	{
		offset = code.Find("mat3(", true, offset);
		if (offset >= code.GetLength()) break;
		uint end (offset += 5);

		for (; end < code.GetLength(); ++end)
		{
			if (code[end] == ')')
			{
				code.GetString(left, 0, offset);
				code.GetString(val, offset, end);
				code.GetString(right, end);

				code = left;
				code << val;
				code << "[0].xyz, ";
				code << val;
				code << "[1].xyz, ";
				code << val;
				code << "[2].xyz";
				code << right;
				offset = end + val.GetLength() * 3 + 26;
				break;
			}
			else if (code[end] == ',') break;
		}
	}
}

//============================================================================================================
// Preprocess a new shader format
//============================================================================================================

Flags GLPreprocessShader (const String& full, String& code, const Flags& desired)
{
	Flags final;
	bool surface = code.Contains("R5_surface", true);

	if (surface || code.Contains("R5_finalColor", true))
	{
		final.Set(IShader::Flag::Fragment, true);

		if (surface)
		{
			final.Set(IShader::Flag::Surface, true);
			::ProcessSurfaceShader(code, desired, final);
		}

		::AddCommonFunctions(code);
		::ProcessMaterials(code);

		// Convenience (fake) functions
		code.Replace("GetPixelTexCoords()", "gl_FragCoord.xy * R5_pixelSize", true);
		code.Replace("GetPixelPosition()", "int2(int(gl_FragCoord.x), int(gl_FragCoord.y))", true);
		code.Replace("R5_fragCoord", "gl_FragCoord", true);

		String prefix;

		if (g_caps.mVersion >= 300)
		{
			prefix = "#version 130\n";

			// Fragment shader output values -- support for up to 4 buffers. If you need more, just add them.
			if		(code.Contains("R5_finalColor[3]", true)) prefix << "out vec4 R5_finalColor[4];\n";
			else if (code.Contains("R5_finalColor[2]", true)) prefix << "out vec4 R5_finalColor[3];\n";
			else if (code.Contains("R5_finalColor[1]", true)) prefix << "out vec4 R5_finalColor[2];\n";
			else if (code.Replace("R5_finalColor[0]", "R5_finalColor", true)) prefix << "out vec4 R5_finalColor;\n";

			// All instances of 'varying' should be replaced
			code.Replace("varying", "in", true);
		}
		else
		{
			// Older GLSL syntax: output to a built-in value
			code.Replace("R5_finalColor", "gl_FragData", true);

			if (!code.Contains("gl_FragData[1]"))
			{
				code.Replace("gl_FragData[0]", "gl_FragColor", true);
			}
		}

		// Add all referenced variables and convert common types
		::AddReferencedVariables(code, true);
		::ConvertCommonTypes(code);

		// Prepend the prefix
		code = prefix + code;

		// For shader code debugging:
		while (code.Replace("\t\t", "\t", true)) {}
	}
	else if (code.Contains("R5_vertexPosition", true) || code.Contains("R5_viewPosition", true))
	{
		final.Set(IShader::Flag::Vertex, true);

		::AddVertexFunctions(full, code, desired, final);
		::AddCommonFunctions(code);
		::AddReferencedVariables(code, false);
		::ConvertCommonTypes(code);

		if (g_caps.mVersion >= 300)
		{
			// Latest syntax: built-in uniforms are no longer present and 'varying' is replaced by 'in'/'out'.
			code.Replace("varying", "out", true);
			code = "#version 130\n" + code;
		}
		else if (g_caps.mVersion >= 210)
		{
			code = "#version 120\n" + code;
		}
		else if (g_caps.mVersion >= 200)
		{
			code = "#version 110\n" + code;
		}

		// For shader code debugging:
		while (code.Replace("\t\t", "\t", true)) {}
	}
	else
	{
		// Lights are no longer supported
		if (code.Contains("gl_Light", true) || code.Contains("gl_FrontLight", true))
		{
			WARNING("Legacy shader format, please upgrade");
		}

		// This shader uses an outdated format
		final.Set(IShader::Flag::LegacyFormat, true);
		::FixLegacyShader(code);
		final.Set(code.Contains("gl_Position") ? IShader::Flag::Vertex : IShader::Flag::Fragment);
	}

	// Convert mat3(mat4) into mat3(mat4[0].xyz, mat4[1].xyz, mat4[2].xyz)
	if (g_caps.mVersion < 300) ExpandMat3(code);
	return final;
}