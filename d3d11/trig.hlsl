

float4 vs(float3 pos : Position) : SV_Position {
	return float4(pos.xyz, 1);
}

float4 ps(float4 pos : SV_Position) : SV_Target {
	return float4(1,1,1,1);
}



