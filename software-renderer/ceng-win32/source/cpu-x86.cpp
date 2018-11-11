/*****************************************************************************
*
* x86-info.cpp
*
* By Jari Korkala 10/2012
*
*****************************************************************************/

#include <memory>

#include <Windows.h>

#include "../include/ceng-win32.h"

#include "../include/cpu-x86.h"

using namespace Ceng;


static Ceng::BOOL firstRun = true;

extern "C" _declspec(dllexport) const Ceng::CRESULT Ceng_GetCpuInfo(Ceng::CPU_Info **out_info)
{
	static X86_FeatureData cpuData;

	static X86_Info::CoreList physicalCores;
	static X86_Info::CoreList logicalCores;
	static X86_Info::FeatureMap featureMap;

	if (firstRun)
	{
		CRESULT cresult = X86_Info::GatherFeatures(cpuData);

		if (cresult != CE_OK)
		{
			return cresult;
		}

		cresult = X86_Info::EnumerateCores(physicalCores,logicalCores);

		if (cresult != CE_OK)
		{
			return cresult;
		}

		cresult = X86_Info::BuildFeatureMap(cpuData,featureMap);

		if (cresult != CE_OK)
		{
			return cresult;
		}

		firstRun = false;
	}

	X86_Info *temp = new X86_Info(&cpuData,physicalCores,logicalCores,&featureMap);

	*out_info = (Ceng::CPU_Info*)temp;

	return CE_OK;
}

X86_Info::X86_Info(X86_FeatureData *featureData,CoreList &physicalCores,
				   CoreList &logicalCores,FeatureMap *featureMap)
				   : featureData(featureData),physicalCores(physicalCores),
				   logicalCores(logicalCores),featureMap(featureMap)
{
}

const CRESULT X86_Info::EnumerateCores(CoreList &physicalCores,CoreList &logicalCores)
{
	physicalCores = CoreList(new std::vector<CoreData>());
	logicalCores = CoreList(new std::vector<CoreData>());

	::BOOL coreResult;
	int coreDataSize=0;

	_SYSTEM_LOGICAL_PROCESSOR_INFORMATION *coreInfo = NULL;

	coreResult = GetLogicalProcessorInformation(coreInfo,(PDWORD)&coreDataSize);

	if (coreResult == FALSE)
	{
		::DWORD error = GetLastError();

		if (error != ERROR_INSUFFICIENT_BUFFER)
		{
			return CE_ERR_FAIL;
		}
	}

	coreInfo = (_SYSTEM_LOGICAL_PROCESSOR_INFORMATION*)calloc(1,coreDataSize);

	if (coreInfo == NULL)
	{
		return CE_ERR_OUT_OF_MEMORY;
	}

	coreResult = GetLogicalProcessorInformation(coreInfo,(PDWORD)&coreDataSize);

	if (coreResult == FALSE)
	{
		::DWORD error = GetLastError();

		return CE_ERR_FAIL;
	}

	int infoElements = coreDataSize / sizeof(_SYSTEM_LOGICAL_PROCESSOR_INFORMATION);

	ULONG_PTR initialCoreList[64];
	memset(initialCoreList,0,sizeof(ULONG_PTR)*64);
	
	INT32 coreCount=0;
	INT32 hyperThreadCount=0;
	
	ULONG_PTR coreMask;

	CoreData physicalCoreData;
	CoreData logicalCoreData;

	logicalCoreData.type = E_CpuCoreType::logical;
	physicalCoreData.type = E_CpuCoreType::physical;

	int k,j;
	for(k=0;k<infoElements;k++)
	{
		if (coreInfo[k].Relationship == RelationProcessorCore)
		{
			physicalCoreData.physicalCoreIndex = coreCount;

			physicalCoreData.hyperThreading = false;
			if (coreInfo[k].ProcessorCore.Flags == 1)
			{
				physicalCoreData.hyperThreading = true;
			}

			physicalCoreData.coreMask = coreInfo[k].ProcessorMask;

			physicalCoreData.coreCount = 0;
			
			logicalCoreData.physicalCoreIndex = coreCount;
			logicalCoreData.hyperThreading = physicalCoreData.hyperThreading;
			logicalCoreData.coreCount = 1;

			for(j=0;j<32;j++)
			{
				coreMask = coreInfo[k].ProcessorMask & (UINT32(1) << j);
				if (coreMask)
				{
					++physicalCoreData.coreCount;

					logicalCoreData.coreMask = coreMask;	
					logicalCores->push_back(logicalCoreData);
				}				
			}

			physicalCores->push_back(physicalCoreData);

			++coreCount;
		}
	}

	free(coreInfo);

	return CE_OK;
}

const CRESULT X86_Info::GatherFeatures(X86_FeatureData &cpuData)
{
	memset(&cpuData,0,sizeof(X86_FeatureData));

	INT32 cpuID[4];

	__cpuid(cpuID,0);

	int cpuidMaxValue = cpuID[0];

	if (cpuidMaxValue >= 1)
	{
		// Get first feature set

		__cpuid(cpuID,1);

		// ***** Determine features

		if (cpuID[3] & (1 << 23))
		{
			cpuData.features |= E_x86_Feature::mmx;
		}

		if (cpuID[3] & (1 << 25))
		{
			cpuData.features |= E_x86_Feature::sse;
		}

		if (cpuID[3] & (1 << 26))
		{
			cpuData.features |= E_x86_Feature::sse_2;
		}

		if (cpuID[2] & 1)
		{
			cpuData.features |= E_x86_Feature::sse_3;
		}

		if (cpuID[2] & (1 << 9)) 
		{
			cpuData.features |= E_x86_Feature::ssse_3;
		}

		if (cpuID[2] & (1 << 19)) 
		{
			cpuData.features |= E_x86_Feature::sse_4_1;
		}

		if (cpuID[2] & (1 << 20)) 
		{
			cpuData.features |= E_x86_Feature::sse_4_2;
		}

		if (cpuID[2] & (1 << 28)) 
		{
			cpuData.features |= E_x86_Feature::avx;
		}

		if (cpuID[2] & (1 << 22))
		{
			cpuData.features |= E_x86_Feature::movbe;
		}

		if (cpuID[2] & (1 << 23))
		{
			cpuData.features |= E_x86_Feature::popcnt;
		}

		if (cpuID[2] & (1 << 25))
		{
			cpuData.features |= E_x86_Feature::aes;
		}

		if (cpuID[2] & (1 << 26))
		{
			cpuData.features |= E_x86_Feature::xsave;
		}

		if (cpuID[2] & (1 << 27))
		{
			cpuData.features |= E_x86_Feature::os_xsave;
		}

		if (cpuID[2] & (1 << 29))
		{
			cpuData.features |= E_x86_Feature::f16c;
		}

		if (cpuID[2] & (1 << 30))
		{
			cpuData.features |= E_x86_Feature::rdrand;
		}

		if (cpuID[3] & (1 << 24))
		{
			cpuData.features |= E_x86_Feature::fxsave;
		}

		// ***** Cache line size
		cpuData.cacheLine = cpuID[1] & (255 << 8);
		cpuData.cacheLine = cpuData.cacheLine >> 5;
	}

	if (cpuidMaxValue >=7)
	{
		// Get second feature set

		__cpuidex(cpuID,7,0);

		int maxSubLeafs = cpuID[0];

		if (cpuID[1] & (1 << 3))
		{
			cpuData.features |= E_x86_Feature::bmi_1;
		}

		if (cpuID[1] & (1 << 8))
		{
			cpuData.features |= E_x86_Feature::bmi_2;
		}

		if (cpuID[1] & (1 << 4))
		{
			cpuData.features |= E_x86_Feature::hle;
		}

		if (cpuID[1] & (1 << 5))
		{
			cpuData.features |= E_x86_Feature::avx_2;
		}
	}

	if (cpuidMaxValue >= 0x80000000)
	{
		__cpuid(cpuID,0x8000000);

		int cpuidMaxExtendedValue = cpuID[0];

		if (cpuidMaxExtendedValue >= 0x80000001)
		{
			__cpuid(cpuID,0x80000001);

			if (cpuID[2] & 1)
			{
				cpuData.features |= E_x86_Feature::lahf_x64;
			}

			if (cpuID[2] & (1 << 5))
			{
				cpuData.features |= E_x86_Feature::lzcnt;
			}

			if (cpuID[2] & (1 << 8))
			{
				cpuData.features |= E_x86_Feature::prefetch_hw;
			}
		}
	}

	// Determine OS support for vector instructions

	if (cpuData.features & E_x86_Feature::os_xsave)
	{
		unsigned __int64 result = _xgetbv(0);

		if ( (result & (3 << 1)) == (3 << 1) )
		{
			cpuData.osFeatures |= E_x86_os_support::avx;

			cpuData.maxVectorSize = 256;
		}
		else if ( (result & 2) )
		{
			cpuData.maxVectorSize = 128;
		}

		cpuData.osFeatures |= E_x86_os_support::sse;
	}
	else if (cpuData.features & E_x86_Feature::fxsave)
	{
		// Check if OS has enabled fxsave

		cpuData.osFeatures |= E_x86_os_support::sse;
	}

	if (cpuData.cacheLine == 0) cpuData.cacheLine = 16;

	return CE_OK;
}

X86_Info::X86_Info()
{
}

X86_Info::~X86_Info()
{
}

void X86_Info::Release()
{
	delete this;
}

const CRESULT X86_Info::Brand(String &output)
{
	return CE_OK;
}

const CRESULT X86_Info::Model(String &output)
{
	return CE_OK;
}

const CRESULT X86_Info::CheckFeature(const String &feature,E_CpuFeatureInfo::value *result)
{
	if (result == nullptr)
	{
		return CE_ERR_NULL_PTR;
	}

	*result = E_CpuFeatureInfo::unknown_feature;

	auto iterator = featureMap->find(feature.ToCString());

	if (iterator == featureMap->end())
	{
		return CE_OK;
	}

	auto pair = *iterator;

	*result = pair.second;

	return CE_OK;
}

const Ceng::UINT32 X86_Info::CacheLine()
{
	return featureData->cacheLine;
}

const Ceng::UINT32 X86_Info::PhysicalCores()
{
	return physicalCores->size();
}

const Ceng::UINT32 X86_Info::LogicalCores()
{
	return logicalCores->size();
}

const CRESULT X86_Info::BuildFeatureMap(X86_FeatureData &cpuData,FeatureMap &featureMap)
{
	cpuData.osFeatures |= E_x86_os_support::always;

	AddToMap(cpuData,featureMap,"avx",E_x86_Feature::avx,E_x86_os_support::avx);
	AddToMap(cpuData,featureMap,"avx_2",E_x86_Feature::avx_2,E_x86_os_support::avx);
	
	AddToMap(cpuData,featureMap,"fma",E_x86_Feature::fma,E_x86_os_support::avx);

	AddToMap(cpuData,featureMap,"sse",E_x86_Feature::sse,E_x86_os_support::sse);
	AddToMap(cpuData,featureMap,"sse_2",E_x86_Feature::sse,E_x86_os_support::sse);
	AddToMap(cpuData,featureMap,"sse_3",E_x86_Feature::sse,E_x86_os_support::sse);
	AddToMap(cpuData,featureMap,"ssse_3",E_x86_Feature::sse,E_x86_os_support::sse);
	AddToMap(cpuData,featureMap,"sse_41",E_x86_Feature::sse,E_x86_os_support::sse);
	AddToMap(cpuData,featureMap,"sse_42",E_x86_Feature::sse,E_x86_os_support::sse);

	AddToMap(cpuData,featureMap,"bmi_1",E_x86_Feature::bmi_1,E_x86_os_support::always);
	AddToMap(cpuData,featureMap,"bmi_2",E_x86_Feature::bmi_2,E_x86_os_support::always);

	AddToMap(cpuData,featureMap,"mmx",E_x86_Feature::mmx,E_x86_os_support::always);

	AddToMap(cpuData,featureMap,"f16c",E_x86_Feature::f16c,E_x86_os_support::always);

	AddToMap(cpuData,featureMap,"popcnt",E_x86_Feature::popcnt,E_x86_os_support::always);
	AddToMap(cpuData,featureMap,"lzcnt",E_x86_Feature::lzcnt,E_x86_os_support::always);

	AddToMap(cpuData,featureMap,"rdrand",E_x86_Feature::rdrand,E_x86_os_support::always);
	AddToMap(cpuData,featureMap,"aes",E_x86_Feature::aes,E_x86_os_support::always);

	AddToMap(cpuData,featureMap,"xsave",E_x86_Feature::xsave,E_x86_os_support::always);
	AddToMap(cpuData,featureMap,"os_xsave",E_x86_Feature::os_xsave,E_x86_os_support::always);

	AddToMap(cpuData,featureMap,"movbe",E_x86_Feature::movbe,E_x86_os_support::always);

	AddToMap(cpuData,featureMap,"carryless_mul",E_x86_Feature::carryless_mul,E_x86_os_support::always);

	AddToMap(cpuData,featureMap,"fxsave",E_x86_Feature::fxsave,E_x86_os_support::always);

	AddToMap(cpuData,featureMap,"prefetch_hw",E_x86_Feature::prefetch_hw,E_x86_os_support::always);

	AddToMap(cpuData,featureMap,"lahf_x64",E_x86_Feature::lahf_x64,E_x86_os_support::always);

	return CE_OK;
}

const CRESULT X86_Info::AddToMap(X86_FeatureData &cpuData,FeatureMap &featureMap,
								 const char *name,E_x86_Feature::value feature,
								 E_x86_os_support::value osFeature)
{
	E_CpuFeatureInfo::value state = E_CpuFeatureInfo::not_supported;

	if ( (cpuData.features & feature) )
	{
		state = E_CpuFeatureInfo::no_os_support;
		if (cpuData.osFeatures & osFeature)
		{
			state = E_CpuFeatureInfo::ok;
		}	
	}

	featureMap.emplace(name,state);

	return CE_OK;
}

const CRESULT X86_Info::GetPhysicalCoreData(const Ceng::UINT32 core,CoreData *output)
{
	if (core > physicalCores->size())
	{
		return CE_ERR_INVALID_PARAM;
	}

	memcpy(output,&(*physicalCores)[core],sizeof(CoreData));

	return CE_OK;
}

const CRESULT X86_Info::GetLogicalCoreData(const Ceng::UINT32 core,CoreData *output)
{
	if (core > logicalCores->size())
	{
		return CE_ERR_INVALID_PARAM;
	}

	memcpy(output,&(*logicalCores)[core],sizeof(CoreData));

	return CE_OK;
}
