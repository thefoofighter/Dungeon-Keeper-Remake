#include "Defines.hlsl"

cbuffer ObjectBuffer : register(b0)
{
    float4x4 modelMatrix; //64
};
struct VS_OUTPUT
{
    float4 positionVS : SV_POSITION;
    float2 uv : UV;
};
struct VS_INPUT
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float2 uv : UV;
};
VS_OUTPUT VShader(VS_INPUT input)
{
    VS_OUTPUT output;

    float4 mPos = float4(modelMatrix._41_42_43_44);
    float4x4 scaleMatrix = modelMatrix;
    scaleMatrix._41_42_43_44 = float4(0, 0, 0, 1);

    output.positionVS = mul(scaleMatrix, float4(input.position, 1.0f)); //scaling
    output.positionVS += mPos; //positioning

    output.uv = float2(input.uv);
    return output;
}
