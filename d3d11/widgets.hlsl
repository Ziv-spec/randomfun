


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

	//uint2 i = { vertexid & 2, (vertexid << 1 & 2) + 1 };
    uint2 i = { vertexid & 2, (vertexid << 1 & 2) ^ 3 };

	float4 rect = widgets[widgetid].rect;
	float4 pos = float4(rect[i.x]*inv_window_width-1, 
						rect[i.y]*inv_window_height+1, 
						0., 1.);

	PS_INPUT o = { pos, rect.x, rect.y, (rect.z-rect.x), (rect.w-rect.y),  widgets[widgetid].color, widgets[widgetid].radius, widgets[widgetid].border };
	return o;
}


float BoarderdRectSDF(float2 pixel, float2 size, float radius) {
	float2 abs_point = float2(abs(pixel.x), abs(pixel.y)); 
	return length(max(abs_point-size+radius, 0.))-radius;
}

float4 ps_main(PS_INPUT o) : SV_Target {

	float2 size = float2(o.size.z, o.size.w);
	float2 loc = float2(o.size.x, o.size.y);
	float2 pos2 = float2(o.pos.x ,o.pos.y);


	float distance_to_shadow = BoarderdRectSDF(pos2-loc-size*.5, (size)*.5, o.radius);
	float clamped_shadow = clamp(distance_to_shadow, 0, 1.0);

	float distance_to_rect = BoarderdRectSDF(pos2-loc-size*.5, (size-o.border)*.5, o.radius);
	float clamped_rect = clamp(distance_to_rect, 0, 1.0);
	
	float4 background_color = o.color; 
	float4 border_color = float4(0,0,0,1);
	float4 no_color = float4(0.,0.,0.,0.);

	float4 shadow = lerp(border_color, no_color, clamped_shadow);
	float4 rect = lerp(background_color, no_color, clamped_rect);
	float4 color = lerp(rect, shadow, clamped_rect);

	if (color.a <= 0) discard;

	return color;
}