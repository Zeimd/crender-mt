/*****************************************************************************
*
* cr-sampler-state.h
*
* Created By Jari Korkala 5/3/2015
*
*****************************************************************************/

#ifndef CENG_CR_SAMPLER_STATE_H
#define CENG_CR_SAMPLER_STATE_H

#include <ceng/datatypes/boolean.h>

#include <ceng/interfaces/sampler-state.h>

#include <ceng/datatypes/sampler-state-desc.h>

namespace Ceng
{
	class CR_SamplerState : public SamplerState
	{
	public:

		SamplerStateDesc desc;

		enum MipMode
		{
			none = 0,
			nearest = 1,
			linear = 2,
		};

		MipMode mipMode;

	protected:
		CR_SamplerState();

	public:

		
		virtual ~CR_SamplerState();

		CR_SamplerState(const SamplerStateDesc &desc);

		virtual void Release() override;

		virtual void GetSamplerDesc(SamplerStateDesc &desc) override;
	};
}

#endif