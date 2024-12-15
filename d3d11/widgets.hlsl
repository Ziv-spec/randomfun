


cbuffer constants : register(b0) {
	float inv_window_width; 
	float inv_window_height;

	float4 foreground_color; 
	float4 hot_color;
	float4 active_color;
};

struct widget { 
	float4 rect;
	uint flags;
}; 

StructuredBuffer<widget> widgets : register(t0);

struct PS_INPUT {
	float4 pos : SV_Position;
	float4 size : SIZE;
	float2 window : WINDOW;
};


PS_INPUT vs_main(uint widgetid : SV_INSTANCEID, uint vertexid : SV_VERTEXID) {

	uint2 i = { vertexid & 2, (vertexid << 1 & 2) + 1 };

	float4 rect = widgets[widgetid].rect;
	float4 pos = float4((rect[i.x]*inv_window_width*2)-1, 
						(rect[i.y]*inv_window_height*2)-1, 
						0., 1.);

	PS_INPUT o = { pos, rect.x, rect.y, (rect.z-rect.x), (rect.w-rect.y), 1/inv_window_width, 1/inv_window_height };
	return o;
}


float BoardedRectSDF(float2 pixel, float2 size, float border_size) {
	float2 abs_point = float2(abs(pixel.x), abs(pixel.y)); 
	return length(max(abs_point-size, 0.));
}

float4 ps_main(PS_INPUT o) : SV_Target {

	float border = 5; // px
	float2 size = float2(o.size.z, o.size.w);
	float2 loc = float2(o.size.x, o.size.y);
	float2 pos2 = float2(o.pos.x ,o.window.y-o.pos.y);

	float distance = BoardedRectSDF(pos2-loc-size*.5, (size-border)*.5, border);
	float4 color = lerp(float4(1., 0., 0., 1.), float4(0., 0., 0., 0.), distance);

	return color;
}