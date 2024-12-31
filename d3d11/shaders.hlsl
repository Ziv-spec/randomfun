
cbuffer vs_constant_buffer : register(b0) {
	row_major float4x4 transform;  // model-view transform
	row_major float4x4 projection;
	float4x4 normal_transform;
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
	float3 eye_position : Position; 
	float3 normal : Normal;
};

// TODO(ziv): add schene directional light + support for multiple point lights

PS_INPUT vs_main(VS_INPUT v) {
	PS_INPUT output; 

	output.eye_position = mul(float4(v.pos, 1), transform).xyz; 
	output.normal = mul(normal_transform, float4(v.norm, 0)).xyz; 
	output.pos = mul(float4(v.pos, 1), mul(transform, projection));
	output.uv = v.uv;

	return output;
}


cbuffer ps_constnat_buffer : register(b0) {
	float3 point_light_position;
	float3 sun_light_direction;
}; 

float4 ps_main(PS_INPUT p) : SV_Target {
	
	// Understanding thje phong model:
	// attenuation * light * (k_d * cos(theta) + K_s * cos(phi)^alpha + k_am * ambient)

	// directional light
	float4 directional_light_color;
	{
		float ambient = 0.1f; 
		float diffuse = max(dot(sun_light_direction, p.normal), 0);
		float specular = pow(max(dot(sun_light_direction, p.normal), 0), 32);
		directional_light_color = (ambient + diffuse + specular); 
	}
	

	// point light 
	float4 point_light_color;
	{	

		float3 light_direction = normalize(point_light_position - p.eye_position);
		float3 view_direction  = normalize(-p.eye_position); // view in the direction of the camera

		// specular constant
		float alpha = 128.;

		float ambient = 0.1f;
		float diffuse = max(dot(light_direction, p.normal), 0);
		float specular_factor = max(0., dot(normalize(light_direction + view_direction), normalize(p.normal)));
		float specular = pow(specular_factor, alpha);
	
		// attenuation constants
		float constant = 1.; 
		float llinear = 0.09;
		float quadratic = 0.0032;

		float d = length(p.eye_position); // distance from fragment
		float attenuation = 1 / (constant + llinear * d + quadratic * d * d);

		point_light_color = (ambient + diffuse + specular)* attenuation;
	}

	float4 color = texture0.Sample(sampler0, p.uv) * (point_light_color + directional_light_color);
	color.a = 1;	
	return color;
}
