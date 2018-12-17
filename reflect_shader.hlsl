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

	float diffuse_amount = dot(dirLightPos.xyz, normal);
	diffuse_amount = saturate(diffuse_amount);
	output.color = ambLightCol + (dirLightCol * diffuse_amount);


    float4 wvpos = mul(worldView, position);
	//output.position = mul(WVPMatrix, position);
    float3 wvnormal = mul((float3x3)worldView, normal);
    wvnormal = normalize(wvnormal);

    float3 eyer = -normalize((float3)wvpos);

    output.texcoord = 2.0 * dot(eyer, wvnormal) * wvnormal - eyer;

	


	return output;
}

float4 ModelPS(in VOut input) : SV_TARGET
{
	return texture0.Sample(sampler0, input.texcoord.xy) * input.color;
}