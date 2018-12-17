

cbuffer CBuffer0
{
	matrix WVPMatrix; //64 bytes
    float4 dirLightVector; //16 bytes
    float4 dirLightCol;
    float4 ambLightcol;
}

struct VOut
{
	float4 position : SV_POSITION;
	float4 color	: COLOR;
    float2 texcoord : TEXCOORD;
};

Texture2D texture0;
SamplerState sampler0;

VOut VShader(float4 position : POSITION, float4 color : COLOR, float2 texcoord : TEXCOORD, float3 normal: NORMAL)
{
	VOut output;
       
	output.position = mul(WVPMatrix, position);
    
    float diffuse_amount = dot(dirLightVector.xyz, normal);
    diffuse_amount = saturate(diffuse_amount);

	output.color = ambLightcol + (dirLightCol * diffuse_amount);

    output.texcoord = texcoord;

	return output;
}

float4 PShader(VOut VIn) : SV_TARGET
{
    return VIn.color * texture0.Sample(sampler0, VIn.texcoord);
}
