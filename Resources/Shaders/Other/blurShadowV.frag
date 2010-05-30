uniform sampler2D R5_texture0;	// Shadow lightmap
uniform sampler2D R5_texture1;	// Depth texture

uniform vec2 R5_pixelSize;
uniform vec4 depthRange;		// R5_clipRange can't be used since we're drawing in 2D

// Get the distance to the specified texture coordinate
float GetDistance (in vec2 texCoord)
{
	float depth = texture2D(R5_texture1, texCoord).r;
	return depthRange.z / (depthRange.y - depth * depthRange.w);
}

// Contribution value is depth-dependent
float GetContribution (in float originalDistance, in float originalValue, in vec2 texCoord, in float variance)
{
	float current = texture2D(R5_texture0, texCoord).r;
	float factor = min(abs((originalDistance - GetDistance(texCoord)) / variance), 1.0);
	return mix(current, originalValue, factor);
}

// Performs a depth-respecting blur
void main()
{
	// Pixel size is intentionally hard-coded so that the shadows look consistent at all resolutions
	float pixelSize = 1.0 / 1200.0;

	vec2 tc = gl_TexCoord[0].xy;
	float originalDistance = GetDistance(tc);
	float originalValue = texture2D(R5_texture0, tc).r;
	float variance = pixelSize * depthRange.w * 10.0;

	// The closer to the camera the larger is the area we can sample
	float strength = (1.0 - originalDistance / depthRange.w);
	strength = max(0.75, strength * strength * 3.0);
	pixelSize = pixelSize * strength;

    // Pixel offsets
	float o1 = pixelSize;
	float o2 = o1 + pixelSize;
	float o3 = o2 + pixelSize;

	// Calculate the color based on sampled contributions
	float final =
		GetContribution(originalDistance, originalValue, vec2(tc.x, tc.y - o3), variance) +
		GetContribution(originalDistance, originalValue, vec2(tc.x, tc.y + o3), variance) +
		GetContribution(originalDistance, originalValue, vec2(tc.x, tc.y - o2), variance) +
	 	GetContribution(originalDistance, originalValue, vec2(tc.x, tc.y + o2), variance) +
	 	GetContribution(originalDistance, originalValue, vec2(tc.x, tc.y - o1), variance) +
	 	GetContribution(originalDistance, originalValue, vec2(tc.x, tc.y + o1), variance);

	gl_FragColor = vec4(final / 6.0);
}