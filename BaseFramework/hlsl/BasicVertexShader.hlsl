#include "BasicShader.hlsli"

VS_OUT main(float4 pos : POSITION, float2 uv : TEXCOORD)
{
	VS_OUT output;
	output.svpos = pos;
	output.uv = uv;
	return output;
}