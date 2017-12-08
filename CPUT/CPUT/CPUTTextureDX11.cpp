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

#include "CPUTTextureDX11.h"

#include "DDSTextureLoader.h"

// TODO: Would be nice to find a better place for this decl.  But, not another file just for this.
const cString gDXGIFormatNames[] =
{
    _L("DXGI_FORMAT_UNKNOWN"),
    _L("DXGI_FORMAT_R32G32B32A32_TYPELESS"),
    _L("DXGI_FORMAT_R32G32B32A32_FLOAT"),
    _L("DXGI_FORMAT_R32G32B32A32_UINT"),
    _L("DXGI_FORMAT_R32G32B32A32_SINT"),
    _L("DXGI_FORMAT_R32G32B32_TYPELESS"),
    _L("DXGI_FORMAT_R32G32B32_FLOAT"),
    _L("DXGI_FORMAT_R32G32B32_UINT"),
    _L("DXGI_FORMAT_R32G32B32_SINT"),
    _L("DXGI_FORMAT_R16G16B16A16_TYPELESS"),
    _L("DXGI_FORMAT_R16G16B16A16_FLOAT"),
    _L("DXGI_FORMAT_R16G16B16A16_UNORM"),
    _L("DXGI_FORMAT_R16G16B16A16_UINT"),
    _L("DXGI_FORMAT_R16G16B16A16_SNORM"),
    _L("DXGI_FORMAT_R16G16B16A16_SINT"),
    _L("DXGI_FORMAT_R32G32_TYPELESS"),
    _L("DXGI_FORMAT_R32G32_FLOAT"),
    _L("DXGI_FORMAT_R32G32_UINT"),
    _L("DXGI_FORMAT_R32G32_SINT"),
    _L("DXGI_FORMAT_R32G8X24_TYPELESS"),
    _L("DXGI_FORMAT_D32_FLOAT_S8X24_UINT"),
    _L("DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS"),
    _L("DXGI_FORMAT_X32_TYPELESS_G8X24_UINT"),
    _L("DXGI_FORMAT_R10G10B10A2_TYPELESS"),
    _L("DXGI_FORMAT_R10G10B10A2_UNORM"),
    _L("DXGI_FORMAT_R10G10B10A2_UINT"),
    _L("DXGI_FORMAT_R11G11B10_FLOAT"),
    _L("DXGI_FORMAT_R8G8B8A8_TYPELESS"),
    _L("DXGI_FORMAT_R8G8B8A8_UNORM"),
    _L("DXGI_FORMAT_R8G8B8A8_UNORM_SRGB"),
    _L("DXGI_FORMAT_R8G8B8A8_UINT"),
    _L("DXGI_FORMAT_R8G8B8A8_SNORM"),
    _L("DXGI_FORMAT_R8G8B8A8_SINT"),
    _L("DXGI_FORMAT_R16G16_TYPELESS"),
    _L("DXGI_FORMAT_R16G16_FLOAT"),
    _L("DXGI_FORMAT_R16G16_UNORM"),
    _L("DXGI_FORMAT_R16G16_UINT"),
    _L("DXGI_FORMAT_R16G16_SNORM"),
    _L("DXGI_FORMAT_R16G16_SINT"),
    _L("DXGI_FORMAT_R32_TYPELESS"),
    _L("DXGI_FORMAT_D32_FLOAT"),
    _L("DXGI_FORMAT_R32_FLOAT"),
    _L("DXGI_FORMAT_R32_UINT"),
    _L("DXGI_FORMAT_R32_SINT"),
    _L("DXGI_FORMAT_R24G8_TYPELESS"),
    _L("DXGI_FORMAT_D24_UNORM_S8_UINT"),
    _L("DXGI_FORMAT_R24_UNORM_X8_TYPELESS"),
    _L("DXGI_FORMAT_X24_TYPELESS_G8_UINT"),
    _L("DXGI_FORMAT_R8G8_TYPELESS"),
    _L("DXGI_FORMAT_R8G8_UNORM"),
    _L("DXGI_FORMAT_R8G8_UINT"),
    _L("DXGI_FORMAT_R8G8_SNORM"),
    _L("DXGI_FORMAT_R8G8_SINT"),
    _L("DXGI_FORMAT_R16_TYPELESS"),
    _L("DXGI_FORMAT_R16_FLOAT"),
    _L("DXGI_FORMAT_D16_UNORM"),
    _L("DXGI_FORMAT_R16_UNORM"),
    _L("DXGI_FORMAT_R16_UINT"),
    _L("DXGI_FORMAT_R16_SNORM"),
    _L("DXGI_FORMAT_R16_SINT"),
    _L("DXGI_FORMAT_R8_TYPELESS"),
    _L("DXGI_FORMAT_R8_UNORM"),
    _L("DXGI_FORMAT_R8_UINT"),
    _L("DXGI_FORMAT_R8_SNORM"),
    _L("DXGI_FORMAT_R8_SINT"),
    _L("DXGI_FORMAT_A8_UNORM"),
    _L("DXGI_FORMAT_R1_UNORM"),
    _L("DXGI_FORMAT_R9G9B9E5_SHAREDEXP"),
    _L("DXGI_FORMAT_R8G8_B8G8_UNORM"),
    _L("DXGI_FORMAT_G8R8_G8B8_UNORM"),
    _L("DXGI_FORMAT_BC1_TYPELESS"),
    _L("DXGI_FORMAT_BC1_UNORM"),
    _L("DXGI_FORMAT_BC1_UNORM_SRGB"),
    _L("DXGI_FORMAT_BC2_TYPELESS"),
    _L("DXGI_FORMAT_BC2_UNORM"),
    _L("DXGI_FORMAT_BC2_UNORM_SRGB"),
    _L("DXGI_FORMAT_BC3_TYPELESS"),
    _L("DXGI_FORMAT_BC3_UNORM"),
    _L("DXGI_FORMAT_BC3_UNORM_SRGB"),
    _L("DXGI_FORMAT_BC4_TYPELESS"),
    _L("DXGI_FORMAT_BC4_UNORM"),
    _L("DXGI_FORMAT_BC4_SNORM"),
    _L("DXGI_FORMAT_BC5_TYPELESS"),
    _L("DXGI_FORMAT_BC5_UNORM"),
    _L("DXGI_FORMAT_BC5_SNORM"),
    _L("DXGI_FORMAT_B5G6R5_UNORM"),
    _L("DXGI_FORMAT_B5G5R5A1_UNORM"),
    _L("DXGI_FORMAT_B8G8R8A8_UNORM"),
    _L("DXGI_FORMAT_B8G8R8X8_UNORM"),
    _L("DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM"),
    _L("DXGI_FORMAT_B8G8R8A8_TYPELESS"),
    _L("DXGI_FORMAT_B8G8R8A8_UNORM_SRGB"),
    _L("DXGI_FORMAT_B8G8R8X8_TYPELESS"),
    _L("DXGI_FORMAT_B8G8R8X8_UNORM_SRGB"),
    _L("DXGI_FORMAT_BC6H_TYPELESS"),
    _L("DXGI_FORMAT_BC6H_UF16"),
    _L("DXGI_FORMAT_BC6H_SF16"),
    _L("DXGI_FORMAT_BC7_TYPELESS"),
    _L("DXGI_FORMAT_BC7_UNORM"),
    _L("DXGI_FORMAT_BC7_UNORM_SRGB")
};
const cString *gpDXGIFormatNames = gDXGIFormatNames;

//-----------------------------------------------------------------------------
CPUTTexture *CPUTTextureDX11::CreateTexture( const cString &name, const cString &absolutePathAndFilename, bool loadAsSRGB )
{
    // TODO:  Delegate to derived class.  We don't currently have CPUTTextureDX11
    ID3D11ShaderResourceView *pShaderResourceView = NULL;
    ID3D11Resource *pTexture = NULL;
    ID3D11Device *pD3dDevice= CPUT_DX11::GetDevice();
    CPUTResult result = CreateNativeTexture( pD3dDevice, absolutePathAndFilename, &pShaderResourceView, &pTexture, loadAsSRGB );
    ASSERT( CPUTSUCCESS(result), _L("Error loading texture: '")+absolutePathAndFilename );
    UNREFERENCED_PARAMETER(result);

    CPUTTextureDX11 *pNewTexture = new CPUTTextureDX11();
    pNewTexture->mName = name;
    pNewTexture->SetTextureAndShaderResourceView( pTexture, pShaderResourceView );
    pTexture->Release();
    pShaderResourceView->Release();

    CPUTAssetLibrary::GetAssetLibrary()->AddTexture( absolutePathAndFilename, _L(""), _L(""), pNewTexture);

    return pNewTexture;
}

//-----------------------------------------------------------------------------
CPUTResult CPUTTextureDX11::CreateNativeTexture(
    ID3D11Device *pD3dDevice,
    const cString &fileName,
    ID3D11ShaderResourceView **ppShaderResourceView,
    ID3D11Resource **ppTexture,
    bool ForceLoadAsSRGB)
{
    HRESULT hr;

    hr = DirectX::CreateDDSTextureFromFileEx(
        pD3dDevice,
        fileName.c_str(),
        0,//maxsize
        D3D11_USAGE_DEFAULT,
        D3D11_BIND_SHADER_RESOURCE,
        0,
        0,
        ForceLoadAsSRGB,
        ppTexture,
        ppShaderResourceView);
	if( FAILED( hr ) )
	{
		ASSERT( false, _L("Failed to load texture: ") + fileName )
		return CPUT_TEXTURE_LOAD_ERROR;
	}

    CPUTSetDebugName( *ppTexture, fileName );
    CPUTSetDebugName( *ppShaderResourceView, fileName );

    return CPUT_SUCCESS;
}

void CPUTTextureDX11::CreateNativeTexture(DXGI_FORMAT internalFormat, int width, int height,DXGI_FORMAT format, UINT BindFlags, UINT        multiSampleCount)
{
    HRESULT result;
    ID3D11Resource *pTexture = NULL;
    ID3D11ShaderResourceView *pShaderResourceView = NULL;

    mWidth			= width;
    mHeight			= height;
    mFormat			= format;
	mInternalFormat = internalFormat;
	mBindFlags = BindFlags;
    mMultiSampleCount	= multiSampleCount;

    D3D_FEATURE_LEVEL featureLevel = gpSample->GetFeatureLevel();
    bool supportsResourceView = ( featureLevel >= D3D_FEATURE_LEVEL_10_1) || (multiSampleCount==1);

    D3D11_TEXTURE2D_DESC depthDesc = {
		(UINT)width,
		(UINT)height,
		(UINT)1, // MIP Levels
		(UINT)1, // Array Size
        internalFormat, 
        multiSampleCount, (UINT)0,
        D3D11_USAGE_DEFAULT,
        BindFlags | (supportsResourceView ? D3D11_BIND_SHADER_RESOURCE : (UINT)0),
		(UINT)0, // CPU Access flags
        (UINT)0 // Misc flags
    };

     ID3D11Device *pD3dDevice = CPUT_DX11::GetDevice();
    result = pD3dDevice->CreateTexture2D( &depthDesc, NULL, (ID3D11Texture2D**)&pTexture );
    ASSERT( SUCCEEDED(result), _L("Failed creating depth texture.\nAre you using MSAA with a DX10.0 GPU?\nOnly DX10.1 and above can create a shader resource view for an MSAA depth texture.") );

    if( supportsResourceView )
    {
        D3D11_SRV_DIMENSION srvDimension = (multiSampleCount>1) ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
        // Create the shader-resource view
        D3D11_SHADER_RESOURCE_VIEW_DESC depthRsDesc =
        {
            format,
            srvDimension,
            0
        };
        // TODO: Support optionally creating MIP chain.  Then, support MIP generation (e.g., GenerateMIPS()).
        depthRsDesc.Texture2D.MipLevels = 1;

        result = pD3dDevice->CreateShaderResourceView( pTexture, &depthRsDesc, &pShaderResourceView );
	}

    SetTextureAndShaderResourceView( pTexture, pShaderResourceView );
    pTexture->Release();
    pShaderResourceView->Release();

}


CPUTTexture *CPUTTextureDX11::CreateTexture(const cString &name, DXGI_FORMAT internalFormat, int width, int height,DXGI_FORMAT format, UINT BindFlags, UINT        multiSampleCount)
{
    CPUTTextureDX11 *pNewTexture = new CPUTTextureDX11();
    pNewTexture->mName = name;
    pNewTexture->CreateNativeTexture( internalFormat,  width,  height, format,  BindFlags, multiSampleCount);
   
	if(BindFlags&D3D11_BIND_RENDER_TARGET)
	{
		pNewTexture->AddRenderTargetView();
	}

    CPUTAssetLibrary::GetAssetLibrary()->AddTexture(name, _L(""), _L(""), pNewTexture);
    return pNewTexture;
}

void CPUTTextureDX11::Resize(int width, int height)
{
    SAFE_RELEASE(mpDepthStencilView);
    SAFE_RELEASE(mpShaderResourceView);
    SAFE_RELEASE(mpColorUAV);
	SAFE_RELEASE(mpColorRenderTargetView);

	CreateNativeTexture( mInternalFormat,  width,  height, mFormat,  mBindFlags, mMultiSampleCount);
	if(mBindFlags&D3D11_BIND_RENDER_TARGET)
	{
		AddRenderTargetView();
	}
	if(mBindFlags&D3D11_BIND_UNORDERED_ACCESS)
	{
		AddUAVView(mUAVFormat);
	}
	if(mBindFlags&D3D11_BIND_DEPTH_STENCIL)
	{
		AddDepthView(mdepthFormat);
	}
}


void CPUTTextureDX11::AddRenderTargetView()
{
    HRESULT result;
    ID3D11Device *pD3dDevice = CPUT_DX11::GetDevice();

	// Choose our render target.  If MSAA, then use the MSAA texture, and use resolve to fill the non-MSAA texture.
	ID3D11Texture2D *pColorTexture = (mMultiSampleCount>1) ? (ID3D11Texture2D*)mpTexture : (ID3D11Texture2D*)mpTexture;

	D3D11_RENDER_TARGET_VIEW_DESC rtDesc = { mFormat, D3D11_RTV_DIMENSION_TEXTURE2D, 0 };
	rtDesc.Texture2D.MipSlice = 0;

	if(mInternalFormat!=mFormat)
		result = pD3dDevice->CreateRenderTargetView( pColorTexture, &rtDesc, &mpColorRenderTargetView );
	else
		result = pD3dDevice->CreateRenderTargetView( pColorTexture,NULL, &mpColorRenderTargetView );
	ASSERT( SUCCEEDED(result), _L("Failed creating render target view") );
	CPUTSetDebugName( mpColorRenderTargetView, mName );

    return;
}

void CPUTTextureDX11::AddDepthView(DXGI_FORMAT depthFormat)
{
    HRESULT result;
	mdepthFormat = depthFormat;

   // Create either a Texture2D, or Texture2DMS, depending on multisample count.
    D3D11_DSV_DIMENSION dsvDimension = (mMultiSampleCount>1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvd = { depthFormat, dsvDimension, 0 };

    ID3D11Device *pD3dDevice = CPUT_DX11::GetDevice();
    result = pD3dDevice->CreateDepthStencilView( mpTexture, &dsvd, &mpDepthStencilView );
    ASSERT( SUCCEEDED(result), _L("Failed creating depth stencil view") );
    CPUTSetDebugName( mpDepthStencilView, mName );

    return;
}

void CPUTTextureDX11::AddUAVView(DXGI_FORMAT uavFormat)
{
    HRESULT result;

	mUAVFormat = uavFormat;
    // D3D11_SHADER_RESOURCE_VIEW_DESC srDesc = { colorFormat, D3D_SRV_DIMENSION_BUFFER, 0 };
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    memset( &uavDesc, 0, sizeof(uavDesc) );
    uavDesc.Format = uavFormat;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Texture2D.MipSlice = 0;
    ID3D11Device *pD3dDevice = CPUT_DX11::GetDevice();
	result = pD3dDevice->CreateUnorderedAccessView( mpTexture, &uavDesc, &mpColorUAV );
    ASSERT( SUCCEEDED(result), _L("Failed creating render target buffer shader resource view") );
    CPUTSetDebugName( mpColorUAV, mName + _L(" UAV Buffer") );

	if(!mpColorBuffer)
	{
		mpColorBuffer = new CPUTBufferDX11(mName, NULL, mpColorUAV); // We don't have an ID3D11Buffer, but we want to track this UAV as if the texture was a buffer.
		CPUTAssetLibrary::GetAssetLibrary()->AddBuffer( mName, _L(""), _L(""), mpColorBuffer );
	}
 	((CPUTBufferDX11*)mpColorBuffer)->SetUnorderedAccessView(mpColorUAV);

    return;
}

// This function returns the DXGI string equivalent of the DXGI format for
// error reporting/display purposes
//-----------------------------------------------------------------------------
const cString &CPUTTextureDX11::GetDXGIFormatString(DXGI_FORMAT format)
{
    ASSERT( (format>=0) && (format<=DXGI_FORMAT_BC7_UNORM_SRGB), _L("Invalid DXGI Format.") );
    return gpDXGIFormatNames[format];
}

//-----------------------------------------------------------------------------
D3D11_MAPPED_SUBRESOURCE CPUTTextureDX11::MapTexture( CPUTRenderParameters &params, eCPUTMapType type, bool wait )
{
    // Mapping for DISCARD requires dynamic buffer.  Create dynamic copy?
    // Could easily provide input flag.  But, where would we specify? Don't like specifying in the .set file
    // Because mapping is something the application wants to do - it isn't inherent in the data.
    // Could do Clone() and pass dynamic flag to that.
    // But, then we have two.  Could always delete the other.
    // Could support programatic flag - apply to all loaded models in the .set
    // Could support programatic flag on model.  Load model first, then load set.
    // For now, simply support CopyResource mechanism.
    HRESULT hr;
    ID3D11Device *pD3dDevice = CPUT_DX11::GetDevice();
    CPUTRenderParametersDX *pParamsDX11 = (CPUTRenderParametersDX*)&params;
    ID3D11DeviceContext *pContext = pParamsDX11->mpContext;

    if( !mpTextureStaging )
    {
        // We need to create the texture differently, based on dimension.
        D3D11_RESOURCE_DIMENSION dimension;
        mpTexture->GetType(&dimension);
        switch( dimension )
        {
        case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
            {
                D3D11_TEXTURE1D_DESC desc;
                ((ID3D11Texture1D*)mpTexture)->GetDesc( &desc );
                desc.Usage = D3D11_USAGE_STAGING;
                switch( type )
                {
                case CPUT_MAP_READ:
                    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
                    desc.BindFlags = 0;
                    break;
                case CPUT_MAP_READ_WRITE:
                    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
                    desc.BindFlags = 0;
                    break;
                case CPUT_MAP_WRITE:
                case CPUT_MAP_WRITE_DISCARD:
                case CPUT_MAP_NO_OVERWRITE:
                    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                    desc.BindFlags = 0;
                    break;
                };
                hr = pD3dDevice->CreateTexture1D( &desc, NULL, (ID3D11Texture1D**)&mpTextureStaging );
                ASSERT( SUCCEEDED(hr), _L("Failed to create staging texture") );
                break;
            }
        case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
            {
                D3D11_TEXTURE2D_DESC desc;
                ((ID3D11Texture2D*)mpTexture)->GetDesc( &desc );
                desc.Usage = D3D11_USAGE_STAGING;
                switch( type )
                {
                case CPUT_MAP_READ:
                    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
                    desc.BindFlags = 0;
                    break;
                case CPUT_MAP_READ_WRITE:
                    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
                    desc.BindFlags = 0;
                    break;
                case CPUT_MAP_WRITE:
                case CPUT_MAP_WRITE_DISCARD:
                case CPUT_MAP_NO_OVERWRITE:
                    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                    desc.BindFlags = 0;
                    break;
                };
                hr = pD3dDevice->CreateTexture2D( &desc, NULL, (ID3D11Texture2D**)&mpTextureStaging );
                ASSERT( SUCCEEDED(hr), _L("Failed to create staging texture") );
                break;
            }
        case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
            {
                D3D11_TEXTURE3D_DESC desc;
                ((ID3D11Texture3D*)mpTexture)->GetDesc( &desc );
                desc.Usage = D3D11_USAGE_STAGING;
                switch( type )
                {
                case CPUT_MAP_READ:
                    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
                    desc.BindFlags = 0;
                    break;
                case CPUT_MAP_READ_WRITE:
                    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
                    desc.BindFlags = 0;
                    break;
                case CPUT_MAP_WRITE:
                case CPUT_MAP_WRITE_DISCARD:
                case CPUT_MAP_NO_OVERWRITE:
                    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                    desc.BindFlags = 0;
                    break;
                };
                hr = pD3dDevice->CreateTexture3D( &desc, NULL, (ID3D11Texture3D**)&mpTextureStaging );
                ASSERT( SUCCEEDED(hr), _L("Failed to create staging texture") );
                break;
            }
        default:
            ASSERT(0, _L("Unkown texture dimension") );
            break;
        }
    }
    else
    {
        ASSERT( mMappedType == type, _L("Mapping with a different CPU access than creation parameter.") );
    }
    D3D11_MAPPED_SUBRESOURCE info;
    switch( type )
    {
    case CPUT_MAP_READ:
    case CPUT_MAP_READ_WRITE:
        // TODO: Copying and immediately mapping probably introduces a stall.
        // Expose the copy externally?
        // TODO: copy only if changed?
        // Copy only first time?
        // Copy the GPU version before we read from it.
        pContext->CopyResource( mpTextureStaging, mpTexture );
        break;
    };
    hr = pContext->Map( mpTextureStaging, wait ? 0 : D3D11_MAP_FLAG_DO_NOT_WAIT, (D3D11_MAP)type, 0, &info );
    mMappedType = type;
    return info;
} // CPUTTextureDX11::Map()

//-----------------------------------------------------------------------------
void CPUTTextureDX11::UnmapTexture( CPUTRenderParameters &params )
{
    ASSERT( mMappedType != CPUT_MAP_UNDEFINED, _L("Can't unmap a render target that isn't mapped.") );

    CPUTRenderParametersDX *pParamsDX11 = (CPUTRenderParametersDX*)&params;
    ID3D11DeviceContext *pContext = pParamsDX11->mpContext;

    pContext->Unmap( mpTextureStaging, 0 );

    // If we were mapped for write, then copy staging buffer to GPU
    switch( mMappedType )
    {
    case CPUT_MAP_READ:
        break;
    case CPUT_MAP_READ_WRITE:
    case CPUT_MAP_WRITE:
    case CPUT_MAP_WRITE_DISCARD:
    case CPUT_MAP_NO_OVERWRITE:
        pContext->CopyResource( mpTexture, mpTextureStaging );
        break;
    };
} // CPUTTextureDX11::Unmap()
