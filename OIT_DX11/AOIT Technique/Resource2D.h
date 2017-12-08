//--------------------------------------------------------------------------------------
// Copyright 2011 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.
//--------------------------------------------------------------------------------------
#pragma once
#include "D3D11.h"

class Resource2D
{
public:
	Resource2D() :
	  m_pT(NULL),
	  m_pSRV(NULL),
	  m_ppUAV(NULL),
	  m_ppRTV(NULL),
	  m_mipLevels(0),
	  m_bindFlags(0)
	  {};
	void Create(ID3D11Device* pD3DDevice, int width, int height, DXGI_FORMAT format, UINT bindflags, int mipLevels);
	void Release();

	ID3D11Texture2D* m_pT;
	ID3D11ShaderResourceView* m_pSRV;
	ID3D11UnorderedAccessView** m_ppUAV;
	ID3D11RenderTargetView** m_ppRTV;
	int m_mipLevels;
	UINT m_bindFlags;

};

class Resource2DArray
{
public:
	Resource2DArray() :
	  m_ppT(NULL),
	  m_ppSRV(NULL),
	  m_ppUAV(NULL),
	  m_ppRTV(NULL),
	  m_levels(0),
	  m_bindFlags(0)
	  {};
	void Create(ID3D11Device* pD3DDevice, int width, int height, DXGI_FORMAT format, UINT bindflags, int levels, bool generateMips = false, int mipLevels = 1);
	void Release();
	ID3D11Texture2D** m_ppT;
	ID3D11ShaderResourceView** m_ppSRV;
	ID3D11UnorderedAccessView** m_ppUAV;
	ID3D11RenderTargetView** m_ppRTV;
	int m_levels;
	UINT m_bindFlags;
};

class ResourceUAV
{
public:
	ResourceUAV() :
	  m_pT(NULL),
	  m_pSRV(NULL),
	  m_pUAV(NULL)
	  {};
	void Create(ID3D11Device* pD3DDevice, int width, int height, int structSize, UINT Flag = 0);
	void Release();

	ID3D11Buffer* m_pT;
	ID3D11ShaderResourceView* m_pSRV;
	ID3D11UnorderedAccessView* m_pUAV;

};
