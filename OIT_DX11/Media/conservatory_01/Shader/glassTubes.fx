/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

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
			  float4    MultiSampleCount;           // .x == multisample count; .y = 1.0/x
};

struct VS_INPUT
{
    float3 Position : POSITION; // Projected position
    float3 Normal   : NORMAL;
    float2 UV0      : TEXCOORD0;
    float2 UV1      : TEXCOORD1;
};

// -------------------------------------
struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float3 Normal   : NORMAL;
    float2 UV0      : TEXCOORD0;
    float2 UV1      : TEXCOORD1;
    float4 LightUV       : TEXCOORD2;
    float3 WorldPosition : TEXCOORD3; // Object space position 
    float3 Reflection : TEXCOORD4;
};

// -------------------------------------
#ifdef _CPUT
    SamplerState SAMPLER0 : register( s0 );
    SamplerComparisonState SHADOW_SAMPLER : register( s1);
    Texture2D texture_AO : register( t0 );
    Texture2D texture_DM : register( t1 );
    Texture2D texture_RM : register( t2 );
//    TextureCube texture_RM : register( t2 );
    Texture2D texture_SM : register( t3 );
    Texture2D _Shadow : register( t4 );
#else
    texture2D texture_AO < string Name = "texture_AO"; string UIName = "texture_AO"; string ResourceType = "2D";>;
    sampler2D SAMPLER0 = sampler_state{ texture = (texture_AO);};
    texture2D texture_DM < string Name = "texture_DM"; string UIName = "texture_DM"; string ResourceType = "2D";>;
    sampler2D SAMPLER1 = sampler_state{ texture = (texture_DM);};
    texture2D texture_RM < string Name = "texture_RM"; string UIName = "texture_RM"; string ResourceType = "2D";>;
    texture2D SAMPLER2 = sampler_state{ texture = (texture_RM);};
//    textureCUBE texture_RM < string Name = "texture_RM"; string UIName = "texture_RM"; string ResourceType = "2D";>;
//    samplerCUBE SAMPLER2 = sampler_state{ texture = (texture_RM);};
    texture2D texture_SM < string Name = "texture_SM"; string UIName = "texture_SM"; string ResourceType = "2D";>;
    sampler2D SAMPLER3 = sampler_state{ texture = (texture_SM);};
#endif

// -------------------------------------
float4 DIFFUSE( PS_INPUT input )
{
    return 
#ifdef _CPUT
texture_DM.Sample( SAMPLER0, (((input.UV0)) *(1)) )
#else
Tex2D( SAMPLER1, (((input.UV0)) *(1)) )
#endif
;
}

// -------------------------------------
float4 SPECULARTMP( PS_INPUT input )
{
    return 
#ifdef _CPUT
texture_SM.Sample( SAMPLER0, (((input.UV0)) *(1)) )
#else
Tex2D( SAMPLER3, (((input.UV0)) *(1)) )
#endif
;
}

// -------------------------------------
float4 EMISSIVETMP( PS_INPUT input )
{
    return 
#ifdef _CPUT
texture_RM.Sample( SAMPLER0, (input.Reflection)  )
#else
Tex2D( SAMPLER2, (input.Reflection)  )
#endif

//#ifdef _CPUT
//texture_RM.Sample( SAMPLER0, (input.Reflection) )
//#else
//TexCUBE( SAMPLER2, (input.Reflection) )
//#endif
;
}

// -------------------------------------
float4 AMBIENTOCC( PS_INPUT input )
{
    return 
#ifdef _CPUT
texture_AO.Sample( SAMPLER0, (((input.UV1)) *(1)) )
#else
Tex2D( SAMPLER0, (((input.UV1)) *(1)) )
#endif
;
}

// -------------------------------------
float4 EMISSIVE( PS_INPUT input )
{
    return (EMISSIVETMP(input)) *(3);
}

// -------------------------------------
float4 SPECULAR( PS_INPUT input )
{
    return (SPECULARTMP(input)) *(2);
}

// -------------------------------------
float4 AMBIENT( PS_INPUT input )
{
    return ((DIFFUSE(input)) *(AMBIENTOCC(input))) *(5);
}

// -------------------------------------
float ComputeShadowAmount( PS_INPUT input )
{
#ifdef _CPUT
    float3  lightUV = input.LightUV.xyz / input.LightUV.w;
    lightUV.xy = lightUV.xy * 0.5f + 0.5f; // TODO: Move to matrix?
    lightUV.y  = 1.0f - lightUV.y;
    float  shadowAmount      = _Shadow.SampleCmp( SHADOW_SAMPLER, lightUV, lightUV.z );
    return shadowAmount;
#else
    return 1.0f;
#endif
}

// -------------------------------------
PS_INPUT VSMain( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;

    output.Position      = mul( float4( input.Position, 1.0f), WorldViewProjection );
    output.WorldPosition = mul( float4( input.Position, 1.0f), World ).xyz;

    // TODO: transform the light into object space instead of the normal into world space
    output.Normal   = mul( input.Normal, (float3x3)World );
    output.UV0 = input.UV0;
    output.UV1 = input.UV1;
    output.LightUV = mul( float4( input.Position, 1.0f), LightWorldViewProjection );

    return output;
}

// -------------------------------------
float4 PSMain( PS_INPUT input ) : SV_Target
{
    float4 result = float4(0,0,0,1);

    float3 normal = normalize(input.Normal);

    // Specular-related computation
    float3 eyeDirection  = normalize(input.WorldPosition - InverseView._m30_m31_m32);
    input.Reflection    = normalize(reflect( eyeDirection, normal ));

    // Emissive-related computation
    float3 emissive = EMISSIVE(input);
    result.xyz +=  emissive;
    float  shadowAmount = ComputeShadowAmount(input);

    // Ambient-related computation
    float3 ambient = AmbientColor * AMBIENT(input);
    result.xyz +=  ambient;
#ifdef _CPUT
   float3 lightDirection = -LightDirection;
#else
   float3 lightDirection = LightDirection;
#endif

    // Diffuse-related computation
    float  nDotL         = saturate( dot( normal, lightDirection ) );
    float3 diffuse       = LightColor * nDotL * shadowAmount  * DIFFUSE(input);
    result.xyz += diffuse;
    float  rDotL         = saturate(dot( input.Reflection, lightDirection ));
    float3 specular      = pow(rDotL,  8.0f );
    specular             = shadowAmount * specular;
    specular            *= SPECULAR(input);
    result.xyz += LightColor * specular;
    return result;
}

// -------------------------------------
technique DefaultTechnique
{
    pass pass1
    {
        VertexShader        = compile vs_3_0 VSMain();
        PixelShader         = compile ps_3_0 PSMain();
        ZWriteEnable        = true;
    }
}

