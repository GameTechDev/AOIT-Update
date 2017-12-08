/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

#include "Resource2D.h"
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) {if((x) != NULL) { (x)->Release(); x = NULL;}}
#endif
#ifndef SAFE_DELETE
#define SAFE_DELETE(x) {if(x != NULL) { delete x; x = NULL; }}
#endif


void Resource2D::Release()
{
	SAFE_RELEASE(m_pT);
	SAFE_RELEASE(m_pSRV);

	if(m_bindFlags & D3D11_BIND_RENDER_TARGET)
	{
		for(int i = 0; i < m_mipLevels; i++)
		{
			SAFE_RELEASE(m_ppRTV[i]);
		}
		SAFE_DELETE(m_ppRTV);
	}
	if(m_bindFlags & D3D11_BIND_UNORDERED_ACCESS)
	{
		for(int i = 0; i < m_mipLevels; i++)
		{
			SAFE_RELEASE(m_ppUAV[i]);
		}
		SAFE_DELETE(m_ppUAV);
	}
	m_mipLevels = 0;
	m_bindFlags = 0;
}


void Resource2D::Create(ID3D11Device* pD3DDevice, int width, int height, DXGI_FORMAT format, UINT bindFlags, int mipLevels)
{
	HRESULT hr = S_OK;

	m_mipLevels = mipLevels;
	m_bindFlags = bindFlags;
	D3D11_TEXTURE2D_DESC tDesc;
    tDesc.ArraySize = 1;
    tDesc.BindFlags = bindFlags;
 
	tDesc.CPUAccessFlags = 0;
    tDesc.Format = format;
    tDesc.Height = height;
    tDesc.Width = width;
    tDesc.MipLevels = m_mipLevels;
    tDesc.MiscFlags = 0;
    tDesc.SampleDesc.Count = 1;
    tDesc.SampleDesc.Quality = 0;
    tDesc.Usage = D3D11_USAGE_DEFAULT;
	
	if(m_mipLevels > 1 && (D3D11_BIND_RENDER_TARGET & m_bindFlags) != 0)
		tDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	hr = pD3DDevice->CreateTexture2D(&tDesc, NULL, &m_pT);

	if(hr != S_OK)
		return;
    
	if(m_bindFlags & D3D11_BIND_RENDER_TARGET)
	{
		D3D11_RENDER_TARGET_VIEW_DESC rDesc;
		rDesc.Format = format;
		rDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		m_ppRTV = (ID3D11RenderTargetView**) new ID3D11RenderTargetView*[m_mipLevels]; // malloc(sizeof(ID3D11RenderTargetView*) * m_mipLevels);
		for(int i = 0; i < m_mipLevels; i++)
		{
			rDesc.Texture2D.MipSlice = i;
			if(m_pT)
    			pD3DDevice->CreateRenderTargetView(m_pT, &rDesc, &m_ppRTV[i]);
		}

	}

	if(m_bindFlags & D3D11_BIND_UNORDERED_ACCESS)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.Flags = 0;
		uavDesc.Buffer.NumElements = width * height;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Format = format;
		m_ppUAV = (ID3D11UnorderedAccessView**) new ID3D11UnorderedAccessView*[m_mipLevels];// malloc(sizeof(ID3D11UnorderedAccessView*) * m_mipLevels);

		for(int i = 0; i < m_mipLevels; i++)
		{
			uavDesc.Texture2D.MipSlice = i;
			if(m_pT)
				pD3DDevice->CreateUnorderedAccessView(m_pT, &uavDesc, &(m_ppUAV[i]));
		}
	}
	if(m_bindFlags & D3D11_BIND_SHADER_RESOURCE)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC sDesc = CD3D11_SHADER_RESOURCE_VIEW_DESC();
		sDesc.Format = format;
		sDesc.Texture2D.MipLevels = m_mipLevels;
		sDesc.Texture2D.MostDetailedMip = 0;
		sDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		if(m_pT)
			hr = pD3DDevice->CreateShaderResourceView(m_pT, &sDesc, &m_pSRV);
	}
}
void Resource2DArray::Release()
{

	for(int i = 0; i < m_levels; i++)
	{
		SAFE_RELEASE(m_ppT[i]);
	}
	SAFE_DELETE(m_ppT);
	
	if(m_bindFlags & D3D11_BIND_SHADER_RESOURCE)
	{
		for(int i = 0; i < m_levels; i++)
		{
			SAFE_RELEASE(m_ppSRV[i]);
		}
		SAFE_DELETE(m_ppSRV);
	}

	if(m_bindFlags & D3D11_BIND_RENDER_TARGET)
	{
		for(int i = 0; i < m_levels; i++)
		{
			SAFE_RELEASE(m_ppRTV[i]);
		}
		SAFE_DELETE(m_ppRTV);
	}
	if(m_bindFlags & D3D11_BIND_UNORDERED_ACCESS)
	{
		for(int i = 0; i < m_levels; i++)
		{
			SAFE_RELEASE(m_ppUAV[i]);
		}
		SAFE_DELETE(m_ppUAV);
	}
	m_levels = 0;
	m_bindFlags = 0;
}


void Resource2DArray::Create(ID3D11Device* pD3DDevice, int width, int height, DXGI_FORMAT format, UINT bindFlags, int levels, bool generateMips, int mipLevels)
{
	HRESULT hr = S_OK;

	m_levels = levels;
	m_bindFlags = bindFlags;
	D3D11_TEXTURE2D_DESC tDesc;
    tDesc.ArraySize = 1;
    tDesc.BindFlags = bindFlags;
 
	tDesc.CPUAccessFlags = 0;
    tDesc.Format = format;
    tDesc.Height = height;
    tDesc.Width = width;
    tDesc.MipLevels = mipLevels;
    tDesc.MiscFlags = 0;
    tDesc.SampleDesc.Count = 1;
    tDesc.SampleDesc.Quality = 0;
    tDesc.Usage = D3D11_USAGE_DEFAULT;
	if(generateMips)
	{
		tDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	}
	m_ppT = (ID3D11Texture2D**)malloc(sizeof(ID3D11Texture2D*) * m_levels);
	for(int i = 0; i < levels; i++)
	{
		hr = pD3DDevice->CreateTexture2D(&tDesc, NULL, &m_ppT[i]);
		tDesc.Width = tDesc.Width / 2;
		tDesc.Height = tDesc.Height / 2;
	}

	if(m_bindFlags & D3D11_BIND_RENDER_TARGET)
	{
		D3D11_RENDER_TARGET_VIEW_DESC rDesc;
		rDesc.Format = format;
		rDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		m_ppRTV = (ID3D11RenderTargetView**)malloc(sizeof(ID3D11RenderTargetView*) * m_levels);
		for(int i = 0; i < m_levels; i++)
		{
			rDesc.Texture2D.MipSlice = 0;
    		pD3DDevice->CreateRenderTargetView(m_ppT[i], &rDesc, &m_ppRTV[i]);
		}
	}

	if(m_bindFlags & D3D11_BIND_UNORDERED_ACCESS)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.Flags = 0;
		uavDesc.Buffer.NumElements = width * height;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Format = format;
		uavDesc.Texture2D.MipSlice = 0;
		m_ppUAV = (ID3D11UnorderedAccessView**)malloc(sizeof(ID3D11UnorderedAccessView*) * m_levels);
		int uavWidth = width;
		int uavHeight = height;
		for(int i = 0; i < m_levels; i++)
		{
			uavDesc.Buffer.NumElements = uavWidth * uavHeight;
			pD3DDevice->CreateUnorderedAccessView(m_ppT[i], &uavDesc, &(m_ppUAV[i]));
			uavWidth = uavWidth/2;
			uavHeight = uavHeight/2;
		}
	}
	if(m_bindFlags & D3D11_BIND_SHADER_RESOURCE)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC sDesc = CD3D11_SHADER_RESOURCE_VIEW_DESC();
		sDesc.Format = format;
		sDesc.Texture2D.MipLevels = mipLevels;
		sDesc.Texture2D.MostDetailedMip = 0;
		sDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		m_ppSRV = (ID3D11ShaderResourceView**)malloc(sizeof(ID3D11ShaderResourceView*) * m_levels);
		for(int i = 0 ; i < m_levels; i++)
		{
			hr = pD3DDevice->CreateShaderResourceView(m_ppT[i], &sDesc, &m_ppSRV[i]);
		}
	}
}


void ResourceUAV::Create(ID3D11Device* pD3DDevice, int width, int height, int structSize, UINT Flag)
{
	HRESULT hr = S_OK;

	const UINT numElem = width * height;
    CD3D11_BUFFER_DESC desc(
        structSize * numElem,
        D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS,
        D3D11_USAGE_DEFAULT,
        0,
        D3D11_RESOURCE_MISC_BUFFER_STRUCTURED,
        structSize);
    hr = pD3DDevice->CreateBuffer(&desc, 0, &m_pT);
		
    CD3D11_UNORDERED_ACCESS_VIEW_DESC unorderedAccessResourceDesc(
        D3D11_UAV_DIMENSION_BUFFER,
        DXGI_FORMAT_UNKNOWN,
        0, numElem, 1, Flag);


    hr = pD3DDevice->CreateUnorderedAccessView(m_pT, 
                                            &unorderedAccessResourceDesc, 
                                            &m_pUAV);

    CD3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceDesc(
        D3D11_SRV_DIMENSION_BUFFER,
        DXGI_FORMAT_UNKNOWN,
        0, numElem, 1);

    hr = pD3DDevice->CreateShaderResourceView(m_pT, 
                                            &shaderResourceDesc, 
                                            &m_pSRV);
}

void ResourceUAV::Release()
{
	SAFE_RELEASE(m_pT);
	SAFE_RELEASE(m_pSRV);
	SAFE_RELEASE(m_pUAV);
}