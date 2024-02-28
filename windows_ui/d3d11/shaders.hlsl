
cbuffer constant_buffer : register(b0) {
	float4x4 transform;
	float3 lightvector;
};

sampler sampler0 : register(s0);
Texture2D texture0 : register(t0);

struct PS_INPUT {
	float4 pos : SV_Position;
	float2 uv : Texture;
	float4 color : COL;
};

struct VS_INPUT {
	float3 pos : Position;
	float3 norm : Normal;
	float2 uv : Texture;
};

PS_INPUT vs_main(VS_INPUT v) {
	PS_INPUT output; 
	
	output.pos = mul(transform, float4(v.pos, 1.f)); 
	output.uv = v.uv;

	float3 tpos = float3(output.pos.x, output.pos.y, output.pos.z);
	float3 tlightvector = mul(transform, float4(lightvector, 1.f)).xyz;
	

	float ambient = 0.1f;
	float3 light_direction = normalize(tlightvector - float3(0, 0, 4) - tpos);
	float diffuse = max(dot(v.norm,light_direction), 0);
	
	float3 color = (diffuse+ambient)*float3(1, 1, 1);
	output.color = float4(color, 1.f);

	return output;
}

float4 ps_main(PS_INPUT p) : SV_Target {
	return texture0.Sample(sampler0, p.uv) * p.color;
}
