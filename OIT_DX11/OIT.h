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

#ifndef __CPUT_SAMPLESTARTDX11_H__
#define __CPUT_SAMPLESTARTDX11_H__

#include <stdio.h>
#include "CPUT_DX11.h"
#include <D3D11.h>
#include <time.h>
#include "CPUTSprite.h"
#include "CPUTScene.h"
#include "CPUTBufferDX11.h"
#include "CPUTBufferDX11.h"
#include "CPUTGPUTimerDX11.h"
#include "AOITDefines.h"
#include "AOIT Technique\AOIT_Technique.h"
#include "..\imgui\imgui.h"
#include "..\imgui\DirectX11\imgui_impl_dx11.h"


// define some controls
const CPUTControlID ID_MAIN_PANEL = 10;
const CPUTControlID ID_SECONDARY_PANEL = 20;
const CPUTControlID ID_FULLSCREEN_BUTTON = 100;
const CPUTControlID ID_NEXTMODEL_BUTTON = 101;
const CPUTControlID ID_VSYNC_CHECKBOX    = 102;
const CPUTControlID ID_TEST_CONTROL = 1000;
const CPUTControlID ID_IGNORE_CONTROL_ID = -1;

// AOIT Controls
const CPUTControlID ID_DEBUG_DISPLAY = 111;
const CPUTControlID ID_ENABLE_STATS = 112;
const CPUTControlID ID_RENDERTYPE = 114;
const CPUTControlID ID_DEPTH = 115;
const CPUTControlID ID_RESOLVE = 116;
const CPUTControlID ID_RENDERTRANSPARENT = 120;





const CPUTControlID ID_AOITT = 200;
const CPUTControlID ID_RT = 201;
const CPUTControlID ID_TT = 202;

struct FrameStats 
{
    float AOITCreationTime;
    float ResolveTime;
    float TotalTime;
	float PrePassTime;
}; 

enum  eRenderType : int
{
	AlphaBlending = 0,
	AlphaBlending_A2C,
	DX11_AOIT,
	ROV_OIT,
	ROV_HDR_OIT
};

//-----------------------------------------------------------------------------
class MySample : public CPUT_DX11
{

	float					m_ScreenWidth;
	float					m_ScreenHeight;
private:
    CPUTAssetSet          *mpConservatorySet;
	CPUTAssetSet          *mpGroundSet;
	CPUTAssetSet		  *mpoutdoorPlantsSet;
	CPUTAssetSet		  *mpindoorPlantsSet;
	CPUTAssetSet		  *mpBarrierSet;
    CPUTCameraController  *mpCameraController;
    CPUTSprite            *mpDebugSprite;
	CPUTSprite			  *mpSkyBoxSprite;
	CPUTSprite			  *mpFSSprite;

	CPUTMaterial			*mpGrassA1Material;
	CPUTMaterial			*mpGrassA2Material;
	CPUTMaterial			*mpLeavesTreeAMaterial;
	CPUTMaterial			*mpPRP_ShrubSmall_DM_AMaterial;
	CPUTMaterial			*mpGlassMaterial;
	CPUTMaterial			*mpFenceMaterial;

    CPUTRenderTargetColor	*mpMSAABackBuffer;
	CPUTRenderTargetDepth	*mpMSAADepthBuffer;

    CPUTText               *mpFPSCounter;
    CPUTRenderTargetDepth  *mpShadowRenderTarget;
	CPUTRenderTargetDepth		  *mpInternalShadowRenderTarget;
    
	// AOIT Stuff
    bool                mROVsSupported;
	AOITTechnique		mPostProcess;

    CPUTSprite            *mpDepthSprite;
	CPUTSprite            *mpResolveSprite;

    // Debug stuff
    CPUTBufferDX11 *    mpAOITDebugViewConsts;
    CPUTMaterial *      mpAOITDebugViewMaterial;
    CPUTMaterial *      mpAOITDebugDepthMaterial;
    CPUTMaterial *      mpAOITDebugMSAADepthMaterial;
	CPUTMaterial *      mpResolveMaterial;
	CPUTMaterial *		mpSkyBoxMaterial;

    bool        mpShowDebugView;
    bool        mpShowDepth;
	bool		mpZoomBox;
	bool 		mbPixelSync;
	bool		mbVsync;
	bool		mbFullscreen;
	bool		mpSortFoliage;
	
#ifdef AOIT_METRICS
	bool        mpShowStats;
#endif


	CPUTGPUTimerDX11    mGPUTimerPrePass;
	CPUTGPUTimerDX11    mGPUTimerAOITCreation;
    CPUTGPUTimerDX11    mGPUTimerResolve;
    CPUTGPUTimerDX11    mGPUTimerAll;



	FrameStats Stats;

    CPUTRenderTargetColor* mpBackBuffer;
    CPUTRenderTargetDepth* mpDepthBuffer;

public:



		eRenderType		mpRenderType;

		enum  eNodeCount : int
		{
			TwoNode = 0,
			FourNode,
			EightNode,
		};
		eNodeCount		mpNodeCount;

		MySample() :
			mpMSAABackBuffer(NULL),
			mpMSAADepthBuffer(NULL),
			mpShadowRenderTarget(NULL),
			mpBackBuffer(NULL),
			mpDepthBuffer(NULL),

			mpInternalShadowRenderTarget(NULL),
			mpConservatorySet(NULL),
			mpGrassA1Material(NULL),
			mpGrassA2Material(NULL),
			mpLeavesTreeAMaterial(NULL),
			mpPRP_ShrubSmall_DM_AMaterial(NULL),
			mpGlassMaterial(NULL),
			mpFenceMaterial(NULL),
			mpGroundSet(NULL),
			mpoutdoorPlantsSet(NULL),
			mpindoorPlantsSet(NULL),
			mpBarrierSet(NULL),
			mpCameraController(NULL),
			mpDebugSprite(NULL),
			mpDepthSprite(NULL),
			mpSkyBoxSprite(NULL),
			mpFSSprite(NULL),
			mbFullscreen(false),
			mbVsync(false),
			mbPixelSync(true),
			mpResolveSprite(NULL),
			mpSortFoliage(false),
			mpShowDebugView(false),
			mpShowStats(false),
			mpZoomBox(false),
		mpNodeCount(TwoNode),
		mpRenderType(eRenderType::AlphaBlending),
		mpShowDepth(false),
        mpAOITDebugViewConsts(NULL),
		mpResolveMaterial(NULL),
		mpSkyBoxMaterial(NULL),
        mpAOITDebugViewMaterial(NULL),
        mpAOITDebugDepthMaterial(NULL),
		mpAOITDebugMSAADepthMaterial(NULL)
    {}
    virtual ~MySample()
    {

	}
    virtual CPUTEventHandledCode HandleKeyboardEvent(CPUTKey key, CPUTKeyState state);
    virtual CPUTEventHandledCode HandleMouseEvent(int x, int y, int wheel, CPUTMouseState state, CPUTEventID message);
    virtual void                 HandleCallbackEvent( CPUTEventID Event, CPUTControlID ControlID, CPUTControl *pControl );
    virtual void                 HandleGUIElementEvent( CPUTEventID Event, CPUTControlID ControlID, CPUTGUIElement* pElement );

    virtual void Create();
    virtual void Render(double deltaSeconds);
    virtual void Update(double deltaSeconds);
    virtual void ResizeWindow(UINT width, UINT height);
  void                 UpdateEnableStatsCheckbox( );
	void RegisterUI();
	void CreateDebugViews();
	void DisplayDebugViews(CPUTRenderParametersDX &renderParams);
	void DeleteDebugViews();
	void RenderText( );
	void CreateCameras(UINT width, UINT height);
	void Shutdown();
	void AddGlobalProperties();
	
	// Added to fix an alignment warning during compilation
	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}

	void operator delete(void* p)
	{
		_mm_free(p);
	}

};
#endif // __CPUT_SAMPLESTARTDX11_H__
