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

#include "d3d11_3.h"

#include "OIT.h"
#include "CPUTRenderTarget.h"
#include <CPUTFont.h>
#include "CPUTBufferDX11.h"
#include <DXGIDebug.h>
#include "CPUTTextureDX11.h"
#include "CPUTZoom.h"

#include "..\imgui\imgui.h"
#include "..\imgui\DirectX11\imgui_impl_dx11.h"


const UINT SHADOW_WIDTH_HEIGHT = 8192;
// set file to open
cString g_OpenFilePath;
cString g_OpenShaderPath;
cString g_OpenFileName;
std::string g_OpenSceneFileName;
CPUTZoomBox                 g_CPUTZoomBox;


struct DrawNode{

	DrawNode(CPUTModelDX11* pModel, CPUTMeshDX11* pMesh, CPUTMaterialEffect *pMaterial, ID3D11InputLayout *mpInputLayout)
	{
		m_pMesh = pMesh;
		m_pModel = pModel;
		m_pMaterial = pMaterial;
		m_mpInputLayout = mpInputLayout;
	}

	CPUTModelDX11* m_pModel;
	CPUTMeshDX11* m_pMesh;
	CPUTMaterialEffect *m_pMaterial;
	ID3D11InputLayout *m_mpInputLayout;
};

static std::vector<DrawNode> m_pMeshNode;
static std::vector<DrawNode> m_pTransparentNode;



void BeginDrawLists()
{
	m_pMeshNode.clear();
	m_pTransparentNode.clear();
}


void AddModel(CPUTModelDX11* pModel,CPUTMeshDX11* pMesh, CPUTMaterialEffect *pMaterial, ID3D11InputLayout *mpInputLayout, bool Transparent)
{
	if(Transparent)
	{
   		m_pTransparentNode.push_back(DrawNode(pModel, pMesh, pMaterial, mpInputLayout ));	
	}
	else
	{
   		m_pMeshNode.push_back(DrawNode(pModel, pMesh, pMaterial, mpInputLayout ));	
	}
}

void DrawSolidLists(CPUTRenderParameters &renderParams)
{
	CPUTModelDX11* pCurrentModel = NULL;

	for(unsigned int x=0; x< m_pMeshNode.size(); x++)
	{
		// Update the model's constant buffer.
		// Note that the materials reference this, so we need to do it only once for all of the model's meshes.
		if(pCurrentModel != m_pMeshNode[x].m_pModel)
		{
			pCurrentModel = m_pMeshNode[x].m_pModel;
			pCurrentModel->UpdateShaderConstants(renderParams);
		}

        m_pMeshNode[x].m_pMaterial->SetRenderStates(renderParams);

		m_pMeshNode[x].m_pMesh->Draw(renderParams, m_pMeshNode[x].m_mpInputLayout);
	}
}

void DrawTransparentLists(CPUTRenderParameters &renderParams, bool RenderTransparent)
{
	CPUTModelDX11* pCurrentModel = NULL;

	if(RenderTransparent)
	{
		for(unsigned int x=0; x< m_pTransparentNode.size(); x++)
		{
			// Update the model's constant buffer.
			// Note that the materials reference this, so we need to do it only once for all of the model's meshes.
			if(pCurrentModel != m_pTransparentNode[x].m_pModel)
			{
				pCurrentModel = m_pTransparentNode[x].m_pModel;
				pCurrentModel->UpdateShaderConstants(renderParams);
			}

			m_pTransparentNode[x].m_pMaterial->SetRenderStates(renderParams);

			m_pTransparentNode[x].m_pMesh->Draw(renderParams, m_pTransparentNode[x].m_mpInputLayout);
		}
	}
}


bool DrawModelCallBack(CPUTModel* pModel, CPUTRenderParameters &renderParams, CPUTMesh* pMesh, CPUTMaterialEffect* pMaterial,CPUTMaterialEffect* pOriginalMaterial, void* pInputLayout)
{
	ID3D11InputLayout *pLayout = (ID3D11InputLayout *)pInputLayout;
	pMaterial->SetRenderStates(renderParams);

	AddModel((CPUTModelDX11*)pModel, (CPUTMeshDX11*)pMesh, (CPUTMaterialEffectDX11*)pMaterial, pLayout,((CPUTMaterialEffectDX11*)pOriginalMaterial)->mLayer == CPUT_LAYER_TRANSPARENT);

	return true;
}

// Application entry point.  Execution begins here.
//-----------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    // Prevent unused parameter compiler warnings
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(nCmdShow);

#ifdef DEBUG
    // tell VS to report leaks at any exit of the program
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	//http://msdn.microsoft.com/en-us/library/x98tx3cf%28v=vs.100%29.aspx
	//Add a watch for “{,,msvcr100d.dll}_crtBreakAlloc” to the watch window
	//Set the value of the watch to the memory allocation number reported by your sample at exit.
	//Note that the “msvcr100d.dll” is for MSVC2010.  Other versions of MSVC use different versions of this dl; you’ll need to specify the appropriate version.

#endif

    CPUTResult result=CPUT_SUCCESS;
    int returnCode=0;

	CPUTModel::SetDrawModelCallBack(DrawModelCallBack);

    // create an instance of my sample
    MySample *pSample = new MySample();

    // window and device parameters
    CPUTWindowCreationParams params;
    params.deviceParams.refreshRate         = 0;
    params.deviceParams.swapChainBufferCount= 1;
    params.deviceParams.swapChainFormat     = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    params.deviceParams.swapChainUsage      = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;

    // parse out the parameter settings or reset them to defaults if not specified
    std::string AssetFilename;
    g_OpenSceneFileName = "../../../Media/defaultscene.scene";
    char *pCommandLine = ws2s(lpCmdLine);
    std::string CommandLine(pCommandLine);
    pSample->CPUTParseCommandLine(CommandLine, &params, &AssetFilename, &g_OpenSceneFileName);
    delete pCommandLine;

    // parse out the filename of the .set file to open (if one was given)
    if(AssetFilename.size())
    {
        // strip off the target .set file, and parse it's full path
        cString PathAndFilename, Drive, Dir, Ext;  
        g_OpenFilePath.clear();
        std::wstring wsAssetFileName = s2ws(AssetFilename.c_str());
        
        // resolve full path, and check to see if the file actually exists
        CPUTFileSystem::ResolveAbsolutePathAndFilename(wsAssetFileName, &PathAndFilename);
        result = CPUTFileSystem::DoesFileExist(PathAndFilename);
        if(CPUTFAILED(result))
        {
            cString ErrorMessage = _L("File specified in the command line does not exist: \n")+PathAndFilename;
            #ifndef DEBUG
            DEBUGMESSAGEBOX(_L("Error loading command line file"), ErrorMessage);
            #else
            ASSERT(false, ErrorMessage);
            #endif
        }
        
        // now parse through the path and removing the trailing \\Asset\\ directory specification
        CPUTFileSystem::SplitPathAndFilename(PathAndFilename, &Drive, &Dir, &g_OpenFileName, &Ext);
        // strip off any trailing \\ 
        size_t index=Dir.size()-1;
        if(Dir[index]=='\\' || Dir[index]=='/')
        {
            index--;
        }

        // strip off \\Asset
        for(size_t ii=index; ii>=0; ii--)
        {
            if(Dir[ii]=='\\' || Dir[ii]=='/')
            {
                Dir = Dir.substr(0, ii+1); 
                g_OpenFilePath = Drive+Dir; 
                index=ii;
                break;
            }
        }        
        
        // strip off \\<setname> 
        for(size_t ii=index; ii>=0; ii--)
        {
            if(Dir[ii]=='\\' || Dir[ii]=='/')
            {
                Dir = Dir.substr(0, ii+1); 
                g_OpenShaderPath = Drive + Dir + _L("\\Shader\\");
                break;
            }
        }
    }

    // create the window and device context
    result = pSample->CPUTCreateWindowAndContext(_L("OIT Sample using Raster Order Views"), params);
    ASSERT( CPUTSUCCESS(result), _L("CPUT Error creating window and context.") );
    
    // start the main message loop
    returnCode = pSample->CPUTMessageLoop();

    pSample->DeviceShutdown();

    // cleanup resources
    SAFE_DELETE(pSample);

#if 0
    //
    // Report on unreleased DX objects. Requires the D3D11_CREATE_DEVICE_DEBUG flag be set.
    //
    typedef HRESULT(__stdcall *fPtrDXGIGetDebugInterface)(const IID&, void**);
    HMODULE hMod = GetModuleHandle(L"Dxgidebug.dll");
    fPtrDXGIGetDebugInterface DXGIGetDebugInterface = (fPtrDXGIGetDebugInterface)GetProcAddress(hMod, "DXGIGetDebugInterface"); 
 
    IDXGIDebug *pDebugInterface;
    DXGIGetDebugInterface(__uuidof(IDXGIDebug), (void**)&pDebugInterface); 

    pDebugInterface->ReportLiveObjects(DXGI_DEBUG_D3D11, DXGI_DEBUG_RLO_ALL);
#endif

    return returnCode;
}

//-----------------------------------------------------------------------------
void MySample::CreateCameras(UINT width, UINT height)
{
    CPUTAssetLibrary *pAssetLibrary = CPUTAssetLibrary::GetAssetLibrary();

    // If no cameras were created from the model sets then create a default simple camera
    if( mpConservatorySet && mpConservatorySet->GetCameraCount() )
    {
        mpCamera = mpConservatorySet->GetFirstCamera();
        mpCamera->AddRef(); // TODO: why?  Shouldn't need this.
    } 
	else
    {
        mpCamera = new CPUTCamera();
        pAssetLibrary->AddCamera( _L("Camera"),_L(""), _L(""),  mpCamera );

        mpCamera->SetPosition( 26.00f, 3.00f, 25.00f );
		mpCamera->LookAt(11.00f,5.33f,15.00f);
        // Set the projection matrix for all of the cameras to match our window.
        // TODO: this should really be a viewport matrix.  Otherwise, all cameras will have the same FOV and aspect ratio, etc instead of just viewport dimensions.
        mpCamera->SetAspectRatio(((float)width)/((float)height));
    }

    const float defaultFOVRadians = 3.14f/2.0f;
    mpCamera->SetFov(defaultFOVRadians); // TODO: Fix converter's FOV bug (Maya generates cameras for which fbx reports garbage for fov)
    mpCamera->SetFarPlaneDistance(100000.0f);
    mpCamera->Update();


    // Position and orient the shadow camera so that it sees the whole scene.
    // Set the near and far planes so that the frustum contains the whole scene.
    // Note that if we are allowed to move the shadow camera or the scene, this may no longer be true.
    // Consider updating the shadow camera when it (or the scene) moves.
    // Treat bounding box half as radius for an approximate bounding sphere.
    // The tightest-fitting sphere might be smaller, but this is close enough for placing our shadow camera.
    // Set up the shadow camera (a camera that sees what the light sees)
    float3 lookAtPoint(0.0f, 0.0f, 0.0f);
    float3 half(1.0f, 1.0f, 1.0f);
    if( mpBarrierSet )
    {
        mpBarrierSet->GetBoundingBox( &lookAtPoint, &half );
    }

    float  length = half.length();
    mpShadowCamera = new CPUTCamera( CPUT_ORTHOGRAPHIC );
    mpShadowCamera->SetAspectRatio(1.0f);
    mpShadowCamera->SetNearPlaneDistance(1.0f);
    mpShadowCamera->SetFarPlaneDistance(2.0f*length + 1.0f);
    mpShadowCamera->SetPosition( lookAtPoint - float3(0, -1, 1) * length );
    mpShadowCamera->LookAt( lookAtPoint.x,lookAtPoint.y,lookAtPoint.z );
    mpShadowCamera->SetWidth( length*2);
    mpShadowCamera->SetHeight(length*2);
    mpShadowCamera->Update();

    mpCameraController = new CPUTCameraControllerFPS();
    mpCameraController->SetCamera(mpCamera);
    mpCameraController->SetLookSpeed(0.004f);
    mpCameraController->SetMoveSpeed(20.0f);

}
//-----------------------------------------------------------------------------
void MySample::Create()
{    



#if 1
	D3D11_FEATURE_DATA_D3D11_OPTIONS2 FeatureData;
	ZeroMemory(&FeatureData, sizeof(FeatureData));

	ID3D11Device3*  device;
	if (SUCCEEDED(mpD3dDevice->QueryInterface(__uuidof(ID3D11Device3), (void**)&device)))
	{
		if (SUCCEEDED(device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS2, &FeatureData, sizeof(FeatureData))))
		{
			mROVsSupported = (FeatureData.ROVsSupported != 0);
		}
	}
#else
	// If you haven't got the Windows 10 SDK installed yet, you can use this hack to check ROV support instead of the above.
	int features[8];
	if (SUCCEEDED(mpD3dDevice->CheckFeatureSupport((D3D11_FEATURE)(D3D11_FEATURE_D3D9_OPTIONS1+1), features, sizeof(features))))
	{
		mROVsSupported = features[2] != 0;
	}
#endif

	// report any problems
	if (!mROVsSupported)
	{
		CPUTOSServices::OpenMessageBox(_L("ROV support not found"), _L("Your system or driver does not support Raster Ordered Views (ROVs).  Please update your driver or run on a system that supports the required functionality."));
	}

    // Call ResizeWindow() because it creates some resources that our blur material needs (e.g., the back buffer)
    int width, height;
    mpWindow->GetClientDimensions(&width, &height);


    CPUTRenderStateBlockDX11 *pBlock = new CPUTRenderStateBlockDX11();
    CPUTRenderStateDX11 *pStates = pBlock->GetState();

    CPUTAssetLibrary *pAssetLibrary = CPUTAssetLibrary::GetAssetLibrary();

    cString ExecutableDirectory;
    cString mediaDirectory;
    CPUTFileSystem::GetExecutableDirectory(&ExecutableDirectory);
    CPUTFileSystem::ResolveAbsolutePathAndFilename(ExecutableDirectory + _L("../../../Media/"), &mediaDirectory);

    pAssetLibrary->SetMediaDirectoryName(mediaDirectory);
    pAssetLibrary->SetSystemDirectoryName(mediaDirectory + _L("System/"));

    CPUTGuiControllerDX11 *pGUI = CPUTGetGuiController();
    pAssetLibrary->SetAssetDirectoryName(mediaDirectory + _L("gui_assets/"));
    pGUI->Initialize(mpContext, mediaDirectory);
    pGUI->SetCallback(this);
    pGUI->SetWindow(mpWindow);

    CPUTFont *pFont = CPUTFont::CreateFont(mediaDirectory + _L("System/Font/"), _L("arial_16.fnt"));
    pGUI->SetFont(pFont);

    // Create some controls
    //
    // Create some controls
    //    


    UpdateEnableStatsCheckbox();
    

    // Add our programatic (and global) material parameters
    CPUTMaterial::mGlobalProperties.AddValue( _L("cbPerFrameValues"), _L("$cbPerFrameValues") );
    CPUTMaterial::mGlobalProperties.AddValue( _L("cbPerModelValues"), _L("$cbPerModelValues") );
    CPUTMaterial::mGlobalProperties.AddValue( _L("_Shadow"), _L("$shadow_depth") );
    CPUTMaterial::mGlobalProperties.AddValue( _L("_InternalShadow"), _L("$internalshadow") );
    CPUTMaterial::mGlobalProperties.AddValue( _L("OffscreenColorBuffer"), _L("$OffscreenColorBuffer") );

	AddGlobalProperties();

	if(GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0)
		mPostProcess.OnCreate(mpD3dDevice, width, height, mpContext, mpSwapChain);

    // load shadow casting material+sprite object
    mpShadowRenderTarget = new CPUTRenderTargetDepth();
    mpShadowRenderTarget->CreateRenderTarget( cString(_L("$shadow_depth")), SHADOW_WIDTH_HEIGHT, SHADOW_WIDTH_HEIGHT, DXGI_FORMAT_D32_FLOAT );
    mpInternalShadowRenderTarget = new CPUTRenderTargetDepth();
    mpInternalShadowRenderTarget->CreateRenderTarget( cString(_L("$internalshadow")), SHADOW_WIDTH_HEIGHT, SHADOW_WIDTH_HEIGHT, DXGI_FORMAT_D32_FLOAT );


    mpBackBuffer = new CPUTRenderTargetColor;
    mpBackBuffer->CreateRenderTarget( cString(_L("$OffscreenColorBuffer")), width, height, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB );
    mpDepthBuffer = new CPUTRenderTargetDepth;
    mpDepthBuffer->CreateRenderTarget( cString(_L("$OffscreenDepthBuffer")), width, height, DXGI_FORMAT_D32_FLOAT );



	g_CPUTZoomBox.OnCreate(mpD3dDevice, mpContext, mpSwapChain);

	const UINT MSAA_COUNT  = 4;

	mpMSAABackBuffer = new CPUTRenderTargetColor();
	mpMSAABackBuffer->CreateRenderTarget(cString(_L("$MSAAColorBuffer")), width, height, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, MSAA_COUNT);

	mpMSAADepthBuffer = new CPUTRenderTargetDepth();
	mpMSAADepthBuffer->CreateRenderTarget(cString(_L("$MSAADepthBuffer")), width, height, DXGI_FORMAT_D32_FLOAT, MSAA_COUNT);

    // Override default sampler desc for our default shadowing sampler
    pStates->SamplerDesc[1].Filter         = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    pStates->SamplerDesc[1].AddressU       = D3D11_TEXTURE_ADDRESS_BORDER;
    pStates->SamplerDesc[1].AddressV       = D3D11_TEXTURE_ADDRESS_BORDER;
    pStates->SamplerDesc[1].ComparisonFunc = D3D11_COMPARISON_GREATER;
    pBlock->CreateNativeResources();
    pAssetLibrary->AddRenderStateBlock( _L("$DefaultRenderStates"), _L(""), _L(""), pBlock );
    pBlock->Release(); // We're done with it.  The library owns it now.

    // ***************************
    // Render the terrain to our height field texture.
    // TODO: How much of this memory can we reclaim after this step?
    // Can theoretically just release.  But, AssetLibrary holds references too.
    // ***************************

	ResizeWindow(width, height);

    pAssetLibrary->SetMediaDirectoryName( ExecutableDirectory +   _L("..\\..\\..\\Media\\conservatory_01\\") );
    mpConservatorySet = pAssetLibrary->GetAssetSet( _L("conservatory_01") );
  	mpGlassMaterial = pAssetLibrary->GetMaterial( _L("Glass1") );
	mpFenceMaterial = pAssetLibrary->GetMaterial( _L("Fence") );


	pAssetLibrary->SetMediaDirectoryName( ExecutableDirectory +    _L("..\\..\\..\\Media\\ground\\") );
    mpGroundSet = pAssetLibrary->GetAssetSet( _L("ground") );

   
	pAssetLibrary->SetMediaDirectoryName(  ExecutableDirectory +   _L("..\\..\\..\\Media\\outdoorPlants_01\\") );
    mpoutdoorPlantsSet = pAssetLibrary->GetAssetSet( _L("outdoorPlants_01") );	
	mpGrassA1Material = pAssetLibrary->GetMaterial( _L("grassA1") );
	mpGrassA2Material = pAssetLibrary->GetMaterial( _L("grassA2") );

    pAssetLibrary->SetMediaDirectoryName( ExecutableDirectory +    _L("..\\..\\..\\Media\\indoorPlants_01\\") );
    mpindoorPlantsSet = pAssetLibrary->GetAssetSet( _L("indoorPlants_01") );
	
	mpLeavesTreeAMaterial = pAssetLibrary->GetMaterial( _L("leavesTreeA") );
	mpPRP_ShrubSmall_DM_AMaterial = pAssetLibrary->GetMaterial( _L("PRP_ShrubSmall_DM_A") );
    
	pAssetLibrary->SetMediaDirectoryName( ExecutableDirectory +    _L("..\\..\\..\\Media\\barrier_01\\") );
	mpBarrierSet = pAssetLibrary->GetAssetSet( _L("barrier_01") );

    pAssetLibrary->SetMediaDirectoryName( ExecutableDirectory +    _L("..\\..\\..\\Media\\"));
    mpSkyBoxMaterial = pAssetLibrary->GetMaterial( L"SkyBox11" );

	mpSkyBoxSprite = CPUTSprite::CreateSprite( -1.0f, -1.0f, 2.0f, 2.0f, _L("SkyBox11"));

	mpFSSprite = CPUTSprite::CreateSprite( -1.0f, -1.0f, 2.0f, 2.0f, _L("FSSprite"));

	CreateDebugViews();
    
   CreateCameras(width, height);


}

//-----------------------------------------------------------------------------
void MySample::Update(double deltaSeconds)
{
	static int sbFullscreen = -1;

    mpCameraController->Update((float)deltaSeconds);
   

    mpSortFoliage = (mpRenderType == eRenderType::AlphaBlending);

	if (sbFullscreen == -1)
	{
		mbFullscreen = sbFullscreen = CPUTGetFullscreenState();
	}

	if (mbFullscreen != ((sbFullscreen == 1) ? true: false))
	{
		sbFullscreen = mbFullscreen;
		CPUTToggleFullScreenMode();
	}

}

// Handle keyboard events
//-----------------------------------------------------------------------------
CPUTEventHandledCode MySample::HandleKeyboardEvent(CPUTKey key, CPUTKeyState state)
{
    static bool panelToggle = false;
    CPUTEventHandledCode    handled = CPUT_EVENT_UNHANDLED;
    cString fileName;
    CPUTGuiControllerDX11*  pGUI = CPUTGetGuiController();

    switch(key)
    {
    case KEY_F1:
        panelToggle = !panelToggle;
        if(panelToggle)
        {
            pGUI->SetActivePanel(ID_SECONDARY_PANEL);
        }
        else
        {
            pGUI->SetActivePanel(ID_MAIN_PANEL);
        }
        handled = CPUT_EVENT_HANDLED;
        break;
    case KEY_L:
        {
            static int cameraObjectIndex = 0;
            CPUTRenderNode *pCameraList[] = { mpCamera, mpShadowCamera };
            cameraObjectIndex = (++cameraObjectIndex) % (sizeof(pCameraList)/sizeof(*pCameraList));
            CPUTRenderNode *pCamera = pCameraList[cameraObjectIndex];
            mpCameraController->SetCamera( pCamera );
        }
        handled = CPUT_EVENT_HANDLED;
        break;
    case KEY_ESCAPE:
        handled = CPUT_EVENT_HANDLED;
        ShutdownAndDestroy();
        break;
    }
    // pass it to the camera controller
    if(handled == CPUT_EVENT_UNHANDLED)
    {
        handled = mpCameraController->HandleKeyboardEvent(key, state);
    }

    return handled;
}


// Handle mouse events
//-----------------------------------------------------------------------------
CPUTEventHandledCode MySample::HandleMouseEvent(int x, int y, int wheel, CPUTMouseState state, CPUTEventID message)
{
	if(state & CPUT_MOUSE_RIGHT_DOWN)
	{
		FLOAT zoomx =  x / (FLOAT) m_ScreenWidth;
		FLOAT zoomy =  y / (FLOAT) m_ScreenHeight;

		g_CPUTZoomBox.SetZoomCenterPosition((float)zoomx, (float)zoomy);
	}

    if( mpCameraController )
    {
        return mpCameraController->HandleMouseEvent(x, y, wheel, state, message);
    }
    return CPUT_EVENT_UNHANDLED;
}

void MySample::HandleGUIElementEvent( CPUTEventID Event, CPUTControlID ControlID, CPUTGUIElement *pElement )
{
}
// Handle any control callback events
//-----------------------------------------------------------------------------
void MySample::HandleCallbackEvent( CPUTEventID Event, CPUTControlID ControlID, CPUTControl *pControl )
{
    UNREFERENCED_PARAMETER(Event);
    UNREFERENCED_PARAMETER(pControl);
    CPUTCheckboxState checkboxState;

    switch(ControlID)
    {
    case ID_FULLSCREEN_BUTTON:
		mbFullscreen = !mbFullscreen;
        break;
    case ID_VSYNC_CHECKBOX:
        checkboxState = ((CPUTCheckbox*)pControl)->GetCheckboxState();
        mSyncInterval = (checkboxState == CPUT_CHECKBOX_CHECKED);
        UpdateEnableStatsCheckbox();
        break;
    case ID_RENDERTYPE:
        UpdateEnableStatsCheckbox();
		break;
	case ID_DEBUG_DISPLAY:
        UpdateEnableStatsCheckbox();
        break;
    default:
        break;
    }
}

//-----------------------------------------------------------------------------
void MySample::ResizeWindow(UINT width, UINT height)
{
    CPUTAssetLibrary *pAssetLibrary = CPUTAssetLibrary::GetAssetLibrary();


	m_ScreenHeight  = (float)height;
	m_ScreenWidth   = (float)width;

    // Before we can resize the swap chain, we must release any references to it.
    // We could have a "AssetLibrary::ReleaseSwapChainResources(), or similar.  But,
    // Generic "release all" works, is simpler to implement/maintain, and is not performance critical.
    pAssetLibrary->ReleaseTexturesAndBuffers();

    // Resize the CPUT-provided render target
    CPUT_DX11::ResizeWindow( width, height );

	if(GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0)
	{
		mPostProcess.OnSize(mpD3dDevice, width, height);
	}

	mpBackBuffer->RecreateRenderTarget(width, height );
	mpDepthBuffer->RecreateRenderTarget(width, height );

	g_CPUTZoomBox.OnSize(mpD3dDevice, width, height);

    // Resize any application-specific render targets here
    if( mpCamera ) mpCamera->SetAspectRatio(((float)width)/((float)height));

    pAssetLibrary->RebindTexturesAndBuffers();
}

//-----------------------------------------------------------------------------
void MySample::Render(double deltaSeconds)
{
    static int frameIndex = 0;

#ifdef AOIT_METRICS
    CPUTGPUProfilerDX11_AutoScopeProfile timerAll( mGPUTimerAll, mpShowStats );
#endif

	ImGui::GetIO().DeltaTime = (float)deltaSeconds;


	ImGui_ImplDX11_NewFrame(true);

	RegisterUI();

	frameIndex++;

    CPUTRenderParametersDX renderParams( mpContext);
    renderParams.mpShadowCamera = NULL;
    renderParams.mpCamera = mpShadowCamera;
    renderParams.mpPerFrameConstants = (CPUTBuffer*)mpPerFrameConstantBuffer;
    renderParams.mpPerModelConstants = (CPUTBuffer*)mpPerModelConstantBuffer;
    int windowWidth, windowHeight;
    mpWindow->GetClientDimensions( &windowWidth, &windowHeight);
    renderParams.mWidth = windowWidth;
    renderParams.mHeight = windowHeight;

 
    UpdatePerFrameConstantBuffer(renderParams, deltaSeconds);
    //*******************************
    // Draw the shadow scene
    //*******************************

    // 2. Draw the shadowed scene using a standard shadow map from the light's point of view  
    // one could also use cascades shadow maps or other shadowing techniques.  We choose simple
    // shadow mapping for simplicity/demonstration purposes
    CPUTCamera *pLastCamera = mpCamera;
	if(frameIndex==1)
	{
	    mpCamera = renderParams.mpCamera = mpShadowCamera;
		mpShadowRenderTarget->SetRenderTarget( renderParams, 0, 0.0f, true );

		BeginDrawLists();
		if( mpConservatorySet ) { mpConservatorySet->RenderRecursive( renderParams, CPUT_MATERIAL_INDEX_SHADOW_CAST ); }
		if( mpindoorPlantsSet ) { mpindoorPlantsSet->RenderRecursive( renderParams, CPUT_MATERIAL_INDEX_SHADOW_CAST ); }
		if( mpoutdoorPlantsSet ) { mpoutdoorPlantsSet->RenderRecursive( renderParams, CPUT_MATERIAL_INDEX_SHADOW_CAST ); }
		if( mpBarrierSet ) { mpBarrierSet->RenderRecursive(renderParams,CPUT_MATERIAL_INDEX_SHADOW_CAST); }
		DrawSolidLists(renderParams);
		DrawTransparentLists(renderParams,true );

		mpShadowRenderTarget->RestoreRenderTarget(renderParams);

		mpInternalShadowRenderTarget->SetRenderTarget( renderParams, 0, 0.0f, true );
		BeginDrawLists();

		if( mpConservatorySet ) { mpConservatorySet->RenderRecursive(renderParams, CPUT_MATERIAL_INDEX_SHADOW_CAST); }
		if( mpindoorPlantsSet ) { mpindoorPlantsSet->RenderRecursive( renderParams, CPUT_MATERIAL_INDEX_SHADOW_CAST ); }
		if( mpoutdoorPlantsSet ) { mpoutdoorPlantsSet->RenderRecursive( renderParams, CPUT_MATERIAL_INDEX_SHADOW_CAST ); }
		DrawSolidLists(renderParams);

		mpInternalShadowRenderTarget->RestoreRenderTarget(renderParams);


	    mpCamera = renderParams.mpCamera = pLastCamera;
	}

    renderParams.mpCamera = mpCamera;
    renderParams.mpShadowCamera = mpShadowCamera;
    UpdatePerFrameConstantBuffer(renderParams, deltaSeconds);

    // Clear back buffer
    const float clearColor[] = { 0.0f, 0.5f, 1.0f, 1.0f };
    mpContext->ClearRenderTargetView( mpBackBufferRTV,  clearColor );
    mpContext->ClearDepthStencilView( mpDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0);

	BeginDrawLists();

	int NodeIndex = 0;

	
	if(mpRenderType == eRenderType::AlphaBlending)
	{
		mpGlassMaterial->SetCurrentEffect(0);
		if(!mpSortFoliage)
		{
			mpGrassA1Material->SetCurrentEffect(0);
			mpGrassA2Material->SetCurrentEffect(0);
			mpLeavesTreeAMaterial->SetCurrentEffect(0);
			mpPRP_ShrubSmall_DM_AMaterial->SetCurrentEffect(0);
		}
		else
		{
			mpGrassA1Material->SetCurrentEffect(1);
			mpGrassA2Material->SetCurrentEffect(1);
			mpLeavesTreeAMaterial->SetCurrentEffect(1);
			mpPRP_ShrubSmall_DM_AMaterial->SetCurrentEffect(1);
		}
		mpFenceMaterial->SetCurrentEffect(0);
	}
	else if(mpRenderType == eRenderType::AlphaBlending_A2C)
	{
		mpGlassMaterial->SetCurrentEffect(0);

		mpGrassA1Material->SetCurrentEffect(2);
		mpGrassA2Material->SetCurrentEffect(2);
		mpLeavesTreeAMaterial->SetCurrentEffect(2);
		mpPRP_ShrubSmall_DM_AMaterial->SetCurrentEffect(2);
		mpFenceMaterial->SetCurrentEffect(1);
	}
	else if((mpRenderType == eRenderType::ROV_OIT|| mpRenderType == eRenderType::ROV_HDR_OIT) && mROVsSupported)
	{
		int MaterialOffset = (mbPixelSync)?3:0;
		int HDROffset = (mpRenderType == eRenderType::ROV_HDR_OIT) ? 6 : 0;

		if(mpNodeCount == eNodeCount::TwoNode)
		{
			mpGlassMaterial->SetCurrentEffect(1+MaterialOffset+ HDROffset);
			mpGrassA1Material->SetCurrentEffect(3+MaterialOffset + HDROffset);
			mpGrassA2Material->SetCurrentEffect(3+MaterialOffset + HDROffset);
			mpLeavesTreeAMaterial->SetCurrentEffect(3+MaterialOffset + HDROffset);
			mpPRP_ShrubSmall_DM_AMaterial->SetCurrentEffect(3+MaterialOffset + HDROffset);
			mpFenceMaterial->SetCurrentEffect(2+MaterialOffset + HDROffset);
		}
		else if(mpNodeCount == eNodeCount::FourNode)
		{
			mpGlassMaterial->SetCurrentEffect(2+MaterialOffset + HDROffset);
			mpGrassA1Material->SetCurrentEffect(4+MaterialOffset + HDROffset);
			mpGrassA2Material->SetCurrentEffect(4+MaterialOffset + HDROffset);
			mpLeavesTreeAMaterial->SetCurrentEffect(4+MaterialOffset + HDROffset);
			mpPRP_ShrubSmall_DM_AMaterial->SetCurrentEffect(4+MaterialOffset + HDROffset);
			mpFenceMaterial->SetCurrentEffect(3+MaterialOffset + HDROffset);
			NodeIndex=1;
		}
		else if(mpNodeCount == eNodeCount::EightNode)
		{
			mpGlassMaterial->SetCurrentEffect(3+MaterialOffset + HDROffset);
			mpGrassA1Material->SetCurrentEffect(5+MaterialOffset + HDROffset);
			mpGrassA2Material->SetCurrentEffect(5+MaterialOffset + HDROffset);
			mpLeavesTreeAMaterial->SetCurrentEffect(5+MaterialOffset + HDROffset);
			mpPRP_ShrubSmall_DM_AMaterial->SetCurrentEffect(5+MaterialOffset + HDROffset);
			mpFenceMaterial->SetCurrentEffect(4+MaterialOffset + HDROffset);
			NodeIndex=2;
		}

	}
	else if(mpRenderType == eRenderType::DX11_AOIT)
	{
		mpGlassMaterial->SetCurrentEffect(13);

		mpGrassA1Material->SetCurrentEffect(15);
		mpGrassA2Material->SetCurrentEffect(15);
		mpLeavesTreeAMaterial->SetCurrentEffect(15);
		mpPRP_ShrubSmall_DM_AMaterial->SetCurrentEffect(15);
		mpFenceMaterial->SetCurrentEffect(14);
	}

	if(mpRenderType == eRenderType::AlphaBlending_A2C)
	{
	    mpMSAABackBuffer->SetRenderTarget( renderParams, mpMSAADepthBuffer, 0, clearColor, true );
	}
	else
	{
		if(mpZoomBox)
			mpBackBuffer->SetRenderTarget(  renderParams, mpDepthBuffer, 0, clearColor, true );
	}

	mpSkyBoxSprite->DrawSprite(renderParams,*mpSkyBoxMaterial);
	if( mpGroundSet ) { mpGroundSet->RenderRecursive(renderParams); }
    if( mpBarrierSet ) { mpBarrierSet->RenderRecursive(renderParams); }
    if( mpindoorPlantsSet ) { mpindoorPlantsSet->RenderRecursive(renderParams); }
	if( mpConservatorySet ) { mpConservatorySet->RenderRecursive(renderParams); }
    if( mpoutdoorPlantsSet ) { mpoutdoorPlantsSet->RenderRecursive(renderParams); }


	// Render Solid Objects and any normal transpartent objects
	{
#ifdef AOIT_METRICS
		CPUTGPUProfilerDX11_AutoScopeProfile timer( mGPUTimerPrePass, mpShowStats );
#endif
		DrawSolidLists(renderParams);

		if(mpRenderType == eRenderType::AlphaBlending_A2C || mpRenderType == eRenderType::AlphaBlending )
		{
			// Basic Pass rebder scene once with no AOIT
			DrawTransparentLists(renderParams,true );
		}
	}


	// Render AOIT Objects a

	if (mpRenderType == eRenderType::ROV_OIT || mpRenderType == eRenderType::ROV_HDR_OIT || mpRenderType == eRenderType::DX11_AOIT)
	{
		if (mpRenderType == eRenderType::ROV_OIT || mpRenderType == eRenderType::ROV_HDR_OIT )
		{
	#ifdef AOIT_METRICS
			CPUTGPUProfilerDX11_AutoScopeProfile timer( mGPUTimerAOITCreation, mpShowStats );
	#endif
			mPostProcess.InitFrameRender(mpContext,mpRenderType, NodeIndex, mpShowDebugView);
			DrawTransparentLists(renderParams,true );
		}
		else if(mpRenderType == eRenderType::DX11_AOIT)
		{
	#ifdef AOIT_METRICS
			CPUTGPUProfilerDX11_AutoScopeProfile timer( mGPUTimerAOITCreation,  mpShowStats );
	#endif
			mPostProcess.InitFrameRender(mpContext, mpRenderType, 0, mpShowDebugView);
			DrawTransparentLists(renderParams,true);
		}

#ifdef AOIT_METRICS
		CPUTGPUProfilerDX11_AutoScopeProfile timer( mGPUTimerResolve,  mpShowStats );
#endif
		mPostProcess.FinalizeFrameRender(mpContext, mpRenderType,  true ,NodeIndex, mpShowDebugView);
	}

	if(mpRenderType == eRenderType::AlphaBlending_A2C)
	{
		mpMSAABackBuffer->RestoreRenderTarget( renderParams );

		if(mpZoomBox)
		{
			mpBackBuffer->SetRenderTarget(  renderParams, mpDepthBuffer, 0, clearColor, true );
			mpResolveSprite->DrawSprite(renderParams, *mpResolveMaterial );
			mpBackBuffer->RestoreRenderTarget( renderParams );
		}

		mpResolveSprite->DrawSprite(renderParams, *mpResolveMaterial );
	}
	else
	{
		if(mpZoomBox)
		{
			mpBackBuffer->RestoreRenderTarget( renderParams );
			mpFSSprite->DrawSprite(renderParams);
		}
	}	
	DisplayDebugViews(renderParams);

    if(mpCameraController->GetCamera() == mpShadowCamera)
    {
        mpDebugSprite->DrawSprite(renderParams);
    }

	RenderText();
	CPUTDrawGUI();

	ImGui::Render();

	if(mpZoomBox)
		g_CPUTZoomBox.OnFrameRender( renderParams, false );


}

void MySample::UpdateEnableStatsCheckbox( )
{
    bool bEnableStats = true;

    if(mpShowDebugView)
        bEnableStats = false;

    if( mSyncInterval )// && ! CPUTGetFullscreenState() )
        bEnableStats = false;

    mpShowStats = bEnableStats;
}
