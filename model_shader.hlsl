cbuffer CB0
{
	matrix WVPMatrix;
    float4 dirLightCol;
    float4 dirLightPos;
    float4 ambLightCol;
};

Texture2D texture0;
SamplerState sampler0;

struct VOut
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD; 
	float3 normal : NORMAL;
};


VOut ModelVS(float4 position : POSITION, float2 texcoord : TEXCOORD, float3 normal : NORMAL)
{
	VOut output;

	float4 default_color	= { 1.0,1.0,1.0,1.0 };
	output.position			= mul(WVPMatrix, position);
    output.texcoord			= texcoord;

    
    float diffuse_amount    = dot(dirLightPos.xyz, normal);
    diffuse_amount          = saturate(diffuse_amount);
    output.color            = ambLightCol + (dirLightCol * diffuse_amount);
	return output;
}


float4 ModelPS(float4 position : SV_POSITION, float4 color : COLOR, float2 texcoord : TEXCOORD) : SV_TARGET
{

	return texture0.Sample(sampler0, texcoord) * color;
}