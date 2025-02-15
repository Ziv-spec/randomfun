
cbuffer vs_constant_buffer : register(b0) {
	row_major float4x4 transform;  // model-view transform
	row_major float4x4 projection;
};

struct Vertex {
	float3 pos : Position;
	float3 norm : Normal;
	float2 uv : Texture;
};

float4  vs(Vertex v) : SV_Position {
	return mul(float4(v.pos, 1), mul(transform, projection));
}

float4 ps(float4 p : SV_Position) : SV_Target {

	return float4(1, 1, 1, 1);
}
