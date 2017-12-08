/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

#include "AOIT_Technique.h"
#include "CPUT_DX11.h"
#include "..\OIT.h"
#include "CPUT.h"
#include "ListTexture.h"
#include "CPUTRenderTarget.h"
#include "CPUTTextureDX11.h"

#define FULL_SCREEN_QUAD		L"..\\..\\..\\AOIT Technique\\FullScreenQuad.hlsl"
#define POST_PROCESS_HLSL		L"..\\..\\..\\AOIT Technique\\AOIT_Resolve.hlsl"
#define POST_PROCESS_DXHLSL		L"..\\..\\..\\AOIT Technique\\DX_Resolve.hlsl"

AOITTechnique::AOITTechnique() : 
	m_pDSView(NULL),
    m_pSwapChainRTV(NULL),
	m_pBackBufferSurfaceDesc(NULL), 
	m_pConstBuffer(NULL),
	m_pQuadVB(NULL),
	mpIntelExt(NULL),
	mpClearMaskRT(NULL),
	m_pDXResolvePS(NULL),
	m_pPointSampler(NULL),
	m_pDXResolvePSReflection(NULL),	
	mFragmentListNodesBuffer(NULL),
	mpAOITSPColorDataUAVBuffer(NULL),
	mpAOITSPDepthDataUAVBuffer(NULL),
	mp8AOITSPColorDataUAVBuffer(NULL),
	mp8AOITSPDepthDataUAVBuffer(NULL),
	mpFragmentListFirstNodeOffseBuffer(NULL),
	m_pFullScreenQuadVS(NULL),
    mFragmentListConstants(NULL),
	mLisTexNodeCount(1 << 22),
	m_pFullScreenQuadLayout(NULL)
{
	m_pBackBufferSurfaceDesc = new DXGI_SURFACE_DESC();
    m_pBackBufferSurfaceDesc->SampleDesc.Count = 1;
    m_pBackBufferSurfaceDesc->SampleDesc.Quality = 0;
    m_pBackBufferSurfaceDesc->Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

    mAOITCompositeBlendState = NULL;
    mAOITCompositeDepthStencilState = NULL;

    mATSPClearMaskInitialized = false;

    memset( m_pAOITSPResolvePS, 0, sizeof(m_pAOITSPResolvePS) );
    memset( m_pAOITSPResolvePSReflection, 0, sizeof(m_pAOITSPResolvePSReflection) );
    memset( m_pAOITSPClearPS, 0, sizeof(m_pAOITSPClearPS) );
};
	
	
AOITTechnique::~AOITTechnique()
{
	SAFE_RELEASE( mpClearMaskRT);
	delete m_pBackBufferSurfaceDesc;
}

HRESULT AOITTechnique::GetBindIndex(ID3D11ShaderReflection* shaderReflection, const char *name, UINT *bindIndex)
{
    HRESULT hr;
    D3D11_SHADER_INPUT_BIND_DESC inputBindDesc;
    hr = shaderReflection->GetResourceBindingDescByName(name,
                                                        &inputBindDesc);
    if (SUCCEEDED(hr)) {
        *bindIndex = inputBindDesc.BindPoint;
    } else {
        char buffer[4096];
        sprintf_s(buffer, "Could not find shader parameter %s\n", name);
        OutputDebugStringA(buffer);
    }
    return hr;
}

HRESULT AOITTechnique::InitFrameRender(ID3D11DeviceContext* pD3DImmediateContext, eRenderType SelectedItem, int NodeIndex, bool debugViewActive)
{
    HRESULT hr = S_OK;
    pD3DImmediateContext->OMGetRenderTargets(1, &(m_pSwapChainRTV), &m_pDSView);

  	if((eRenderType::ROV_OIT == SelectedItem ) || (eRenderType::ROV_HDR_OIT == SelectedItem))
	{
		// Clearing the whole AOIT data structure can incur in performance degradation.
		// We clear a small control surface/clear mask instead, which will let us know if a given
		// pixels needs to be initialized at transparent fragments insertion time.
        // We only clear this ClearMask whole once, and then clear it only on modified pixels (masked by stencil)
        if( !mATSPClearMaskInitialized || debugViewActive )
        {
            Clear( pD3DImmediateContext, NULL, NodeIndex );
            mATSPClearMaskInitialized = true;
        }
	}
  
	if ((eRenderType::ROV_OIT == SelectedItem) || (eRenderType::ROV_HDR_OIT == SelectedItem))
	{
        D3D11_RENDER_TARGET_VIEW_DESC desc;
        m_pSwapChainRTV->GetDesc( &desc );

        // Set output UAVs
		ID3D11UnorderedAccessView* pUAVs[3];
		switch(NodeIndex)
		{
		case 0:
			pUAVs[0] = mpClearMaskRT->GetColorUAV();
			pUAVs[1] = NULL;
			pUAVs[2] = mpAOITSPColorDataUAVBuffer->GetUnorderedAccessView();
			break;
		case 1:
			pUAVs[0] = mpClearMaskRT->GetColorUAV();
			pUAVs[1] = mpAOITSPDepthDataUAVBuffer->GetUnorderedAccessView();
			pUAVs[2] = mpAOITSPColorDataUAVBuffer->GetUnorderedAccessView();
			break;
		case 2:
			pUAVs[0] = mpClearMaskRT->GetColorUAV();
			pUAVs[1] = mp8AOITSPDepthDataUAVBuffer->GetUnorderedAccessView();
			pUAVs[2] = mp8AOITSPColorDataUAVBuffer->GetUnorderedAccessView();
			break;
		}

        pD3DImmediateContext->OMSetRenderTargetsAndUnorderedAccessViews( 0, NULL, m_pDSView, 1, 3, pUAVs, NULL ); // <- this disables MSAA
	}
	else if(eRenderType::DX11_AOIT == SelectedItem )
	{
		bool resetUAVCounter = true;

        ID3D11UnorderedAccessView* fragmentListFirstNodeOffsetUAV =  mFragmentListFirstNodeOffset.m_ppUAV[0];

		// Initialize the first node offset RW UAV with a NULL offset (end of the list)
        UINT clearValuesFirstNode[4] = {
            0x0UL, 
            0x0UL, 
            0x0UL, 
            0x0UL
        };

        pD3DImmediateContext->ClearUnorderedAccessViewUint(fragmentListFirstNodeOffsetUAV, clearValuesFirstNode); 
		FillFragmentListConstants(pD3DImmediateContext, mLisTexNodeCount * 2);

		ID3D11UnorderedAccessView* pUAVs[] = { mFragmentListFirstNodeOffset.m_ppUAV[0], mFragmentListNodes.m_pUAV  };

		UINT pUAVInitialCounts[2] = {1, 1};
		pD3DImmediateContext->OMSetRenderTargetsAndUnorderedAccessViews(0, NULL, m_pDSView,0, 2, pUAVs, resetUAVCounter ? pUAVInitialCounts : NULL);
	}
    return hr;
}



HRESULT AOITTechnique::FinalizeFrameRender(ID3D11DeviceContext* pD3DImmediateContext, eRenderType SelectedItem, bool doResolve,int NodeIndex, bool debugViewActive)
{
    HRESULT hr = S_OK;
	D3D11_VIEWPORT viewport;
	UINT numViewPorts = 1;
	pD3DImmediateContext->RSGetViewports(&numViewPorts, &viewport);

	unsigned int width = 512;
	unsigned int height = 512;

	D3D11_VIEWPORT quadViewPort;
	quadViewPort.Height = (float) height;
	quadViewPort.Width = (float)width;
	quadViewPort.MaxDepth = 1.0f;
	quadViewPort.MinDepth = 0.0f;
	quadViewPort.TopLeftX = 0;
	quadViewPort.TopLeftY = 0;

	if(doResolve)
	{
		pD3DImmediateContext->PSSetSamplers(0, 1, &m_pPointSampler);
		pD3DImmediateContext->RSSetViewports(1, &quadViewPort);
		UpdateConstantBuffer(pD3DImmediateContext, 0, 0, m_pBackBufferSurfaceDesc->Width, m_pBackBufferSurfaceDesc->Height);
		pD3DImmediateContext->RSSetViewports(1, &viewport);
		if (eRenderType::ROV_OIT == SelectedItem)
		{
			Resolve(pD3DImmediateContext, m_pSwapChainRTV, m_pDSView, NodeIndex);

            // Clear only on touched pixels (using stencil mask) so that it's clear for the next frame.
            if( !debugViewActive ) 
                Clear( pD3DImmediateContext, m_pDSView, NodeIndex );
		}
		else if (eRenderType::ROV_HDR_OIT == SelectedItem)
		{
			Resolve(pD3DImmediateContext, m_pSwapChainRTV, m_pDSView, NodeIndex+3);

			// Clear only on touched pixels (using stencil mask) so that it's clear for the next frame.
			if (!debugViewActive)
				Clear(pD3DImmediateContext, m_pDSView, NodeIndex);

		}
		else if(eRenderType::DX11_AOIT== SelectedItem )
		{
			ResolveDX(pD3DImmediateContext,  m_pSwapChainRTV);
		}
	}		
	pD3DImmediateContext->OMSetRenderTargets(1, &m_pSwapChainRTV, m_pDSView);
	
    SAFE_RELEASE(m_pSwapChainRTV);
    SAFE_RELEASE(m_pDSView);

	pD3DImmediateContext->PSSetShader(NULL, NULL, NULL);
	pD3DImmediateContext->VSSetShader(NULL, NULL, NULL);
	pD3DImmediateContext->GSSetShader(NULL, 0, 0);
	

    ID3D11ShaderResourceView* nullViews[16] = {0, 0, 0, 0, 0, 0, 0, 0,
                                               0, 0, 0, 0, 0, 0, 0, 0};
    pD3DImmediateContext->VSSetShaderResources(0, 16, nullViews);
    pD3DImmediateContext->GSSetShaderResources(0, 16, nullViews);
    pD3DImmediateContext->PSSetShaderResources(0, 16, nullViews);
    pD3DImmediateContext->CSSetShaderResources(0, 16, nullViews);

	ID3D11UnorderedAccessView* nullUAViews[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	pD3DImmediateContext->CSSetUnorderedAccessViews(0, 8, nullUAViews, 0);

    return hr;
}


HRESULT AOITTechnique::OnCreate(ID3D11Device* pD3DDevice, int width, int height, ID3D11DeviceContext* pContext, IDXGISwapChain* pSwapChain)
{
    HRESULT hr = S_OK;
	ID3DBlob* pShaderBlob;
	ID3DBlob* pErrorBlob;
 
	const int MAX_DEFINES = 128; // Note: use MAX_DEFINES to avoid dynamic allocation.  Arbitrarily choose 128.  Not sure if there is a real limit.
     CPUT_SHADER_MACRO pFinalShaderMacros[MAX_DEFINES]; 

    CPUT_SHADER_MACRO AOITMacro2 = { "AOIT_NODE_COUNT", "2" };
    CPUT_SHADER_MACRO AOITMacro4 = { "AOIT_NODE_COUNT", "4" };
    CPUT_SHADER_MACRO AOITMacro8 = { "AOIT_NODE_COUNT", "8" };
	CPUT_SHADER_MACRO AOITHDRMacro = { "dohdr", "1"};
	CPUT_SHADER_MACRO nullMacro = { NULL, NULL };

    cString ExecutableDirectory;
    CPUTFileSystem::GetExecutableDirectory(&ExecutableDirectory);

	for (UINT i = 0; i < MAX_SHADER_VARIATIONS; ++i)
	{
		switch(i)
		{
		case 0:		pFinalShaderMacros[0]   = AOITMacro2; pFinalShaderMacros[1] = nullMacro; break;
		case 1:		pFinalShaderMacros[0]   = AOITMacro4; pFinalShaderMacros[1] = nullMacro; break;
		case 2:		pFinalShaderMacros[0]   = AOITMacro8; pFinalShaderMacros[1] = nullMacro; break;
		case 3:		pFinalShaderMacros[0] = AOITMacro2; pFinalShaderMacros[1] = AOITHDRMacro;  pFinalShaderMacros[2] = nullMacro; break;
		case 4:		pFinalShaderMacros[0] = AOITMacro4; pFinalShaderMacros[1] = AOITHDRMacro; pFinalShaderMacros[2] = nullMacro; break;
		case 5:		pFinalShaderMacros[0] = AOITMacro8; pFinalShaderMacros[1] = AOITHDRMacro; pFinalShaderMacros[2] = nullMacro; break;
		}
	
		cString shaderpath(ExecutableDirectory);
		
		shaderpath += POST_PROCESS_HLSL;

        {
		    hr = D3DCompileFromFile(shaderpath.c_str(), (D3D_SHADER_MACRO*)pFinalShaderMacros, D3D_COMPILE_STANDARD_FILE_INCLUDE,  "AOITSPResolvePS", "ps_5_0", 
			    NULL, NULL, &pShaderBlob, &pErrorBlob);
		    if(!SUCCEEDED(hr)) 
		    {
			    cString msg = s2ws((char*)pErrorBlob->GetBufferPointer());
			    OutputDebugString(msg.c_str()); DEBUG_PRINT(_L("Assert %s"), msg ); 
		    }
  
		    hr = pD3DDevice->CreatePixelShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(),
			    NULL, &m_pAOITSPResolvePS[i]);

    		hr = D3DReflect(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**) &m_pAOITSPResolvePSReflection[i]);

            SAFE_RELEASE(pShaderBlob);
        }

        {
            hr = D3DCompileFromFile(shaderpath.c_str(), (D3D_SHADER_MACRO*)pFinalShaderMacros, D3D_COMPILE_STANDARD_FILE_INCLUDE,  "AOITSPClearPS", "ps_5_0",                 NULL, NULL, &pShaderBlob, &pErrorBlob);
            if(!SUCCEEDED(hr)) 
            {
                cString msg = s2ws((char*)pErrorBlob->GetBufferPointer());
			    OutputDebugString(msg.c_str()); DEBUG_PRINT(_L("Assert %s"), msg ); 
            }

            hr = pD3DDevice->CreatePixelShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(),
                NULL, &m_pAOITSPClearPS[i]);
            SAFE_RELEASE(pShaderBlob);
        }

	}

	cString shaderpath(ExecutableDirectory);
	shaderpath += POST_PROCESS_DXHLSL;

	hr = D3DCompileFromFile(shaderpath.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "AOITResolvePS", "ps_5_0", 
		NULL, NULL, &pShaderBlob, &pErrorBlob);
	if(!SUCCEEDED(hr)) 
	{
		cString msg = s2ws((char*)pErrorBlob->GetBufferPointer());
	    OutputDebugString(msg.c_str()); DEBUG_PRINT(_L("Assert %s"), msg ); 

	}
  
	hr = pD3DDevice->CreatePixelShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), NULL, &m_pDXResolvePS);

	hr = D3DReflect(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**) &m_pDXResolvePSReflection);

	SAFE_RELEASE(pShaderBlob);


	ID3DBlob* pBlob = NULL;


	cString quadshaderpath(ExecutableDirectory);
	quadshaderpath += FULL_SCREEN_QUAD;

	hr = D3DCompileFromFile( quadshaderpath.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_4_0", NULL, NULL, &pBlob, NULL);
    hr = pD3DDevice->CreateVertexShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(),
                                        NULL, &m_pFullScreenQuadVS );

	D3D11_INPUT_ELEMENT_DESC InputLayout[] =
    { { "SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }, 
    };
    hr = pD3DDevice->CreateInputLayout( InputLayout, ARRAYSIZE( InputLayout ),  pBlob->GetBufferPointer(), pBlob->GetBufferSize(),   &m_pFullScreenQuadLayout );
    pBlob->Release();

    float data[] = 
    {	-1.0f,  1.0f, 0.0f, 
		 1.0f,  1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
    };
	
    D3D11_BUFFER_DESC BufferDesc;
    ZeroMemory( &BufferDesc, sizeof( BufferDesc ) );
    BufferDesc.ByteWidth = sizeof(float) * 3 * 4;
    BufferDesc.Usage = D3D11_USAGE_DEFAULT;
    BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    BufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA subresourceData;
    subresourceData.pSysMem = data;
    subresourceData.SysMemPitch = 0;
    subresourceData.SysMemSlicePitch = 0;

    pD3DDevice->CreateBuffer(&BufferDesc, &subresourceData, &m_pQuadVB);

    BufferDesc.ByteWidth = sizeof(int) * 4;
    BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    pD3DDevice->CreateBuffer(&BufferDesc, NULL, &m_pConstBuffer);


	D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC();
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MaxLOD = FLT_MAX;
	samplerDesc.MinLOD = FLT_MIN;
	samplerDesc.MipLODBias = 0.0f;

	pD3DDevice->CreateSamplerState(&samplerDesc, &m_pPointSampler);

    // Create OIT blend state
    {
        CD3D11_BLEND_DESC desc(D3D11_DEFAULT);
        desc.RenderTarget[0].BlendEnable = true;
        desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
        desc.RenderTarget[0].DestBlend = D3D11_BLEND_SRC_ALPHA;
        desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        pD3DDevice->CreateBlendState(&desc, &mAOITCompositeBlendState);
    }

    // Create AOIT depth stencil desc
    {
        D3D11_DEPTH_STENCIL_DESC DSDesc;
        DSDesc.DepthEnable                  = FALSE;
        DSDesc.DepthFunc                    = D3D11_COMPARISON_GREATER;
        DSDesc.DepthWriteMask               = D3D11_DEPTH_WRITE_MASK_ZERO;
        DSDesc.StencilEnable                = TRUE;
        DSDesc.StencilReadMask              = 0xFF;
        DSDesc.StencilWriteMask             = 0x00;
        DSDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
        DSDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        DSDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
        DSDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_EQUAL;
        DSDesc.BackFace.StencilFailOp       = D3D11_STENCIL_OP_KEEP;
        DSDesc.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP_KEEP;
        DSDesc.BackFace.StencilPassOp       = D3D11_STENCIL_OP_KEEP;
        DSDesc.BackFace.StencilFunc         = D3D11_COMPARISON_EQUAL;
        hr = pD3DDevice->CreateDepthStencilState(&DSDesc, &mAOITCompositeDepthStencilState );
    }

    D3D11_BUFFER_DESC bd = {0};
    bd.ByteWidth = sizeof(FL_Constants);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    ID3D11Buffer * pOurConstants;
    hr = (CPUT_DX11::GetDevice())->CreateBuffer( &bd, NULL, &pOurConstants );

    cString name = _L("$FL_Constants");
    mFragmentListConstants = new CPUTBufferDX11( name, pOurConstants );

    CPUTAssetLibrary::GetAssetLibrary()->AddConstantBuffer( name, _L(""), _L(""),  mFragmentListConstants );
    SAFE_RELEASE(pOurConstants); // We're done with it.  The CPUTBuffer now owns it.
    //mGlobalProperties

    mpClearMaskRT = (CPUTTextureDX11*)CPUTTextureDX11::CreateTexture(_L("$ClearMaskRT"), DXGI_FORMAT_R32_UINT, width, height, DXGI_FORMAT_R32_UINT, D3D11_BIND_RENDER_TARGET |D3D11_BIND_UNORDERED_ACCESS);
	mpClearMaskRT->AddUAVView(DXGI_FORMAT_R32_UINT);


    return hr;
}

HRESULT AOITTechnique::OnShutdown()
{
    HEAPCHECK;
    HRESULT hr = S_OK;

	ReleaseResources();

	SAFE_RELEASE(m_pQuadVB);
	SAFE_RELEASE(m_pFullScreenQuadVS);
	SAFE_RELEASE(m_pFullScreenQuadLayout);

	for (UINT i = 0; i < MAX_SHADER_VARIATIONS; ++i)
	{
		SAFE_RELEASE(m_pAOITSPResolvePS[i]);
		SAFE_RELEASE(m_pAOITSPResolvePSReflection[i]);
		SAFE_RELEASE(m_pAOITSPClearPS[i]);
	}

	SAFE_RELEASE(m_pDXResolvePS);
	SAFE_RELEASE(m_pDXResolvePSReflection);

	SAFE_RELEASE (mpIntelExt);
	SAFE_RELEASE( mFragmentListNodesBuffer );
	SAFE_RELEASE( mpAOITSPColorDataUAVBuffer);
	SAFE_RELEASE (mpAOITSPDepthDataUAVBuffer);
	SAFE_RELEASE( mp8AOITSPColorDataUAVBuffer);
	SAFE_RELEASE (mp8AOITSPDepthDataUAVBuffer);
	SAFE_RELEASE( mpFragmentListFirstNodeOffseBuffer);

	SAFE_RELEASE (mFragmentListConstants);

	SAFE_RELEASE(m_pConstBuffer);

	SAFE_RELEASE(mAOITCompositeBlendState);
    SAFE_RELEASE(mAOITCompositeDepthStencilState);
		
	if(m_pBackBufferSurfaceDesc != NULL)
	{
		delete m_pBackBufferSurfaceDesc;
		m_pBackBufferSurfaceDesc = NULL;
	}

	SAFE_RELEASE(m_pPointSampler);

	    HEAPCHECK;
    return hr;
}


void AOITTechnique::OnSize(ID3D11Device* pD3DDevice, int width, int height)
{
   CPUTAssetLibraryDX11 *pAssetLibrary = (CPUTAssetLibraryDX11*)CPUTAssetLibraryDX11::GetAssetLibrary();


	if(width == 0 || height == 0)
		return;


	mpClearMaskRT->Resize(width, height);

    m_pBackBufferSurfaceDesc->Height = height;
    m_pBackBufferSurfaceDesc->Width = width;

	ReleaseResources();


	mAOITSPDepthData[0].Create(pD3DDevice, width, height, 4 * 4 );
	mAOITSPColorData[0].Create(pD3DDevice, width, height, 4 * 4);
	mAOITSPDepthData[1].Create(pD3DDevice, width, height, 4 * 8 );
	mAOITSPColorData[1].Create(pD3DDevice, width, height, 4 * 8);


	UINT bindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

    UINT structSize = sizeof(FragmentNode);
    const unsigned int nodeCount = mLisTexNodeCount * 2;
	mFragmentListNodes.Create(pD3DDevice, 1, nodeCount, structSize,D3D11_BUFFER_UAV_FLAG_COUNTER);


    mATSPClearMaskInitialized = false;
	mFragmentListFirstNodeOffset.Create(pD3DDevice, width, height, DXGI_FORMAT_R32_UINT, bindFlags | D3D11_BIND_UNORDERED_ACCESS, 1);


	if(mpFragmentListFirstNodeOffseBuffer)
	{
		((CPUTBufferDX11*)mpFragmentListFirstNodeOffseBuffer)->SetBufferAndViews( NULL, mFragmentListFirstNodeOffset.m_pSRV, mFragmentListFirstNodeOffset.m_ppUAV[0] );
	}
	else
	{
		cString FragmentListFirstNodeAddressName = _L("$gFragmentListFirstNodeAddressUAV"); 
		mpFragmentListFirstNodeOffseBuffer  = new CPUTBufferDX11( FragmentListFirstNodeAddressName, NULL, mFragmentListFirstNodeOffset.m_ppUAV[0] );
		mpFragmentListFirstNodeOffseBuffer->SetShaderResourceView(mFragmentListFirstNodeOffset.m_pSRV);
		pAssetLibrary->AddBuffer( FragmentListFirstNodeAddressName, _L(""), _L(""),  mpFragmentListFirstNodeOffseBuffer );
	}

	if(!mFragmentListNodesBuffer)
	{
		cString gFragmentListNodesUAVName = _L("$gFragmentListNodesUAV"); 
		mFragmentListNodesBuffer  = new CPUTBufferDX11( gFragmentListNodesUAVName, NULL, mFragmentListNodes.m_pUAV );
		pAssetLibrary->AddBuffer( gFragmentListNodesUAVName, _L(""), _L(""),  mFragmentListNodesBuffer );
	}
	else
	{
		((CPUTBufferDX11*)mFragmentListNodesBuffer)->SetBufferAndViews( NULL, NULL, mFragmentListNodes.m_pUAV );
	}

	if(!mpAOITSPDepthDataUAVBuffer)
	{
		cString gAOITSPDepthDataUAVName = _L("$gAOITSPDepthDataUAV"); 
		mpAOITSPDepthDataUAVBuffer  = new CPUTBufferDX11( gAOITSPDepthDataUAVName, NULL, mAOITSPDepthData[0].m_pUAV );
		pAssetLibrary->AddBuffer( gAOITSPDepthDataUAVName, _L(""), _L(""),  mpAOITSPDepthDataUAVBuffer );
	}

	if(!mp8AOITSPDepthDataUAVBuffer)
	{
		cString gAOITSPDepthDataUAVName = _L("$g8AOITSPDepthDataUAV"); 
		mp8AOITSPDepthDataUAVBuffer  = new CPUTBufferDX11( gAOITSPDepthDataUAVName, NULL, mAOITSPDepthData[1].m_pUAV );
		pAssetLibrary->AddBuffer( gAOITSPDepthDataUAVName, _L(""), _L(""),  mp8AOITSPDepthDataUAVBuffer );
	}

	if(!mpAOITSPColorDataUAVBuffer)
	{
		cString gAOITSPColorDataUAVName = _L("$gAOITSPColorDataUAV"); 
		mpAOITSPColorDataUAVBuffer  = new CPUTBufferDX11( gAOITSPColorDataUAVName, NULL, mAOITSPColorData[0].m_pUAV );
		pAssetLibrary->AddBuffer( gAOITSPColorDataUAVName, _L(""), _L(""),  mpAOITSPColorDataUAVBuffer );
	}

	if(!mp8AOITSPColorDataUAVBuffer)
	{
		cString gAOITSPColorDataUAVName = _L("$g8AOITSPColorDataUAV"); 
		mp8AOITSPColorDataUAVBuffer  = new CPUTBufferDX11( gAOITSPColorDataUAVName, NULL, mAOITSPColorData[1].m_pUAV );
		pAssetLibrary->AddBuffer( gAOITSPColorDataUAVName, _L(""), _L(""),  mp8AOITSPColorDataUAVBuffer );
	}


	mpAOITSPDepthDataUAVBuffer->SetBufferAndViews( NULL, mAOITSPDepthData[0].m_pSRV, mAOITSPDepthData[0].m_pUAV );
	mpAOITSPColorDataUAVBuffer->SetBufferAndViews( NULL, mAOITSPColorData[0].m_pSRV, mAOITSPColorData[0].m_pUAV );
	mp8AOITSPDepthDataUAVBuffer->SetBufferAndViews( NULL, mAOITSPDepthData[1].m_pSRV, mAOITSPDepthData[1].m_pUAV );
	mp8AOITSPColorDataUAVBuffer->SetBufferAndViews( NULL, mAOITSPColorData[1].m_pSRV, mAOITSPColorData[1].m_pUAV );



	if(mpIntelExt)
	{
		((CPUTBufferDX11*)mpIntelExt)->SetBufferAndViews( NULL, NULL, NULL );
	}
	else
	{
		cString g_IntelExtName = _L("$g_IntelExt"); 
		mpIntelExt  = new CPUTBufferDX11( g_IntelExtName, NULL );
		pAssetLibrary->AddBuffer( g_IntelExtName, _L(""), _L(""),  mpIntelExt );
	}
	
}



void AOITTechnique::UpdateConstantBuffer(ID3D11DeviceContext* pD3DImmediateContext, int mipLevel0, int mipLevel1, unsigned int width, unsigned int height)
{
    int src[] = {mipLevel0, mipLevel1, (int)width, (int)height};
    int rowpitch = sizeof(int) * 4;
    pD3DImmediateContext->UpdateSubresource(m_pConstBuffer, 0, NULL, &src, rowpitch, 0);
    pD3DImmediateContext->PSSetConstantBuffers(0, 1, &m_pConstBuffer);
}

void AOITTechnique::ReleaseResources()
{
	//create float render target
    SAFE_RELEASE(m_pDSView);
    SAFE_RELEASE(m_pSwapChainRTV);

	for (UINT i = 0; i < 2; ++i)
	{
		mAOITSPDepthData[i].Release();
		mAOITSPColorData[i].Release();
	}

	mFragmentListNodes.Release();
	mFragmentListFirstNodeOffset.Release();

}

void AOITTechnique::DrawFullScreenQuad(ID3D11DeviceContext* pD3DImmediateContext)
{
	pD3DImmediateContext->GSSetShader(NULL, NULL, 0);
    pD3DImmediateContext->VSSetShader(m_pFullScreenQuadVS, NULL, 0);
    pD3DImmediateContext->IASetInputLayout( m_pFullScreenQuadLayout );
    ID3D11Buffer* pVBs[] = { m_pQuadVB };
    UINT strides[] = {sizeof(float) * 3};
    UINT offsets[] = {0};
    pD3DImmediateContext->IASetVertexBuffers(0, 1, pVBs, strides, offsets);
    pD3DImmediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    pD3DImmediateContext->Draw(4, 0);
}

void AOITTechnique::Clear(ID3D11DeviceContext* pD3DImmediateContext, ID3D11DepthStencilView * pDSV,int NodeIndex)
{
    ID3D11UnorderedAccessView* pUAVs[3];

	switch(NodeIndex)
	{
	case 0:
		pUAVs[0] = mpClearMaskRT->GetColorUAV();
		pUAVs[1] = NULL;
		pUAVs[2] = mpAOITSPColorDataUAVBuffer->GetUnorderedAccessView();
		break;
	case 1:
		pUAVs[0] = mpClearMaskRT->GetColorUAV();
		pUAVs[1] = mpAOITSPDepthDataUAVBuffer->GetUnorderedAccessView();
		pUAVs[2] = mpAOITSPColorDataUAVBuffer->GetUnorderedAccessView();
		break;
	case 2:
		pUAVs[0] = mpClearMaskRT->GetColorUAV();
		pUAVs[1] = mp8AOITSPDepthDataUAVBuffer->GetUnorderedAccessView();
		pUAVs[2] = mp8AOITSPColorDataUAVBuffer->GetUnorderedAccessView();
		break;
	}

    pD3DImmediateContext->OMSetRenderTargetsAndUnorderedAccessViews( 0, NULL, pDSV, 1, 3, pUAVs, NULL );

    pD3DImmediateContext->PSSetShader(m_pAOITSPClearPS[NodeIndex], NULL, NULL);

    pD3DImmediateContext->OMSetBlendState(mAOITCompositeBlendState, 0, 0xffffffff);

    ID3D11DepthStencilState * pBackupState = NULL; UINT backupStencilRef = 0;
    pD3DImmediateContext->OMGetDepthStencilState( &pBackupState, &backupStencilRef );
    if( pDSV != NULL )
    {
        pD3DImmediateContext->OMSetDepthStencilState( mAOITCompositeDepthStencilState, 0x01 );
    }
    DrawFullScreenQuad(pD3DImmediateContext);

    pD3DImmediateContext->OMSetDepthStencilState( pBackupState, backupStencilRef );
    SAFE_RELEASE( pBackupState );
}

void AOITTechnique::Resolve(ID3D11DeviceContext* pD3DImmediateContext, ID3D11RenderTargetView* pOutput, ID3D11DepthStencilView * pDSV,int NodeIndex)
{
	pD3DImmediateContext->OMSetRenderTargets(1, &pOutput, pDSV);

	ID3D11ShaderResourceView* pAOITClearMaskSRV[] = {  mpClearMaskRT->GetShaderResourceView() };
	
	UINT bindIndex = -1;
	UINT bindIndex2 = -1;
	UINT clearBindIndex = -1;


	if((NodeIndex==2) || (NodeIndex == 5)) // 8 node version for normal and HDR
	{
		ID3D11ShaderResourceView* pSRVs[] = { mp8AOITSPColorDataUAVBuffer->GetShaderResourceView() };
		if (GetBindIndex(m_pAOITSPResolvePSReflection[NodeIndex], "g8AOITSPColorDataSRV", &bindIndex) == S_OK) {
			pD3DImmediateContext->PSSetShaderResources(bindIndex,1,  pSRVs);
		}

		if (NodeIndex == 5)
		{
				ID3D11ShaderResourceView* pDSRVs[] = { mp8AOITSPDepthDataUAVBuffer->GetShaderResourceView() };
				if (GetBindIndex(m_pAOITSPResolvePSReflection[NodeIndex], "g8AOITSPDepthDataSRV", &bindIndex2) == S_OK) {
					pD3DImmediateContext->PSSetShaderResources(bindIndex2, 1, pDSRVs);
				}
		}
	}else
	{
		ID3D11ShaderResourceView* pSRVs[] = { mpAOITSPColorDataUAVBuffer->GetShaderResourceView() };
		if (GetBindIndex(m_pAOITSPResolvePSReflection[NodeIndex], "gAOITSPColorDataSRV", &bindIndex) == S_OK) {
			pD3DImmediateContext->PSSetShaderResources(bindIndex,1,  pSRVs);
		}
		if (NodeIndex == 4)
		{
			ID3D11ShaderResourceView* pDSRVs[] = { mpAOITSPDepthDataUAVBuffer->GetShaderResourceView() };
			if (GetBindIndex(m_pAOITSPResolvePSReflection[NodeIndex], "gAOITSPDepthDataSRV", &bindIndex2) == S_OK) {
				pD3DImmediateContext->PSSetShaderResources(bindIndex2, 1, pDSRVs);
			}
		}

	}
	if (GetBindIndex(m_pAOITSPResolvePSReflection[NodeIndex], "gAOITSPClearMaskSRV", &clearBindIndex) == S_OK) {
		pD3DImmediateContext->PSSetShaderResources(clearBindIndex,1,  &pAOITClearMaskSRV[0]);
	}

	pD3DImmediateContext->PSSetShader(m_pAOITSPResolvePS[NodeIndex], NULL, NULL);

    pD3DImmediateContext->OMSetBlendState(mAOITCompositeBlendState, 0, 0xffffffff);
    
    ID3D11DepthStencilState * pBackupState = NULL; UINT backupStencilRef = 0;
    pD3DImmediateContext->OMGetDepthStencilState( &pBackupState, &backupStencilRef );
    pD3DImmediateContext->OMSetDepthStencilState( mAOITCompositeDepthStencilState, 0x01 );

    DrawFullScreenQuad(pD3DImmediateContext);

    pD3DImmediateContext->OMSetDepthStencilState( pBackupState, backupStencilRef );
    SAFE_RELEASE( pBackupState );

    if( bindIndex != -1 )
    {
        ID3D11ShaderResourceView * nullSRV = NULL;
        pD3DImmediateContext->PSSetShaderResources( bindIndex, 1, &nullSRV );
    }
	if (bindIndex2 != -1)
	{
		ID3D11ShaderResourceView * nullSRV = NULL;
		pD3DImmediateContext->PSSetShaderResources(bindIndex2, 1, &nullSRV);
	}
	if( clearBindIndex != -1 )
    {
        ID3D11ShaderResourceView * nullSRV = NULL;
        pD3DImmediateContext->PSSetShaderResources( clearBindIndex, 1, &nullSRV );
    }
}


void AOITTechnique::FillFragmentListConstants(ID3D11DeviceContext* d3dDeviceContext, unsigned int listNodeCount)
{
	ID3D11Buffer *pBuffer = mFragmentListConstants->GetNativeBuffer();

    // List texture related constants  
	D3D11_MAPPED_SUBRESOURCE mapInfo;
	d3dDeviceContext->Map( pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapInfo );
	{
		FL_Constants & consts = *((FL_Constants*)mapInfo.pData);
            
		consts.mMaxListNodes = listNodeCount;

		d3dDeviceContext->Unmap(pBuffer,0);
	}
}

void AOITTechnique::ResolveDX(ID3D11DeviceContext* pD3DImmediateContext, ID3D11RenderTargetView* pOutput)
{
	pD3DImmediateContext->OMSetRenderTargets(1, &pOutput, NULL);

	ID3D11ShaderResourceView* pAOITClearMaskSRV[] = { mpClearMaskRT->GetShaderResourceView()};
	
    UINT bindIndex;
    if (GetBindIndex(m_pDXResolvePSReflection, "gFragmentListFirstNodeAddressSRV", &bindIndex) == S_OK) {
        pD3DImmediateContext->PSSetShaderResources(bindIndex,1,  &mFragmentListFirstNodeOffset.m_pSRV);
	}
    if (GetBindIndex(m_pDXResolvePSReflection, "gFragmentListNodesSRV", &bindIndex) == S_OK) {
        pD3DImmediateContext->PSSetShaderResources(bindIndex,1,  &mFragmentListNodes.m_pSRV);
	}

	pD3DImmediateContext->PSSetShader(m_pDXResolvePS, NULL, NULL);
    pD3DImmediateContext->OMSetBlendState(mAOITCompositeBlendState, 0, 0xffffffff);

    DrawFullScreenQuad(pD3DImmediateContext);
}