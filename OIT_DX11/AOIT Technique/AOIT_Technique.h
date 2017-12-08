/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <D3D11.h>

#define NUM_BLUR_LEVELS 4
#define NUM_BLURRED_RESOURCES 4
#define MAX_SHADER_VARIATIONS 6

#include "Resource2D.h"
#include "CPUTBufferDX11.h"

__declspec(align(16))
struct FL_Constants
{
    int   mMaxListNodes;    
};


enum eRenderType;

class AOITTechnique
{
public:
	AOITTechnique();
	~AOITTechnique();

	/* 
		InitFrameRender
		This method should be called prior to any rendering (or clearing). This implementation discards anything
		prior to the InitFrameRender invocation. Stores a pointer to the current backbuffer that is used in FinalizeFrameRender
	*/
	HRESULT InitFrameRender(ID3D11DeviceContext* pD3DImmediateContext, eRenderType, int NodeIndex, bool debugViewActive);

	/*
		FinalizeFrameRender
		Finalize the the frame, performs the tonemapping and bloom calculations and copies the results into
		the original backbuffer.
	*/
	HRESULT FinalizeFrameRender(ID3D11DeviceContext* pD3DImmediateContext, eRenderType, bool doResolve,int NodeIndex, bool debugViewActive);
	
	HRESULT OnCreate(ID3D11Device* pD3dDevice, int width, int height, ID3D11DeviceContext* pContext, IDXGISwapChain* pSwapChain);
	HRESULT OnShutdown();
	void OnSize(ID3D11Device* pD3DDevice, int width, int height);
	void FillFragmentListConstants(ID3D11DeviceContext* d3dDeviceContext, unsigned int listNodeCount);

	void UpdateConstantBuffer(ID3D11DeviceContext* pD3DImmediateContext, int mipLevel0, int mipLevel1, unsigned int width, unsigned int height);
	HRESULT GetBindIndex(struct ID3D11ShaderReflection* shaderReflection, const char *name, UINT *bindIndex);	

    void Clear(ID3D11DeviceContext* pD3DImmediateContext, ID3D11DepthStencilView * pDSV,int NodeIndex);
	void Resolve(ID3D11DeviceContext* pD3DImmediateContext,  ID3D11RenderTargetView* pOutput, ID3D11DepthStencilView * pDSV,int NodeIndex);
	void ResolveDX(ID3D11DeviceContext* pD3DImmediateContext, ID3D11RenderTargetView* pOutput);

	void ReleaseResources();

	ID3D11PixelShader* m_pAOITSPResolvePS[MAX_SHADER_VARIATIONS];
	ID3D11ShaderReflection* m_pAOITSPResolvePSReflection[MAX_SHADER_VARIATIONS];
    ID3D11PixelShader* m_pAOITSPClearPS[MAX_SHADER_VARIATIONS];

	ID3D11PixelShader* m_pDXResolvePS;
	ID3D11ShaderReflection* m_pDXResolvePSReflection;

	ResourceUAV			mAOITSPColorData[MAX_SHADER_VARIATIONS];
	ResourceUAV			mAOITSPDepthData[MAX_SHADER_VARIATIONS];
	ResourceUAV			mFragmentListNodes;
	Resource2D			mFragmentListFirstNodeOffset;
    bool                mATSPClearMaskInitialized;
	
	Resource2D   mATSPClearMask;



    ID3D11BlendState* mAOITCompositeBlendState;
    ID3D11DepthStencilState* mAOITCompositeDepthStencilState;

	CPUTBufferDX11* mpFragmentListFirstNodeOffseBuffer;
	CPUTBufferDX11* mFragmentListNodesBuffer;
	CPUTBufferDX11* mpAOITSPDepthDataUAVBuffer;
	CPUTBufferDX11* mpAOITSPColorDataUAVBuffer;
	CPUTBufferDX11* mp8AOITSPDepthDataUAVBuffer;
	CPUTBufferDX11* mp8AOITSPColorDataUAVBuffer;
	CPUTBufferDX11* mpIntelExt;
    CPUTBufferDX11 *mFragmentListConstants;
	CPUTBufferDX11* mpATSPClearMaskBuffer;
	ID3D11Buffer* m_pConstBuffer;		// Buffer constants (dimensions and miplevels) for compute shaders
	
	CPUTTextureDX11*   mpClearMaskRT;

	ID3D11SamplerState* m_pPointSampler;

	ID3D11RenderTargetView* m_pSwapChainRTV;	// render target view retrieved at InitFrameRender
	ID3D11DepthStencilView* m_pDSView;			// depth stencil view retried at InitFrameRender
	DXGI_SURFACE_DESC* m_pBackBufferSurfaceDesc;// back buffer surface desc of current render target 

	/* 
		DrawFullScreenQuad
		Helper functions for drawing a full screen quad (used for the final tonemapping and bloom composite.
		Renders the quad with and passes vertex position and texture coordinates to current pixel shader
	*/
	void DrawFullScreenQuad(ID3D11DeviceContext* pD3DImmediateContext);
	ID3D11Buffer* m_pQuadVB;
	ID3D11VertexShader* m_pFullScreenQuadVS;
	ID3D11InputLayout* m_pFullScreenQuadLayout;

    unsigned int mLisTexNodeCount;
};







