/*****************************************************************************
*
* api-command.h
*
* Created By Jari Korkala 30/9/2014
*
*****************************************************************************/

#ifndef CENG_API_COMMAND_H
#define CENG_API_COMMAND_H

#include <ceng/datatypes/return-val.h>

#include <ceng/enums/primitive-type.h>
#include <ceng/datatypes/basic-types.h>

#include <ceng/datatypes/color-format.h>

#include <memory>

namespace Ceng
{
	class CR_RenderContext;

	class RenderState;

	class ApiCommand
	{
	protected:

		Ceng::UINT32 id;

		std::shared_ptr<RenderState> renderState;

		ApiCommand();

	public:

		
		virtual ~ApiCommand();

		ApiCommand(const Ceng::UINT32 id);

		const Ceng::UINT32 Id() const;

		virtual const CRESULT Prepare(CR_RenderContext *renderer)=0;

		virtual const CRESULT Execute(CR_RenderContext *renderer)=0;

	};

	class Cmd_DrawPrimitive : public ApiCommand
	{
	protected:

		Ceng::PRIMITIVE_TYPE::value type;
		Ceng::UINT32 firstVertex;
		Ceng::UINT32 amount;

		Cmd_DrawPrimitive();

	public:		

		Cmd_DrawPrimitive(const Ceng::UINT32 id,const Ceng::PRIMITIVE_TYPE::value type,
								const Ceng::UINT32 firstVertex,
								const Ceng::UINT32 amount,
								std::shared_ptr<RenderState> &renderState);

		virtual ~Cmd_DrawPrimitive();

		virtual const CRESULT Prepare(CR_RenderContext *renderer) override;
		virtual const CRESULT Execute(CR_RenderContext *renderer) override;
	};

	class RenderTarget;

	class Cmd_ClearTarget : public ApiCommand
	{
	protected:

		RenderTarget *renderTarget;
		CE_Color color;

		Cmd_ClearTarget();

	public:
		

		Cmd_ClearTarget(const Ceng::UINT32 id,Ceng::RenderTarget *renderTarget,
									 const CE_Color &color,
									 std::shared_ptr<RenderState> &renderState);

		virtual ~Cmd_ClearTarget();

		virtual const CRESULT Prepare(CR_RenderContext *renderer) override;
		virtual const CRESULT Execute(CR_RenderContext *renderer) override;
	};

	class Cmd_ClearDepth : public ApiCommand
	{
	protected:

		Ceng::FLOAT32 depth;

		Cmd_ClearDepth();
	public:
		

		Cmd_ClearDepth(const Ceng::UINT32 id,const Ceng::FLOAT32 depth,
			std::shared_ptr<RenderState> &renderState);

		virtual ~Cmd_ClearDepth();

		virtual const CRESULT Prepare(CR_RenderContext *renderer) override;
		virtual const CRESULT Execute(CR_RenderContext *renderer) override;
	};

	class Cmd_ClearDepthStencil : public ApiCommand
	{
	protected:

		Ceng::FLOAT32 depth;
		Ceng::FLOAT32 stencil;

	public:
		Cmd_ClearDepthStencil();

		Cmd_ClearDepthStencil(const Ceng::UINT32 id,const Ceng::FLOAT32 depth,const Ceng::UINT32 stencil,
								std::shared_ptr<RenderState> &renderState);

		virtual ~Cmd_ClearDepthStencil();

		virtual const CRESULT Prepare(CR_RenderContext *renderer) override;
		virtual const CRESULT Execute(CR_RenderContext *renderer) override;
	};

	class CR_SwapChain;

	class Cmd_FlipBuffers : public ApiCommand
	{
	protected:

		CR_SwapChain *swapChain;

	public:
		Cmd_FlipBuffers(const Ceng::UINT32 id,CR_SwapChain *swapChain);
		virtual ~Cmd_FlipBuffers();

		virtual const CRESULT Prepare(CR_RenderContext *renderer) override;
		virtual const CRESULT Execute(CR_RenderContext *renderer) override;
	};
}

#endif