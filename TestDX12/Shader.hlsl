cbuffer cbuff0 : register(b0)
{
	matrix _MVP;
}


Texture2D<float4> _Texture : register(t0);
SamplerState _Sampler : register(s0);


struct I2V
{
	float4 Position : POSITION;
	float2 Texcord : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float4 Color : COLOR;
};
struct V2F
{
	float4 ScreenPosition : SV_POSITION;
	float4 Position : POSITION;
	float2 Texcord : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float4 Color : COLOR;
};

V2F vs(I2V i2v)
{
	V2F v2f;
	v2f.ScreenPosition = mul(_MVP, i2v.Position);
	v2f.Position = mul(_MVP, i2v.Position);
	v2f.Texcord = i2v.Texcord;
	v2f.Normal = i2v.Normal;
	v2f.Tangent = i2v.Tangent;
	v2f.Color = i2v.Color;
	return v2f;
}

float4 ps(V2F v2f) : SV_TARGET
{
	float4 v = float4(v2f.Texcord, 0, 1);
	float4 c = _Texture.Sample(_Sampler, v2f.Texcord);
	// return c;
	// return c.bgra;
	float4 pos = float4(-v2f.Position.x * v2f.Position.y, v2f.ScreenPosition.x * v2f.ScreenPosition.y / 800 / 800, 0, 1);
	return c * pos;
}
