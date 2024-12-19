
cbuffer constants : register(b0)
{
    float2 rn_screensize;
    float2 r_atlassize;
}

//~

struct sprite
{
    float2 screenpos;
    float2 size;
    int2 atlaspos;
};

struct pixel
{
    float4 xy : SV_POSITION;
    float2 uv : UV;
};

//~

StructuredBuffer<sprite> spritebuffer : register(t0);
Texture2D<float4>        atlastexture : register(t1);

SamplerState             pointsampler : register(s0);


//~


pixel vs_main(uint spriteid : SV_INSTANCEID, uint vertexid : SV_VERTEXID)
{
    sprite spr = spritebuffer[spriteid];

    float4 pos = float4(spr.screenpos, spr.screenpos + spr.size);
    float4 tex = float4(spr.atlaspos,  spr.atlaspos  + float2(5, 7)); // known character size (5,7)

    uint2 i = { vertexid & 2, (vertexid << 1 & 2) ^ 3 };

    pixel p;

    p.xy = float4(float2(pos[i.x], pos[i.y]) * rn_screensize - float2(1, -1), 0, 1);
    p.uv =        float2(tex[i.x], tex[i.y]) * r_atlassize;

    return p;
}

float4 ps_main(pixel p) : SV_TARGET
{

    float4 color = atlastexture.Sample(pointsampler, p.uv);

    if (color.a == 0) discard;

    return color;
}
