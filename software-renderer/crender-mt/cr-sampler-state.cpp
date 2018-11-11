/*****************************************************************************
*
* cr-sampler-state.cpp
*
* Created By Jari Korkala 5/3/2015
*
*****************************************************************************/

#include <cstring>

#include "cr-sampler-state.h"

using namespace Ceng;

CR_SamplerState::CR_SamplerState()
{
}

CR_SamplerState::CR_SamplerState(const SamplerStateDesc &desc)
{
	memcpy(&this->desc, &desc, sizeof(SamplerStateDesc));

	/*
	const Ceng::UINT32 nearestMipMask = TextureMinFilter::compare_linear_mip_nearest
		 | TextureMinFilter::compare_nearest_mip_nearest |
		 TextureMinFilter::linear_mip_nearest |
		 TextureMinFilter::nearest_mip_nearest;

	const Ceng::UINT32 linearMipMask = TextureMinFilter::compare_linear_mip_linear
		| TextureMinFilter::compare_nearest_mip_linear |
		TextureMinFilter::linear_mip_linear |
		TextureMinFilter::nearest_mip_linear;

	mipMode = none;

	if (desc.minFilter & nearestMipMask)
	{
		mipMode = nearest;
	}
	else if (desc.minFilter & linearMipMask)
	{
		mipMode = linear;
	}
	*/
}

CR_SamplerState::~CR_SamplerState()
{
}

void CR_SamplerState::Release()
{
	delete this;
}

void CR_SamplerState::GetSamplerDesc(SamplerStateDesc &desc)
{
	std::memcpy(&desc, &this->desc, sizeof(SamplerStateDesc));
}