/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _CPUTTEXTUREDX11_H
#define _CPUTTEXTUREDX11_H

#include "CPUTTexture.h"
#include "CPUT_DX11.h"
#include "CPUTBufferDX11.h"
#include <d3d11.h>

class CPUTTextureDX11 : public CPUTTexture
{
    UINT                    mWidth;
    UINT                    mHeight;
    DXGI_FORMAT             mFormat;
	DXGI_FORMAT				mInternalFormat;
	DXGI_FORMAT				mdepthFormat;
	DXGI_FORMAT				mUAVFormat;
    UINT                    mMultiSampleCount;
	UINT					mBindFlags;
private:
    // resource view pointer
    CD3D11_TEXTURE2D_DESC     mDesc;
    ID3D11DepthStencilView   *mpDepthStencilView;
    ID3D11ShaderResourceView *mpShaderResourceView;
    ID3D11UnorderedAccessView     *mpColorUAV;
    ID3D11RenderTargetView        *mpColorRenderTargetView;
    ID3D11Resource           *mpTexture;
    ID3D11Resource           *mpTextureStaging;
    CPUTBuffer                    *mpColorBuffer;

    // Destructor is not public.  Must release instead of delete.
    ~CPUTTextureDX11() {
        SAFE_RELEASE( mpShaderResourceView );
        SAFE_RELEASE( mpTexture );
        SAFE_RELEASE(mpDepthStencilView);
        SAFE_RELEASE(mpShaderResourceView);
        SAFE_RELEASE(mpColorUAV);
		SAFE_RELEASE( mpTextureStaging );
		SAFE_RELEASE( mpColorBuffer );
		SAFE_RELEASE(mpColorRenderTargetView);
		
    }

public:
	static const cString &GetDXGIFormatString(DXGI_FORMAT Format);
	static CPUTTexture   *CreateTexture( const cString &name, const cString &absolutePathAndFilename, bool loadAsSRGB );
	static CPUTTexture   *CPUTTextureDX11::CreateTexture(const cString &name);
    static CPUTResult     CreateNativeTexture(
                              ID3D11Device *pD3dDevice,
                              const cString &fileName,
                              ID3D11ShaderResourceView **ppShaderResourceView,
                              ID3D11Resource **ppTexture,
                              bool forceLoadAsSRGB
							  ); 

	void AddDepthView(DXGI_FORMAT depthFormat);
	void AddUAVView(DXGI_FORMAT uavFormat);
	void AddRenderTargetView();

	void GetDimensions(UINT *pWidth, UINT *pHeight){
    *pWidth  = mWidth;
    *pHeight = mHeight;
	}


    static CPUTTexture *CreateTexture(const cString &name, DXGI_FORMAT internalFormat, int width, int height, 
                   DXGI_FORMAT format, UINT BindFlags, UINT        multiSampleCount = 1);

	void CreateNativeTexture(DXGI_FORMAT internalFormat, int width, int height,DXGI_FORMAT format, UINT BindFlags, UINT        multiSampleCount);


	void Resize(int width, int height);
    CPUTTextureDX11() :
		mpColorBuffer(NULL),
        mpShaderResourceView(NULL),
        mpTexture(NULL),
		mpDepthStencilView(NULL),
		mpColorUAV(NULL),
		mpColorRenderTargetView(NULL),
        mpTextureStaging(NULL)
    {}
    CPUTTextureDX11(cString &name) :
		mpColorBuffer(NULL),
        mpShaderResourceView(NULL),
        mpTexture(NULL),
		mpDepthStencilView(NULL),
		mpColorRenderTargetView(NULL),
		mpColorUAV(NULL),
        mpTextureStaging(NULL),
        CPUTTexture(name)
    {}
    CPUTTextureDX11(cString &name, ID3D11Resource *pTextureResource, ID3D11ShaderResourceView *pSrv ) :
		mpColorBuffer(NULL),
  		mpColorRenderTargetView(NULL),
        mpTextureStaging(NULL),
		mpDepthStencilView(NULL),
		mpColorUAV(NULL),
        CPUTTexture(name)
    {
        mpShaderResourceView = pSrv;
        if(mpShaderResourceView) pSrv->AddRef();
        mpTexture = pTextureResource;
        if(mpTexture) mpTexture->AddRef();
    }

    void ReleaseTexture()
    {
        SAFE_RELEASE(mpDepthStencilView);
        SAFE_RELEASE(mpShaderResourceView);
        SAFE_RELEASE(mpColorUAV);
        SAFE_RELEASE(mpTexture);
        SAFE_RELEASE(mpColorBuffer);
		SAFE_RELEASE(mpColorRenderTargetView);

		
    }
    void SetTexture(ID3D11Resource *pTextureResource, ID3D11ShaderResourceView *pSrv )
    {
        mpShaderResourceView = pSrv;
        if(mpShaderResourceView) pSrv->AddRef();

        mpTexture = pTextureResource;
        if(mpTexture) mpTexture->AddRef();
    }


    ID3D11Resource* GetTexture()
    {
        return 	mpTexture;
    }
	
	ID3D11ShaderResourceView* GetShaderResourceView()
    {
        return mpShaderResourceView;
    }
    ID3D11RenderTargetView* GetRenderTargetView()
    {
        return mpColorRenderTargetView;
    }
    void SetRenderTargetView(ID3D11RenderTargetView* pColorRenderTargetView)
    {
         SAFE_RELEASE( mpColorRenderTargetView );
         mpColorRenderTargetView = pColorRenderTargetView;
		 if(pColorRenderTargetView) pColorRenderTargetView->AddRef();
    }

    ID3D11DepthStencilView* GetDepthStencilView()
    {
        return mpDepthStencilView;
    }
    void  SetDepthStencilView(ID3D11DepthStencilView* pDepthStencilView)
    {
        // release any resources we might already be pointing too
        SAFE_RELEASE( mpDepthStencilView );
        mpDepthStencilView = pDepthStencilView;
        mpDepthStencilView->AddRef();

    }
    ID3D11UnorderedAccessView* GetColorUAV()
    {
        return mpColorUAV;
    }	

	DXGI_FORMAT GetFormat(){return mFormat;}

    void SetTextureAndShaderResourceView(ID3D11Resource *pTexture, ID3D11ShaderResourceView *pShaderResourceView)
    {
        // release any resources we might already be pointing too
        SAFE_RELEASE( mpTexture );
        SAFE_RELEASE( mpTextureStaging ); // Now out-of sync.  Will be recreated on next Map().
        SAFE_RELEASE( mpShaderResourceView );
        mpTexture = pTexture;
        if( mpTexture ) mpTexture->AddRef();
        mpShaderResourceView = pShaderResourceView;
        mpShaderResourceView->AddRef();
    }
	D3D11_MAPPED_SUBRESOURCE  MapTexture(   CPUTRenderParameters &params, eCPUTMapType type, bool wait=true );
	void                      UnmapTexture( CPUTRenderParameters &params );
};

#endif //_CPUTTEXTUREDX11_H
