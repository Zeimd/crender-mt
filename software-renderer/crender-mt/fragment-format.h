/*****************************************************************************
*
* cr-fragment-format.h
*
* By Jari Korkala 4/2013
*
*****************************************************************************/

#ifndef _CENG_CR_FRAGMENT_FORMAT_H
#define _CENG_CR_FRAGMENT_FORMAT_H

#include <vector>

#include "crender-base.h"
#include "fragment-var.h"

#include "vshader-output.h"
#include "pshader-input.h"

#include "pshader-input-semantic.h"

namespace Ceng
{
	class CR_FragmentFormat
	{
	public:
		/**
		 * Size of the format in bytes.
		 */
		UINT32 size;

		/**
		 * Which semantics are present.
		 */
		UINT32 semanticFlags;

		std::vector<CR_FragmentVariable> variables;

		/**
		 * Set to 1 if POSITION is DOUBLE4, and
		 * DOUBLE data blocks come first. Otherwise
		 * POSITION is FLOAT4, and FLOAT blocks
		 * come first.
		 */
		UINT32 positionIsDouble;

		/**
		 * Offset of first FLOAT variable block.
		 */
		UINT32 floatStart;

		/**
		 * Number of 16-byte FLOAT variable blocks.
		 */
		UINT32 floatBlocks; 

		/**
		 * Offset of first DOUBLE type block.
		 */
		UINT32 doubleStart;

		/**
		 * Number of 16-byte DOUBLE blocks.
		 */
		UINT32 doubleBlocks;

		/**
		 * Offset of first 16-byte constant block.
		 */
		UINT32 constStart;

		/**
		 * Number of 16-byte constant blocks.
		 */
		UINT32 constBlocks;


	public:
		CR_FragmentFormat()
		{
		}

		~CR_FragmentFormat()
		{
			variables.clear();
			variables = std::vector<CR_FragmentVariable>();
		}

		CRESULT Configure(std::vector<Ceng::CR_vsOutputSemantic> &vshaderOut,
							std::vector<Ceng::CR_PixelShaderSemantic> &pshaderIn);

		CRESULT LinkSemantics(std::vector<CR_vsOutputSemantic> &linkedSemantics,
										 std::vector<CR_vsOutputSemantic> &vshaderOut,
										 std::vector<CR_PixelShaderSemantic> &pshaderIn);


		void Reset();

		UINT32 AssignFloats(UINT32 startOffset,std::vector<CR_vsOutputSemantic> &linkSemantics);
		UINT32 AssignDoubles(UINT32 startOffset,std::vector<CR_vsOutputSemantic> &linkSemantics);
		UINT32 AssignConstants(UINT32 startOffset,std::vector<CR_vsOutputSemantic> &linkSemantics);
	};

};

#endif