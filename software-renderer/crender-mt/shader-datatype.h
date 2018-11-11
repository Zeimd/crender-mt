/*****************************************************************************
*
* shader-float4.h
*
* By Jari Korkala 4/2013
*
* Vector[4] that avoids the creation of temporary copies.
*
*****************************************************************************/

#ifndef CENG_SHADER_DATATYPE_H
#define CENG_SHADER_DATATYPE_H

#include <vector>

#include "crender-base.h"
#include "pshader-input.h"

#include "texture-unit.h"

#include "cr-shader-view.h"

namespace Ceng
{
	namespace Shader
	{
		void PS_MOV_FLOAT4_FLOAT4_X86_SSE(void *dest,void *source);

		void PS_MUL_FLOAT4_LOCAL_FLOAT_X86_SSE(void *dest,void *source);

		void PS_MUL_FLOAT4_FLOAT4_X86_SSE(void *dest,void *source);
		void PS_MUL_FLOAT4_FLOAT_X86_SSE(void *dest,void *source);

		class SampleTexture2D;

		class Float
		{
		public:
			void *dataAddress;

		public:
			Float();
			Float(const Float &other);

			Float& operator = (const Float &other);
			Float& operator = (const CR_PixelShaderInput &source);
		};

		class Float2
		{
		public:
			void *dataAddress;

		public:
			Float2();
			Float2(const Float2 &other);

			Float2& operator = (const CR_PixelShaderInput &source);
		};

		class Float4
		{
		public:
			// Pointer to actual data. Must be 16-byte aligned.
			void *dataAddress;

		public:
			Float4();
			Float4(const Float4 &other);

			Float4& operator = (const Float4 &other);

			Float4& operator = (const CR_PixelShaderInput &source);

			Float4& operator = (const SampleTexture2D &source);

			Float4& operator += (const Float4 &other);
			Float4& operator -= (const Float4 &other);

			/**
			 * Multiply by varying scalar.
			 */
			Float4& operator *= (const Shader::Float &other);

			Float4& operator *= (const Float4 &other);
			
			/**
			 * Multiply by local scalar.
			 */
			Float4& operator *= (const FLOAT32 &other);

			Float4& operator /= (const Float4 &other);

		public:

			static void (*call_mov_float4)(void *dest,void *source);

			static void (*call_mul_varying_float4)(void *dest,void *source);
			static void (*call_mul_local_float)(void *dest,void *source);
			
			static void (*call_mul_varying_float)(void *dest,void *source);

			
		};

		class SampleTexture2D
		{
		public:

			TextureUnit *unit;
			Shader::Float2 *uv;

			void *dataAddress;

			//std::vector<TextureUnit> *textureUnits;

		public:

			SampleTexture2D() : unit(nullptr),uv(nullptr)
			{
			}

			//SampleTexture2D& operator()(const Ceng::UINT32 index, Shader::Float2 &uv);
			SampleTexture2D& operator()(TextureUnit &texture, Shader::Float2 &uv);

			void SampleToFloat4(void *destBuffer) const;
		};

		//************************************************************
		// Shader::Float methods

		inline Float::Float()
		{
			dataAddress = nullptr;
		}

		inline Float& Float::operator= (const Float &other)
		{
			FLOAT32 *destPtr = (FLOAT32*)dataAddress;
			FLOAT32 *sourcePtr = (FLOAT32*)other.dataAddress;

			*destPtr = *sourcePtr;
		}

		inline Float& Float::operator = (const CR_PixelShaderInput &source)
		{
			source.MoveToFloat(dataAddress);
			return *this;
		}

		//************************************************************
		// Shader::Float2 methods

		inline Float2::Float2()
		{
			dataAddress = nullptr;
		}

		inline Float2::Float2(const Float2 &other)
		{
		}

		inline Float2& Float2::operator = (const CR_PixelShaderInput &source)
		{
			source.MoveToFloat2(dataAddress);
			return *this;
		}

		//************************************************************
		// Shader::Float4 methods


		inline Float4::Float4()
		{
			dataAddress = nullptr;
		}

		inline Float4& Float4::operator = (const Float4 &other)
		{
			return *this;
		}

		inline Float4& Float4::operator = (const CR_PixelShaderInput &source)
		{
			source.MoveToFloat4(dataAddress);
			return *this;
		}

		inline Float4& Float4::operator = (const SampleTexture2D &source)
		{
			(*call_mov_float4)((void*)dataAddress, (void*)source.dataAddress);
			return *this;
		}

		inline Float4& Float4::operator += (const Float4 &other)
		{
			return *this;
		}

		inline Float4& Float4::operator -= (const Float4 &other)
		{
			return *this;
		}

		inline Float4& Float4::operator *= (const Float &other)
		{
			(*call_mul_varying_float)((void*)dataAddress,(void*)other.dataAddress);
			return *this;
		}

		inline Float4& Float4::operator *= (const Float4 &other)
		{
			(*call_mul_varying_float4)((void*)dataAddress,(void*)other.dataAddress);
			return *this;
		}

		inline Float4& Float4::operator *= (const FLOAT32 &other)
		{
			(*call_mul_local_float)((void*)dataAddress,(void*)&other);
			return *this;
		}

		inline Float4& Float4::operator /= (const Float4 &other)
		{
			return *this;
		}

		//**********************************************************
		// SampleTexture2D methods

		inline SampleTexture2D& SampleTexture2D::operator()(TextureUnit &texture, Shader::Float2 &uv)
		{
			unit = &texture;
			this->uv = &uv;

			//texture.view->Sample2D(uv, texture.sampler, (Ceng::FLOAT32*)dataAddress);

			return *this;
		}

		inline void SampleTexture2D::SampleToFloat4(void *destBuffer) const
		{
			unit->view->Sample2D(*uv, unit->sampler, (Ceng::FLOAT32*)destBuffer);
		}

	}; // namespace Shader

}; // namespace Ceng

#endif // Include guard