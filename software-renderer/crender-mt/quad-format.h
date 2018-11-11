/*****************************************************************************
*
* cr-quad-format.h
*
* By Jari Korkala 4/2013
*
*****************************************************************************/

#ifndef _CENG_CR_QUAD_FORMAT_H
#define _CENG_CR_QUAD_FORMAT_H

#include <vector>

#include <ceng/enums/shader-datatype.h>
#include <ceng/enums/shader-semantic.h>

#include "crender-base.h"

namespace Ceng
{
	struct CR_TriangleData;

	class CR_FragmentFormat;
	class CR_FloatFragment;
	class CR_DoubleFragment;

	/**
	 * Quad format's fixed header.
	 *
	 * NOTE: Struct size must be divisible by 16
	 */
	struct CR_QuadHeader
	{
		/**
		 * Selects correct constants and variable
		 * deltas.
		 */
		CR_TriangleData *triangle;

		/**
		 * How many continuous horizontal quads.
		 */
		UINT32 chainLength;

		/**
		 * Screen location of the top-left corner.
		 * Must be 8-byte aligned.
		 */
		UINT32 screenX;
		UINT32 screenY;

		/**
		 * Coverage values for up to 32 quads.
		 */
		UINT32 coverageMask[4];
	};

	class CR_QuadVariable
	{
	public:
		Ceng::SHADER_SEMANTIC::value semantic;
		Ceng::SHADER_DATATYPE::value format;

		/**
		 * Position within vertex shader output struct.
		 */
		UINT32 fragmentOffset;

		/**
		 * Position within quad struct.
		 */
		UINT32 quadOffset;

		/**
		 * Address for pixel stepping values
		 */
		UINT32 gradientOffset;

		/**
		 * For pixel shader input register optimization.
		 */
		UINT32 options;

		CR_QuadVariable()
		{
			semantic = SHADER_SEMANTIC::FORMAT_END;
			format = SHADER_DATATYPE::UNKNOWN;

			fragmentOffset = 0;
			quadOffset = 0;
			gradientOffset = 0;
			options = 0;
		}
	};

	class CR_QuadFormat
	{
	public:

		/**
		 * Amount of data per quad in bytes.
		 */
		UINT32 quadSize;

		/**
		 * Amount of 16-byte FLOAT blocks.
		 */
		UINT32 floatBlocks;

		/**
		 * Offset of first FLOAT block within
		 * the format.
		 */
		POINTER floatStart;

		/**
		 * Amount of 16-byte DOUBLE blocks.
		 */
		UINT32 doubleBlocks;

		/**
		 * Offset of first DOUBLE block within
		 * the format.
		 */
		POINTER doubleStart;

		/**
		 * Size of the buffer for variable stepping in pixel shader.
		 */
		UINT32 gradientBufferSize;

		/**
		 * Amount of 4 x POINTER32 or
		 * 2 x POINTER64 render target blocks.
		 */
		UINT32 targetBlocks;

		/**
		 * The number of render targets assumed for
		 * size calculations.
		 */
		UINT32 renderTargets;

		/**
		 * Offset of first render target
		 * address block.
		 */
		POINTER targetStart;

		std::vector<CR_QuadVariable> variables;

	public:
		CR_QuadFormat();
		~CR_QuadFormat();

		CRESULT Configure(CR_FragmentFormat *fragmentFormat);

		CRESULT SetRenderTargets(UINT32 targetNum);

		/**
		 * Converts a vertex shader output fragment into a format optimal for
		 * pixel shader.
		 */
		CRESULT TranslateFragment(UINT8 *fragmentIn[3],
										 FLOAT32 *positionW[3],
										 FLOAT32 *vdy10,FLOAT32 *vdy21,
										 FLOAT32 *vdx10,FLOAT32 *vdx21,
										 FLOAT32 *gradientDiv,
										 FLOAT32 *initialStepX,FLOAT32 *initialStepY,
										 CR_FloatFragment *floatVariables,
										 CR_DoubleFragment *doubleVariables,
										 UINT8 *variableStep);

		
	};

};


#endif