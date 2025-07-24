#ifndef _SHADOW_FX_
#define _SHADOW_FX_

#include "params.fx"
#include "utils.fx"

struct VS_IN
{
    float3 pos : POSITION;
    float4 weight : WEIGHT;
    float4 indices : INDICES;
};

struct VS_OUT
{
    float4 pos : SV_Position;
    float4 clipPos : POSITION;
};

VS_OUT VS_Main(VS_IN input)
{
    VS_OUT output = (VS_OUT) 0.f;
    
    float3 finalPos = input.pos;
    
    if (g_int_1 == 1)
    {
        float3 tempNormal = 0.f;
        float3 tempTangent = 0.f;
        Skinning(finalPos, tempNormal, tempTangent, input.weight, input.indices);
    }

    output.pos = mul(float4(finalPos, 1.f), g_matWVP);
    output.clipPos = output.pos;

    return output;
}

float4 PS_Main(VS_OUT input) : SV_Target
{
    return float4(input.clipPos.z / input.clipPos.w, 0.f, 0.f, 0.f);
}

#endif