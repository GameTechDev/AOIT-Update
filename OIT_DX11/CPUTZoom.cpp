/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

#include "CPUT.h"
#include "CPUTZoom.h"
#include "CPUTAssetLibraryDX11.h"
#include "CPUT_DX11.h"
#include "CPUTMaterial.h"
#include "CPUTTextureDX11.h"

struct ZoomBoxConstants
{
    float4 ZoomCenterPos;

    float4 ZoomSrcRectUV;
    float4 ZoomDestRectUV;

    float4 ZoomSrcRectScreen;
    float4 ZoomDestRectScreen;

    float  ZoomScale;
    float  ZoomShowEdges;
    float2 dummy1;
};


CPUTZoomBox::CPUTZoomBox() : m_pZoomT(NULL), m_pZoomSRV(NULL), mvZoomCenterPos(0.5f, 0.5f)
{
    mpBorderSprite = NULL;
	mpBorderMaterial = NULL;
	mpZoomMaterial = NULL;
	mpZoomBoxConstants = NULL;
	mZoomTxT = NULL;
    
    mvZoomSourceSize = float2( 0.04f, 0.04f );
    mZoomFactor = 8;
}

CPUTZoomBox::~CPUTZoomBox() 
{

}


void CPUTZoomBox::SetZoomCenterPosition(FLOAT x, FLOAT y)
{
    mvZoomCenterPos.x = x;
    mvZoomCenterPos.y = y;
}


HRESULT CPUTZoomBox::OnCreate(ID3D11Device* pD3DDevice, ID3D11DeviceContext* pContext, IDXGISwapChain* pSwapChain)
{
	HRESULT hr = S_OK;
	
    CPUTAssetLibrary *pAssetLibrary = CPUTAssetLibrary::GetAssetLibrary();

 
    D3D11_BUFFER_DESC bd = {0};
    bd.ByteWidth = sizeof(ZoomBoxConstants);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    ID3D11Buffer * pOurConstants;
    hr = (CPUT_DX11::GetDevice())->CreateBuffer( &bd, NULL, &pOurConstants );

    cString name = _L("$cbZoomBoxConstants");
    mpZoomBoxConstants = new CPUTBufferDX11( name, pOurConstants );

    CPUTAssetLibrary::GetAssetLibrary()->AddConstantBuffer( name, _L(""), _L(""), mpZoomBoxConstants );
    SAFE_RELEASE(pOurConstants); // We're done with it.  The CPUTBuffer now owns it.

	CPUTMaterial::mGlobalProperties.AddValue( _L("cbZoomBoxConstants"), _L("$cbZoomBoxConstants") );
	CPUTMaterial::mGlobalProperties.AddValue( _L("g_txZoomSrcColor"), _L("$g_txZoomSrcColor") );
	CPUTMaterial::mGlobalProperties.AddValue( _L("g_txZoomSrcEdges"), _L("$g_txZoomSrcEdges") );


    mpBorderSprite = new CPUTGeometrySprite();
    mpBorderSprite->CreateSprite( -1.0f, -1.0f, 2.0f, 2.0f, _L("ZoomBorder") );
	mpBorderMaterial = pAssetLibrary->GetMaterial( L"ZoomBorder" );
	mpZoomMaterial = pAssetLibrary->GetMaterial( L"Zoom" );

	return hr;
}

HRESULT CPUTZoomBox::OnSize(ID3D11Device* pD3DDevice, int width, int height)
{
    CPUTAssetLibrary *pAssetLibrary = CPUTAssetLibrary::GetAssetLibrary();

    mScreenWidth    = width;
    mScreenHeight   = height;

    SAFE_RELEASE(m_pZoomT);
    SAFE_RELEASE(m_pZoomSRV);

    D3D11_TEXTURE2D_DESC ZoomTDesc;
    ZeroMemory(&ZoomTDesc, sizeof(ZoomTDesc));
    ZoomTDesc.Width                = width;
    ZoomTDesc.Height               = height;
    ZoomTDesc.MipLevels            = 1;
    ZoomTDesc.ArraySize            = 1;
    ZoomTDesc.Format               = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    ZoomTDesc.SampleDesc.Count     = 1;
    ZoomTDesc.Usage                = D3D11_USAGE_DEFAULT;
    ZoomTDesc.CPUAccessFlags       = 0;
    ZoomTDesc.BindFlags            = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SHADER_RESOURCE_VIEW_DESC RSVDesc;
    ZeroMemory(&RSVDesc, sizeof(RSVDesc));
    RSVDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
    RSVDesc.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    RSVDesc.Texture2D.MostDetailedMip = 0;
    RSVDesc.Texture2D.MipLevels       = 1;

    HRESULT hr;
    hr = pD3DDevice->CreateTexture2D(&ZoomTDesc, NULL, &m_pZoomT);
    hr = pD3DDevice->CreateShaderResourceView(m_pZoomT, &RSVDesc, &m_pZoomSRV);

	if( mZoomTxT )
    {
	    mZoomTxT->SetTextureAndShaderResourceView(m_pZoomT, m_pZoomSRV);
    }
    else
    {
		// make an internal/system-generated texture
		cString name = _L("$g_txZoomSrcColor");                                         // $ name indicates not loaded from file
		mZoomTxT = new CPUTTextureDX11(name);                      
		pAssetLibrary->AddTexture( name, _L(""), _L(""), mZoomTxT );

		// wrap the previously created objects
		mZoomTxT->SetTextureAndShaderResourceView(m_pZoomT, m_pZoomSRV);
	}


    ZeroMemory(&ZoomTDesc, sizeof(ZoomTDesc));
    ZoomTDesc.Width                = width;
    ZoomTDesc.Height               = height;
    ZoomTDesc.MipLevels            = 1;
    ZoomTDesc.ArraySize            = 1;
    ZoomTDesc.Format               = DXGI_FORMAT_R8_UNORM;
    ZoomTDesc.SampleDesc.Count     = 1;
    ZoomTDesc.Usage                = D3D11_USAGE_DEFAULT;
    ZoomTDesc.CPUAccessFlags       = 0;
    ZoomTDesc.BindFlags            = D3D11_BIND_SHADER_RESOURCE;

    ZeroMemory(&RSVDesc, sizeof(RSVDesc));
    RSVDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
    RSVDesc.Format                    = DXGI_FORMAT_R8_UNORM;
    RSVDesc.Texture2D.MostDetailedMip = 0;
    RSVDesc.Texture2D.MipLevels       = 1;




    return hr;
}

void CPUTZoomBox::OnShutdown()
{
	SAFE_RELEASE( mpZoomBoxConstants );
	SAFE_RELEASE( mpBorderMaterial);
	SAFE_RELEASE( mpZoomMaterial);
	SAFE_DELETE( mpBorderSprite );
    SAFE_RELEASE(m_pZoomT);
    SAFE_RELEASE(m_pZoomSRV);
    SAFE_RELEASE(mZoomTxT);
}

static float lerp( float x, float y, float a )
{
    return x * (1.0f-a) + y * a;
}

void CPUTZoomBox::OnFrameRender(CPUTRenderParametersDX &renderParams , bool showEdgeInfo)
{
	ID3D11Buffer *pBuffer = mpZoomBoxConstants->GetNativeBuffer();

	D3D11_MAPPED_SUBRESOURCE MappedCB;
	if( SUCCEEDED( renderParams.mpContext->Map(pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedCB) ) )
	{
        ZoomBoxConstants *pCB = (ZoomBoxConstants *) MappedCB.pData;
	    pCB->ZoomCenterPos.x        = mvZoomCenterPos.x;
        pCB->ZoomCenterPos.y        = mvZoomCenterPos.y;

        int centerX                 = (int)( 0.5 + mvZoomCenterPos.x * mScreenWidth  );
        int centerY                 = (int)( 0.5 + mvZoomCenterPos.y * mScreenHeight );
        int halfSrcSizeX            = (int)( 0.5 + 0.5 * mvZoomSourceSize.x * mScreenWidth  );
        int halfSrcSizeY            = (int)( 0.5 + 0.5 * mvZoomSourceSize.y * mScreenHeight );

        pCB->ZoomSrcRectScreen.x    = (float)( centerX - halfSrcSizeX );
        pCB->ZoomSrcRectScreen.y    = (float)( centerY - halfSrcSizeY ); 
        pCB->ZoomSrcRectScreen.z    = (float)( centerX + halfSrcSizeX );
        pCB->ZoomSrcRectScreen.w    = (float)( centerY + halfSrcSizeY );

        float2 screenCenter     = float2( mScreenWidth * 0.5f, mScreenHeight * 0.5f );
        float2 screenSize       = float2( (float)mScreenWidth, (float)mScreenHeight );
        float2 srcRectSize      = float2( halfSrcSizeX*2.0f, halfSrcSizeY*2.0f );
        float2 srcRectCenter    = float2( (float)centerX, (float)centerY );
        float2 destRectSize  = float2( halfSrcSizeX*2.0f * mZoomFactor, halfSrcSizeY*2.0f  * mZoomFactor );
        float2 destRectCenter; 
        int srcDestDist = 100;
        destRectCenter.x = (srcRectCenter.x > screenCenter.x)?(srcRectCenter.x - srcRectSize.x * 0.5f - destRectSize.x * 0.5f - srcDestDist):(srcRectCenter.x + srcRectSize.x * 0.5f + destRectSize.x * 0.5f + srcDestDist);
        destRectCenter.y = lerp( destRectSize.y/2, screenSize.y - destRectSize.y/2, srcRectCenter.y / screenSize.y );

        pCB->ZoomDestRectScreen.x   = destRectCenter.x - destRectSize.x * 0.5f;
        pCB->ZoomDestRectScreen.y   = destRectCenter.y - destRectSize.y * 0.5f;
        pCB->ZoomDestRectScreen.z   = destRectCenter.x + destRectSize.x * 0.5f;
        pCB->ZoomDestRectScreen.w   = destRectCenter.y + destRectSize.y * 0.5f;

        pCB->ZoomSrcRectUV.x        = (pCB->ZoomSrcRectScreen.x + 0.5f) / (float)mScreenWidth;
        pCB->ZoomSrcRectUV.y        = (pCB->ZoomSrcRectScreen.y + 0.5f) / (float)mScreenHeight;
        pCB->ZoomSrcRectUV.z        = (pCB->ZoomSrcRectScreen.z + 0.5f) / (float)mScreenWidth;
        pCB->ZoomSrcRectUV.w        = (pCB->ZoomSrcRectScreen.w + 0.5f) / (float)mScreenHeight;

        pCB->ZoomDestRectUV.x       = (pCB->ZoomDestRectScreen.x + 0.5f) / (float)mScreenWidth;
        pCB->ZoomDestRectUV.y       = (pCB->ZoomDestRectScreen.y + 0.5f) / (float)mScreenHeight;
        pCB->ZoomDestRectUV.z       = (pCB->ZoomDestRectScreen.z + 0.5f) / (float)mScreenWidth;
        pCB->ZoomDestRectUV.w       = (pCB->ZoomDestRectScreen.w + 0.5f) / (float)mScreenHeight;

        pCB->ZoomShowEdges          = (showEdgeInfo)?(1.0f):(0.0f);

        renderParams.mpContext->Unmap(pBuffer, 0);
    }

	mpBorderSprite->DrawSprite(renderParams,*mpZoomMaterial);
    mpBorderSprite->DrawSprite(renderParams,*mpBorderMaterial);
}