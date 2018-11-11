/*****************************************************************************
*
* quad-format.cpp
*
* By Jari Korkala 2/2012
*
* ------------------------------------
*
* Jari Korkala 8/2014
*
* - Renamed to quad-format.cpp
*
*****************************************************************************/

#include "quad-format.h"

#include "triangle-data.h"

#include "fragment-format.h"

#include <ceng/lib/liblog.h>

using namespace Ceng;

CR_QuadFormat::CR_QuadFormat()
{
	quadSize = 0;

	renderTargets = 0;

	floatBlocks = 0;
	floatStart = 0;

	doubleBlocks = 0;
	doubleStart = 0;

	targetBlocks = 0;
	targetStart = 0;
}

CR_QuadFormat::~CR_QuadFormat()
{
	variables.clear();
	variables = std::vector<CR_QuadVariable>();
}

CRESULT CR_QuadFormat::TranslateFragment(Ceng::UINT8 *fragmentIn[3],
										 FLOAT32 *positionW[3],
										 FLOAT32 *vdy10,FLOAT32 *vdy21,
										 FLOAT32 *vdx10,FLOAT32 *vdx21,
										 FLOAT32 *gradientDiv,
										 FLOAT32 *initialStepX,FLOAT32 *initialStepY,
										 CR_FloatFragment *floatVariables,
										 CR_DoubleFragment *doubleVariables,
										 Ceng::UINT8 *variableStep)
{
	UINT32 k;

	UINT32 frgIndex = 0;

	for(k=0;k<variables.size();k++)
	{
		UINT32 fragmentOffset = variables[k].fragmentOffset;

		switch(variables[k].format)
		{
		case Ceng::SHADER_DATATYPE::FLOAT:
			
			// NOTE: Scoping operator avoids variable name clashes between
			//       switch statements

			{
				FLOAT32 *param2 = (FLOAT32*)&fragmentIn[2][fragmentOffset];
				FLOAT32 *param1 = (FLOAT32*)&fragmentIn[1][fragmentOffset];
				FLOAT32 *param0 = (FLOAT32*)&fragmentIn[0][fragmentOffset];

				FLOAT32 paramSub21 = (param2[0]) * (*positionW[2]) - (param1[0]) * (*positionW[1]);
				FLOAT32 paramSub10 = (param1[0]) * (*positionW[1]) - (param0[0]) * (*positionW[0]);

				FLOAT32 numerator = paramSub21 * (*vdy10) - paramSub10 * (*vdy21);
				FLOAT32 step_dx = numerator * -(*gradientDiv);

				numerator = paramSub21 * (*vdx10) - paramSub10 * (*vdx21);
				FLOAT32 step_dy = numerator * (*gradientDiv);

				floatVariables[frgIndex].startValue.x = (param0[0]) * (*positionW[0]) + step_dx * (*initialStepX) +
													step_dy * (*initialStepY);

				floatVariables[frgIndex].startValue.y = floatVariables[frgIndex].startValue.x + step_dx;
				floatVariables[frgIndex].startValue.z = floatVariables[frgIndex].startValue.x + step_dy;
				floatVariables[frgIndex].startValue.w = floatVariables[frgIndex].startValue.x + step_dx + step_dy;

				floatVariables[frgIndex].step_dx.x = step_dx;
				floatVariables[frgIndex].step_dx.y = step_dx;
				floatVariables[frgIndex].step_dx.z = step_dx;
				floatVariables[frgIndex].step_dx.w = step_dx;

				floatVariables[frgIndex].step_dy.x = step_dy;
				floatVariables[frgIndex].step_dy.y = step_dy;
				floatVariables[frgIndex].step_dy.z = step_dy;
				floatVariables[frgIndex].step_dy.w = step_dy;

				VectorF4 *step = (VectorF4*)&variableStep[variables[k].gradientOffset];

				// + 2*dx
				step[0].x = step_dx * FLOAT32(2.0f);
				step[0].y = step[0].x;
				step[0].z = step[0].x;
				step[0].w = step[0].x;

				frgIndex++;
			}			

			break;
		case Ceng::SHADER_DATATYPE::FLOAT2:

			{
				VectorF2 *param2 = (VectorF2*)&fragmentIn[2][fragmentOffset];
				VectorF2 *param1 = (VectorF2*)&fragmentIn[1][fragmentOffset];
				VectorF2 *param0 = (VectorF2*)&fragmentIn[0][fragmentOffset];

				VectorF2 paramSub21 = (param2[0]) * (*positionW[2]) - (param1[0]) * (*positionW[1]);
				VectorF2 paramSub10 = (param1[0]) * (*positionW[1]) - (param0[0]) * (*positionW[0]);

				VectorF2 numerator = paramSub21 * (*vdy10) - paramSub10 * (*vdy21);
				VectorF2 step_dx = numerator * -(*gradientDiv);

				numerator = paramSub21 * (*vdx10) - paramSub10 * (*vdx21);
				VectorF2 step_dy = numerator * (*gradientDiv);

				// {y1,y0,x1,x0} variant
				floatVariables[frgIndex].startValue.x = (param0[0].x) * (*positionW[0]) + step_dx.x * (*initialStepX) +
					step_dy.x * (*initialStepY);

				floatVariables[frgIndex].startValue.y = floatVariables[frgIndex].startValue.x + step_dx.x;

				floatVariables[frgIndex].startValue.z = (param0[0].y) * (*positionW[0]) + step_dx.y * (*initialStepX) +
					step_dy.y * (*initialStepY);

				floatVariables[frgIndex].startValue.w = floatVariables[frgIndex].startValue.z + step_dx.y;

				floatVariables[frgIndex].step_dx.x = step_dx.x;
				floatVariables[frgIndex].step_dx.y = step_dx.x;
				floatVariables[frgIndex].step_dx.z = step_dx.y;
				floatVariables[frgIndex].step_dx.w = step_dx.y;

				floatVariables[frgIndex].step_dy.x = step_dy.x;
				floatVariables[frgIndex].step_dy.y = step_dy.x;
				floatVariables[frgIndex].step_dy.z = step_dy.y;
				floatVariables[frgIndex].step_dy.w = step_dy.y;

				VectorF4 *step = (VectorF4*)&variableStep[variables[k].gradientOffset];

				// + dy
				step[0].x = step_dy.x;
				step[0].y = step_dy.x;
				step[0].z = step_dy.y;
				step[0].w = step_dy.y;

				// - dy + 2*dx

				step[1].x = step_dx.x * FLOAT32(2.0f) - step_dy.x;
				step[1].y = step[1].x;

				step[1].z = step_dx.y * FLOAT32(2.0f) - step_dy.y; 
				step[1].w = step[1].z;

				/*
				// {y1,x1,y0,x0} variant
				floatVariables[frgIndex].startValue.x = (param0[0].x) * (*positionW[0]) + step_dx.x * (*initialStepX) +
													step_dy.x * (*initialStepY);

				floatVariables[frgIndex].startValue.y = (param0[0].y) + (*positionW[0]) + step_dx.y * (*initialStepX) +
													step_dy.y * (*initialStepY);

				floatVariables[frgIndex].startValue.z = floatVariables[frgIndex].startValue.x + step_dx.x;
				floatVariables[frgIndex].startValue.w = floatVariables[frgIndex].startValue.y + step_dx.y;

				floatVariables[frgIndex].step_dx.x = step_dx.x;
				floatVariables[frgIndex].step_dx.y = step_dx.y;
				floatVariables[frgIndex].step_dx.z = step_dx.x;
				floatVariables[frgIndex].step_dx.w = step_dx.y;

				floatVariables[frgIndex].step_dy.x = step_dy.x;
				floatVariables[frgIndex].step_dy.y = step_dy.y;
				floatVariables[frgIndex].step_dy.z = step_dy.x;
				floatVariables[frgIndex].step_dy.w = step_dy.y;

				VectorF4 *step = (VectorF4*)&variableStep[variables[k].gradientOffset];

				// + dy
				step[0].x = step_dy.x;
				step[0].y = step_dy.y;
				step[0].z = step_dy.x;
				step[0].w = step_dy.y;

				// - dy + 2*dx

				step[1].x = step_dx.x * FLOAT32(2.0f) - step_dy.x;
				step[1].y = step_dx.y * FLOAT32(2.0f) - step_dy.y;
				
				step[1].z = step[1].x;
				step[1].w = step[1].y;
				*/

				frgIndex++;
			}			

			break;
		case Ceng::SHADER_DATATYPE::FLOAT3:
			break;
		case Ceng::SHADER_DATATYPE::FLOAT4:

			// Horizontal mode
			
			{
				VectorF4 *param2 = (VectorF4*)&fragmentIn[2][fragmentOffset];
				VectorF4 *param1 = (VectorF4*)&fragmentIn[1][fragmentOffset];
				VectorF4 *param0 = (VectorF4*)&fragmentIn[0][fragmentOffset];

				// Affine
				/*
				VectorF4 paramSub21 = (*param2) - (*param1);
				VectorF4 paramSub10 = (*param1) - (*param0);
				*/
				

				// Perspective correct
				VectorF4 paramSub21 = (*param2) * (*positionW[2]) - (*param1) * (*positionW[1]);
				VectorF4 paramSub10 = (*param1) * (*positionW[1]) - (*param0) * (*positionW[0]);
				
				VectorF4 numerator = paramSub21 * (*vdy10) - paramSub10 * (*vdy21);
			
				floatVariables[frgIndex].step_dx = numerator * -(*gradientDiv);

				numerator = paramSub21 * (*vdx10) - paramSub10 * (*vdx21);
				floatVariables[frgIndex].step_dy = numerator * (*gradientDiv);

				// Affine
				/*
				floatVariables[frgIndex].startValue = (*param0) +
					floatVariables[frgIndex].step_dx * (*initialStepX) +
					floatVariables[frgIndex].step_dy * (*initialStepY);
					*/
					
				
				// Perspective correct
				
				floatVariables[frgIndex].startValue = (*param0) * (*positionW[0]) +
					floatVariables[frgIndex].step_dx * (*initialStepX) +
					floatVariables[frgIndex].step_dy * (*initialStepY);
					
					

				VectorF4 *step = (VectorF4*)&variableStep[variables[k].gradientOffset];

				step[0] = floatVariables[k].step_dx;
				step[1] = floatVariables[k].step_dy - floatVariables[k].step_dx;

				frgIndex++;
			}
						

			break;
		}
	}
	
	
	return CE_OK;
}

CRESULT CR_QuadFormat::Configure(CR_FragmentFormat *fragmentFormat)
{
	//Log::Print("CR_QuadFormat: Configure()\n");

	UINT32 quadOffset = 0;
	UINT32 gradientOffset = 0;

	UINT32 k;

	// TODO: Insert pixel shader's POSITION-semantic if used

	// TODO: Sort variables by pixel shader usage order

	floatBlocks = 0;
	doubleBlocks = 0;

	variables.clear();

	CR_QuadVariable temp;

	for(k=0;k<fragmentFormat->variables.size();k++)
	{
		if (fragmentFormat->variables[k].semantic == Ceng::SHADER_SEMANTIC::POSITION)
		{
			// Vertex shader's POSITION-semantic doesn't go directly to pixel shader,
			// so skip it
			continue;
		}

		temp.semantic = fragmentFormat->variables[k].semantic;
		temp.format = fragmentFormat->variables[k].format;
		temp.options = fragmentFormat->variables[k].options;

		temp.fragmentOffset = fragmentFormat->variables[k].offset;

		temp.quadOffset = quadOffset; //HEADER_SIZE + quadOffset;
		temp.gradientOffset = gradientOffset;

		variables.push_back(temp);

		switch(fragmentFormat->variables[k].format)
		{
		case Ceng::SHADER_DATATYPE::FLOAT:

			quadOffset += 16; // Allocate space for 4 x FLOAT

			gradientOffset += 16; // +2*dx


			floatBlocks++;
			break;

		case Ceng::SHADER_DATATYPE::FLOAT2:

			quadOffset += 16; // Allocate space for 2 x FLOAT2

			gradientOffset += 16; // +dy
			gradientOffset += 16; // +2*dx-dy

			floatBlocks++;
			break;

		case Ceng::SHADER_DATATYPE::FLOAT3:
		case Ceng::SHADER_DATATYPE::FLOAT4:

			// Vertical mode
			
			/*
			quadOffset += 64;
			gradientOffset += 64;
			//gradientOffset += 16; // Packed gradient
			floatBlocks += 4;
			*/
			
			
			
			// Horizontal mode

			
			quadOffset += 16; // Allocate space for FLOAT4

			gradientOffset += 16; // +dx
			gradientOffset += 16; // -dx+dy

			floatBlocks++;
			
			
			break;

		case Ceng::SHADER_DATATYPE::DOUBLE:

			quadOffset += 16; // Allocate space for 2 x DOUBLE

			gradientOffset += 16; // +dy
			gradientOffset += 16; // 2*dx - dy

			doubleBlocks++;
			break;

		case Ceng::SHADER_DATATYPE::DOUBLE2:

			quadOffset += 16; // Allocate space for 1 x DOUBLE2
			
			gradientOffset += 16; // + dx
			gradientOffset += 16; // -dx+dy

			doubleBlocks++;

			break;
		case Ceng::SHADER_DATATYPE::DOUBLE3:
		case Ceng::SHADER_DATATYPE::DOUBLE4:

			quadOffset += 32; // Allocate space for DOUBLE4

			gradientOffset += 32; // +dx
			gradientOffset += 32; // -dx+dy

			doubleBlocks += 2;

			break;
		}
	}

	gradientBufferSize = gradientOffset;

	if (fragmentFormat->positionIsDouble == 0)
	{
		floatStart = 0;
		
		doubleStart = floatStart + 16 * floatBlocks;
		targetStart = doubleStart + 16 * doubleBlocks;
	}
	else
	{
		doubleStart = 0;

		floatStart = doubleStart + 16 * doubleBlocks;
		targetStart = floatStart + 16 * floatBlocks;
	}

	Log::Print("\nQuadFormat.Configure:");

	String text;

	text = "quad size = ";
	text += quadSize;
	text += "\n";

	text += "float blocks = ";
	text += floatBlocks;
	text += "\n";

	text += "double blocks = ";
	text += doubleBlocks;
	text += "\n";

	text += "rendertargets = ";
	text += renderTargets;
	text += " , target blocks = ";
	text += targetBlocks;
	text += "\n";

	text += "float start offset = ";
	text += floatStart;
	text += "\n";

	text += "double start offset = ";
	text += doubleStart;
	text += "\n";

	text += "target start offset = ";
	text += targetStart;
	text += "\n";

	Log::Print(text);

	SetRenderTargets(renderTargets);

	return CE_OK;
}

CRESULT CR_QuadFormat::SetRenderTargets(Ceng::UINT32 targetNum)
{
	// Add space for render targets

	quadSize = targetStart + sizeof(POINTER)*targetNum;

	UINT32 remainder = quadSize % 16;

	// Pad quad size to a multiply of 16 bytes
	if (remainder)
	{
		quadSize = (quadSize & ~15) + 16;
	}

	renderTargets = targetNum;

	return CE_OK;
}