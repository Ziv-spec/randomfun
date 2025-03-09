

float4 vs(uint vid : SV_VERTEXID) : SV_Position {
	if (vid == 0) return float4(-1, -1, 0, 1); 
	if (vid == 1) return float4(-1, 1, 0, 1); 
	if (vid == 2) return float4(1, -1, 0, 1); 
	if (vid == 3) return float4(1, 1, 0, 1); 
	return float4(0,0,0,0);		
}


sampler sampler0 : register(s0);
Texture2D<float> mask : register(t0);

cbuffer ps_constant_buffer : register(b0) {
	float2 inv_size;
};

float2 ps(float4 pos : SV_Position) : SV_Target {

	//  This shader pass samples 9 texels in a grid and gets the distance from the invoking pixel to the position stored in each of those 9 texels, then outputs the value of the closest one.

	float smallest_dist = 1.#INF;
	float2 correct_uv = float2(0, 0);

	int step = 2; // TODO(ziv): not a constant!!!
	// find the closest one to the pixel itself
	for (int u = -1; u <= 1; u++) {
		for (int v = -1; v <= 1; v++) {
			float2 offset_uv = clamp(float2(pos.x+u*step, pos.y+v*step)*inv_size, float2(0,0), float2(1, 1) );
	
			// decode position from buffer 
			float2 decoded_pos = mask.Sample(sampler0, offset_uv);

			float dist = distance(decoded_pos, offset_uv);
			
			if (dist < smallest_dist && decoded_pos.x != 0 && decoded_pos.y != 0) {
				smallest_dist = dist; 
				correct_uv = offset_uv;
			}
		} 
	}

	return isinf(smallest_dist) ? float2(0,0) : correct_uv;
}