/*****************************************************************************
*
* graphics-2d-d3d10.h
*
* By Jari Korkala 5/2013
*
*****************************************************************************/

#ifndef CENG_GRAPHICS_2D_D3D10_H
#define CENG_GRAPHICS_2D_D3D10_H

#include <d3d10.h>

#include <sstream>

#include <ceng/interfaces/graphics-2d.h>
#include <ceng/lib/liblog.h>


namespace Ceng
{

	class Graphics2D_D3D10 : public Ceng::Graphics2D
	{
	public:
		ID3D10Device *d3d_Device;
		IDXGISwapChain *swapChain;

		ID3D10Buffer *vertexBuffer;

		ID3D10Texture2D *backBuffer;
		ID3D10RenderTargetView *backBufferView;

		ID3D10Texture2D *screenTexture;

		ID3D10SamplerState *screenSampler;
		ID3D10ShaderResourceView *frameResource;

		ID3D10InputLayout *inputLayout;

		ID3D10Effect *effect;

		ID3D10EffectShaderResourceVariable *textureHandle;

		DXGI_MODE_DESC modeDesc;

		UINT32 displayWidth;
		UINT32 displayHeight;

		struct Float2
		{
			float x;
			float y;
		};

		struct Float3
		{
			float x;
			float y;
			float z;
		};

		struct TexCoord2
		{
			float u;
			float v;
		};

		struct ScreenVertex
		{
			Float2 position;
			TexCoord2 texCoord;
		};

		struct Square
		{
			ScreenVertex vertex[4];
			ID3D10Texture2D *texture;
		};

		Square screenQuad;
	
	public:
		Graphics2D_D3D10();
		virtual ~Graphics2D_D3D10();

		virtual void Release() override;
		
		virtual const CRESULT Restore();



		void Reset();
		
		const CRESULT ConfigureOutput(ID3D10Device *sourceDevice,IDXGISwapChain *swapChain,DXGI_MODE_DESC *sourceMode);

		virtual const CRESULT ChangeWindowSize(const Ceng::UINT32 width,const Ceng::UINT32 height);
		
		virtual const CRESULT ShowFrame(Ceng::ImageBuffer *frameBuffer);

		virtual const CRESULT FillDisplay(const Ceng::UINT8 red,const Ceng::UINT8 green,const Ceng::UINT8 blue);
	};

} // Namespace Ceng

#endif