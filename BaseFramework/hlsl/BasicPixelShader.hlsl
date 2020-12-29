#include "BasicShader.hlsli"

float4 main(VS_OUT input) : SV_TARGET
{
	return float4(input.uv,1,1);
}