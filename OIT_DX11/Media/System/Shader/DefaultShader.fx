/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

// ********************************************************************************************************
struct VS_INPUT
{
    float3 Pos      : POSITION; // Projected position
    float3 Norm     : NORMAL;
    float2 Uv       : TEXCOORD0;
};
struct PS_INPUT
{
    float4 Pos      : SV_POSITION;
    float3 Norm     : NORMAL;
    float2 Uv       : TEXCOORD0;
    float4 LightUv  : TEXCOORD1;
    float3 Position : TEXCOORD2; // Object space position 
};
// ********************************************************************************************************
    Texture2D    TEXTURE0 : register( t0 );
    SamplerState SAMPLER0 : register( s0 );
    Texture2D    _Shadow  : register( t1 );
    SamplerComparisonState SAMPLER1 : register( s1 );
// ********************************************************************************************************
// -------------------------------------
cbuffer cbPerModelValues
{
    row_major float4x4 World : WORLD;
    row_major float4x4 NormalMatrix : WORLD;
    row_major float4x4 WorldViewProjection : WORLDVIEWPROJECTION;
    row_major float4x4 InverseWorld : WORLDINVERSE;
    row_major float4x4 LightWorldViewProjection;
              float4   BoundingBoxCenterWorldSpace  < string UIWidget="None"; >;
              float4   BoundingBoxHalfWorldSpace    < string UIWidget="None"; >;
              float4   BoundingBoxCenterObjectSpace < string UIWidget="None"; >;
              float4   BoundingBoxHalfObjectSpace   < string UIWidget="None"; >;
};

// -------------------------------------
cbuffer cbPerFrameValues
{
    row_major float4x4  View;
    row_major float4x4  InverseView : ViewInverse	< string UIWidget="None"; >;
    row_major float4x4  Projection;
    row_major float4x4  ViewProjection;
              float4    AmbientColor < string UIWidget="None"; > = .20;
              float4    LightColor < string UIWidget="None"; >   = 1.0f;
              float4    LightDirection  : Direction < string UIName = "Light Direction";  string Object = "TargetLight"; string Space = "World"; int Ref_ID=0; > = {0,0,-1, 0};
              float4    EyePosition;
              float4    TotalTimeInSeconds < string UIWidget="None"; >;
};

// ********************************************************************************************************
PS_INPUT VSMain( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos      = mul( float4( input.Pos, 1.0f), WorldViewProjection );
    output.Position = mul( float4( input.Pos, 1.0f), World ).xyz;
    // TODO: transform the light into object space instead of the normal into world space
    output.Norm = mul( input.Norm, (float3x3)World );
    output.Uv   = float2(input.Uv.x, input.Uv.y);
    output.LightUv   = mul( float4( input.Pos, 1.0f), LightWorldViewProjection );
    return output;
}
// ********************************************************************************************************
float4 PSMain( PS_INPUT input ) : SV_Target
{
    float3  lightUv = input.LightUv.xyz / input.LightUv.w;
    lightUv.xy = lightUv.xy * 0.5f + 0.5f; // TODO: Move scale and offset to matrix.
    lightUv.y  = 1.0f - lightUv.y;
    float   shadowAmount = _Shadow.SampleCmp( SAMPLER1, lightUv, lightUv.z );
    float3 normal         = normalize(input.Norm);
    float  nDotL          = saturate( dot( normal, -LightDirection ) );
    float3 eyeDirection   = normalize(input.Position - InverseView._m30_m31_m32);
    float3 reflection     = reflect( eyeDirection, normal );
    float  rDotL          = saturate(dot( reflection, -LightDirection ));
    float3 specular       = pow(rDotL, 16.0f);
    specular              = min( shadowAmount, specular );
    float4 diffuseTexture = TEXTURE0.Sample( SAMPLER0, input.Uv );
    float ambient = 0.05;
    float3 result = (min(shadowAmount, nDotL)+ambient) * diffuseTexture + shadowAmount*specular;
    return float4( result, 1.0f );
}

// ********************************************************************************************************
struct VS_INPUT_NO_TEX
{
    float3 Pos      : POSITION; // Projected position
    float3 Norm     : NORMAL;
};
struct PS_INPUT_NO_TEX
{
    float4 Pos      : SV_POSITION;
    float3 Norm     : NORMAL;
    float4 LightUv  : TEXCOORD1;
    float3 Position : TEXCOORD0; // Object space position 
};
// ********************************************************************************************************
PS_INPUT_NO_TEX VSMainNoTexture( VS_INPUT_NO_TEX input )
{
    PS_INPUT_NO_TEX output = (PS_INPUT_NO_TEX)0;
    output.Pos      = mul( float4( input.Pos, 1.0f), WorldViewProjection );
    output.Position = mul( float4( input.Pos, 1.0f), World ).xyz;
    // TODO: transform the light into object space instead of the normal into world space
    output.Norm = mul( input.Norm, (float3x3)World );
    output.LightUv   = mul( float4( input.Pos, 1.0f), LightWorldViewProjection );
    return output;
}
// ********************************************************************************************************
float4 PSMainNoTexture( PS_INPUT_NO_TEX input ) : SV_Target
{
    float3 lightUv = input.LightUv.xyz / input.LightUv.w;
    float2 uv = lightUv.xy * 0.5f + 0.5f;
    float2 uvInvertY = float2(uv.x, 1.0f-uv.y);
    float shadowAmount = _Shadow.SampleCmp( SAMPLER1, uvInvertY, lightUv.z );
    float3 eyeDirection = normalize(input.Position - InverseView._m30_m31_m32);
    float3 normal       = normalize(input.Norm);
    float  nDotL = saturate( dot( normal, -normalize(LightDirection.xyz) ) );
    nDotL = shadowAmount * nDotL;
    float3 reflection   = reflect( eyeDirection, normal );
    float  rDotL        = saturate(dot( reflection, -LightDirection.xyz ));
    float  specular     = 0.2f * pow( rDotL, 4.0f );
    specular = min( shadowAmount, specular );
    return float4( (nDotL + specular).xxx, 1.0f);
}



