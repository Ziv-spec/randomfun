
cbuffer CBuf {
	matrix transform;
};

struct VS_INPUT {
	float3 pos : Position;
};

struct PS_INPUT { 
	float4 pos : SV_Position;
}; 

PS_INPUT main(VS_INPUT input)
{
	PS_INPUT psi;
	psi.pos = mul(float4(input.pos, 1), transform); // float4(input.pos, 0, 1);
	return psi;
}