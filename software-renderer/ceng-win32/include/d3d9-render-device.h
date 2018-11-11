/*****************************************************************************
*
* d3d9-render-device.h
*
* Created By Jari Korkala 3/3/2015
*
*****************************************************************************/

#ifndef CENG_D3D9_RENDER_DEVICE_H
#define CENG_D3D9_RENDER_DEVICE_H

#include <d3d9.h>
#include <d3d9types.h>

#include <ceng/interfaces/render-device.h>

namespace Ceng
{
	class Direct3D9_RenderDevice : public RenderDevice
	{
	protected:

		IDirect3DDevice9 *parentDevice;

	public:

		Direct3D9_RenderDevice();
		virtual ~Direct3D9_RenderDevice();

		virtual CRESULT CreateVertexShader(Ceng::VertexShader **shaderPtr) override;

		virtual CRESULT CreateVertexFormat(const std::vector<Ceng::VertexDeclData>
			&vertexDecl,
			Ceng::VertexFormat **format) override;

		virtual CRESULT CreateVertexBuffer(UINT32 vertexSizeBytes,
			UINT32 vertexNum,
			UINT32 usage,
			Ceng::VertexBuffer **destPtr) override;

		virtual CRESULT CreatePixelShader(Ceng::PixelShader **shaderPtr) override;

		virtual CRESULT CreateTexture2D(UINT16 width,
			UINT16 height,
			Ceng::IMAGE_FORMAT::value format,
			Ceng::RESOURCE_USAGE::value usage,
			UINT8 multiSampleLevel,
			UINT8 mipLevels, bool tiled,
			Ceng::Texture2D **texturePtr) override;

		virtual CRESULT CreateProjectionMatrix(UINT32 displayWidth,
			UINT32 displayHeight,
			FLOAT32 horizontalField,
			FLOAT32 zNear,
			FLOAT32 zFar,
			Matrix4 *dest) override;

	};
}

#endif