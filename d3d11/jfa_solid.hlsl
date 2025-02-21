
float4 vs(uint vid : SV_VERTEXID) : SV_Position {
	if (vid == 0) return float4(-1, -1, 0, 1); 
	if (vid == 1) return float4(-1, 1, 0, 1); 
	if (vid == 2) return float4(1, -1, 0, 1); 
	if (vid == 3) return float4(1, 1, 0, 1); 
	return float4(0,0,0,0);		
}

sampler sampler0 : register(s0);
Texture2D<float2> jfa_tex : register(t0);

float4 ps(float4 pos : SV_Position) : SV_Target {
	float2 color =  jfa_tex.Sample(sampler0, pos.xy*float2(1.f/800.f, 1.f/600.f));
	if (color.y == 0) discard; 
	if (color.x == 0) discard; 

	return float4(1 ,1, 1, 1); 
}