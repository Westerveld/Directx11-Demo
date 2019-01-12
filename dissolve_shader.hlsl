cbuffer CB0
{
	matrix WVPMatrix;
    float4 dirLightCol;
    float4 dirLightPos;
    float4 ambLightCol;
};

cbuffer CB_dissolve
{
	float dissolveAmount;
	float4 borderColor;
};

Texture2D texture0;
Texture2D alphaCutout;
SamplerState sampler0;

struct VOut
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD; 
	float3 normal : NORMAL;
};


VOut DissolveVS(float4 position : POSITION, float2 texcoord : TEXCOORD, float3 normal : NORMAL)
{
	VOut output;

	float4 default_color	= { 1.0,1.0,1.0,1.0 };
	output.position			= mul(WVPMatrix, position);
    output.texcoord			= texcoord;

    
    float diffuse_amount    = dot(dirLightPos.xyz, normal);
    diffuse_amount          = saturate(diffuse_amount);
    output.color            = ambLightCol + (dirLightCol * diffuse_amount);
	output.normal = normal;
	return output;
}


float4 DissolvePS(float4 position : SV_POSITION, float4 color : COLOR, float2 texcoord : TEXCOORD) : SV_TARGET
{
	float4 defCol = texture0.Sample(sampler0, texcoord);
	float noiseSample = alphaCutout.Sample(sampler0, texcoord).w;
	clip(noiseSample - dissolveAmount  < 0.01f ? -1 : 1);
	float4 emissive = { 0,0,0,0 };
	if (noiseSample - dissolveAmount < 0.05f)
	{
		emissive = borderColor;
	}
	return (color + emissive) * defCol;
}