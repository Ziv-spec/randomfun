


cbuffer vs_constants : register(b0) {
	float inv_window_width; 
	float inv_window_height;
	float window_height;
};

struct widget { 
	float4 rect;
	uint flags;
	float hot_t; 
	float active_t;
};

StructuredBuffer<widget> widgets : register(t0);

struct PS_INPUT {
	float4 pos : SV_Position;
	float4 size : SIZE;
	uint flags : FLAGS;
	float hot_t : HOT_TIME;
};


PS_INPUT vs_main(uint widgetid : SV_INSTANCEID, uint vertexid : SV_VERTEXID) {

	//uint2 i = { vertexid & 2, (vertexid << 1 & 2) + 1 };
    uint2 i = { vertexid & 2, (vertexid << 1 & 2) ^ 3 };

	float4 rect = widgets[widgetid].rect;
	float4 pos = float4(rect[i.x]*inv_window_width-1, 
						rect[i.y]*inv_window_height+1, 
						0., 1.);

	PS_INPUT o = { pos, rect.x, rect.y, (rect.z-rect.x), (rect.w-rect.y),  widgets[widgetid].flags, widgets[widgetid].hot_t };
	return o;
}


float BoarderdRectSDF(float2 pixel, float2 size, float radius) {
	float2 abs_point = float2(abs(pixel.x), abs(pixel.y)); 
	return length(max(abs_point-size+radius, 0.))-radius;
}

float4 ps_main(PS_INPUT o) : SV_Target {

	float4 foreground_color = float4(1., 0., 0., 1.);
	float4 hot_color = float4(1., 0., 1., 1.);
	float4 active_color = float4(1., 0., 0., 1.);
	float border = 5; // size








	float radius = 0; // currently I don't care about rounded corners

	float2 size = float2(o.size.z, o.size.w);
	float2 loc = float2(o.size.x, o.size.y);
	float2 pos2 = float2(o.pos.x ,o.pos.y);

	float distance = BoarderdRectSDF(pos2-loc-size*.5, (size-border)*.5, radius);
	float clamped_distance = clamp(round(distance), 0, 1.0);

	float4 border_color = float4(0., 0., 0., 1.);


	float4 box_color = foreground_color; // float4(1., 0., 0., 1.);
	
	// show or don't show border
	if (!(o.flags & (1<<6))) {
		border_color = box_color;
	}
	
	float hot_t = o.hot_t;
	if (!(o.flags & (1<<7))) { // !UI_ANIMATE_HOT
		hot_t = 0;
	}
	box_color = lerp(box_color, hot_color, hot_t);


	float4 color = lerp(box_color, border_color, clamped_distance);

	if (color.a <= 0) discard;

	return color;
}