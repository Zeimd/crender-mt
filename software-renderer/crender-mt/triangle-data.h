/*****************************************************************************
*
* cr-triangle-data.h
*
* By Jari Korkala 4/2013
*
* File created as part of project refactoring.
*
*****************************************************************************/

#ifndef _CENG_CR_TRIANGLE_DATA_H
#define _CENG_CR_TRIANGLE_DATA_H

#include "crender-base.h"

#include "fragment-var.h"

namespace Ceng
{
	struct CR_QuadEdges
	{
		INT32 pixel[4];
	};

	struct CR_TileValues
	{
		/*
		 * If [0],[1],[2] are all negative, the tile is completely
		 * inside of the triangle.
		 *
		 * NOTE: Last element for padding.
		 */
		INT32 trivialAccept[4];

		/*
		 * If any of [0],[1],[2] is non-zero, the tile is completely
		 * outside of the triangle.
		 *
		 * NOTE: Last element for padding.
		 */
		INT32 trivialReject[4];
	};

	struct CR_TileStepData
	{
		/*
		 * Steps trivialAccept and trivialReject to the next tile on
		 * the right.
		 *
		 * NOTE: Last element for padding.
		 */
		INT32 stepX[4];

		/*
		 * Steps trivialAccept and trivialReject from the end of current
		 * row to the beginning of next row.
		 *
		 * NOTE: Last element for padding.
		 */
		INT32 toNextRow[4];
	};

	struct Base_TriangleData
	{
		VectorF4 packedZ;
		VectorF4 packedZ_dx;
		VectorF4 packedZ_dy;
		VectorF4 packedZ_quad_dx;
		VectorF4 packedZ_quad_dy;

		VectorF4 packedW;
		VectorF4 packedW_dx;
		VectorF4 packedW_dy;
		VectorF4 packedW_quad_dx;
		VectorF4 packedW_quad_dy;	

		/**
		 * Steps edge equations of a quad 2 pixels right.
		 */
		INT32 quadEdgeStepX[4]; 

		/**
		 * Steps edge equations of a quad 6 pixels left, 2 down.
		 */
		INT32 quadEdgeStepY[4];

		/*
		 * Steps back to the beginning of current tile. Usually
		 * -TILE_SIZE*pixelStepY.
		 */
		INT32 quadStepTileBack[4];

		CR_TileStepData tile;

		INT32 minPixelStepX[4];
		INT32 minPixelStepY[4];
	
		FLOAT32 gradientDiv;

		CR_FragmentData fragment;
	};

	struct CR_TriangleData : public Base_TriangleData
	{
		UINT8 padding[ (( sizeof(Base_TriangleData) + 15) & ~15) - sizeof(Base_TriangleData)];
	};
};

#endif