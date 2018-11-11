/*****************************************************************************
*
* render-device.h
*
* By Jari Korkala 6/2011
*
* Update 10/2012 : Removed references to platform layer
*
* Update 4/2013 : Moved all interfaces to separate files
*
* Platform-independent interface for 3d render devices.
*
*****************************************************************************/

#ifndef _CONCEPT2_RENDER_DEVICE_H
#define _CONCEPT2_RENDER_DEVICE_H

#include <vector>

#include "base-interface.h"

#include "image-buffer.h"

#include "render-enums.h"
#include "../math/ce-vector.h"
#include "../math/ce-matrix.h"

#include "render-resource.h"

#include "vertex-decl.h"
#include "vertex-buffer.h"
#include "vertex-format.h"

#include "shader-const.h"
#include "vertex-shader.h"

#include "render-target.h"

#include "texture-2d.h"

#include "pixel-shader.h"

#include "depth-stencil-desc.h"

#include "rasterizer-state.h"

#include "../datatypes/sampler-state-desc.h"
#include "sampler-state.h"

#include "shader-resource-view.h"
#include "../datatypes/shader-resource-view-desc.h"

#include "../datatypes/tex2d-desc.h"

#include "../datatypes/sub-resource-data.h"

namespace Ceng
{
	class CE_Color;
		
	//**************************************************
	// Render device interface

	class RenderDevice : public Ceng::BASE_INTERFACE
	{
	protected:
		virtual ~RenderDevice()
		{
		}
	
	public:
		RenderDevice()
		{
		}

		virtual CRESULT CreateVertexShader(Ceng::VertexShader **shaderPtr)=0;
		
		virtual CRESULT CreateVertexFormat(const std::vector<Ceng::VertexDeclData> 
											&vertexDecl,
											Ceng::VertexFormat **format)=0;
		
		virtual CRESULT CreateVertexBuffer(UINT32 vertexSizeBytes,
											UINT32 vertexNum,
											UINT32 usage,
											Ceng::VertexBuffer **destPtr)=0;

		virtual CRESULT CreatePixelShader(Ceng::PixelShader **shaderPtr)=0;

		virtual const CRESULT CreateSamplerState(const SamplerStateDesc &desc,
			Ceng::SamplerState **out_state) = 0;

		virtual const CRESULT CreateShaderResourceView(RenderResource *resource,
			ShaderResourceViewDesc &desc,
			ShaderResourceView **out_resourceView) = 0;


		virtual const CRESULT CreateTexture2D(const Texture2dDesc &desc,
										const SubResourceData *initialData,
										Ceng::Texture2D **texturePtr)=0;

		virtual CRESULT CreateProjectionMatrix(UINT32 displayWidth,
												UINT32 displayHeight,
												FLOAT32 horizontalField,
												FLOAT32 zNear,
												FLOAT32 zFar,
												Matrix4 *dest)=0;
		
	};

} // Namespace Ceng

#endif // Include guard