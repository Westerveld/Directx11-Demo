cbuffer CBuffer0
{
	float red_fraction; //4 bytes
	float green_fraction;
	float blue_fraction;
	float alpha;
}

cbuffer CBuffer1
{
	matrix WVPMatrix; //64 bytes
	float vertical;
	float horizontal;
	float width;
	float height;
}

struct VOut
{
	float4 position : SV_POSITION;
	float4 color	: COLOR;
};

VOut VShader(float4 position : POSITION, float4 color : COLOR)
{
	VOut output;

	color.r *= red_fraction;
	color.g *= green_fraction;
	color.b *= blue_fraction;

	output.position = mul(WVPMatrix, position);
	//output.position = position;
	//output.position.x *= horizontal;
	//output.position.y *= vertical;
	//output.position.z = alpha;
	output.color = color;

	return output;
}

float4 PShader(float4 position : SV_POSITION, float4 color : COLOR) : SV_TARGET
{
	return color;
}
