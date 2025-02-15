





float4 vs(uint vid : SV_VERTEXID) : SV_Position {
	
	if (vid == 0) return float4(-1, -1, 0, 1); 
	if (vid == 1) return float4(-1, 1, 0, 1); 
	if (vid == 2) return float4(1, -1, 0, 1); 
	if (vid == 3) return float4(1, 1, 0, 1); 
	return float4(0,0,0,0);		
}

sampler sampler0 : register(s0);
Texture2D<float> texture0 : register(t0);

float4 ps(float4 p : SV_Position) : SV_Target {
	// TODO(ziv): figure out how the hell am I supposed to divide by this stuff
	float  color = texture0.Sample(sampler0, float2(p.x/800, p.y/600));
	return float4(color, 0, 0, 1);
}
