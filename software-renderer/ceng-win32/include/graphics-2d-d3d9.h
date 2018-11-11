/*****************************************************************************
*
* graphics-2d-d3d9.h
*
* By Jari Korkala 10/2012
*
*****************************************************************************/

#ifndef _CONCEPT2_GRAPHICS_2D_D3D9_H
#define _CONCEPT2_GRAPHICS_2D_D3D9_H

#include <vector>

#include <d3d9.h>
#include <d3d9types.h>

#include <ceng/interfaces/graphics-2d.h>

#include <ceng/datatypes/thread.h>
#include <ceng/interfaces/thread-task.h>
#include <ceng/datatypes/critical-section.h>
#include <ceng/datatypes/condition-variable.h>

namespace Ceng
{
	class Graphics2D_D3D9;

	class Graphics2D_RenderTask : public ThreadTask
	{
	public:

		Graphics2D_D3D9 *renderDevice;

		volatile bool exitLoop;

	public:

		Graphics2D_RenderTask();

		virtual ~Graphics2D_RenderTask();

		virtual void Release() override;

		virtual const CRESULT Execute() override;

		virtual const CRESULT Exit() override;
	};

	class Graphics2D_D3D9 : public Ceng::Graphics2D
	{
	public:
		D3DPRESENT_PARAMETERS d3d_Params;
		
		IDirect3DDevice9 *d3d_Device;
		IDirect3DVertexBuffer9 *vertexBuffer[2];

		struct D3D_SCREEN_VERTEX
		{
			float x;
			float y;
			float z;
			float rhw;
			float u;
			float v;
		};
		
		DWORD vertexStyle; // flexible vertex format
		
		// A format that describes a single quad with
		// one texture

		volatile UINT32 backBuffer;
		volatile UINT32 frontBuffer;

		volatile UINT32 bufferState[2];

		CriticalSection *crit_buffer[2];

		ConditionVariable *condVar;
		
		struct D3D_SCREEN_TEXTURE
		{
			D3D_SCREEN_VERTEX corner[4];
			IDirect3DTexture9 *texture[2];
		};
		
		// A group of textures which covers the entire
		// screen

		typedef std::vector<D3D_SCREEN_TEXTURE> PolygonGroup;

		PolygonGroup framePolygons;		
		
		int polyWidth,polyHeight;
		
		unsigned char *texturePtr;	
		
		/*
		 * Current width of output window.
		 * Equal to displayWidth for full-screen.
		 */
		int windowWidth;

		/*
		 * Current height of output window.
		 * Equal to displayHeight for full-screen.
		 */
		int windowHeight;

		/*
		 *  Horizontal display resolution. 
		 */
		int displayWidth;

		/*
		 * Vertical display resolution.
		 */
		int displayHeight;

		int colorDepth,bufferPitch;
		
		Ceng::IMAGE_FORMAT::value apiFormat;

		INT32 displayFormat;
		bool vsync;

		Graphics2D_RenderTask renderTask;

		Thread *renderThread;
	
	public:
		Graphics2D_D3D9();
		virtual ~Graphics2D_D3D9();

		virtual void Release() override;
		
		virtual const CRESULT Restore();
		
		const CRESULT ConfigureOutput(IDirect3DDevice9 *sourceDevice,
								D3DPRESENT_PARAMETERS *sourceParams);

		virtual const CRESULT ChangeWindowSize(const Ceng::UINT32 width,const Ceng::UINT32 height);
		
		virtual const CRESULT ShowFrame(Ceng::ImageBuffer *frameBuffer);

		virtual const CRESULT FillDisplay(const Ceng::UINT8 red,const Ceng::UINT8 green,const Ceng::UINT8 blue);

		const CRESULT TestCooperativeLevel();
	
	protected:
		const CRESULT GeneratePolygons();
	};

} // Namespace Ceng

#endif // Include guard