/*****************************************************************************
*
* graphics-2d-d3d9.cpp
*
* By Jari Korkala 10/2012
*
*****************************************************************************/

#include <math.h>

#include <ceng/platform.h>

#include <ceng/datatypes/rectangle.h>

#include "../include/graphics-2d-d3d9.h"

using namespace Ceng;

Graphics2D_D3D9::Graphics2D_D3D9()
{
	d3d_Device = nullptr;

	vertexBuffer[0] = nullptr;
	vertexBuffer[1] = nullptr;
	
	displayWidth = 0;
	displayHeight = 0;

	memset(&d3d_Params,0,sizeof(D3DPRESENT_PARAMETERS));

	renderThread = nullptr;

	crit_buffer[0] = nullptr;
	crit_buffer[1] = nullptr;
	
	condVar = nullptr;
}

Graphics2D_D3D9::~Graphics2D_D3D9()
{
	if (renderThread != nullptr)
	{
		renderThread->Release();
		renderThread = nullptr;
	}

	if (crit_buffer[0] != nullptr)
	{
		crit_buffer[0]->Release();
	}

	if (crit_buffer[1] != nullptr)
	{
		crit_buffer[1]->Release();
	}

	if (condVar != nullptr)
	{
		condVar->Release();
	}

	if (vertexBuffer[0] != nullptr)
	{
		vertexBuffer[0]->Release();
	}

	if (vertexBuffer[1] != nullptr)
	{
		vertexBuffer[1]->Release();
	}

	UINT32 k;

	
	for(k=0;k<framePolygons.size();k++)
	{
		framePolygons[k].texture[0]->Release();
		framePolygons[k].texture[1]->Release();
	}

	if (d3d_Device != nullptr)
	{
		d3d_Device->Release();
		d3d_Device = nullptr;
	}
}

void Graphics2D_D3D9::Release()
{
	delete this;
}


const CRESULT Graphics2D_D3D9::FillDisplay(const Ceng::UINT8 red,
										   const Ceng::UINT8 green,
										   const Ceng::UINT8 blue)
{
	if (d3d_Device == nullptr)
	{
		return CE_ERR_FAIL;
	}

	d3d_Device->Clear(0,nullptr,D3DCLEAR_TARGET,
						D3DCOLOR_XRGB(red,green,blue),1.0,0);
	
	return CE_OK;
}

const CRESULT Graphics2D_D3D9::ConfigureOutput(IDirect3DDevice9 *sourceDevice,
										 D3DPRESENT_PARAMETERS *sourceParams)
{
	if (sourceDevice == nullptr)
	{
		return CE_ERR_NULL_PTR;
	}

	if (sourceParams == nullptr)
	{
		return CE_ERR_NULL_PTR;
	}

	d3d_Device = sourceDevice;

	memcpy(&d3d_Params,sourceParams,sizeof(D3DPRESENT_PARAMETERS));

	displayWidth = d3d_Params.BackBufferWidth;
	displayHeight = d3d_Params.BackBufferHeight;

	// Determine size of color buffer pixel in bytes

	switch(d3d_Params.BackBufferFormat)
	{
	case D3DFMT_R5G6B5:
	case D3DFMT_A1R5G5B5:
		colorDepth = 2;
		break;
	case D3DFMT_A8R8G8B8:
	case D3DFMT_X8R8G8B8:
	case D3DFMT_A2R10G10B10:
		colorDepth = 4;
		break;
	}

	/* Vertex format
	 * D3DFVF_XYZRHW = Allows use of screen coordinates
	 * D3DFVF_TEX1 - Vertex has texture coordinates
	 */
	vertexStyle = D3DFVF_XYZRHW | D3DFVF_TEX1;

	d3d_Device->SetVertexShader(nullptr); // No shaders
	d3d_Device->SetFVF(vertexStyle);

	CRESULT cresult;

	cresult = GeneratePolygons();
	if (cresult != CE_OK)
	{
		d3d_Device->Release();
		d3d_Device = nullptr;

		return CE_ERR_FAIL;
	}

	// *****************************************
	// Create vertex buffer

	// Note: D3DPOOL_MANAGED is used so that
	//       there is no need to release
	//       and recreate objects in case
	//       the application loses focus.

	HRESULT hr;

	hr = d3d_Device->CreateVertexBuffer(4*framePolygons.size()*sizeof(D3D_SCREEN_VERTEX),
										NULL,vertexStyle,
										D3DPOOL_MANAGED,&vertexBuffer[0],nullptr);
	if (hr != D3D_OK)
	{
		d3d_Device->Release();
		d3d_Device = nullptr;

		return CE_ERR_FAIL;
	}

	hr = d3d_Device->CreateVertexBuffer(4*framePolygons.size()*sizeof(D3D_SCREEN_VERTEX),
										NULL,vertexStyle,
										D3DPOOL_MANAGED,&vertexBuffer[1],nullptr);
	if (hr != D3D_OK)
	{
		vertexBuffer[0]->Release();
		d3d_Device->Release();
		d3d_Device = nullptr;

		return CE_ERR_FAIL;
	}

	D3D_SCREEN_VERTEX *vertexData;

	// Copy vertex data to graphics memory
	vertexBuffer[0]->Lock(0,0,(void**)&vertexData,NULL);

	int k;
	for(k=0;k<framePolygons.size();k++)
	{
		memcpy(&vertexData[4*k],&framePolygons[k].corner,4*sizeof(D3D_SCREEN_VERTEX));
	}

	vertexBuffer[0]->Unlock();	

	vertexBuffer[1]->Lock(0,0,(void**)&vertexData,NULL);

	for(k=0;k<framePolygons.size();k++)
	{
		memcpy(&vertexData[4*k],&framePolygons[k].corner,4*sizeof(D3D_SCREEN_VERTEX));
	}

	vertexBuffer[1]->Unlock();	

	//*******************************************************
	// Various render states suitable for 2D drawing

	// Disable lighting 
	d3d_Device->SetRenderState(D3DRS_LIGHTING, FALSE);

	// Disable alpha blending
	d3d_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

	// Ensure source is rendered completely opaque
	d3d_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	d3d_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	d3d_Device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	//***************************************************
	// Create textures for the screen-covering polygons

	// Note: D3DPOOL_MANAGED is used so that
	//       there is no need to release
	//       and recreate objects in case
	//       the application loses focus.

	frontBuffer = 0;
	backBuffer = 0;

	bufferState[0] = 0;
	bufferState[1] = 0;

	for(k=0;k<framePolygons.size();k++)
	{
		
		hr = d3d_Device->CreateTexture(polyWidth,polyHeight,1,
										 D3DUSAGE_DYNAMIC ,
										d3d_Params.BackBufferFormat,
										D3DPOOL_DEFAULT,
										&framePolygons[k].texture[0],
										nullptr);
										

		if (hr != D3D_OK)
		{
			int j;
			for(j=0;j<k;j++)
			{
				framePolygons[j].texture[0]->Release();
				framePolygons[j].texture[1]->Release();
			}

			vertexBuffer[0]->Release();
			vertexBuffer[1]->Release();
			
			d3d_Device->Release();
			d3d_Device = nullptr;

			return CE_ERR_FAIL;
		}

		hr = d3d_Device->CreateTexture(polyWidth,polyHeight,1,
										 D3DUSAGE_DYNAMIC ,
										d3d_Params.BackBufferFormat,
										D3DPOOL_DEFAULT,
										&framePolygons[k].texture[1],
										nullptr);
										

		if (hr != D3D_OK)
		{
			int j;
			for(j=0;j<k;j++)
			{
				framePolygons[j].texture[0]->Release();
				framePolygons[j].texture[1]->Release();
			}

			framePolygons[k].texture[0]->Release();

			vertexBuffer[0]->Release();
			vertexBuffer[1]->Release();
			
			d3d_Device->Release();
			d3d_Device = nullptr;

			return CE_ERR_FAIL;
		}
	}

	cresult = Ceng_CreateCriticalSection(&crit_buffer[0]);
	cresult = Ceng_CreateCriticalSection(&crit_buffer[1]);
	
	cresult = Ceng_CreateConditionVar(&condVar);

	renderTask.renderDevice = this;

	cresult = Ceng_CreateThread(&renderTask,false,&renderThread);

	return CE_OK;
}

const CRESULT Graphics2D_D3D9::GeneratePolygons()
{
	int xnum,ynum,k,j,index;

	HRESULT hr;

	D3DCAPS9 deviceCapability;

	hr = d3d_Device->GetDeviceCaps(&deviceCapability);

	bool squares = false;

	if (deviceCapability.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY)
	{
		squares = true;
	}
	else
	{
		if (deviceCapability.MaxTextureWidth < displayWidth ||
			deviceCapability.MaxTextureHeight < displayHeight)
		{
			squares = true;
		}
	}

	if (squares == true)
	{
		/*
		polyWidth = deviceCapability.MaxTextureWidth;

		if (deviceCapability.MaxTextureHeight < polyWidth)
		{
			polyWidth = deviceCapability.MaxTextureHeight;
		}
		
		// Find largest size that fits within screen boundaries
		while (displayWidth < polyWidth || displayHeight < polyWidth)
		{
			polyWidth /= 2;
		}

		polyHeight = polyWidth;
		
		xnum = int(ceil(float(displayWidth)/float(polyWidth)));
		ynum = int(ceil(float(displayHeight)/float(polyWidth)));
		
		Ceng::INT32 polyNum = xnum*ynum;

		D3D_SCREEN_TEXTURE tempPoly;
		
		for(k=0;k<ynum;k++)
		{
			for(j=0;j<xnum;j++)
			{
				index = xnum*k+j;
				
				// Top-left vertex
				tempPoly.corner[0].x = float(polyWidth*j)-0.5f;
				tempPoly.corner[0].y = float(polyWidth*k)-0.5f;
				tempPoly.corner[0].z = 0.0f;
				tempPoly.corner[0].rhw = 1.0f;
				tempPoly.corner[0].u = 0.0f;
				tempPoly.corner[0].v = 0.0f;
				
				// Top-right vertex
				tempPoly.corner[1].x = float(polyWidth*(j+1))-0.5f;
				tempPoly.corner[1].y = float(polyWidth*k)-0.5f;
				tempPoly.corner[1].z = 0.0f;
				tempPoly.corner[1].rhw = 1.0f;
				tempPoly.corner[1].u = 1.0f;
				tempPoly.corner[1].v = 0.0f;
				
				// Bottom-right vertex
				tempPoly.corner[2].x = float(polyWidth*(j+1))-0.5f;
				tempPoly.corner[2].y = float(polyWidth*(k+1))-0.5f;
				tempPoly.corner[2].z = 0.0f;
				tempPoly.corner[2].rhw = 1.0f;
				tempPoly.corner[2].u = 1.0f;
				tempPoly.corner[2].v = 1.0f;
				
				// Bottom-left vertex
				
				tempPoly.corner[3].x = float(polyWidth*j)-0.5f;
				tempPoly.corner[3].y = float(polyWidth*(k+1))-0.5f;
				tempPoly.corner[3].z = 0.0f;
				tempPoly.corner[3].rhw = 1.0f;
				tempPoly.corner[3].u = 0.0f;
				tempPoly.corner[3].v = 1.0f;
				
				if (tempPoly.corner[1].x >= float(displayWidth)) 
				{
					tempPoly.corner[1].x = float(displayWidth)-0.5f;
					tempPoly.corner[2].x = float(displayWidth)-0.5f;
					
					tempPoly.corner[1].u = (tempPoly.corner[1].x -
						tempPoly.corner[0].x ) /
						float(polyWidth);
					
					tempPoly.corner[2].u = tempPoly.corner[1].u;
				}
				
				if (tempPoly.corner[2].y >= float(displayHeight)) 
				{
					tempPoly.corner[2].y = float(displayHeight)-0.5f;
					tempPoly.corner[3].y = float(displayHeight)-0.5f;
					
					tempPoly.corner[2].v = (tempPoly.corner[2].y -
						tempPoly.corner[0].y ) /
						float(polyWidth);
					
					tempPoly.corner[3].v = tempPoly.corner[2].v;
				}

				framePolygons.push_back(tempPoly);
			}
		}
		*/
	}
	else
	{
		polyWidth = displayWidth;
		polyHeight = displayHeight;

		D3D_SCREEN_TEXTURE tempPoly;

		// Top-left vertex
		tempPoly.corner[0].x = -0.5f;
		tempPoly.corner[0].y = -0.5f;
		tempPoly.corner[0].z = 0.0f;
		tempPoly.corner[0].rhw = 1.0f;
		tempPoly.corner[0].u = 0.0f;
		tempPoly.corner[0].v = 0.0f;

		// Top-right vertex
		tempPoly.corner[1].x = float(polyWidth)-0.5f;
		tempPoly.corner[1].y = -0.5f;
		tempPoly.corner[1].z = 0.0f;
		tempPoly.corner[1].rhw = 1.0f;
		tempPoly.corner[1].u = 1.0f;
		tempPoly.corner[1].v = 0.0f;
		
		// Bottom-right vertex
		tempPoly.corner[2].x = float(polyWidth)-0.5f;
		tempPoly.corner[2].y = float(polyHeight)-0.5f;
		tempPoly.corner[2].z = 0.0f;
		tempPoly.corner[2].rhw = 1.0f;
		tempPoly.corner[2].u = 1.0f;
		tempPoly.corner[2].v = 1.0f;

		// Bottom-left vertex
		tempPoly.corner[3].x = -0.5f;
		tempPoly.corner[3].y = float(polyHeight)-0.5f;
		tempPoly.corner[3].z = 0.0f;
		tempPoly.corner[3].rhw = 1.0f;
		tempPoly.corner[3].u = 0.0f;
		tempPoly.corner[3].v = 1.0f;

		framePolygons.push_back(tempPoly);
	}

	return CE_OK;
}

const CRESULT Graphics2D_D3D9::Restore()
{
	crit_buffer[0]->Lock();
	//crit_buffer[1]->Lock();

	D3DPRESENT_PARAMETERS tempParams;
	HRESULT hr;

	// Release textures

	INT32 k;

	for(k=0;k<framePolygons.size();k++)
	{
		framePolygons[k].texture[0]->Release();
		framePolygons[k].texture[1]->Release();
	}

	// device->Reset() zeroes a number of parameters, so
	// make a copy
	memcpy(&tempParams,&d3d_Params,sizeof(D3DPRESENT_PARAMETERS));

	hr = d3d_Device->Reset(&tempParams);

	if (hr != D3D_OK)
	{
		return CE_ERR_FAIL;
	}

	// Restore rendering state
	
	/*
	hr = d3d_Device->SetStreamSource(0,vertexBuffer,0,sizeof(D3D_SCREEN_VERTEX));
	if (hr != D3D_OK)
	{		
		return CE_ERR_FAIL;
	}
	*/

	d3d_Device->SetVertexShader(nullptr);
	d3d_Device->SetFVF(vertexStyle);

	// Various render states suitable for 2D drawing
	d3d_Device->SetRenderState(D3DRS_LIGHTING, FALSE);
	d3d_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	d3d_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	d3d_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	d3d_Device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	// Re-create textures

	for(k=0;k<framePolygons.size();k++)
	{
		hr = d3d_Device->CreateTexture(polyWidth,polyHeight,1,
										 D3DUSAGE_DYNAMIC ,
										d3d_Params.BackBufferFormat,
										D3DPOOL_DEFAULT,
										&framePolygons[k].texture[0],
										nullptr);

		hr = d3d_Device->CreateTexture(polyWidth,polyHeight,1,
										 D3DUSAGE_DYNAMIC ,
										d3d_Params.BackBufferFormat,
										D3DPOOL_DEFAULT,
										&framePolygons[k].texture[1],
										nullptr);
	}


	//crit_buffer[1]->Unlock();
	crit_buffer[0]->Unlock();

	return CE_OK;
}

const CRESULT Graphics2D_D3D9::ChangeWindowSize(const Ceng::UINT32 windowWidth,const Ceng::UINT32 windowHeight)
{
	// TODO: Lock thread mutex

	//crit_buffer[0]->Lock();
	//crit_buffer[1]->Lock();


	//****************************************************************
	// TODO: clamp window size to desktop resolution

	if (windowWidth > displayWidth)
	{
		
	}

	this->windowWidth = windowWidth;
	this->windowHeight = windowHeight;

	//****************************************************************


	// TODO: Unlock thread mutex

	//crit_buffer[1]->Unlock();
	//crit_buffer[0]->Unlock();

	return CE_OK;
}

const CRESULT Graphics2D_D3D9::TestCooperativeLevel()
{
	HRESULT hr;
	
	hr = d3d_Device->TestCooperativeLevel();

	if (hr == D3D_OK)
	{
		return CE_OK;
	}

	if (hr == D3DERR_DRIVERINTERNALERROR) 
	{
		return CE_ERR_FAIL;
	}

	if (hr == D3DERR_DEVICELOST)
	{
		while (hr != D3DERR_DEVICENOTRESET)
		{
			hr = d3d_Device->TestCooperativeLevel();
		}
		
		CRESULT cresult;
		
		cresult = Restore();
		if (cresult != CE_OK)
		{
			return cresult;
		}
	}
	else if (hr == D3DERR_DEVICENOTRESET)
	{
		CRESULT cresult;
		
		cresult = Restore();
		if (cresult != CE_OK)
		{
			return cresult;
		}
	}

	return CE_OK;
}

const CRESULT Graphics2D_D3D9::ShowFrame(Ceng::ImageBuffer *frameBuffer)
{
	// lock backbuffer here

	if (d3d_Device == nullptr)
	{
		return CE_ERR_FAIL;
	}

	CRESULT cresult = TestCooperativeLevel();

	if (cresult != CE_OK)
	{
		return cresult;
	}

	/*
	while(bufferState[backBuffer] == 1)
	{
		// Spin if designated back buffer contains data for
		// rendering
	};
	*/

	//crit_buffer[backBuffer]->Lock();

	crit_buffer[0]->Lock();

	HRESULT hr;

		float delta = 0.01f;

	// Top-left vertex
	framePolygons[0].corner[0].x = -0.5f;
	framePolygons[0].corner[0].y = -0.5f;
	framePolygons[0].corner[0].z = 0.0f;
	framePolygons[0].corner[0].rhw = 1.0f;
	framePolygons[0].corner[0].u = 0.0f;
	framePolygons[0].corner[0].v = 0.0f;

	// Top-right vertex
	framePolygons[0].corner[1].x = float(windowWidth)-0.5f;
	framePolygons[0].corner[1].y = -0.5f;
	framePolygons[0].corner[1].z = 0.0f;
	framePolygons[0].corner[1].rhw = 1.0f;
	framePolygons[0].corner[1].u = float(windowWidth) / (float(displayWidth) + delta);
	framePolygons[0].corner[1].v = 0.0f;
		
	// Bottom-right vertex
	framePolygons[0].corner[2].x = float(windowWidth)-0.5f;
	framePolygons[0].corner[2].y = float(windowHeight)-0.5f;
	framePolygons[0].corner[2].z = 0.0f;
	framePolygons[0].corner[2].rhw = 1.0f;
	framePolygons[0].corner[2].u = float(windowWidth) / (float(displayWidth) + delta);
	framePolygons[0].corner[2].v = float(windowHeight) / (float(displayHeight) + delta);

	// Bottom-left vertex
	framePolygons[0].corner[3].x = -0.5f;
	framePolygons[0].corner[3].y = float(windowHeight)-0.5f;
	framePolygons[0].corner[3].z = 0.0f;
	framePolygons[0].corner[3].rhw = 1.0f;
	framePolygons[0].corner[3].u = 0.0f;
	framePolygons[0].corner[3].v = float(windowHeight) / (float(displayHeight)+delta);

	// Copy to vertex buffer

	D3D_SCREEN_VERTEX *vertexData;

	vertexBuffer[backBuffer]->Lock(0,0,(void**)&vertexData,NULL);

	memcpy(&vertexData[0],&framePolygons[0].corner,4*sizeof(D3D_SCREEN_VERTEX));

	vertexBuffer[backBuffer]->Unlock();	

	D3DLOCKED_RECT rect;
	UINT8 *localTexturePtr;
	UINT32 texturePitch;

	Ceng::Rectangle sourceArea;

	Ceng::BufferData textureData;

	textureData.format = apiFormat;

	// TODO: Query framebuffer object for padding requirements when locking
	//       areas from texture.

	INT32 k;

	for(k=0;k<framePolygons.size();k++)
	{
		// Only marks the specified area as dirty. Useful for optimization,
		// but requires further coordination with framebuffer object to avoid
		// writing out of bounds.
		//hr = polygons[k].texture->LockRect(0,&rect,&targetArea,nullptr);

		hr = framePolygons[k].texture[backBuffer]->LockRect(0,&rect,NULL,NULL);
		
		localTexturePtr = (unsigned char*)rect.pBits;
		texturePitch = rect.Pitch;
		
		sourceArea.left = int(framePolygons[k].corner[0].x+0.5f);
		sourceArea.top = int(framePolygons[k].corner[0].y+0.5f);

		sourceArea.right = int(framePolygons[k].corner[2].x+0.5f);
		sourceArea.bottom = int(framePolygons[k].corner[2].y+0.5f);

		textureData.width = displayWidth;
		textureData.height = displayHeight;

		textureData.pitch = texturePitch;
		textureData.data = localTexturePtr;

		frameBuffer->ToFrameBuffer(sourceArea,textureData);
	
		framePolygons[k].texture[backBuffer]->UnlockRect(0);	

		//d3d_Device->UpdateTexture(polygons[k].texture,polygons[k].texture);
	}	

	bufferState[backBuffer] = 1;

	Ceng::UINT32 temp = backBuffer;

	backBuffer ^= 1;

	//crit_buffer[temp]->Unlock();

	crit_buffer[0]->Unlock();

	condVar->WakeAll();
	
	return CE_OK;
}

Graphics2D_RenderTask::Graphics2D_RenderTask()
{
	exitLoop = false;
}

Graphics2D_RenderTask::~Graphics2D_RenderTask()
{
}

void Graphics2D_RenderTask::Release()
{
}

const CRESULT Graphics2D_RenderTask::Exit()
{
	exitLoop = true;
	return CE_OK;
}

const CRESULT Graphics2D_RenderTask::Execute()
{
	CRESULT cresult;
	HRESULT hr;

	RECT targetArea;

	targetArea.top = 0;
	targetArea.left = 0;

	renderDevice->crit_buffer[0]->Lock();

	while(exitLoop == false)
	{
		Ceng::UINT32 frontBuffer = renderDevice->frontBuffer;

		// lock critical section for front buffer

		//renderDevice->crit_buffer[frontBuffer]->Lock();

		CRESULT cresult = CE_ERR_FAIL;

		do
		{
			cresult = renderDevice->condVar->WaitFor(renderDevice->crit_buffer[0],200);

			if (cresult != CE_OK && exitLoop == true) return CE_OK;

		} while (cresult != CE_OK);

		//renderDevice->crit_buffer[0]->Lock();

		
		if (renderDevice->bufferState[frontBuffer] == 1)
		{			
			CRESULT cresult = renderDevice->TestCooperativeLevel();

			if (cresult != CE_OK)
			{
				renderDevice->crit_buffer[frontBuffer]->Unlock();
				continue;
			}
			

			targetArea.right = renderDevice->windowWidth;
			targetArea.bottom = renderDevice->windowHeight;

			hr = renderDevice->d3d_Device->BeginScene();

			hr = renderDevice->d3d_Device->SetStreamSource(0,
				renderDevice->vertexBuffer[frontBuffer],0,sizeof(Graphics2D_D3D9::D3D_SCREEN_VERTEX));

			Ceng::INT32 k;

			for(k=0; k < renderDevice->framePolygons.size();k++)
			{
				renderDevice->d3d_Device->SetTexture(0,
					renderDevice->framePolygons[k].texture[frontBuffer]);

				renderDevice->d3d_Device->DrawPrimitive(D3DPT_TRIANGLEFAN,4*k,2);
			}

			hr = renderDevice->d3d_Device->EndScene();

			// Display frame

			if (renderDevice->d3d_Params.Windowed == TRUE)
			{
				renderDevice->d3d_Device->Present(&targetArea,NULL,NULL,NULL);
			}
			else
			{
				renderDevice->d3d_Device->Present(NULL,NULL,NULL,NULL);
			}

			renderDevice->bufferState[frontBuffer] = 0;

			renderDevice->frontBuffer ^= 1;

		}		

		//renderDevice->crit_buffer[frontBuffer]->Unlock();	

		//renderDevice->crit_buffer[0]->Unlock();	
	}

	renderDevice->crit_buffer[0]->Unlock();

	return CE_OK;
}

