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

#include "CPUT.h"
#include "OIT.h"
#include "CPUTRenderTarget.h"
#include "CPUTTextureDX11.h"
#include "CPUTZoom.h"

extern CPUTZoomBox                 g_CPUTZoomBox;


struct AOITDebugViewConsts
{
    int a;
    int b;
    int c;
    int d;
};


void MySample::Shutdown()
{
	SAFE_RELEASE(mpResolveMaterial);
	SAFE_RELEASE(mpAOITDebugDepthMaterial);
	SAFE_RELEASE(mpAOITDebugMSAADepthMaterial);
	SAFE_RELEASE(mpAOITDebugViewMaterial);
	SAFE_RELEASE(mpAOITDebugViewConsts);

	g_CPUTZoomBox.OnShutdown();

	DeleteDebugViews();

	if(GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0)
		mPostProcess.OnShutdown();

       // Note: these two are defined in the base.  We release them because we addref them.
		SAFE_RELEASE(mpGrassA1Material);
		SAFE_RELEASE(mpGrassA2Material);
		SAFE_RELEASE(mpLeavesTreeAMaterial);
		SAFE_RELEASE(mpPRP_ShrubSmall_DM_AMaterial);
		SAFE_RELEASE(mpGlassMaterial);
		SAFE_RELEASE(mpFenceMaterial);
		SAFE_RELEASE(mpSkyBoxMaterial);

        SAFE_RELEASE(mpCamera);
		SAFE_RELEASE(mpShadowCamera);
        SAFE_RELEASE(mpConservatorySet);
		SAFE_RELEASE(mpGroundSet);
		SAFE_RELEASE(mpoutdoorPlantsSet);
		SAFE_RELEASE( mpBarrierSet );
        SAFE_DELETE( mpShadowRenderTarget );
		SAFE_DELETE( mpInternalShadowRenderTarget);

		SAFE_RELEASE(mpindoorPlantsSet);
        SAFE_DELETE( mpCameraController );
        SAFE_DELETE( mpDebugSprite);
		SAFE_DELETE( mpSkyBoxSprite );
		SAFE_DELETE( mpFSSprite );
        SAFE_DELETE( mpDebugSprite);
        SAFE_DELETE( mpMSAABackBuffer);
		SAFE_DELETE( mpMSAADepthBuffer);
        SAFE_DELETE( mpBackBuffer);
        SAFE_DELETE( mpDepthBuffer);

    CPUT_DX11::Shutdown();
}

//-----------------------------------------------------------------------------
void MySample::RegisterUI()
{
    CPUTGuiControllerDX11 *pGUI = CPUTGetGuiController();

	int windowWidth, windowHeight;
	mpWindow->GetClientDimensions(&windowWidth, &windowHeight);


	ImGui::SetNextWindowPos(ImVec2((float)(windowWidth-210), 10), ImGuiSetCond_Always);     // Normally user code doesn't need/want to call it because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
	ImGui::SetNextWindowSize(ImVec2((float)200, (float)300), ImGuiSetCond_Always);
	ImGui::SetNextWindowCollapsed(false, ImGuiSetCond_Always);
	ImGui::Begin("Application-Controls", 0, ImVec2(0.f, 0.f), 0.5f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse);

	ImGui::Checkbox("Fullscreen", &mbFullscreen);
	if (ImGui::Checkbox("VSYNC", &mbVsync))
	{
		mSyncInterval = mbVsync;
		UpdateEnableStatsCheckbox();
	}

	const char* itemnames[] = { "Alpha Blending", "Alpha Coverage", "DX11 AOIT", "ROV OIT" ,"ROV HDR OIT" };
	const char* items[6];
	eRenderType itemsenums[6];

	int NumItems = 2;

	items[0] = itemnames[0];
	itemsenums[0] = eRenderType::AlphaBlending;
	items[1] = itemnames[1];
	itemsenums[1] = eRenderType::AlphaBlending_A2C;

	if (GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0)
	{
		itemsenums[NumItems] = eRenderType::DX11_AOIT;
		items[NumItems] = itemnames[2];
		NumItems++;
	}


	if (true == mROVsSupported)
	{
		itemsenums[NumItems] = eRenderType::ROV_OIT;
		itemsenums[NumItems + 1] = eRenderType::ROV_HDR_OIT;
		items[NumItems] = itemnames[3];
		items[NumItems + 1] = itemnames[4];
		NumItems += 2;
	}


	static int RenterOption = 0;
	ImGui::Combo("Type", (int*)&RenterOption, items, NumItems);   // Combo using proper array. You can also pass a callback to retrieve array value, no need to create/copy an array just for that.

	mpRenderType = itemsenums[RenterOption];

	if (mpRenderType == eRenderType::ROV_OIT || mpRenderType == eRenderType::ROV_HDR_OIT)
	{
		if (true == mROVsSupported)
		{
			const char* Nodes[] = { "2 Node", "4 Node", "8 Node" };
			ImGui::Combo("Nodes", (int*)&mpNodeCount, Nodes, ((int)(sizeof(Nodes) / sizeof(*Nodes))));   // Combo using proper array. You can also pass a callback to retrieve array value, no need to create/copy an array just for that.
		}
	}

#ifdef AOIT_METRICS
	if(!mbVsync)
		ImGui::Checkbox("Show Stats", &mpShowStats);
#endif

	ImGui::Checkbox("Write Foliage to Depth", &mpSortFoliage);
	ImGui::Checkbox("DepthBuffer", &mpShowDepth);
	if (mpRenderType == eRenderType::ROV_OIT || mpRenderType == eRenderType::ROV_HDR_OIT)
	{
		ImGui::Checkbox("Enable ROV", &mbPixelSync);
	}

	if (GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0)
	{
		ImGui::Checkbox("Enable Zoom", &mpZoomBox);
	}
	if (mpRenderType == eRenderType::ROV_OIT || mpRenderType == eRenderType::ROV_HDR_OIT)
	{
		if (true == mROVsSupported)
		{
			ImGui::Checkbox("Show ROV debug view", &mpShowDebugView);
		}
	}
	ImGui::End();
}


void MySample::AddGlobalProperties()
{
    CPUTMaterial::mGlobalProperties.AddValue( _L("gFragmentListNodesUAV"), _L("$gFragmentListNodesUAV") );
    CPUTMaterial::mGlobalProperties.AddValue( _L("gFragmentListFirstNodeAddressUAV"), _L("$gFragmentListFirstNodeAddressUAV") );
    CPUTMaterial::mGlobalProperties.AddValue( _L("gAOITSPClearMaskUAV"), _L("$gAOITSPClearMaskUAV") );
    CPUTMaterial::mGlobalProperties.AddValue( _L("gAOITSPDepthDataUAV"), _L("$gAOITSPDepthDataUAV") );
    CPUTMaterial::mGlobalProperties.AddValue( _L("gAOITSPColorDataUAV"), _L("$gAOITSPColorDataUAV") );
    CPUTMaterial::mGlobalProperties.AddValue( _L("g8AOITSPDepthDataUAV"), _L("$g8AOITSPDepthDataUAV") );
    CPUTMaterial::mGlobalProperties.AddValue( _L("g8AOITSPColorDataUAV"), _L("$g8AOITSPColorDataUAV") );
    CPUTMaterial::mGlobalProperties.AddValue( _L("g_IntelExt"), _L("$g_IntelExt") );

	CPUTMaterial::mGlobalProperties.AddValue( _L("cbAOITDebugViewConsts"), _L("$cbAOITDebugViewConsts") );
    CPUTMaterial::mGlobalProperties.AddValue( _L("FL_Constants"), _L("$FL_Constants") );
}

void MySample::CreateDebugViews()
{
    CPUTAssetLibrary *pAssetLibrary = CPUTAssetLibrary::GetAssetLibrary();

    // Create our constant buffer.

    D3D11_BUFFER_DESC bd = {0};
    bd.ByteWidth = sizeof(AOITDebugViewConsts);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    ID3D11Buffer * pOurConstants;
    HRESULT hr = (CPUT_DX11::GetDevice())->CreateBuffer( &bd, NULL, &pOurConstants );

    cString name = _L("$cbAOITDebugViewConsts");
    mpAOITDebugViewConsts = new CPUTBufferDX11( name, pOurConstants );

    CPUTAssetLibrary::GetAssetLibrary()->AddConstantBuffer( name, _L(""), _L(""), mpAOITDebugViewConsts );
    SAFE_RELEASE(pOurConstants); // We're done with it.  The CPUTBuffer now owns it.
    //mGlobalProperties

    cString ExecutableDirectory;
	CPUTFileSystem::GetExecutableDirectory(&ExecutableDirectory);

   pAssetLibrary->SetMediaDirectoryName(ExecutableDirectory +     _L("..\\..\\..\\Media\\DebugAssets\\") );
   mpAOITDebugViewMaterial = pAssetLibrary->GetMaterial( L"AOITDebugView" );
   mpAOITDebugDepthMaterial = pAssetLibrary->GetMaterial( L"AOITDebugDepth" );
   mpAOITDebugMSAADepthMaterial = pAssetLibrary->GetMaterial( L"AOITDebugMSAADepth" );
   mpResolveMaterial = pAssetLibrary->GetMaterial( L"ResolveTarget" );

   mpDebugSprite = CPUTSprite::CreateSprite(0.5f, 0.5f, 0.5f, 0.5f, _L("Sprite") );


   mpDepthSprite = CPUTSprite::CreateSprite(-1.0f, 0.5f, 0.5f, 0.5f, _L("Sprite") );

   mpResolveSprite = CPUTSprite::CreateSprite(-1.0f, -1.0f, 2.0f, 2.0f, _L("ResolveTarget") );

}


void MySample::DisplayDebugViews(CPUTRenderParametersDX &renderParams)
{
	CPUTAssetLibrary *pAssetLibrary = CPUTAssetLibrary::GetAssetLibrary();

	if (mpRenderType == eRenderType::ROV_OIT || mpRenderType == eRenderType::ROV_HDR_OIT)
	{
		if (true == mROVsSupported)
		{
			ID3D11Buffer *pBuffer = mpAOITDebugViewConsts->GetNativeBuffer();
			// update parameters of constant buffer
			D3D11_MAPPED_SUBRESOURCE mapInfo;
			mpContext->Map(pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapInfo);
			{
				AOITDebugViewConsts & consts = *((AOITDebugViewConsts*)mapInfo.pData);

				// for future use
				consts.a = 0;
				consts.b = 8;
				consts.c = 0;
				consts.d = 2;
				mpContext->Unmap(pBuffer, 0);
			}
			mpContext->VSSetConstantBuffers(4, 1, &pBuffer);
			mpContext->PSSetConstantBuffers(4, 1, &pBuffer);
			mpContext->CSSetConstantBuffers(4, 1, &pBuffer);
			if ((mpAOITDebugViewConsts != NULL) && (mpShowDebugView))
			{

				mpDebugSprite->DrawSprite(renderParams, *mpAOITDebugViewMaterial);
				ID3D11ShaderResourceView* nullViews[32] = { 0 };
				mpContext->PSSetShaderResources(0, 32, nullViews);
			}
		}
	}
	if( (mpAOITDebugViewConsts != NULL) && (mpShowDepth) )
	{
		ID3D11RenderTargetView* pSwapChainRTV;		// render target view retrieved at InitFrameRender
		ID3D11DepthStencilView* pDSView;			// depth stencil view retried at InitFrameRender

		mpContext->OMGetRenderTargets(1, &(pSwapChainRTV), &pDSView);

		mpContext->OMSetRenderTargets(1, &pSwapChainRTV, NULL);
		
		if(mpRenderType == eRenderType::AlphaBlending_A2C)
		{
			mpDepthSprite->DrawSprite( renderParams, *mpAOITDebugMSAADepthMaterial );
		}
		else
		{
			mpDepthSprite->DrawSprite( renderParams, *mpAOITDebugDepthMaterial );
		}
		ID3D11ShaderResourceView* nullViews[32] = {0};
		mpContext->PSSetShaderResources(0, 32, nullViews);
		mpContext->OMSetRenderTargets(1, &pSwapChainRTV, pDSView);
		SAFE_RELEASE(pSwapChainRTV);
		SAFE_RELEASE(pDSView);
	}
}


void MySample::DeleteDebugViews(  )
{
	SAFE_DELETE( mpDebugSprite);
	SAFE_DELETE( mpDepthSprite);
	SAFE_DELETE( mpResolveSprite);
}


void MySample::RenderText( )
{
	FrameStats Stats;
    cString SelectedItem;
#ifdef AOIT_METRICS
    if( mpShowStats )
    {
        Stats.AOITCreationTime	= (float)(mGPUTimerAOITCreation.GetAvgTime() * 1000.0);
        Stats.ResolveTime       = (float)(mGPUTimerResolve.GetAvgTime()* 1000.0);
        Stats.TotalTime         = (float)(mGPUTimerAll.GetAvgTime() * 1000.0);
        Stats.PrePassTime         = (float)(mGPUTimerPrePass.GetAvgTime() * 1000.0);
    }
    else
    {
        memset( &Stats, 0, sizeof(Stats) );
    }


	if (mpShowStats)
	{
		ImGui::SetNextWindowPos(ImVec2((float)(10), 10), ImGuiSetCond_Always);     // Normally user code doesn't need/want to call it because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
		if ((mpRenderType == eRenderType::DX11_AOIT) || mpRenderType == eRenderType::ROV_OIT || mpRenderType == eRenderType::ROV_HDR_OIT)
			ImGui::SetNextWindowSize(ImVec2((float)300, (float)80), ImGuiSetCond_Always);
		else
			ImGui::SetNextWindowSize(ImVec2((float)300, (float)50), ImGuiSetCond_Always);
		ImGui::SetNextWindowCollapsed(false, ImGuiSetCond_Always);
		ImGui::Begin("Metrics", 0, ImVec2(0.f, 0.f), 0.5f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse);

		if (mpRenderType == eRenderType::ROV_OIT || mpRenderType == eRenderType::ROV_HDR_OIT || mpRenderType == eRenderType::DX11_AOIT)
		{
			ImGui::Text("Transparent Render Time (ms): %.2f", Stats.AOITCreationTime);
			ImGui::Text("ResolveTime time (ms): %.2f", Stats.ResolveTime);
		}
		ImGui::Text("Solid Geometry Render time (ms): %.2f", Stats.PrePassTime);
		ImGui::Text("TotalTime (ms) (ms): %.2f", Stats.TotalTime);

		ImGui::End();
	}
#endif
}

