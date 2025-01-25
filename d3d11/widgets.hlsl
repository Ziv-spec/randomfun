
cbuffer vs_constants : register(b0) {
	float inv_window_width; 
	float inv_window_height;
	float window_height;
};

struct widget { 
	float4 rect;
	float4 color; 
	float radius;
	float border;
};

StructuredBuffer<widget> widgets : register(t0);

struct PS_INPUT {
	float4 pos : SV_Position;
	float4 size :  SIZE;
	float4 color : COLOR; 
	float radius : RAIDUS;
	float border : BORDER;
};

PS_INPUT vs_main(uint widgetid : SV_INSTANCEID, uint vertexid : SV_VERTEXID) {

    uint2 i = { vertexid & 2, (vertexid << 1 & 2) ^ 3 };

	float4 rect = widgets[widgetid].rect;
	float4 pos = float4(rect[i.x]*inv_window_width-1,
						rect[i.y]*inv_window_height+1,
						0., 1.);

	PS_INPUT o = { 
		pos, 
		rect.x, rect.y, 
		(rect.z-rect.x), (rect.w-rect.y), 
		widgets[widgetid].color, 
		widgets[widgetid].radius, 
		widgets[widgetid].border 
	};
	return o;
}


// https://iquilezles.org/articles/distfunctions2d/
float BoarderdRectSDF(float2 pixel, float2 halfsize, float radius) {
	return length(max(abs(pixel)-halfsize+radius, 0.))-radius;
}

float4 ps_main(PS_INPUT o) : SV_Target {

	float2 size = float2(o.size.z, o.size.w);
	float2 loc = o.size.xy;
	float2 pos2 = o.pos.xy;

	float2 half_size = size * 0.5;
	float distance  = BoarderdRectSDF(pos2-loc-half_size, size-half_size-o.border, max(o.radius-o.border, 0));
	float t_rect_inside = smoothstep(0, .1, distance); 
	
	float distance2 = BoarderdRectSDF(pos2-loc-half_size, size-half_size, o.radius);
	float t_rect_outside = 1-smoothstep(-0.75, 0.5, distance2); 

	float4 background_color = o.color;
	float4 color = lerp(background_color, float4(0.1,0.2,0.3,1), t_rect_inside);
	color.a *= t_rect_outside;

	if (color.a == 0 ) discard;

	return color;
}
