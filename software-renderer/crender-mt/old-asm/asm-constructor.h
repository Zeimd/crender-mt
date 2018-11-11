/*****************************************************************************
*
* asm-constructor.h
*
* By Jari Korkala 10/2013
*
*****************************************************************************/

#ifndef CENG_ASM_INITIALIZE_H
#define CENG_ASM_INITIALIZE_H

#include "asm-base.h"

namespace Ceng
{
	class InitializerItem : public AssemblerObject
	{
	public:

	public:

		InitializerItem()
		{
		}

		virtual ~InitializerItem()
		{
		}

		virtual String ThisToSource(String &indent,const Ceng::UINT32 options) const override
		{
			return String("InitializerItem");
		}
	};

	template<class T,Ceng::UINT32 SIZE>
	class BasicInitializer : public InitializerItem
	{
	public:
		T value[SIZE];

	public:

		BasicInitializer()
		{
			UINT32 k;

			for(k=0;k<SIZE;k++)
			{
				value[k] = 0;
			}
		}

		BasicInitializer(const T input[SIZE])
		{
			UINT32 k;

			for(k=0;k<SIZE;k++)
			{
				value[k] = input[k];
			}
		}

		virtual ~BasicInitializer()
		{
		}

		virtual String ThisToSource(String &indent,const Ceng::UINT32 options) const override
		{
			String temp;

			if (SIZE > 1)
			{
				temp += "{ ";
			}

			UINT32 k;

			for(k=0; k < (SIZE-1); k++)
			{
				temp += value[k];
				temp += " , ";
			}

			temp += value[k];

			if (SIZE > 1)
			{
				temp += " }";
			}

			return temp;
		}
	};

	template<class T,Ceng::UINT32 SIZE>
	class SmartInitializer : public BasicInitializer<T,SIZE>
	{
	public:
		virtual ~SmartInitializer()
		{
		}

		/*
		virtual String ThisToSource(String &indent,const Ceng::UINT32 options) const override
		{
			return String("SmartInitializer");
		}
		*/
	};

	template<class T>
	class SmartInitializer<T,1>
	{
	public:
		SmartInitializer(const T x)
		{
			value[0] = x;
		}
	};

	template<class T>
	class SmartInitializer<T,2> 
	{
	public:
		SmartInitializer(const T x,const T y)
		{
			value[0] = x;
			value[1] = y;
		}
	};

	template<class T>
	class SmartInitializer<T,3>
	{
	public:
		SmartInitializer(const T x,const T y,const T z)
		{
			value[0] = x;
			value[1] = y;
			value[2] = z;
		}

		virtual ~SmartInitializer()
		{
		}

		/*
		virtual String ThisToSource(String &indent,const Ceng::UINT32 options) const override
		{
			return String("SmartInitializer : float4");
		}
		*/
	};

	
	template<class T>
	class SmartInitializer<T,4> : public BasicInitializer<T,4>
	{
	public:
		SmartInitializer(const T x,const T y,const T z,const T w)
		{
			value[0] = x;
			value[1] = y;
			value[2] = z;
			value[3] = w;
		}

		virtual ~SmartInitializer()
		{
		}

		/*
		virtual String ThisToSource(String &indent,const Ceng::UINT32 options) const override
		{
			return String("SmartInitializer : float4");
		}
		*/
	};

	typedef SmartInitializer<Ceng::FLOAT32,1> InitFloat;
	typedef SmartInitializer<Ceng::FLOAT32,2> InitFloat2;
	typedef SmartInitializer<Ceng::FLOAT32,3> InitFloat3;
	typedef SmartInitializer<Ceng::FLOAT32,4> InitFloat4;

	class InitializerList : public AssemblerObject
	{
	public:
		std::vector<SharedPtr<InitializerItem>> items;

	public:

		InitializerList()
		{
		}

		~InitializerList()
		{
		}

		SharedPtr<InitializerItem>& operator[] (const Ceng::UINT32 index)
		{
			return items[index];
		}

		CRESULT AddItem(InitializerItem *item)
		{
			items.push_back(item);

			return CE_OK;
		}

		virtual String ThisToSource(String &indent,const Ceng::UINT32 options) const override
		{
			return String("InitializerList");
		}

	};

}

#endif