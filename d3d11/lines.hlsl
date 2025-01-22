
cbuffer lines_vs_cnostant_buffer {
	row_major float4x4 projection;
};

struct Line {
	float3 p[2];
};

StructuredBuffer<Line> lines : register(t0); 

float4 vs(uint lineid : SV_INSTANCEID, uint vertexid : SV_VERTEXID) : SV_Position {
	float3 pos = lines[lineid].p[vertexid];
	return mul(float4(pos, 1), projection);
}

float4 ps(float4 pos : SV_Position) : SV_Target {
	return float4(1,1,1,1);
}



