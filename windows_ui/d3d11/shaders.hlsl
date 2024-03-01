
cbuffer constant_buffer : register(b0) {
	float4x4 transform;
	float4x4 projection;
	float3 lightvector;
};

sampler sampler0 : register(s0);
Texture2D texture0 : register(t0);

struct PS_INPUT {
	float4 pos : SV_Position;
	float2 uv : Texture;
	float4 color : COL;
	float3 light_direction : LIGHT_DIR; 
	float3 normal : Normal;
};

struct VS_INPUT {
	float3 pos : Position;
	float3 norm : Normal;
	float2 uv : Texture;
};

PS_INPUT vs_main(VS_INPUT v) {
	PS_INPUT output; 
	
	float ambient = 0.1f;
	float3 transformed_pos = mul(transform, float4(v.pos, 1)).xyz;
	float3 light_direction = normalize(lightvector - transformed_pos);
	float3 transformed_normal =  normalize(mul(transform, float4(v.norm, 0)).xyz);
	
	output.pos = mul(mul(projection, transform), float4(v.pos, 1));
	output.light_direction = light_direction; 
	output.normal = transformed_normal;
	output.uv = v.uv;
	output.color = float4(1, 1, 1, 1);
	return output;
}

float4 ps_main(PS_INPUT p) : SV_Target {
	float ambient = 0.1f; 
	float diffuse = max(dot(p.light_direction, p.normal), 0);
	
	return texture0.Sample(sampler0, p.uv) * (diffuse + ambient);
}
