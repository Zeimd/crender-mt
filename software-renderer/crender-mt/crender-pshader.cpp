/*****************************************************************************
*
* setup-pshader.cpp
*
* By Jari Korkala 8/2014
*
*****************************************************************************/

#include "crender.h"

#include "cr-pixel-shader.h"

using namespace Ceng;

CRESULT ConceptRenderer::CreatePixelShader(Ceng::PixelShader **shaderPtr)
{
	Ceng::CR_PixelShader *pixelShader;

	*shaderPtr = nullptr;

	try
	{
		pixelShader = new CR_PixelShader();
	}
	catch(std::bad_alloc&)
	{
		return CE_ERR_OUT_OF_MEMORY;
	}

	Ceng::CR_PixelShaderSemantic psTempRegister;

	/*
	psTempRegister.semantic = Ceng::SHADER_SEMANTIC::NORMAL;
	pixelShader->inputSemantics.push_back(psTempRegister);
	*/

	/*
	psTempRegister.semantic = Ceng::SHADER_SEMANTIC::COLOR_0;
	pixelShader->inputSemantics.push_back(psTempRegister);
	*/
	psTempRegister.semantic = Ceng::SHADER_SEMANTIC::TEXCOORD_0;
	pixelShader->inputSemantics.push_back(psTempRegister);
	
	psTempRegister.semantic = Ceng::SHADER_SEMANTIC::TEXCOORD_1;
	pixelShader->inputSemantics.push_back(psTempRegister);
	/*
	psTempRegister.semantic = Ceng::SHADER_SEMANTIC::TEXCOORD_2;
	pixelShader->inputSemantics.push_back(psTempRegister);
	*/
	Ceng::CR_ShaderConstant tempConst;

	tempConst.dataType = SHADER_DATATYPE::UINT;
	tempConst.name = "diffuseTexture";

	pixelShader->uniformList.push_back(tempConst);

	pixelShader->ConfigureConstants();

	Ceng::CR_PixelShaderTarget psTempTarget;

	psTempTarget.target = Ceng::CR_SHADER_TARGET0;
	pixelShader->renderTargets.push_back(psTempTarget);

	*shaderPtr = (Ceng::PixelShader*)pixelShader;

	return CE_OK;
}
