/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//AOIT CODE////////////////////////////////////////////////////////////////////////////////////////////////////



//AOIT CODE////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct AOITDebugViewConsts
{
    int a;
    int b;
    int c;
    int d;
};

cbuffer cbAOITDebugViewConsts : register(b4)
{
   AOITDebugViewConsts  g_AOITDebugViewConsts;
};

// ********************************************************************************************************

struct PS_INPUT
{
    float4 Pos     : SV_POSITION;
    float2 Uv      : TEXCOORD0;
};

// ********************************************************************************************************
//Texture2D    TEXTURE0 : register( t0 );
Texture2DMS<float, 4>  TEXTURE0 : register( t0 );
SamplerState SAMPLER0 : register( s0 );





// ********************************************************************************************************
float4 PSMain( PS_INPUT input ) : SV_Target
{

    uint2 dim;
	uint samples;
	TEXTURE0.GetDimensions(dim[0], dim[1],samples);    

	uint2 index = uint3(input.Uv.x*dim[0], input.Uv.y*dim[1], 0);
	float val = TEXTURE0.Load(index,0).r;
	float4 output = float4(val, val, val, 1);

    return pow((output), 0.4f);
}
