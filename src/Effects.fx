cbuffer perObject
{
	float4x4 WVP;
};

Texture2D ObjTexture;
SamplerState ObjSamplerstate;

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 TexCoord : ObjTexture;
};


VS_OUTPUT VS(float4 inPos : POSITION, float2 inTexture : TEXCOORD)
{
	VS_OUTPUT output;

    output.Pos = mul(inPos, WVP);
    output.TexCoord = inTexture;

    return output;
}	

float4 PS(VS_OUTPUT input) : SV_TARGET
{
    return ObjTexture.Sample(ObjSamplerstate, input.TexCoord);
}