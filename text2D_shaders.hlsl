Texture2D texture0;
SamplerState sampler0;

struct VOut
{
	float4 position : SV_POSITION;
	float2 texcoord :TEXCOORD;
};

VOut TextVS(float4 position : POSITION,  float2 texcoord : TEXCOORD)
{
	VOut output;

	output.position = position;

	output.texcoord = texcoord;

	return output;
}


 float4 TextPS(float4 position : SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET 
 {
	return texture0.Sample(sampler0, texcoord);
 }