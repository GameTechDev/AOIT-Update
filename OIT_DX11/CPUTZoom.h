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

#pragma once

#include "CPUT.h"
#include "CPUTGeometrySprite.h"
#include "CPUTBufferDX11.h"


class CPUTZoomBox
{
    float2						mvZoomCenterPos;
    float2                      mvZoomSourceSize;
    int                         mZoomFactor;
	CPUTGeometrySprite*					mpBorderSprite;
	CPUTMaterial*				mpBorderMaterial;
	CPUTMaterial*				mpZoomMaterial;
	CPUTBufferDX11*			    mpZoomBoxConstants;
    ID3D11Texture2D*            m_pZoomT;
    ID3D11ShaderResourceView*   m_pZoomSRV;
	CPUTTextureDX11*            mZoomTxT;

    int                         mScreenWidth;
    int                         mScreenHeight;

public:
    CPUTZoomBox();
    virtual ~CPUTZoomBox();

    void SetZoomCenterPosition(float x, float y);

    HRESULT OnCreate(ID3D11Device* pD3DDevice, ID3D11DeviceContext* pContext, IDXGISwapChain* pSwapChain);
    void    OnShutdown();
	void	OnFrameRender(CPUTRenderParametersDX &renderParams ,bool showEdgeInfo );
    HRESULT OnSize(ID3D11Device* pD3DDevice, int width, int height);

    float2  GetCenterPos( )     { return mvZoomCenterPos; }

    ID3D11Texture2D * GetZoomColourTxtNoAddRef()    { return m_pZoomT; }

private:

};