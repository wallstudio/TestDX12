
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
	float4 Position : SV_POSITION;
	float2 Texcord : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float4 Color : COLOR;
};

V2F vs(I2V i2v)
{
	V2F v2f;
	v2f.Position = i2v.Position;
	v2f.Texcord = i2v.Texcord;
	v2f.Normal = i2v.Normal;
	v2f.Tangent = i2v.Tangent;
	v2f.Color = i2v.Color;
	return v2f;
}

float4 ps(V2F v2f) : SV_TARGET
{
	return float4(v2f.Texcord, 0, 1);
}