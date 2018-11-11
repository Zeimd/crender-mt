/*****************************************************************************
*
* graphics-2d-d3d10.cpp
*
* By Jari Korkala 5/2013
*
*****************************************************************************/

#include "../include/graphics-2d-d3d10.h"

using namespace Ceng;

Graphics2D_D3D10::Graphics2D_D3D10()
{
	d3d_Device = nullptr;
	swapChain = nullptr;

	backBuffer = nullptr;
	backBufferView = nullptr;

	vertexBuffer = nullptr;

	screenTexture = nullptr;
	screenSampler = nullptr;

	frameResource = nullptr;

	effect = nullptr;

	inputLayout = nullptr;
}

Graphics2D_D3D10::~Graphics2D_D3D10()
{
	Reset();
}

void Graphics2D_D3D10::Release()
{
	delete this;
}

void Graphics2D_D3D10::Reset()
{
	if (swapChain != nullptr)
	{
		swapChain->SetFullscreenState(FALSE,nullptr);

		swapChain->Release();
		swapChain = nullptr;
	}

	if (d3d_Device != nullptr)
	{
		d3d_Device->Release();
		d3d_Device = nullptr;
	}

	if (backBufferView != nullptr)
	{
		backBufferView->Release();
		backBufferView = nullptr;
	}

	if (backBuffer != nullptr)
	{
		backBuffer->Release();
		backBuffer = nullptr;
	}

	if (vertexBuffer != nullptr)
	{
		vertexBuffer->Release();
		vertexBuffer = nullptr;
	}

	if (screenTexture != nullptr)
	{
		screenTexture->Release();
		screenTexture = nullptr;
	}

	if (screenSampler != nullptr)
	{
		screenSampler->Release();
		screenSampler = nullptr;
	}

	if (frameResource != nullptr)
	{
		frameResource->Release();
		frameResource = nullptr;
	}

	if (effect != nullptr)
	{
		effect->Release();
		effect = nullptr;
	}

	if (inputLayout != nullptr)
	{
		inputLayout->Release();
		inputLayout = nullptr;
	}
}

const CRESULT Graphics2D_D3D10::ConfigureOutput(ID3D10Device *sourceDevice,IDXGISwapChain *swapChain,
												DXGI_MODE_DESC *sourceMode)
{
	HRESULT hr;

	d3d_Device = sourceDevice;
	this->swapChain = swapChain;

	memcpy(&modeDesc,sourceMode,sizeof(DXGI_MODE_DESC));

	displayWidth = modeDesc.Width;
	displayHeight = modeDesc.Height;

	// Create render target view to back buffer

	hr = swapChain->GetBuffer(0,__uuidof(ID3D10Texture2D),(void**)&backBuffer);

	if (hr != S_OK)
	{
		//Ceng::Log::Print("SwapChain.GetBuffer : Failed");

		Reset();
		return CE_ERR_FAIL;
	}

	hr = d3d_Device->CreateRenderTargetView(backBuffer,nullptr,&backBufferView);

	if (hr != S_OK)
	{
		if (hr == E_FAIL)
		{
			//Ceng::Log::Print("CreateRenderTargetView : No debug layer"); 
		}
		else if (hr == E_INVALIDARG)
		{
			//Ceng::Log::Print("CreateRenderTargetView : Invalid argument");
		}
		else
		{
			//Ceng::Log::Print("CreateRenderTargetView : Failed");
		}

		Reset();
		return CE_ERR_FAIL;
	}

	//*******************************************************
	// Create full-screen quad

	memset(&screenQuad,0,sizeof(Square));

	float uMax = float(modeDesc.Width) / 1366.0f;
	float vMax = float(modeDesc.Height) / 768.0f;

	// Top-left corner
	screenQuad.vertex[0].position.x = -1.0f;
	screenQuad.vertex[0].position.y = 1.0f;

	screenQuad.vertex[0].texCoord.u = 0.0f;
	screenQuad.vertex[0].texCoord.v = 0.0f;

	// Top-right corner
	screenQuad.vertex[1].position.x = 1.0f;
	screenQuad.vertex[1].position.y = 1.0f;

	screenQuad.vertex[1].texCoord.u = uMax;
	screenQuad.vertex[1].texCoord.v = 0.0f;
	
	// Bottom-left corner
	screenQuad.vertex[2].position.x = -1.0f;
	screenQuad.vertex[2].position.y = -1.0f;

	screenQuad.vertex[2].texCoord.u = 0.0f;
	screenQuad.vertex[2].texCoord.v = vMax;

	// Bottom-right corner
	screenQuad.vertex[3].position.x = 1.0f;
	screenQuad.vertex[3].position.y = -1.0f;

	screenQuad.vertex[3].texCoord.u = uMax;
	screenQuad.vertex[3].texCoord.v = vMax;
	
	// Create vertex buffer

	D3D10_BUFFER_DESC vbufferDesc;

	memset(&vbufferDesc,0,sizeof(D3D10_BUFFER_DESC));

	vbufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	vbufferDesc.ByteWidth = 4*sizeof(ScreenVertex);
	vbufferDesc.Usage = D3D10_USAGE_DYNAMIC;
	vbufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	vbufferDesc.MiscFlags = 0;

	D3D10_SUBRESOURCE_DATA initialCopy;

	initialCopy.pSysMem = &screenQuad.vertex[0];
	initialCopy.SysMemPitch = 4*sizeof(ScreenVertex);
	initialCopy.SysMemSlicePitch = 4*sizeof(ScreenVertex);

	hr = d3d_Device->CreateBuffer(&vbufferDesc,&initialCopy,&vertexBuffer);

	if (hr != S_OK)
	{
		//Ceng::Log::Print("CreateBuffer : Failed");

		Reset();
		return CE_ERR_FAIL;
	}

	//**********************************'
	// Create full-screen texture

	D3D10_TEXTURE2D_DESC frameBufferDesc;

	memset(&frameBufferDesc,0,sizeof(D3D10_TEXTURE2D_DESC));

	frameBufferDesc.Width = 1366;//displayWidth;
	frameBufferDesc.Height = 768;//displayHeight;

	frameBufferDesc.Format = sourceMode->Format;
	frameBufferDesc.MipLevels = 1;

	frameBufferDesc.ArraySize = 1;

	frameBufferDesc.SampleDesc.Count = 1;
	frameBufferDesc.SampleDesc.Quality = 0;
	
	frameBufferDesc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
	frameBufferDesc.Usage = D3D10_USAGE_DYNAMIC;
	frameBufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;

	frameBufferDesc.MiscFlags = 0;

	hr = d3d_Device->CreateTexture2D(&frameBufferDesc,nullptr,&screenTexture);
	
	if (hr != S_OK)
	{
		if (hr == E_FAIL)
		{
			//Ceng::Log::Print("CreateTexture2D : No debug layer"); 
		}
		else if (hr == E_INVALIDARG)
		{
			//Ceng::Log::Print("CreateTexture2D : Invalid argument");
		}
		else
		{
			//Ceng::Log::Print("CreateTexture2D : Failed");
		}

		Reset();
		return CE_ERR_FAIL;
	}

	//**********************************************************
	// Create shader resource view

	hr = d3d_Device->CreateShaderResourceView(screenTexture,nullptr,&frameResource);

	if (hr != S_OK)
	{
		if (hr == E_FAIL)
		{
			//Ceng::Log::Print("CreateShaderView : No debug layer"); 
		}
		else if (hr == E_INVALIDARG)
		{
			//Ceng::Log::Print("CreateShaderView : Invalid argument");
		}
		else
		{
			//Ceng::Log::Print("CreateShaderView : Failed");
		}

		Reset();
		return CE_ERR_FAIL;
	}

	//******************************************************************
	// Create an effect

	std::stringstream effectCode;

	// Add vertex shader

	effectCode << "struct VS_INPUT" << std::endl;
	effectCode << "{" << std::endl;
	effectCode << "float2 position : in_position;" << std::endl;
	effectCode << "float2 texcoord : in_texcoord;" << std::endl;
	effectCode << "};" << std::endl;

	effectCode << "struct VS_OUTPUT" << std::endl;
	effectCode << "{" << std::endl;
	effectCode << "float4 position : SV_Position;" << std::endl;
	effectCode << "float2 texcoord : ps_texcoord;" << std::endl;
	effectCode << "};" << std::endl;

	effectCode << "VS_OUTPUT vs_main(VS_INPUT vs_in)" << std::endl;
	effectCode << "{" << std::endl;
	effectCode << "VS_OUTPUT vs_out;" << std::endl;
	effectCode << "vs_out.position = float4(vs_in.position,0.0,1.0);" << std::endl;
	effectCode << "vs_out.texcoord = vs_in.texcoord;" << std::endl;
	effectCode << "return vs_out;" << std::endl;
	effectCode << "}" << std::endl;

	// Add pixel shader

	effectCode << "Texture2D<float4> screenTexture;"<< std::endl;

	effectCode << "SamplerState screenSampler" << std::endl;
	effectCode << "{" << std::endl;
	effectCode << "Filter = MIN_MAG_MIP_POINT;" << std::endl;
	effectCode << "AddressU = Wrap;" << std::endl;
	effectCode << "AddressV = Wrap;" << std::endl;
	effectCode << "};" << std::endl;

	effectCode << "float4 ps_main(VS_OUTPUT ps_in) : SV_Target0" << std::endl;
	effectCode << "{" << std::endl;
	//effectCode << "return float4(1.0,1.0,1.0,1.0);" << endl;
	effectCode << "return screenTexture.Sample(screenSampler,ps_in.texcoord).bgra;" << std::endl;
	effectCode << "}" << std::endl;

	// Add technique

	effectCode << "technique10 screenQuad" << std::endl;
	effectCode << "{" << std::endl;

	effectCode << "pass pass0" << std::endl;
	effectCode << "{" << std::endl;
	effectCode << "SetVertexShader( CompileShader(vs_4_0,vs_main() ) );" << std::endl;
	effectCode << "SetPixelShader( CompileShader(ps_4_0,ps_main() ) );" << std::endl;
	effectCode << "SetGeometryShader(nullptr);" << std::endl;
	effectCode << "}" << std::endl;

	effectCode << "}" << std::endl;

	ID3D10Blob *compiledEffect;
	ID3D10Blob *errorList = nullptr;

	hr = D3D10CompileEffectFromMemory((void*)effectCode.str().c_str(),effectCode.str().length(),nullptr,nullptr,nullptr,
											0,0,&compiledEffect,&errorList);

	if (hr != S_OK)
	{
		//Ceng::Log::Print((char*)errorList->GetBufferPointer());

		errorList->Release();

		if (hr == E_FAIL)
		{
			//Ceng::Log::Print("CompileEffect : No debug layer"); 
		}
		else if (hr == E_INVALIDARG)
		{
			//Ceng::Log::Print("CompileEffect : Invalid argument");
		}
		else
		{
			//Ceng::Log::Print("CompileEffect : Failed");
		}

		Reset();
		return CE_ERR_FAIL;
	}

	hr = D3D10CreateEffectFromMemory(compiledEffect->GetBufferPointer(),compiledEffect->GetBufferSize(),
										0,d3d_Device,nullptr,&effect);

	if (hr != S_OK)
	{
		if (hr == E_FAIL)
		{
			//Ceng::Log::Print("CreateEffect : No debug layer"); 
		}
		else if (hr == E_INVALIDARG)
		{
			//Ceng::Log::Print("CreateEffect : Invalid argument");
		}
		else
		{
			//Ceng::Log::Print("CreateEffect : Failed");
		}

		Reset();
		return CE_ERR_FAIL;
	}

	ID3D10EffectTechnique *technique = effect->GetTechniqueByName("screenQuad");

	ID3D10EffectPass *pass = technique->GetPassByName("pass0");

	D3D10_PASS_DESC passDesc;

	pass->GetDesc(&passDesc);

	//******************************************************************
	// Create vertex input format

	D3D10_INPUT_ELEMENT_DESC inputVar[2];

	memset(inputVar,0,2*sizeof(D3D10_INPUT_ELEMENT_DESC));

	inputVar[0].SemanticName = "in_position";
	inputVar[0].Format = DXGI_FORMAT_R32G32_FLOAT; // float2
	inputVar[0].SemanticIndex = 0;
	inputVar[0].InputSlot = 0;
	inputVar[0].InstanceDataStepRate = 0;
	inputVar[0].AlignedByteOffset = 0;
	inputVar[0].InputSlotClass = D3D10_INPUT_PER_VERTEX_DATA;

	inputVar[1].SemanticName = "in_texcoord";
	inputVar[1].Format = DXGI_FORMAT_R32G32_FLOAT; // float2
	inputVar[1].SemanticIndex = 0;
	inputVar[1].InputSlot = 0;
	inputVar[1].InstanceDataStepRate = 0;
	inputVar[1].AlignedByteOffset = 8;
	inputVar[1].InputSlotClass = D3D10_INPUT_PER_VERTEX_DATA;
	
	hr = d3d_Device->CreateInputLayout(inputVar,2,passDesc.pIAInputSignature,passDesc.IAInputSignatureSize,&inputLayout);

	if (hr != S_OK)
	{
		if (hr == E_FAIL)
		{
			//Ceng::Log::Print("CreateInputLayout : No debug layer"); 
		}
		else if (hr == E_INVALIDARG)
		{
			//Ceng::Log::Print("CreateInputLayout : Invalid argument");
		}
		else
		{
			//Ceng::Log::Print("CreateInputLayout : Failed");
		}

		Reset();
		return CE_ERR_FAIL;
	}

	compiledEffect->Release();

	//*********************************************************************
	// Bind screenTexture to unit 0
	
	textureHandle = effect->GetVariableByName("screenTexture")->AsShaderResource();

	textureHandle->SetResource(frameResource);

	//*********************************************************************
	// Set input assembler state

	d3d_Device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	UINT32 vbufferStride = sizeof(ScreenVertex);
	UINT32 vbufferOffset = 0;

	d3d_Device->IASetVertexBuffers(0,1,&vertexBuffer,&vbufferStride,&vbufferOffset);

	d3d_Device->IASetInputLayout(inputLayout);

	//*********************************************************************
	// Set output merger state

	d3d_Device->OMSetRenderTargets(1,&backBufferView,nullptr);

	pass->Apply(0);

	//*********************************************************************
	// Set rasterizer state

	D3D10_VIEWPORT viewPort;

	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;

	viewPort.Width = modeDesc.Width;
	viewPort.Height = modeDesc.Height;

	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;

	d3d_Device->RSSetViewports(1,&viewPort);

	return CE_OK;
}

const CRESULT Graphics2D_D3D10::ChangeWindowSize(const Ceng::UINT32 width,const Ceng::UINT32 height)
{
	//return CE_OK;

	//*********************************************'
	// Change portion of texture to render

	float uMax = float(width) / 1366.0f;
	float vMax = float(height) / 768.0f;

	// Top-left corner
	screenQuad.vertex[0].position.x = -1.0f;
	screenQuad.vertex[0].position.y = 1.0f;

	screenQuad.vertex[0].texCoord.u = 0.0f;
	screenQuad.vertex[0].texCoord.v = 0.0f;

	// Top-right corner
	screenQuad.vertex[1].position.x = 1.0f;
	screenQuad.vertex[1].position.y = 1.0f;

	screenQuad.vertex[1].texCoord.u = uMax;
	screenQuad.vertex[1].texCoord.v = 0.0f;
	
	// Bottom-left corner
	screenQuad.vertex[2].position.x = -1.0f;
	screenQuad.vertex[2].position.y = -1.0f;

	screenQuad.vertex[2].texCoord.u = 0.0f;
	screenQuad.vertex[2].texCoord.v = vMax;

	// Bottom-right corner
	screenQuad.vertex[3].position.x = 1.0f;
	screenQuad.vertex[3].position.y = -1.0f;

	screenQuad.vertex[3].texCoord.u = uMax;
	screenQuad.vertex[3].texCoord.v = vMax;

	UINT8 *bufferPtr;

	vertexBuffer->Map(D3D10_MAP_WRITE_DISCARD,0,(void**)&bufferPtr);

	memcpy(bufferPtr,&screenQuad.vertex[0],4*sizeof(ScreenVertex));

	vertexBuffer->Unmap();

	modeDesc.Width = width;
	modeDesc.Height = height;

	// Release back buffer

	UINT8 *empty = nullptr;

	d3d_Device->ClearState();

	if (backBufferView != nullptr)
	{
		backBufferView->Release();
		backBufferView = nullptr;
	}
	
	if (backBuffer != nullptr)
	{
		backBuffer->Release();
		backBuffer = nullptr;
	}
	
	HRESULT hr;

	hr = swapChain->ResizeTarget(&modeDesc);

	if (hr != S_OK)
	{
		if (hr == DXGI_ERROR_INVALID_CALL)
		{
			//Ceng::Log::Print("ResizeTarget : Invalid call"); 
		}
		else if (hr == DXGI_ERROR_WAS_STILL_DRAWING)
		{
			//Ceng::Log::Print("ResizeTarget : Still drawing");
		}
		else
		{
			//Ceng::Log::Print("ResizeTarget : Failed");
		}
	}

	hr = swapChain->ResizeBuffers(0,0,0,DXGI_FORMAT_UNKNOWN,DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

	if (hr != S_OK)
	{
		if (hr == DXGI_ERROR_INVALID_CALL)
		{
			//Ceng::Log::Print("ResizeBuffers : Invalid call"); 
		}
		else if (hr == DXGI_ERROR_WAS_STILL_DRAWING)
		{
			//Ceng::Log::Print("ResizeBuffers : Still drawing");
		}
		else
		{
			//Ceng::Log::Print("ResizeBuffers : Failed");
		}
	}
	
	hr = swapChain->GetBuffer(0,__uuidof(ID3D10Texture2D),(void**)&backBuffer);

	if (hr != S_OK)
	{
		//Ceng::Log::Print("SwapChain.GetBuffer : Failed");
	}

	if (backBuffer == nullptr)
	{
		//Ceng::Log::Print("backBuffer = nullptr");
	}

	hr = d3d_Device->CreateRenderTargetView(backBuffer,nullptr,&backBufferView);
	if (hr != S_OK)
	{
		//Ceng::Log::Print("CreateRenderTargetView : Failed");
	}

	if (backBufferView == nullptr)
	{
		//Ceng::Log::Print("backBufferView = nullptr");
	}

	d3d_Device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	UINT32 vbufferStride = sizeof(ScreenVertex);
	UINT32 vbufferOffset = 0;

	d3d_Device->IASetVertexBuffers(0,1,&vertexBuffer,&vbufferStride,&vbufferOffset);

	d3d_Device->IASetInputLayout(inputLayout);

	d3d_Device->OMSetRenderTargets(1,&backBufferView,nullptr);

	ID3D10EffectTechnique *technique = effect->GetTechniqueByName("screenQuad");

	ID3D10EffectPass *pass = technique->GetPassByName("pass0");

	textureHandle->SetResource(frameResource);

	pass->Apply(0);

	D3D10_VIEWPORT viewPort;

	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;

	viewPort.Width = width;
	viewPort.Height = height;

	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;

	d3d_Device->RSSetViewports(1,&viewPort);

	return CE_OK;
}

const CRESULT Graphics2D_D3D10::ShowFrame(Ceng::ImageBuffer *frameBuffer)
{
	HRESULT hr;

	D3D10_MAPPED_TEXTURE2D map;

	hr = screenTexture->Map(0,D3D10_MAP_WRITE_DISCARD,NULL,&map);

	if (hr == S_OK)
	{

		INT32 width = frameBuffer->GetWidth();
		INT32 height = frameBuffer->GetHeight();

		/*
		frameBuffer->ToFrameBuffer(0,0,width,height,Ceng::IMAGE_FORMAT::C32_ARGB,
									width,height,
									map.RowPitch,map.pData);
									*/

		screenTexture->Unmap(0);
	}

	const float clearColor[4] = {1.0f,0.0f,0.0f,1.0f};

	d3d_Device->Draw(4,0);

	swapChain->Present(0,0);

	return CE_OK;
}

const CRESULT Graphics2D_D3D10::Restore()
{
	return CE_OK;
}

const CRESULT Graphics2D_D3D10::FillDisplay(const Ceng::UINT8 red,const Ceng::UINT8 green,const Ceng::UINT8 blue)
{
	return CE_OK;
}