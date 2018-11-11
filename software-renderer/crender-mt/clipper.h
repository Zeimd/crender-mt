/*****************************************************************************
*
* clipper.h
*
* By Jari Korkala 10/2013
*
* class Clipper : Clipper moved from ConceptRenderer.
*
******************************************************************************/

#ifndef CENG_CLIPPER_H
#define CENG_CLIPPER_H

#include <memory>

#include "crender-base.h"

#include <ceng/datatypes/ring-buffer.h>
#include "clipper-batch.h"
#include "triangle-batch.h"

#include "locking-stage.h"

namespace Ceng
{
	class CR_ShaderLink;

	class ClipperState;

	class CR_Clipper
	{
	protected:

		/*
		FLOAT32 xMinClipModifier;
		FLOAT32 xMaxClipModifier;

		FLOAT32 yMinClipModifier;
		FLOAT32 yMaxClipModifier;
		*/

	public:
		CR_Clipper();
		virtual ~CR_Clipper();

		static const CRESULT CreateClipperState(const FLOAT32 top,const FLOAT32 left,
									 const FLOAT32 bottom,const FLOAT32 right,
									 const FLOAT32 minTop,const FLOAT32 minLeft,
									 const FLOAT32 maxBottom,const FLOAT32 maxTop,
									 ClipperState &out_state);



		const CRESULT HomogeneousClipTriangle(CR_PrimitiveData &primitive,
											Ceng::UINT32 *cacheIndex,
											std::shared_ptr<TriangleBatch> &outputBatch);

		const CRESULT ClipPrimitives(std::shared_ptr<ClipperBatch> &batch,
										LockingStage &outputStage);

	protected:
		const UINT32 ClipEdgeAgainstMinimum(Ceng::UINT8 *startVertex,
										Ceng::UINT8 *endVertex,
										Ceng::UINT8 *generatedVertex,
										Ceng::UINT8 **indexOut,
										Ceng::FLOAT32 startCoord,
										Ceng::FLOAT32 startW,
										Ceng::FLOAT32 endCoord,
										Ceng::FLOAT32 endW,
										CR_ShaderLink *shaderLink);

		const UINT32 ClipEdgeAgainstMaximum(Ceng::UINT8 *startVertex,
										Ceng::UINT8 *endVertex,
										Ceng::UINT8 *generatedVertex,
										Ceng::UINT8 **indexOut,
										Ceng::FLOAT32 startCoord,
										Ceng::FLOAT32 startW,
										Ceng::FLOAT32 endCoord,
										Ceng::FLOAT32 endW,
										CR_ShaderLink *shaderLink);
	};
};

#endif
