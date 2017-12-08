/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

// ********************************************************************************************************
struct VS_INPUT
{
    float3 Pos      : POSITION; // Projected position
};
struct PS_INPUT
{
    float4 Pos      : SV_POSITION;
};

// ********************************************************************************************************
cbuffer cbPerModelValues
{
    row_major float4x4 World : WORLD;
    row_major float4x4 WorldViewProjection : WORLDVIEWPROJECTION;
    row_major float4x4 InverseWorld : INVERSEWORLD;
              float4   LightDirection;
              float4   EyePosition;
    row_major float4x4 LightWorldViewProjection;
    row_major float4x4 ViewProjection : VIEWPROJECTION;
              float4   BoundingBoxCenterWorldSpace;
              float4   BoundingBoxHalfWorldSpace;
              float4   BoundingBoxCenterObjectSpace;
              float4   BoundingBoxHalfObjectSpace;
};

// ********************************************************************************************************
// TODO: Note: nothing sets these values yet
cbuffer cbPerFrameValues
{
    row_major float4x4  View;
    row_major float4x4  Projection;
};

// ********************************************************************************************************
PS_INPUT VSMain( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
#define WORLD_SPACE_BOUNDING_BOX 0
#if WORLD_SPACE_BOUNDING_BOX
    float3 position = BoundingBoxCenterWorldSpace + input.Pos * BoundingBoxHalfWorldSpace;
    output.Pos      = mul( float4( position, 1.0f), ViewProjection );
#else
    float3 position = BoundingBoxCenterObjectSpace + input.Pos * BoundingBoxHalfObjectSpace;
    output.Pos      = mul( float4( position, 1.0f), WorldViewProjection );
#endif
    return output;
}

// ********************************************************************************************************
float4 PSMain( PS_INPUT input ) : SV_Target
{
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
}

// ********************************************************************************************************
technique testNotSkinned
{
    pass pass1
    {
        VertexShader = compile vs_3_0 VSMain();
        PixelShader  = compile ps_3_0 PSMain();
    }
}

