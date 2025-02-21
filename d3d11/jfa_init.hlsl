





float4 vs(uint vid : SV_VERTEXID) : SV_Position {
	if (vid == 0) return float4(-1, -1, 0, 1); 
	if (vid == 1) return float4(-1, 1, 0, 1); 
	if (vid == 2) return float4(1, -1, 0, 1); 
	if (vid == 3) return float4(1, 1, 0, 1); 
	return float4(0,0,0,0);		
}

sampler sampler0 : register(s0);
Texture2D<float> texture0 : register(t0);

float2 ps(float4 p : SV_Position) : SV_Target {

	float inv_window_height = 1.f/600.f; 
	float inv_window_width  = 1.f/800.f;
	
	// TODO(ziv): figure out how the hell am I supposed to divide by this stuff
	float  color = texture0.Sample(sampler0, float2(p.x*inv_window_width, p.y*inv_window_height));

	float2 uv = p.xy * color;

	return float2(uv.x*inv_window_width, uv.y*inv_window_height); // , 0, 1);
}
