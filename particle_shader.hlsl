cbuffer CB0
{
	matrix WVPMatrix;
	float4 color;
};

struct VOut
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD; 
};

VOut VShader(float4 position : POSITION)
{
	VOut output;

	output.position = mul(WVPMatrix, position);
	output.color = color;
	output.texcoord = position.xy;
	return output;
}

float4 PShader(in VOut input) : SV_TARGET
{
	float distsq = input.texcoord.x * input.texcoord.x + input.texcoord.y * input.texcoord.y;
	clip(1.0f - distsq);
	return input.color;
}