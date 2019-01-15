cbuffer CB0
{
	matrix WVPMatrix;
    float4 dirLightCol;
    float4 dirLightPos;
    float4 ambLightCol;


	float4 pointLightPos;
	float4 pointLightCol;
	float pointLightRange;

	float4 spotLightPos;
	float4 spotLightDir;
	float4 spotLightCol;
	float spotLightRange;
	float spotLightInnerCone;
	float spotLightOuterCone;
};

cbuffer CB_dissolve
{
	float dissolveAmount;
	float specIntensity;
	float specExp;
	float fill;
};

Texture2D texture0;
Texture2D alphaCutout;
SamplerState sampler0;
SamplerState alphaSampler;

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
	output.color			= default_color;
	output.normal			= normal;
	return output;
}


float4 DissolvePS(float4 position : SV_POSITION, float4 color : COLOR, float2 texcoord : TEXCOORD) : SV_TARGET
{
	float4 defCol = texture0.Sample(sampler0, texcoord);
	float noiseSample = alphaCutout.Sample(alphaSampler, texcoord).x;
	//Discard the pixel if the value is below zero
	clip(noiseSample - dissolveAmount);
	float4 emissive = { 0,0,0,0 };
	//Make the pixel emissive if the value is below 0.05f
	if (noiseSample - dissolveAmount < 0.05)
	{
		emissive = float4(1,1,1,1);
	}
	return (color + emissive) * defCol;
}