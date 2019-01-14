 cbuffer CB0
{
	matrix WVPMatrix;
	float4 worldView;
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

Texture2D texture0;
SamplerState sampler0;

struct VOut
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD; 
	float3 normal : NORMAL;
};

struct Material
{
	float3 normal;
	float4 diffuseColor;
	float specExp;
	float specIntensity;
};


VOut ModelVS(float4 position : POSITION, float2 texcoord : TEXCOORD, float3 normal : NORMAL)
{
	VOut output;

	float4 default_color	= { 1.0,1.0,1.0,1.0 };
	output.position			= mul(WVPMatrix, position);
    output.texcoord			= texcoord;

    
    float diffuse_amount    = dot(dirLightPos.xyz, normal);
    diffuse_amount          = saturate(diffuse_amount);
	output.color			   = (default_color.rgb * default_color.rgb, default_color.a);
	output.normal			= normal;
	return output;
}

float3 CalcAmbLight(float3 normal, float3 color)
{
	//Convert from -1, 1 to 0, 1
	float3 AmbientDown = ambLightCol.rgb;
	float3 AmbientUp = (0.2f, 0.2f, 0.2f);
	float up = normal.y * 0.5 + 0.5;
	float3 ambient = AmbientDown + up * AmbientUp;

	return ambient * color;
}

float3 CalcDirLight(float3 position, Material mat)
{
	//Phong Diffuse
	float NDotL = dot(dirLightPos.xyz, mat.normal);
	float3 finalColor = dirLightCol.rgb * saturate(NDotL);

	//Blinn specular
	float3 ToEye = worldView.xyz - position;
	ToEye = normalize(ToEye);
	float3 Halfway = normalize(ToEye + dirLightPos.xyz);
	float NDotH = saturate(dot(Halfway, mat.normal));
	finalColor += dirLightCol.rgb * pow(NDotH, mat.specExp) * mat.specIntensity;
	return finalColor * mat.diffuseColor.rgb;
}

float3 CalcPointLight(float3 position, Material mat)
{
	float3 ToLight = pointLightPos.xyz - position;
	float3 ToEye = worldView.xyz - position;
	float DistToLight = length(ToLight);

	//Phong Diffuse
	ToLight /= DistToLight;
	float NDotL = saturate(dot(ToLight, mat.normal));
	float3 finalColor = pointLightCol.rgb * NDotL;

	//Blinn Specular
	ToEye = normalize(ToEye);
	float3 Halfway = normalize(ToEye + ToLight);
	float NDotH = saturate(dot(Halfway, mat.normal));
	finalColor += pointLightCol.rgb * pow(NDotH, mat.specExp) * mat.specIntensity;

	//Attenuation
	float DistToLightNorm = 1.0 - saturate(DistToLight * pointLightRange);
	float Attn = DistToLightNorm * DistToLightNorm;

	finalColor *= mat.diffuseColor * Attn;

	return finalColor;
}

float3 CalcSpotLight(float3 position, Material mat)
{
	float3 ToLight = spotLightPos.xyz - position;
	float3 ToEye = worldView.xyz - position;
	float DistToLight = length(ToLight);

	//Phong Diffuse
	ToLight /= DistToLight; //Normalise
	float NDotL = saturate(dot(ToLight, mat.normal));
	float3 finalColor = spotLightCol.rgb * NDotL;

	//Blinn Specular
	ToEye = normalize(ToEye);
	float3 Halfway = normalize(ToEye + ToLight);
	float NDotH = saturate(dot(Halfway, mat.normal));
	finalColor += spotLightCol.rgb * pow(NDotH, mat.specExp) * mat.specIntensity;

	float cosAng = cos(NDotL);
	//Cone attenuation
	float coneAtt = saturate((cosAng - spotLightOuterCone) * spotLightInnerCone);
	coneAtt *= coneAtt;

	//Attenuation
	float DistToLightNorm = 1.0 - saturate(DistToLight * spotLightRange);
	float Attn = DistToLightNorm * DistToLightNorm;
	finalColor *= mat.diffuseColor * Attn * coneAtt;

	return finalColor;

}

float4 ModelPS(in VOut IN) : SV_TARGET
{
	Material mat;
	mat.normal = IN.normal;
	mat.diffuseColor = IN.color;
	mat.specExp = 0.0f;
	mat.specIntensity = 0.1f;
	float4 finalColor;
	finalColor = (CalcAmbLight(mat.normal, mat.diffuseColor.rgb), mat.diffuseColor.a);
	finalColor.rgb += CalcDirLight(IN.position.xyz, mat);
	finalColor.rgb += CalcPointLight(IN.position.xyz, mat);
	finalColor.rgb += CalcSpotLight(IN.position.xyz, mat);
	return texture0.Sample(sampler0, IN.texcoord) * finalColor;
}