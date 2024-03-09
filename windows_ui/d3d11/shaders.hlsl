
cbuffer constant_buffer : register(b0) {
	float4x4 transform;  // model-view transform
	float4x4 projection;
	float4x4 normal_transform;
	float3 lightposition;
};

sampler sampler0 : register(s0);
Texture2D texture0 : register(t0);

struct VS_INPUT {
	float3 pos : Position;
	float3 norm : Normal;
	float2 uv : Texture;
};

struct PS_INPUT {
	float4 pos : SV_Position;
	float2 uv : Texture;
	float3 light_position : Light_Position; // TODO(ziv): turn to ps constant 
	float3 eye_position : Position; 
	float3 normal : Normal;
};

PS_INPUT vs_main(VS_INPUT v) {
	PS_INPUT output; 
	
	// NOTE(ziv): light_direction is inverted aka -light_direction
	output.eye_position = mul(transform, float4(v.pos, 1)).xyz; 
	output.light_position = lightposition;
	// TODO(ziv): check to see if I need to normzlie the vector
	output.normal = mul(normal_transform, float4(v.norm, 0)).xyz; 
	output.pos = mul(mul(projection, transform), float4(v.pos, 1));
	output.uv = v.uv;

	return output;
}

float4 ps_main(PS_INPUT p) : SV_Target {
	
	// Understanding the phong model:
	// light * (k_d * cos(theta) + K_s * cos(phi)^alpha)+ k_am * ambient
	// theta = dot(v_normal, -lightdirection)
	// phi   = dot( normalize(-lightdirection + eyedirection) , view_dir)    or     dot(reflect(-lightdirection, v_normal), view_dir)


	// point light 
	float4 point_light_color;
	{	

		float3 light_direction = normalize(p.light_position - p.eye_position);
		float3 view_direction  = normalize(-p.eye_position); // view in the direction of the camera

		// specular constant
		float alpha = 128.;

		float ambient = 0.1f;
		float diffuse = max(dot(light_direction, p.normal), 0);
		float specular_factor = max(0., dot(reflect(-light_direction, p.normal), view_direction));
		float specular = 0.9 * pow(specular_factor, alpha);
	
		// attenuation constants
		float constant = 1.; 
		float llinear = 0.09;
		float quadratic = 0.032;

		float d = length(p.eye_position); // distance from fragment
		float attenuation = 1 / (constant + llinear * d + quadratic * d * d);

		point_light_color = (ambient + diffuse + specular)*texture0.Sample(sampler0, p.uv) * attenuation;
	}

	

	return point_light_color; 
}
