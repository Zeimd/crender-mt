/****************************************************************************
*
* cr-pixel-shader.cpp
*
* --------------------------------------------------------
*
* Jari Korkala 8/2014
*
* - Renamed to cr-pixel-shader.cpp
*
* ---------------------------------------------------------
*
* Created By Jari Korkala 1/2012
*
****************************************************************************/

#include "cr-pixel-shader.h"

#include "rtarget-data.h"

#include "quad-format.h"

#include "shader-common.h"

using namespace Ceng;

CR_PixelShader::CR_PixelShader()
{
	cacheLine = 64;

	wrapper.shader = this;

	nextInstance = std::shared_ptr<Ceng::PixelShaderInstance>(new PixelShaderInstance(this));

	// Set up a NULL input and output registers 
	// to guard against exceptions from use
	// of undefined semantic links

	//nullInput.semantic = Ceng::SHADER_SEMANTIC::POSITION;
	nullInput.inputFormat = Ceng::SHADER_DATATYPE::UNKNOWN;

	//nullOutput.target = CR_SHADER_TARGET0;
	nullOutput.bufferFormat = Ceng::IMAGE_FORMAT::UNKNOWN;
}

CR_PixelShader::~CR_PixelShader()
{
}

void CR_PixelShader::Release()
{
	delete this;
}

CRESULT CR_PixelShader::ConfigureConstants()
{
	UINT32 k;
	UINT32 currentOffset = 0;

	for(k=0;k<uniformList.size();k++)
	{
		uniformList[k].bufferOffset = currentOffset;

		uniformList[k].size = CR_SHADER_DATA_TYPE_SIZE[uniformList[k].dataType];
		currentOffset += CR_SHADER_DATA_TYPE_SIZE[uniformList[k].dataType];
	}

	uniformBufferSize = currentOffset;

	return nextInstance->ConfigureUniforms(uniformList,uniformBufferSize);
}

CRESULT CR_PixelShader::GetConstant(const char *variableName,
									Ceng::ShaderConstant **handle)
{
	UINT32 k;

	for(k=0;k<uniformList.size();k++)
	{
		if (uniformList[k].name == variableName)
		{
			try
			{
				*handle = (Ceng::ShaderConstant*)
							new CR_ShaderConstHandle(k,&wrapper);
			}
			catch(std::bad_alloc&)
			{
				return CE_ERR_OUT_OF_MEMORY;
			}

			return CE_OK;
		}
	}

	if (k == uniformList.size())
	{
		return CE_ERR_FAIL;
	}

	return CE_OK;
}

const CRESULT CR_PixelShader::ReadUniform(const Ceng::UINT32 index,void *destBuffer)
{
	memcpy(destBuffer,nextInstance->uniformPtr[index],uniformList[index].size);

	return CE_OK;
}


const CRESULT CR_PixelShader::WriteUniform(const Ceng::UINT32 index,void *sourceBuffer)
{
	memcpy(nextInstance->uniformPtr[index],sourceBuffer,uniformList[index].size);

	return CE_OK;
}

CRESULT CR_PixelShader::SetFragmentFormat(CR_QuadFormat *newQuadFormat)
{
	nextInstance->quadFormat = newQuadFormat;

	return CE_OK;
}

const CRESULT CR_PixelShader::SetTextureUnits(std::vector<TextureUnit> &source)
{
	nextInstance->textureUnits = source;
	return CE_OK;
}

CRESULT CR_PixelShader::SetRenderTargets(Ceng::UINT32 amount,
										 std::shared_ptr<CR_NewTargetData> targets[],
										 std::shared_ptr<CR_NewTargetData> &depthStencil)
{

	UINT32 k;

	UINT32 tempFlags = 0;

	for(k=0;k<amount;k++)
	{
		nextInstance->targetHandles[k] = targets[k];
	}

	nextInstance->activeRenderTargets = amount;

	return CE_OK;	
}

const CRESULT CR_PixelShader::GetInstances(std::vector<std::shared_ptr<PixelShaderInstance>> &instances,
											const Ceng::UINT32 renderThreads)
{
	// Create an instance using *nextState*

	currentInstance = nextInstance;

	nextInstance = std::shared_ptr<PixelShaderInstance>(new PixelShaderInstance(*currentInstance));
	
	instances = std::vector<std::shared_ptr<PixelShaderInstance>>(renderThreads);

	for(Ceng::UINT32 k=0;k<instances.size();k++)
	{
		instances[k] = std::shared_ptr<PixelShaderInstance>(new PixelShaderInstance(*currentInstance));

		instances[k]->ConfigureInput(inputSemantics);

		instances[k]->ConfigureOutput(renderTargets);

		instances[k]->SetFragmentFormat(inputSemantics,renderTargets);

		instances[k]->SetRenderTargets(renderTargets);
	
		instances[k]->ConfigureUniforms(uniformList, uniformBufferSize);
		instances[k]->ConfigureLocals();		
	}

	return CE_OK;
}