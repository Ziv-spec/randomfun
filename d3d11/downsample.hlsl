
// this shader just takes any texture and draws it

float4 vs(float2 pos : Position) : SV_Position {
	return float4(pos, 0, 1);
}

sampler sampler0 : register(s0); 
Texture2D texture0 : register(t0);

float4 ps(float4 pos : SV_Position) : SV_Target {
	return texture0.Sample(sampler0, pos.xy/float2(800.f, 600.f)); 
}
