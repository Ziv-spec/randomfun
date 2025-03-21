

float4 vs(uint vid : SV_VERTEXID) : SV_Position {
	if (vid == 0) return float4(-1, -1, 0, 1); 
	if (vid == 1) return float4(-1, 1, 0, 1); 
	if (vid == 2) return float4(1, -1, 0, 1); 
	if (vid == 3) return float4(1, 1, 0, 1); 
	return float4(0,0,0,0);		
}

sampler sampler0 : register(s0);
Texture2D<float> texture0 : register(t0);

cbuffer ps_constant_buffer : register(b0) {
	float2 inv_size;
};

float2 ps(float4 p : SV_Position) : SV_Target {

	// TODO(ziv): figure out how the hell am I supposed to divide by this stuff
	float  color =  texture0.Sample(sampler0, float2(p.x, p.y)*inv_size);

	float2 uv = p.xy * color;
	float2 result = uv * inv_size;

	return result;
}
