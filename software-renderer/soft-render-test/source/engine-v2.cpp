/*****************************************************************************
*
* engine-v2.cpp
*
* By Jari Korkala 1/6/2011
*
* Main executable for Concept Engine version 0.2
*
*****************************************************************************/

#define CONCEPT2_FILE_MAIN

#include "../include/engine-v2.h"

#include <ceng/lib/liblog.h>

#include <timerlib.h>

// Debug information
#ifdef _MSC_VER

#ifdef _DEBUG

#define _CRTDBG_MAP_ALLOC
#define _CRT_SECURE_NO_WARNINGS
#include <crtdbg.h>

#endif

#ifdef _WIN32

#include <windows.h>
#include <sstream>

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine,int nCmdShow)
{

#ifdef _DEBUG
	// Detect memory leaks after main-function ends
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF);
#endif

	int k;

	Ceng::CRESULT cresult;

	Ceng::FileLog engineLog;
	Ceng::Log::SetLog(&engineLog);

	std::stringstream writer;

	cresult = engineLog.OpenFile("ce-log.txt");
	if (cresult != Ceng::CE_OK)
	{
		Ceng::Log::SetLog(NULL);
		return 0;
	}

	Ceng::Log::Print("Program started");

	//*******************************************************
	// Select resolution and full screen here

	
	Ceng::UINT32 resX = 640;
	Ceng::UINT32 resY = 480;

	Ceng::BOOL fullScreenMode = false;
	
	/*
	Ceng::UINT32 resX = 1366;
	Ceng::UINT32 resY = 768;

	Ceng::BOOL fullScreenMode = true;	
	*/

	/*
	Ceng::UINT32 resX = 1920;
	Ceng::UINT32 resY = 1080;

	Ceng::BOOL fullScreenMode = true;	
	*/
	
	
	
	
	Ceng::String programName;
	Ceng::String captionInfo;

	programName = "Concept Engine v ";
	programName += CONCEPT_VERSION;
	programName += ".";
	programName += CONCEPT_MINOR;
	programName += " Build ";
	programName += CONCEPT_BUILD;

	Ceng::PlatformWindow *window;

	Ceng::UINT32 windowOptions = 0;

	if (fullScreenMode)
	{
		windowOptions = Ceng::E_WindowOptions::popup | Ceng::E_WindowOptions::topmost;
	}
	else
	{
		windowOptions = Ceng::E_WindowOptions::resizable
			| Ceng::E_WindowOptions::center_x
			| Ceng::E_WindowOptions::center_y;
	}

	Ceng_CreateWindow(programName,0,0,resX,resY,
												windowOptions,nullptr,
												&window);

	if (window == nullptr)
	{
		return 0;
	}

	Ceng::CPU_Info *cpuData;
	
	Ceng_GetCpuInfo(&cpuData);

	Ceng::E_CpuFeatureInfo::value testResult;

	cpuData->CheckFeature("sse_2",&testResult);

	if (testResult != Ceng::E_CpuFeatureInfo::ok)
	{
		window->Show();
		Ceng::String title = "Error";
		Ceng::String message = "CPU doesn't support SSE2";

		cresult = Ceng_MessageWindow(window,title,message);
		
		cpuData->Release();
		return 0;
	}

	cpuData->Release();

	Ceng::GraphicsSystem *renderCore;

	Ceng_CreateGraphics3D(Ceng::RENDERER_TYPE::SOFTWARE,&renderCore);
	//Ceng_CreateGraphics3D(Ceng::RENDERER_TYPE::D3D_9,&renderCore);

	if (renderCore == nullptr)
	{
		Ceng::Log::Print("Error:  GetRenderer() failed");

		window->Release();
		return 0;
	}

	//**************************************************************************
	
	// Enumerate display adapters

	Ceng::GraphicsAdapter *adapter;
	Ceng::GraphicsAdapterDesc adapterDesc;

	Ceng::UINT32 adapterCount = renderCore->AdapterCount();

	renderCore->EnumAdapter(0, &adapter);

	/*
	Ceng::StringUtf8 out;

	out = "adapter:";
	Ceng::Log::Print(out);

	adapter->GetDesc(adapterDesc);

	Ceng::Log::Print(adapterDesc.description);
	Ceng::Log::Print("\n");
	*/

	//**************************************************************************
	// Enumerate display modes for C32_ARGB

	Ceng::IMAGE_FORMAT::value format = Ceng::IMAGE_FORMAT::C32_XRGB;

	/*
	out = "Checking back buffer format : ";
	out += ImageFormatToString(format);
	out += "\n";

	Ceng::UINT32 modeCount = renderCore->DisplayModeCount(adapter,format);

	out += "modeCount = ";
	out += modeCount;
	out += "\n";
	Ceng::Log::Print(out);

	std::vector<Ceng::DisplayMode> modeList;

	for (Ceng::UINT32 k = 0; k < modeCount; k++)
	{
		Ceng::DisplayMode displayMode;

		renderCore->EnumDisplayMode(adapter, k,format, displayMode);

		if (displayMode.refreshHz < 59)
		{
			continue;
		}

		modeList.push_back(displayMode);

		out = "display mode ";
		out += modeList.size()-1;
		out += " :\n";
		out += "width = ";
		out += displayMode.width;
		out += "\n";
		out += "height = ";
		out += displayMode.height;
		out += "\n";
		out += "hz = ";
		out += displayMode.refreshHz;
		Ceng::Log::Print(out);
		Ceng::Log::Print("\n");
	}
	*/

	//**************************************************************************
	// Test framebuffer format support

	Ceng::IMAGE_FORMAT::value displayFormat = Ceng::IMAGE_FORMAT::C32_XRGB;
	Ceng::IMAGE_FORMAT::value backBufferFormat = Ceng::IMAGE_FORMAT::C32_ARGB;

	/*
	cresult = renderCore->CheckFramebufferSupport(adapter,displayFormat,backBufferFormat, true);

	out = "Testing frame buffer format with:\n";
	out += "display format = ";
	out += ImageFormatToString(displayFormat);
	out += "\nback buffer format = ";
	out += ImageFormatToString(backBufferFormat);
	out += "\nresult = ";

	if (cresult == Ceng::CE_OK)
	{
		out += "OK";
	}
	else
	{
		out += "FAIL";
	}

	out += "\n";

	Ceng::Log::Print(out);
	*/
	//**************************************************************************
	// Test dxt5 support

	/*
	cresult = renderCore->CheckBufferSupport(adapter, displayFormat,
		Ceng::BufferType::texture_2d,
		Ceng::IMAGE_FORMAT::dxt5,
		Ceng::BufferBinding::shader_resource,
		Ceng::BufferUsage::gpu_read_only,
		0,
		0);

	Ceng::Log::Print("Testing dxt5 support with C32_ARGB display format");

	out = "result = ";

	if (cresult == Ceng::CE_OK)
	{
		out += "OK";
	}
	else
	{
		out += "FAIL";
	}

	out += "\n";

	Ceng::Log::Print(out);
	*/

	//**************************************************************************
	// Test depth buffer support

	/*
	cresult = renderCore->CheckBufferSupport(adapter, displayFormat,
		Ceng::BufferType::back_buffer,
		Ceng::IMAGE_FORMAT::D32F_LOCKABLE,
		Ceng::BufferBinding::shader_resource,
		Ceng::BufferUsage::gpu_read_only,
		0,
		0);

	Ceng::Log::Print("Testing D24_S8 support with C32_ARGB display format");

	out = "result = ";

	if (cresult == Ceng::CE_OK)
	{
		out += "OK";
	}
	else
	{
		out += "FAIL";
	}

	out += "\n";

	Ceng::Log::Print(out);
	*/

	//**************************************************************************

	Ceng::RenderDevice *renderer = nullptr;
	Ceng::RenderContext *renderContext = nullptr;
	Ceng::SwapChain *swapChain = nullptr;

	Ceng::SwapChainDesc swapChainDesc;

	memset(&swapChainDesc,0,sizeof(Ceng::SwapChainDesc));

	swapChainDesc.outputWindow = window;

	swapChainDesc.displayMode.width = resX;
	swapChainDesc.displayMode.height = resY;
	
	swapChainDesc.displayMode.format = Ceng::IMAGE_FORMAT::C32_ARGB;

	swapChainDesc.autoDepthStencil = true;
	//swapChainDesc.autoDepthStencilFormat = Ceng::CE_FORMAT_DF32;

	swapChainDesc.autoDepthStencilFormat = Ceng::IMAGE_FORMAT::D32F_S8;

	swapChainDesc.windowed = !fullScreenMode;
	swapChainDesc.displayMode.refreshHz = 60;

	swapChainDesc.multisampleDesc.count = 0;
	swapChainDesc.multisampleDesc.quality = 0;

	swapChainDesc.swapEffect = Ceng::SwapEffect::flip;
	
	swapChainDesc.presentInterval = 0;
	//swapChainDesc.presentInterval = 1; //vsync

	//swapChainDesc.displayFlags = Ceng::CE_DISPLAY_CLIP_TO_WINDOW;

	// NOTE: Do not remove multi_threaded. Single-threaded library is
	//       currently not among build dependencies.
	Ceng::UINT32 deviceOptions = Ceng::E_DeviceOption::multi_threaded;

	window->Show();
	
	cresult = renderCore->GetRenderDevice(adapter,
										  deviceOptions,
										  &swapChainDesc,
										  &swapChain,
										  &renderer,&renderContext);

	if (cresult != Ceng::CE_OK)
	{
		Ceng::Log::Print("Error : Failed to create render device\n");
		return 0;
	}
	
	bool exitLoop = false;
	Ceng::CRESULT messageResult;


	Ceng::KeyboardDevice *keyboard;

	//keyboard = Ceng::KeyboardDevice::GetInstance(window);

	Ceng_GetKeyboard(window,&keyboard);

	if (keyboard == nullptr)
	{
		Ceng::Log::Print("Error : Failed to obtain keyboard device\n");
		return 0;
	}

	Ceng::RenderTarget *backBuffer;

	swapChain->GetFrameBufferTarget(&backBuffer);
	
	//*********************************************************************
	// Set vertex format

	std::vector<Ceng::VertexDeclData> progVertexDecl;

	Ceng::VertexDeclData declVar;

	declVar = Ceng::VertexDeclData(0,0,Ceng::VTX_DATATYPE::FLOAT4,Ceng::SHADER_SEMANTIC::POSITION);
	progVertexDecl.push_back(declVar);

	declVar = Ceng::VertexDeclData(0, 16, Ceng::VTX_DATATYPE::FLOAT2, Ceng::SHADER_SEMANTIC::TEXCOORD_0);
	progVertexDecl.push_back(declVar);

	declVar = Ceng::VertexDeclData(0, 24, Ceng::VTX_DATATYPE::FLOAT2, Ceng::SHADER_SEMANTIC::TEXCOORD_1);
	progVertexDecl.push_back(declVar);

	progVertexDecl.push_back(Ceng::VTX_DECL_END);

	Ceng::VertexFormat *vertexFormat;

	cresult = renderer->CreateVertexFormat(progVertexDecl,
										  (Ceng::VertexFormat**)&vertexFormat);
	
	if (cresult != Ceng::CE_OK)
	{
		Ceng::Log::Print("Error : Failed to create vertex format");
		return 0;
	}

	writer.clear();
	writer.str("");

	cresult = renderContext->SetVertexFormat(vertexFormat);

	if (cresult != Ceng::CE_OK)
	{
		Ceng::Log::Print("Error : Failed to set vertex format");
		return 0;
	}




	//*****************************************************************
	// Allocate vertex buffer

	Ceng::VertexBuffer *vertexBuffer;
	ProgVertexData *vBufferPtr;

	cresult = renderer->CreateVertexBuffer(sizeof(ProgVertexData),
											12,0,
											&vertexBuffer);

	if (cresult != Ceng::CE_OK)
	{
		Ceng::Log::Print("Error : Failed to create vertex buffer");
		return 0;
	}

	vertexBuffer->BufferPtr((Ceng::UINT8**)&vBufferPtr);

	Ceng::VertexShader *vShaderPtr;

	cresult = renderer->CreateVertexShader(&vShaderPtr);

	if (cresult != Ceng::CE_OK)
	{
		Ceng::Log::Print("Error : CreateVertexShader() failed");
		return 0;
	}


	Ceng::ShaderConstant *vsVertexTransform;

	cresult = vShaderPtr->GetConstant("fullVertexTransform",&vsVertexTransform);

	if (cresult != Ceng::CE_OK)
	{
		Ceng::Log::Print("Error : Failed to obtain uniform handle");
		return 0;
	}

	cresult = renderContext->SetVertexShader(vShaderPtr);

	if (cresult != Ceng::CE_OK)
	{
		Ceng::Log::Print("Vertex shader setup error");

		writer.clear();
		writer.str("");

		writer << cresult;
		Ceng::Log::Print(writer.str().c_str());
		return 0;
	}

	//****************************************************************
	// Create test pixel shader

	Ceng::PixelShader *pixelShader;

	cresult = renderer->CreatePixelShader(&pixelShader);

	cresult = renderContext->SetPixelShader(pixelShader);
	if (cresult != Ceng::CE_OK)
	{
		Ceng::Log::Print("Pixel shader setup error!");
		return 0;
	}

	Ceng::ShaderConstant *diffuseTexHandle;

	pixelShader->GetConstant("diffuseTexture", &diffuseTexHandle);

	if (cresult != Ceng::CE_OK)
	{
		Ceng::Log::Print("pixelShader->GetConstant() failed");
		return 0;
	}

	Ceng::UINT32 diffuseTexUnit = 0;

	diffuseTexHandle->SetValue(&diffuseTexUnit);

	//**********************************

	// Test mesh

	Actor cubeActor;
	Mesh cubeMesh;

	ProgVertexData tempVertex;

	Ceng::FLOAT32 squareWidth = 4.0f; // default
	Ceng::FLOAT32 squareHeight = 2.5f; 
	
	//Ceng::FLOAT32 squareSide = 1.0f;

	memset(&tempVertex,0,sizeof(ProgVertexData));

	tempVertex.location.x = -squareWidth;
	tempVertex.location.y = -squareHeight;
	tempVertex.location.z = 0.0f;

	/*
	tempVertex.color.red = 0.25f;
	tempVertex.color.green = 0.25f;
	tempVertex.color.blue = 0.25f;
	*/

	/*
	tempVertex.color.red = 0.0f;
	tempVertex.color.green = 0.8f;
	tempVertex.color.blue = 0.0f;
	*/
	
	tempVertex.texcoord.u = 0.0f;
	tempVertex.texcoord.v = 1.0f;

	/*
	tempVertex.colorMul = 0.25f;
	tempVertex.testConst = 0.0f;
	*/

	cubeMesh.vertices.push_back(tempVertex);

	tempVertex.location.x = squareWidth;
	tempVertex.location.y = -squareHeight;
	tempVertex.location.z = 0.0f;	

	tempVertex.texcoord.u = 1.0f;
	tempVertex.texcoord.v = 1.0f;
	
	/*
	tempVertex.color.red = 0.8f;
	tempVertex.color.green = 0.0f;
	tempVertex.color.blue = 0.0f;

	tempVertex.colorMul = 1.25f;
	*/

	cubeMesh.vertices.push_back(tempVertex);
	
	tempVertex.location.x = squareWidth;
	tempVertex.location.y = squareHeight;
	tempVertex.location.z = 0.0f;

	tempVertex.texcoord.u = 1.0f;
	tempVertex.texcoord.v = 0.0f;
	
	/*
	tempVertex.color.red = 0.0f;
	tempVertex.color.green = 0.8f;
	tempVertex.color.blue = 0.0f;
	
	tempVertex.colorMul = 1.25f;
	*/

	cubeMesh.vertices.push_back(tempVertex);

	tempVertex.location.x = -squareWidth;
	tempVertex.location.y = squareHeight;
	tempVertex.location.z = 0.0f;

	tempVertex.texcoord.u = 0.0f;
	tempVertex.texcoord.v = 0.0f;
	
	/*
	tempVertex.color.red = 0.8f;
	tempVertex.color.green = 0.0f;
	tempVertex.color.blue = 0.0f;

	tempVertex.colorMul = 1.25f;
	*/

	cubeMesh.vertices.push_back(tempVertex);

	cubeActor.SetMesh(&cubeMesh);
	cubeActor.SetWorldPosition(Ceng::VectorF4(0.0f,0.0f,0.0f));

	std::vector<Actor*> objectList;
	objectList.push_back(&cubeActor);
	
	//**********************************************************
	// Create texture

	Ceng::Texture2D *texture = nullptr;

	Ceng::Texture2dDesc textureDesc;

	textureDesc.width = 8;
	textureDesc.height = 8;
	//textureDesc.format = Ceng::IMAGE_FORMAT::C32_ARGB;
	textureDesc.format = Ceng::IMAGE_FORMAT::C24_RGB;
	textureDesc.arraySize = 1;
	textureDesc.mipLevels = 0;// 8;
	textureDesc.miscFlags = Ceng::BufferOptions::generate_mip_maps;

	/*
	// Initial data

	struct Color32
	{
		Ceng::UINT8 blue;
		Ceng::UINT8 green;
		Ceng::UINT8 red;
		Ceng::UINT8 alpha;
	};

	Color32 textureData[8][8];

	Color32 white = { 200, 60, 0, 255 };
	Color32 orange = { 50, 50, 50, 255 };

	Ceng::UINT32 j;

	for (k = 0; k < 8; ++k)
	{
		for (j = 0; j < 8; ++j)
		{
			textureData[k][j] = white;
		}
	}

	for (j = 0; j < 8; j++)
	{
		textureData[0][j] = orange;
		textureData[7][j] = orange;

		textureData[3][j] = orange;
		textureData[4][j] = orange;

		textureData[j][3] = orange;
		textureData[j][4] = orange;

		textureData[j][0] = orange;
		textureData[j][7] = orange;
	}

	Ceng::SubResourceData textureFill;

	textureFill.sourcePtr = &textureData[0][0];
	textureFill.rowPitch = 4*8;
	textureFill.depthPitch = 4 * 8 * 8;

	cresult = renderer->CreateTexture2D(textureDesc,&textureFill, &texture);
	*/

	//cresult = CreateTexture2dFromFile("ceiling.bmp", textureDesc, renderer, &texture);
	cresult = CreateTexture2dFromFile("brickwall.bmp", textureDesc, renderer,&texture);
	//cresult = CreateTexture2dFromFile("checkerboard.bmp", textureDesc, renderer, &texture);

	if (cresult != Ceng::CE_OK)
	{
		return 0;
	}

	/*
	keyboard->Release();

	vertexBuffer->Release();

	vsVertexTransform->Release();
	vShaderPtr->Release();

	vertexFormat->Release();

	diffuseTexHandle->Release();
	pixelShader->Release();

	texture->Release();

	backBuffer->Release();

	swapChain->Release();
	renderContext->Release();
	renderer->Release();
	adapter->Release();

	renderCore->Release();

	window->Release();

	return 0;
	*/

	Ceng::ShaderResourceView *texView;

	Ceng::ShaderResourceViewDesc texViewDesc;

	texViewDesc.dimensions = Ceng::BufferType::texture_2d;
	texViewDesc.format = textureDesc.format;
	texViewDesc.tex2d.baseMipLevel = 0;
	texViewDesc.tex2d.maxMipLevel = 8;

	cresult = renderer->CreateShaderResourceView(texture,texViewDesc, &texView);

	if (cresult != Ceng::CE_OK)
	{
		return 0;
	}

	Ceng::SamplerState *texSamplerLinear;
	Ceng::SamplerStateDesc texSamplerDesc;

	texSamplerDesc.addressU = Ceng::TextureAddressMode::wrap;
	texSamplerDesc.addressV = Ceng::TextureAddressMode::wrap;
	texSamplerDesc.maxAnisotrophy = 0;
	
	texSamplerDesc.minFilter = Ceng::TextureMinFilter::linear;
	//texSamplerDesc.minFilter = Ceng::TextureMinFilter::linear_mip_nearest;
	texSamplerDesc.magFilter = Ceng::TextureMagFilter::linear;

	texSamplerDesc.minLod = 0;
	texSamplerDesc.maxLod = 0;
	texSamplerDesc.mipLodBias = 0.0f;

	cresult = renderer->CreateSamplerState(texSamplerDesc, &texSamplerLinear);

	if (cresult != Ceng::CE_OK)
	{
		return 0;
	}

	texSamplerDesc.minFilter = Ceng::TextureMinFilter::nearest;
	texSamplerDesc.magFilter = Ceng::TextureMagFilter::nearest;

	Ceng::SamplerState *texSamplerNearest;
	cresult = renderer->CreateSamplerState(texSamplerDesc, &texSamplerNearest);

	if (cresult != Ceng::CE_OK)
	{
		return 0;
	}

	renderContext->SetSamplerState(0, texSamplerLinear);
	//renderContext->SetSamplerState(0, texSamplerNearest);
	renderContext->SetShaderResource(0, texView);

	//**********************************************************
	// Camera setup

	Camera camera;

	//camera.SetWorldPosition(Ceng::VectorF4(0.0f,0.0f,1.0f));

	camera.SetWorldPosition(Ceng::VectorF4(0.0f,0.0f,6.0f));

	//************************************************************

	Ceng::Matrix4 objectFullTransform;
	Ceng::Matrix4 cameraFullTransform;

	Ceng::Matrix4 projectionMatrix;
	Ceng::Matrix4 fullVertexTransform;

	Ceng::FLOAT32 zNear = 0.005f;
	Ceng::FLOAT32 zFar =  1000.0f;
	Ceng::FLOAT32 viewAngle = 70.0f;

	renderer->CreateProjectionMatrix(640,480,viewAngle,zNear,zFar,&projectionMatrix);
	
	Ceng::FLOAT32 objectAngle = 0.0f;

	Ceng::FLOAT32 moveSpeed = 2.0f;

	Ceng::FLOAT32 cameraRotateSpeed = 90.0f;

	//**********************************************************************
	// Set rasterizer state

	Ceng::RasterizerState rasterState;

	rasterState.cullingMode = Ceng::CULLING_MODE::BACK;
	rasterState.scissorEnable = false;

	Ceng::Rectangle scissorRect;

	scissorRect.top = 0;
	scissorRect.left = 0;

	scissorRect.right = 640;
	scissorRect.bottom = 480;

	renderContext->SetScissorRect(&scissorRect);

	renderContext->SetRasterizerState(&rasterState);

	//*********************************************************************
	// Set depth-stencil test

	Ceng::DepthStencilDesc dsTest;

	dsTest.stencilEnable = true;
	dsTest.depthEnable = true;

	dsTest.depthWrite = true;
	dsTest.depthTest = Ceng::TEST_TYPE::LESS;

	dsTest.frontFace.stencilWriteMask = 255;
	dsTest.frontFace.stencilReadMask = 255;

	dsTest.frontFace.stencilRef = 1;

	dsTest.frontFace.stencilTest = Ceng::TEST_TYPE::EQUAL;

	dsTest.frontFace.stencilFail = Ceng::STENCIL_ACTION::KEEP;
	dsTest.frontFace.depthFail = Ceng::STENCIL_ACTION::ZERO;
	dsTest.frontFace.depthPass = Ceng::STENCIL_ACTION::KEEP;

	renderContext->SetDepthStencilState(&dsTest);

	//*********************************************************************
	// Set render targets

	renderContext->SetRenderTarget(0,backBuffer);

	//************************************************************
	// Application loop start

	// FPS measurement variables
	double startTime,endTime;
	int frameCount = 0;
	int fps = 0;

	startTime = Timer();

	Ceng::UINT32 displayWidth,displayHeight;

	window->GetClientArea(&displayWidth,&displayHeight);

	window->Show();

	Ceng::FLOAT32 depthClear = 1.0f; //0.9955f;

	// Stencil buffer test
	
	renderContext->ClearDepthStencil(depthClear,1);
	
	//renderer->ClearTarget(backBuffer,Ceng::CE_Color(Ceng::UINT8(0),0,0,0));

	//cubeActor.RotateByDeltas(0.0f,70.0f,0.0f);

	Ceng::FLOAT64 frameStart,frameEnd,frameTime;

	frameTime = 0;

	Ceng::FLOAT64 physTime,physCurrentTime,physDeltaTime;
	
	physTime = Timer();

	do
	{
		physCurrentTime = Timer();

		physDeltaTime = physCurrentTime - physTime;

		if (physDeltaTime < 0.0)
		{
			physDeltaTime = 0.0;
		}

		/*
		if (physDeltaTime > 0.1)
		{
			physDeltaTime = 0.1;
		}
		*/

		physTime = Timer();

		messageResult = window->ProcessMessages();

		if (messageResult != Ceng::CE_OK)
		{
			exitLoop = true;
		}
		else
		{
			if (window->IsDestroyed())
			{
				exitLoop = true;
			}		

			// Process input if program is active
			if (window->IsActive())
			{
				cresult = keyboard->Restore();
				if (cresult != Ceng::CE_OK)
				{
					exitLoop = true;
				}
				
				if (keyboard->IsPressed(Ceng::KEYBOARD_KEY::ESC))
				{
					exitLoop = true;
				}

				// Move camera forward
				if (keyboard->IsPressed(Ceng::KEYBOARD_KEY::W))
				{
					camera.MoveByDelta(Ceng::VectorF4(0.0f,0.0f,-moveSpeed*physDeltaTime));
				}
				
				// Move camera backwards
				if (keyboard->IsPressed(Ceng::KEYBOARD_KEY::S))
				{
					camera.MoveByDelta(Ceng::VectorF4(0.0f, 0.0f, moveSpeed*physDeltaTime));
				}

				// Move camera left
				if (keyboard->IsPressed(Ceng::KEYBOARD_KEY::A))
				{
					camera.MoveByDelta(Ceng::VectorF4(-moveSpeed*physDeltaTime, 0.0f, 0.0f));
				}
				
				// Move camera right
				if (keyboard->IsPressed(Ceng::KEYBOARD_KEY::D))
				{
					camera.MoveByDelta(Ceng::VectorF4(moveSpeed*physDeltaTime, 0.0f, 0.0f));
				}

				// Move camera up
				if (keyboard->IsPressed(Ceng::KEYBOARD_KEY::Q))
				{
					camera.MoveByDelta(Ceng::VectorF4(0.0f, moveSpeed*physDeltaTime, 0.0f));
				}
				
				// Move camera down
				if (keyboard->IsPressed(Ceng::KEYBOARD_KEY::E))
				{
					camera.MoveByDelta(Ceng::VectorF4(0.0f, -moveSpeed*physDeltaTime, 0.0f));
				}

				// Turn camera left
				if (keyboard->IsPressed(Ceng::KEYBOARD_KEY::LEFT_ARROW))
				{
					camera.RotateByDeltas(0.0f, cameraRotateSpeed*physDeltaTime, 0.0f);
				}
				
				// Turn camera right
				if (keyboard->IsPressed(Ceng::KEYBOARD_KEY::RIGHT_ARROW))
				{
					camera.RotateByDeltas(0.0f, -cameraRotateSpeed*physDeltaTime, 0.0f);
				}

				// Turn camera up
				if (keyboard->IsPressed(Ceng::KEYBOARD_KEY::UP_ARROW))
				{
					camera.RotateByDeltas(cameraRotateSpeed*physDeltaTime, 0.0f, 0.0f);
				}
				
				// Turn camera down
				if (keyboard->IsPressed(Ceng::KEYBOARD_KEY::DOWN_ARROW))
				{
					camera.RotateByDeltas(-cameraRotateSpeed*physDeltaTime, 0.0f, 0.0f);
				}

				// Rotate camera left
				// NOTE : Direction reversed because
				//        -z is used as rotation axis
				if (keyboard->IsPressed(Ceng::KEYBOARD_KEY::HOME))
				{
					camera.RotateByDeltas(0.0f, 0.0f, cameraRotateSpeed*physDeltaTime);
				}
				
				// Rotate camera right
				// NOTE : Direction reversed because
				//        -z is used as rotation axis
				if (keyboard->IsPressed(Ceng::KEYBOARD_KEY::END))
				{
					camera.RotateByDeltas(0.0f, 0.0f, -cameraRotateSpeed*physDeltaTime);
				}

				
				// Rotate cube counterclockwise around y-axis
				if (keyboard->IsPressed(Ceng::KEYBOARD_KEY::KEY_1))
				{
					cubeActor.RotateByDeltas(0.0f, cameraRotateSpeed*physDeltaTime, 0.0f);
				}
				
				// Rotate cube clockwise around y-axis
				if (keyboard->IsPressed(Ceng::KEYBOARD_KEY::KEY_2))
				{
					cubeActor.RotateByDeltas(0.0f, -cameraRotateSpeed*physDeltaTime, 0.0f);
				}

				if (keyboard->IsPressed(Ceng::KEYBOARD_KEY::F1))
				{
					renderContext->SetSamplerState(0, texSamplerNearest);
				}

				if (keyboard->IsPressed(Ceng::KEYBOARD_KEY::F2))
				{
					renderContext->SetSamplerState(0, texSamplerLinear);
				}

				if (window->IsResized())
				{
					window->GetClientArea(&displayWidth,&displayHeight);

					renderer->CreateProjectionMatrix(displayWidth,
														displayHeight,
														viewAngle,zNear,zFar,
														&projectionMatrix);
				}
			}

			if (window->IsVisible())
			{
				// Calculate FPS
				endTime = Timer();
				if (endTime - startTime >= 1.0)
				{
					fps = frameCount;
					frameCount = 0;
					
					startTime = Timer();
				
					captionInfo = " | ";
					captionInfo += (int)displayWidth;
					captionInfo += " x ";
					captionInfo += (int)displayHeight;
					captionInfo += " | FPS = ";
					captionInfo += fps;
					captionInfo += " | t = ";
					captionInfo += 1000.0f * (frameTime/fps);
					captionInfo += " ms";

					frameTime = 0.0f;
				
					window->SetCaptionExtra(captionInfo);
				}
			
				// Draw scene if application is visible

				objectFullTransform = cubeActor.GetFullTransformation();
				
				frameStart = Timer();

				renderContext->StartScene();
				
				renderContext->ClearTarget(backBuffer,Ceng::CE_Color(Ceng::UINT8(0),0,0,0));

				//renderer->ClearTarget(backBuffer,Ceng::CE_Color(0,0,Ceng::UINT8(255),0));
					
				//renderer->ClearDepthStencil(depthClear,1);
				renderContext->ClearDepth(depthClear);
				
				cameraFullTransform = camera.GetFullTransformation();
					
				fullVertexTransform = cameraFullTransform * objectFullTransform;
				fullVertexTransform = projectionMatrix * fullVertexTransform;
					
				vsVertexTransform->SetValue((void*)&fullVertexTransform);
					
				// Move vertex data to vertex buffer
				memcpy(vBufferPtr,&cubeMesh.vertices[0],4*sizeof(ProgVertexData));
					
				renderContext->SetVertexStream(0,vertexBuffer);
				renderContext->DrawPrimitive(Ceng::PRIMITIVE_TYPE::TRIANGLE_FAN,0,2);

				// DEBUG: only one triangle
				//renderer->DrawPrimitive(Ceng::PRIMITIVE_TYPE::TRIANGLE_FAN,0,1);
					
				renderContext->EndScene();
			
				cresult = swapChain->Present();
				if (cresult != Ceng::CE_OK)
				{
					exitLoop = true;
				}

				frameEnd = Timer();
				frameTime += frameEnd-frameStart;

				frameCount++;
			}
		}

		//Sleep(1000);

	} while (exitLoop == false);
	

	Ceng::Log::Print("Program exit");

	keyboard->Release();

	vsVertexTransform->Release();

	texSamplerNearest->Release();
	texSamplerLinear->Release();

	texView->Release();
	texture->Release();

	vertexBuffer->Release();
	vertexFormat->Release();

	backBuffer->Release();

	vShaderPtr->Release();

	diffuseTexHandle->Release();
	pixelShader->Release();

	renderContext->Release();
	swapChain->Release();
	renderer->Release();
	
	adapter->Release();
	renderCore->Release();

	window->Release();

	engineLog.CloseFile();

	return 0;
}

#endif // _WIN32

#endif // _MSC_VER


