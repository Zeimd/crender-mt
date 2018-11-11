/*****************************************************************************
*
* cr-swap-chain.h
*
* Created By Jari Korkala 3/3/2015
*
*****************************************************************************/

#ifndef CENG_CR_SWAP_CHAIN_H
#define CENG_CR_SWAP_CHAIN_H

#include <memory>

#include <ceng/interfaces/swap-chain.h>

namespace Ceng
{
	class Graphics2D;
	class CR_NewTargetData;
	class CR_RenderContext;

	class ConditionVariable;
	class CriticalSection;

	class CR_SwapChain : public SwapChain
	{
	public:

		CR_RenderContext *renderContext;

		Graphics2D *outputDevice;

		std::shared_ptr<CR_NewTargetData> backBuffer;

		ConditionVariable *presentReady;
		CriticalSection *presentCaller;

	public:

		CR_SwapChain();
		virtual ~CR_SwapChain();

		CR_SwapChain(CR_RenderContext *renderContext,Graphics2D *outputDevice, CR_NewTargetData *backBuffer);

		virtual void Release() override;

		// Default present according to device config
		virtual const CRESULT Present() override;

		virtual const CRESULT Present(const Ceng::UINT32 syncInterval, const Ceng::UINT32 options,
			const Ceng::UINT32 dirtyRectCount,
			const Ceng::Rectangle *dirtyRects,
			const Ceng::Rectangle *scrollRect,
			const Ceng::UINT32 scrollOffsetX,
			const Ceng::UINT32 scrollOffsetY) override;

		virtual const Ceng::CRESULT CR_SwapChain::GetFrameBufferTarget(Ceng::RenderTarget **renderTarget) override;
	};
}

#endif