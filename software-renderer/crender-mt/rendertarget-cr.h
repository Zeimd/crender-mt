/*****************************************************************************
*
* rendertarget-cr.h
*
* By Jari Korkala 2/2012
*
* Render target interface declarations moved here from "crender.h"
*
*****************************************************************************/

#ifndef _CONCEPT2_RENDERTARGET_CR_H
#define _CONCEPT2_RENDERTARGET_CR_H

#include <memory>

#include <ceng/interfaces/render-target.h>

#include "crender-base.h"
#include "rtarget-data.h"

namespace Ceng
{

	class CR_RenderTarget : public Ceng::RenderTarget
	{
	protected:
	
		// Use shared pointer to prevent deallocation of
		// the buffer while it is in use
		std::shared_ptr<CR_NewTargetData> bufferObject;
	
	public:
		CR_RenderTarget();
		CR_RenderTarget(const std::shared_ptr<CR_NewTargetData> &newBuffer);
		
		virtual ~CR_RenderTarget();		

		virtual void Release() override;

		virtual const BufferType::value GetResourceType() override;

		const std::shared_ptr<CR_NewTargetData> GetBufferObject();

		CR_NewTargetData* GetBufferPtr();
	};

} // Namespace Ceng

#endif // Include guard