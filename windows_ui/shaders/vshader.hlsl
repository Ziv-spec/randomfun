
cbuffer CBuf {
	row_major matrix transform;
};

float4 main(float2 pos : Position) : SV_Position
{
	return mul(float4(pos, 0, 1), transform);
	//return float4(pos, 0, 1);
}