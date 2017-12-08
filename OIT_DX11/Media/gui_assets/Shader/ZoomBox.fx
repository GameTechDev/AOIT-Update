// Copyright 2011 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies. Intel makes no representations about the
// suitability of this software for any purpose. THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.


//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

cbuffer cbZoomBoxConstants : register( b0 )
{
	float4 g_ZoomCenterPos;

    float4 g_ZoomSrcRectUV;
    float4 g_ZoomDestRectUV;

    float4 g_ZoomSrcRectScreen;
    float4 g_ZoomDestRectScreen;

    float  g_ZoomScale;
    float  g_ZoomShowEdges;
    float2 dummy1;
};


//-----------------------------------------------------------------------------
// Textures and Samplers
//-----------------------------------------------------------------------------

Texture2D           OffscreenColorBuffer : register( t0 );
SamplerState        g_samZoom            : register( s0 );

//-----------------------------------------------------------------------------
// Input / Output structures
//-----------------------------------------------------------------------------

// ********************************************************************************************************
struct VS_INPUT
{
    float3 Pos     : POSITION;
    float2 Uv      : TEXCOORD0;
};

struct GS_DUMMY_INPUT
{
};

struct PS_INPUT_BORDER
{
    float4 vPos             : SV_POSITION;
};

struct PS_INPUT_ZOOM_QUAD
{
    float4 vPos             : SV_POSITION;
	float2 vTCZoom          : TEXCOORD0;
};

//-----------------------------------------------------------------------------
// Vertex Shader
//-----------------------------------------------------------------------------

VS_INPUT VSMain( VS_INPUT input )
{
    VS_INPUT output;
    output.Pos  = input.Pos;
    output.Uv   = input.Uv;
    return output;
}

//-----------------------------------------------------------------------------
// Helper functions
//-----------------------------------------------------------------------------

bool IsInRect( float2 pt, float4 rect )
{
    return ( (pt.x >= rect.x) && (pt.x <= rect.z) && (pt.y >= rect.y) && (pt.y <= rect.w) );
}

void DistToClosestRectEdge( float2 pt, float4 rect, out float dist, out int edge )
{
    edge = 0;
    dist = 1e20;

    float distTmp;
    distTmp = abs( pt.x - rect.x );
    if( distTmp <= dist ) { dist = distTmp; edge = 2; }  // left

    distTmp = abs( pt.y - rect.y );
    if( distTmp <= dist ) { dist = distTmp; edge = 3; }  // top

    distTmp = abs( pt.x - rect.z ); 
    if( distTmp <= dist ) { dist = distTmp; edge = 0; }  // right

    distTmp = abs( pt.y - rect.w );
    if( distTmp <= dist ) { dist = distTmp; edge = 1; }  // bottom
}

void DistToClosestRectEdge( float2 pt, float4 rect, out float dist, out int edge, int ignoreEdge )
{
    edge = 0;
    dist = 1e20;

    float distTmp;
    if( ignoreEdge != 2 )
    {
        distTmp = abs( pt.x - rect.x );
        if( distTmp <= dist ) { dist = distTmp; edge = 2; }  // left
    }

    if( ignoreEdge != 3 )
    {
        distTmp = abs( pt.y - rect.y );
        if( distTmp <= dist ) { dist = distTmp; edge = 3; }  // top
    }

    if( ignoreEdge != 0 )
    {
        distTmp = abs( pt.x - rect.z ); 
        if( distTmp <= dist ) { dist = distTmp; edge = 0; }  // right
    }

    if( ignoreEdge != 1 )
    {
        distTmp = abs( pt.y - rect.w );
        if( distTmp <= dist ) { dist = distTmp; edge = 1; }  // bottom
    }
}

float2 RectToRect( float2 pt, float2 srcRCentre, float2 srcRSize, float2 dstRCentre, float2 dstRSize )
{
    pt -= srcRCentre;
    pt /= srcRSize;

    pt *= dstRSize;
    pt += dstRCentre;
    
    return pt;
}

//-----------------------------------------------------------------------------
// Geometry Shaders
//-----------------------------------------------------------------------------

PS_INPUT_ZOOM_QUAD PS_INPUT_ZOOM_QUAD_FROM_UV( float2 uv )
{ 
    float2 srcRectCenter    = float2( g_ZoomSrcRectUV.zw + g_ZoomSrcRectUV.xy ) * 0.5;
    float2 srcRectSize      = float2( g_ZoomSrcRectUV.zw - g_ZoomSrcRectUV.xy );
    float2 destRectCenter   = float2( g_ZoomDestRectUV.zw + g_ZoomDestRectUV.xy ) * 0.5;
    float2 destRectSize     = float2( g_ZoomDestRectUV.zw - g_ZoomDestRectUV.xy );

    PS_INPUT_ZOOM_QUAD ret; 
    ret.vPos.xy = uv.xy * float2( 2, -2 ) + float2( -1, 1 );
    ret.vPos.z  = 0.0;
    ret.vPos.w  = 1.0;

    ret.vTCZoom = RectToRect( uv, destRectCenter, destRectSize, srcRectCenter, srcRectSize );

    return ret;
}

[MaxVertexCount(12)] 
void GS_Zoom( point GS_DUMMY_INPUT input[1], inout TriangleStream<PS_INPUT_ZOOM_QUAD> TriStream )
{
    TriStream.Append( PS_INPUT_ZOOM_QUAD_FROM_UV( g_ZoomDestRectUV.zy ) );
    TriStream.Append( PS_INPUT_ZOOM_QUAD_FROM_UV( g_ZoomDestRectUV.xy ) );
    TriStream.Append( PS_INPUT_ZOOM_QUAD_FROM_UV( g_ZoomDestRectUV.zw ) );
    TriStream.Append( PS_INPUT_ZOOM_QUAD_FROM_UV( g_ZoomDestRectUV.xw ) );

    TriStream.RestartStrip();
}


PS_INPUT_BORDER PS_INPUT_BORDER_FROM_UVC( float2 uv, float col )
{ 
    PS_INPUT_BORDER ret; 
    ret.vPos.xy = uv.xy * float2( 2, -2 ) + float2( -1, 1 );
    ret.vPos.z = col;
    ret.vPos.w = 1.0;
    return ret;
}

[MaxVertexCount(14)]
void GS_Border( point GS_DUMMY_INPUT input[1], inout LineStream<PS_INPUT_BORDER> LineStrip )
{
    LineStrip.Append( PS_INPUT_BORDER_FROM_UVC( g_ZoomSrcRectUV.xy + float2( -0.0004, -0.0004 ), 0.0 ) );
    LineStrip.Append( PS_INPUT_BORDER_FROM_UVC( g_ZoomSrcRectUV.zy + float2(  0.0004, -0.0004 ), 0.0 ) );
    LineStrip.Append( PS_INPUT_BORDER_FROM_UVC( g_ZoomSrcRectUV.zw + float2(  0.0004,  0.0004 ), 0.0 ) );
    LineStrip.Append( PS_INPUT_BORDER_FROM_UVC( g_ZoomSrcRectUV.xw + float2( -0.0004,  0.0004 ), 0.0 ) );
    LineStrip.Append( PS_INPUT_BORDER_FROM_UVC( g_ZoomSrcRectUV.xy + float2( -0.0004, -0.0004 ), 0.0 ) );
    LineStrip.RestartStrip( );

    LineStrip.Append( PS_INPUT_BORDER_FROM_UVC( g_ZoomDestRectUV.xy + float2( -0.0004, -0.0004 ), 1.0 ) );
    LineStrip.Append( PS_INPUT_BORDER_FROM_UVC( g_ZoomDestRectUV.zy + float2(  0.0004, -0.0004 ), 1.0 ) );
    LineStrip.Append( PS_INPUT_BORDER_FROM_UVC( g_ZoomDestRectUV.zw + float2(  0.0004,  0.0004 ), 1.0 ) );
    LineStrip.Append( PS_INPUT_BORDER_FROM_UVC( g_ZoomDestRectUV.xw + float2( -0.0004,  0.0004 ), 1.0 ) );
    LineStrip.Append( PS_INPUT_BORDER_FROM_UVC( g_ZoomDestRectUV.xy + float2( -0.0004, -0.0004 ), 1.0 ) );
    LineStrip.RestartStrip( );

    float2 srcRectCenter    = float2( g_ZoomSrcRectUV.zw + g_ZoomSrcRectUV.xy ) * 0.5;
    float2 destRectCenter   = float2( g_ZoomDestRectUV.zw + g_ZoomDestRectUV.xy ) * 0.5;
    
    if( srcRectCenter.x < destRectCenter.x )
    {
        LineStrip.Append( PS_INPUT_BORDER_FROM_UVC( g_ZoomSrcRectUV.zy + float2(  0.0004, -0.0004 ), 0.0 ) );
        LineStrip.Append( PS_INPUT_BORDER_FROM_UVC( g_ZoomDestRectUV.xy + float2(  0.0004, -0.0004 ), 1.0 ) );
        LineStrip.RestartStrip( );
        LineStrip.Append( PS_INPUT_BORDER_FROM_UVC( g_ZoomSrcRectUV.zw + float2(  0.0004,  0.0004 ), 0.0 ) );
        LineStrip.Append( PS_INPUT_BORDER_FROM_UVC( g_ZoomDestRectUV.xw + float2(  0.0004,  0.0004 ), 1.0 ) );
        LineStrip.RestartStrip( );
    }
    else
    {
        LineStrip.Append( PS_INPUT_BORDER_FROM_UVC( g_ZoomSrcRectUV.xy + float2( -0.0004, -0.0004 ), 0.0 ) );
        LineStrip.Append( PS_INPUT_BORDER_FROM_UVC( g_ZoomDestRectUV.zy + float2( -0.0004, -0.0004 ), 1.0 ) );
        LineStrip.RestartStrip( );
        LineStrip.Append( PS_INPUT_BORDER_FROM_UVC( g_ZoomSrcRectUV.xw + float2( -0.0004,  0.0004 ), 0.0 ) );
        LineStrip.Append( PS_INPUT_BORDER_FROM_UVC( g_ZoomDestRectUV.zw + float2( -0.0004,  0.0004 ), 1.0 ) );
        LineStrip.RestartStrip( );
    }
}

//-----------------------------------------------------------------------------
// Pixel Shaders
//-----------------------------------------------------------------------------

float4 PS_Zoom( const float4 screenPos : SV_Position ) : SV_Target
{
    // avoid using sampler as then we need 0.5 * screenSize offset to correctly sample in the middle (otherwise it works differently on different hardware)
    float2 srcRectCenter    = float2( g_ZoomSrcRectScreen.zw + g_ZoomSrcRectScreen.xy ) * 0.5;
    float2 srcRectSize      = float2( g_ZoomSrcRectScreen.zw - g_ZoomSrcRectScreen.xy );
    float2 destRectCenter   = float2( g_ZoomDestRectScreen.zw + g_ZoomDestRectScreen.xy ) * 0.5;
    float2 destRectSize     = float2( g_ZoomDestRectScreen.zw - g_ZoomDestRectScreen.xy );

    float2 texCoord = RectToRect( screenPos.xy, destRectCenter, destRectSize, srcRectCenter, srcRectSize );
    return float4( OffscreenColorBuffer.Load( int3( texCoord, 0 ) ).rgb, 1 );
}


float4 PS_Border( PS_INPUT_BORDER input ) : SV_Target
{
    return float4(1.0f, 1.0f, input.vPos.z, 1.0f);
}

