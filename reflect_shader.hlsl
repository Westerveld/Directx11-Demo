cbuffer CB0
{
	matrix WVPMatrix;
	float4 dirLightCol;
	float4 dirLightPos;
	float4 ambLightCol;
};

cbuffer CBShiny
{
	matrix worldView;
};

Texture2D texture0;
TextureCube skyboxTexture;
SamplerState sampler0;

struct VOut
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float3 texcoord : TEXCOORD; 
	float3 normal : NORMAL;
};

VOut ModelVS(float4 position : POSITION, float3 texcoord : TEXCOORD, float3 normal : NORMAL)
{
	VOut output;
	
	output.position = mul(WVPMatrix, position);
	output.normal = normal;
	//output.normal = normalize(output.normal);

	output.color = float4(1, 1, 1, 1);


	float3 wvpos = mul(worldView, position).xyz;
	float3 wvnormal = mul((float3x3)worldView, normal);
	wvnormal = normalize(wvnormal);

	float3 eyer = -normalize(wvpos);
	output.texcoord = 2.0 * dot(eyer, wvnormal) * wvnormal - eyer;

	
	return output;
}

float4 ModelPS(in VOut input) : SV_TARGET
{
	return skyboxTexture.Sample(sampler0, input.texcoord) * texture0.Sample(sampler0, input.texcoord) ;
}