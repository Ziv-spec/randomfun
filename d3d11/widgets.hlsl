


cbuffer constants : register(b0) {
	float inv_window_width; 
	float inv_window_height;
};

struct widget { 
	int4 b;
}; 

StructuredBuffer<widget> widgets : register(t0);
StructuredBuffer<float4> colors : register(t1);

struct INPUT {
	float4 pos : SV_Position;
	float4 color : COLOR;
};


INPUT vs_main(uint widgetid : SV_INSTANCEID, uint vertexid : SV_VERTEXID) {

	int4 b = widgets[widgetid].b;
	uint2 i = { vertexid & 2, (vertexid << 1 & 2) + 1 };
	
	//0  0 1
	//1  0 3
	//2  2 1
	//3  2 3
	
	float4 p = float4((b[i.x]*inv_window_width)*2 -1., (b[i.y]*inv_window_height)*2 -1., 0., 1.);
	INPUT o = { p, colors[widgetid] };
	return o;
}


float4 ps_main(INPUT o) : SV_Target {
  //	float4 color = float4(((o.color >> 16) & 0xff)/255.,((o.color >> 8) & 0xff)/255., (o.color & 0xff)/255., 1.);
	return o.color;
}
