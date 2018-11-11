/*****************************************************************************
*
* cpu-x86.h
*
* By Jari Korkala 10/2012
*
*****************************************************************************/

#ifndef _CONCEPT2_CPU_X86_H
#define _CONCEPT2_CPU_X86_H

#include <unordered_map>
#include <string>
#include <memory>

#include <ceng/datatypes/cpu-info.h>

namespace Ceng
{
	namespace E_x86_Feature
	{
		enum value
		{
			mmx = 1 ,
			
			sse = 2 ,
			sse_2 = 1 << 2 ,
			
			sse_3 = 1 << 3 ,
			ssse_3 = 1 << 4 ,

			sse_4_1 = 1 << 5 ,
			sse_4_2 = 1 << 6 ,

			avx = 1 << 7 ,
			avx_2 = 1 << 8 ,

			/**
			 * Half-precision floating point instructions.
			 */
			f16c = 1 << 9 ,

			rdrand = 1 << 10 ,

			aes = 1 << 11 ,

			/**
			 * Processor supports save / restore of YMM-registers.
			 */
			xsave = 1 << 12 ,

			/**
			 * Operating system supports save / restore of YMM-registers.
			 */
			os_xsave = 1 << 13 ,

			popcnt = 1 << 14 ,

			movbe = 1 << 15 ,

			fma = 1 << 16 ,

			carryless_mul = 1 << 17 ,

			bmi_1 = 1 << 18 ,
			bmi_2 = 1 << 19 ,

			rep_movsb = 1 << 20 ,

			hle = 1 << 21 ,

			fxsave = 1 << 22 ,

			lzcnt = 1 << 23 ,

			lahf_x64 = 1 << 24 ,

			prefetch_hw = 1 << 25 ,

			force_32b = (1 << 30) ,
		};
	};
	
	namespace E_x86_os_support
	{
		enum value
		{
			sse = 1 ,
			avx = 2 ,
			always = 4 ,

			force_32b = 1 << 30 ,
		};
	}

	struct X86_FeatureData
	{
		CHAR brandName[128];
		CHAR model[128];
		
		INT32 features; // Supported X86_PROCESSOR_FEATURES

		INT32 osFeatures;

		INT32 maxVectorSize;
		
		UINT32 cacheLine; // Size of cache line in bytes
	};

	
	
	class X86_Info : public CPU_Info
	{
	public:
		typedef std::unordered_map<std::string,E_CpuFeatureInfo::value> FeatureMap;
		typedef std::shared_ptr<std::vector<CoreData>> CoreList;

	protected:
		
		X86_FeatureData *featureData;
		
		FeatureMap *featureMap;

		
		CoreList physicalCores;
		CoreList logicalCores;

		X86_Info();

	public:

		X86_Info(X86_FeatureData *featureData,CoreList &physicalCores,
					CoreList &logicalCores,FeatureMap *featureMap);
		
		virtual ~X86_Info();

		virtual void Release() override;

		X86_Info* GetInstance();
				
		virtual const CRESULT Brand(String &output) override;
		virtual const CRESULT Model(String &output) override;
		
		virtual const CRESULT CheckFeature(const String &feature,E_CpuFeatureInfo::value *result) override;
		
		virtual const Ceng::UINT32 CacheLine() override;
		virtual const Ceng::UINT32 PhysicalCores() override;
		virtual const Ceng::UINT32 LogicalCores() override;

		virtual const CRESULT GetPhysicalCoreData(const Ceng::UINT32 core,CoreData *output) override;
		virtual const CRESULT GetLogicalCoreData(const Ceng::UINT32 core,CoreData *output) override;

		static const CRESULT GatherFeatures(X86_FeatureData &cpuData);
		
		static const CRESULT EnumerateCores(CoreList &physicalCores,CoreList &logicalCores);

		static const CRESULT BuildFeatureMap(X86_FeatureData &cpuData,FeatureMap &featureMap);

		static const CRESULT AddToMap(X86_FeatureData &cpuData,FeatureMap &featureMap,
								const char *name,E_x86_Feature::value feature,
								 E_x86_os_support::value osFeature);
	};

}


#endif