/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

#include "CPUT.h"
#ifdef CPUT_FOR_DX11
#include "CPUT_DX11.h"
#include "CPUTGeometrySprite.h"
#include "CPUTAssetLibrary.h"
#include "CPUTMaterial.h"
#include "CPUTMaterialEffectDX11.h"
#include "CPUTTextureDX11.h"
#include "CPUTInputLayoutCacheDX11.h"

class SpriteVertex
{
public:
    float mpPos[3];
    float mpUV[2];
};

//-----------------------------------------------
CPUTGeometrySprite::~CPUTGeometrySprite()
{
    SAFE_RELEASE( mpVertexBuffer );
    SAFE_RELEASE( mpMaterial );
    SAFE_RELEASE( mpInputLayout );
}

//-----------------------------------------------
HRESULT CPUTGeometrySprite::CreateSprite(
    float          spriteX,
    float          spriteY,
    float          spriteWidth,
    float          spriteHeight,
    const cString &spriteMaterialName
)
{
    HRESULT result;

    // Create resources so we can draw a sprite using the render target as a texture
    mpMaterial = CPUTAssetLibrary::GetAssetLibrary()->GetMaterial( spriteMaterialName, false );

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC pLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        {0}
    };
    CPUTVertexShaderDX11 *pVertexShader = ((CPUTMaterialEffectDX11*)mpMaterial->GetMaterialEffects()[0])->GetVertexShader();
    ID3D11Device *pD3dDevice = CPUT_DX11::GetDevice();
	if(pVertexShader)
	{
		CPUTInputLayoutCacheDX11::GetInputLayoutCache()->GetLayout( pD3dDevice, pLayout, pVertexShader, &mpInputLayout);

		// ***************************************************
		// Create Vertex Buffers
		// ***************************************************
		D3D11_BUFFER_DESC bd;
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(SpriteVertex) * 1; // 2 tris, 3 verts each vertices
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bd.MiscFlags = 0;

		SpriteVertex pVertices[] = {
			{  spriteX,    -spriteY, 1.0f,   spriteWidth, spriteHeight },
		};
		D3D11_SUBRESOURCE_DATA initialData;
		initialData.pSysMem = pVertices;
		initialData.SysMemPitch = sizeof( SpriteVertex );
		initialData.SysMemSlicePitch = 0;

		result = pD3dDevice->CreateBuffer( &bd, &initialData, &mpVertexBuffer );
		ASSERT( SUCCEEDED(result), _L("Failed creating render target debug-sprite vertex buffer") );
		CPUTSetDebugName( mpVertexBuffer, _L("CPUTGeometrySprite vertex buffer") );
	}
    return S_OK;
} // CPUTGeometrySprite::CreateSprite()

//-----------------------------------------
void CPUTGeometrySprite::DrawSprite(
    CPUTRenderParameters &renderParams,
    CPUTMaterial         &material
)
{
    // TODO: Should we warn here?
    // If it doesn't draw, make sure you created it with createDebugSprite == true
    if( mpVertexBuffer )
    {
        ID3D11DeviceContext *pContext = ((CPUTRenderParametersDX*)&renderParams)->mpContext;

        material.GetMaterialEffects()[0]->SetRenderStates(renderParams);

        UINT stride = sizeof( SpriteVertex );
        UINT offset = 0;
        pContext->IASetVertexBuffers( 0, 1, &mpVertexBuffer, &stride, &offset );

        // Set the input layout
        pContext->IASetInputLayout( mpInputLayout );

        // Set primitive topology
        pContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );

        pContext->Draw( 1, 0 );
    }

} // CPUTGeometrySprite::DrawSprite()

#endif

