/*****************************************************************************
*
* pipeline-stage.cpp
*
* Created By Jari Korkala 2/2015
*
*****************************************************************************/

#include "pipeline-stage.h"

using namespace Ceng;

PipelineStage::PipelineStage() 
{
	remainingTasks.store(0);
	activeThreads.store(0);
}

PipelineStage::~PipelineStage()
{
}

PipelineStage& PipelineStage::operator = (const PipelineStage &source)
{
	CopyAtomics(source);

	return *this;
}

PipelineStage& PipelineStage::operator = (PipelineStage &&source)
{
	CopyAtomics(source);

	return *this;
}

void PipelineStage::CopyAtomics(const PipelineStage &source)
{
	remainingTasks.store(source.remainingTasks);
	activeThreads.store(source.activeThreads);
}

const Ceng::UINT32 PipelineStage::RemainingTasks() const
{
	return remainingTasks.load();
}

const Ceng::UINT32 PipelineStage::ActiveThreads() const
{
	return activeThreads.load();
}