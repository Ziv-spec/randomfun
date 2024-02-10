

cbuffer CBuf { 
	float4 face_colors[6];
};

struct PS_INPUT { 
	float4 pos : SV_Position;
	float3 color : Color;
}; 


float4 main(uint tid : SV_PrimitiveID) : SV_Target
{
	return face_colors[tid / 2];
}