



float4 vs(float2 pos : Position) : SV_Position {
	return float4(pos.xy, 0, 1);
}


float4 ps(float4 pos : SV_Position) : SV_Target {
	return float4(1,1,1,1);
}



