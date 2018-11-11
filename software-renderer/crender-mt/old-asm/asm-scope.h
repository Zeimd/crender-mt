/*****************************************************************************
*
* scope.h
*
* By Jari Korkala 10/2013
*
*****************************************************************************/

#ifndef CENG_SCOPE_H
#define CENG_SCOPE_H

#include <ceng/lib/liblog.h>

#include "asm-base.h"

#include "asm-variable.h"

namespace Ceng
{
	class Function;
	class Struct;

	class TypeDeclaration : public CodeItem
	{
	public:
		SharedPtr<Datatype> datatype;

		TypeDeclaration()
		{
		}

		TypeDeclaration(const UINT32 declareIndex,const SharedPtr<Datatype> &datatype)
		{
			this->declareIndex = declareIndex;
			this->datatype = datatype;
		}

		virtual ~TypeDeclaration()
		{
		}

		virtual String ThisToSource(String &indent,const UINT32 options) const override
		{
			String temp = "\n";

			temp += datatype->ToSource(indent,
										PRINT_OPTION::COMMAND_END |
										PRINT_OPTION::NEW_LINE);			

			return temp;
		}
	};

	class VariableDeclaration : public CodeItem
	{
	public:
		SharedPtr<Variable> variable;

		VariableDeclaration()
		{
		}

		VariableDeclaration(const UINT32 declareIndex,const SharedPtr<Variable> &variable)
		{
			this->declareIndex = declareIndex;
			this->variable = variable;
		}

		virtual ~VariableDeclaration()
		{
		}

		virtual String ThisToSource(String &indent,const UINT32 options) const override
		{
			String temp;

			temp += variable->ToSource(indent,
										PRINT_OPTION::COMMAND_END |
										PRINT_OPTION::NEW_LINE);			

			return temp;
		}

	};

		/*
	 * A self-contained block that can contain variables, datatypes and functions.
	 */
	class Scope : public CodeItem
	{
	public:

		Scope *parent;

		Ceng::UINT32 options;

		/*
		 * Datatypes only visible within this code block.
		 */
		std::vector<Ceng::SharedPtr<Datatype>> datatypes;

		/*
		 * Functions only visible within this code block.
		 */
		std::vector<Ceng::SharedPtr<Function>> functions;

		/*
		 * Variables declared within this code block.
		 */
		std::vector<Ceng::SharedPtr<Variable>> variables;

		/*
		 * All program components in the order in which they appear in source
		 * code.
		 */
		std::vector<Ceng::SharedPtr<CodeItem>> body;

		Scope();

		Scope(SharedPtr<ProgramType> &programType,Scope *parent);
	
		virtual ~Scope();

		virtual String ThisToSource(String &indent,const UINT32 options) const override;

		CRESULT DeclareVariable(UINT32 flags,const String &datatype,const String &name,
								const std::vector<UINT32> *arraySizes);

		CRESULT DeclareConstant(UINT32 flags,const String &datatype,const String &name,
								const std::vector<UINT32> *arraySizes,
								const InitializerList *initializer);

		/**
		 * Used to provide printing of Scope symbols from derived classes.
		 */
		String LocalPrint(String &source,const UINT32 options) const;
		
		/*
		 * Search given category of items for a matching name. If not found, search
		 * parent's item list.
		 */
		template<class T>
		SharedPtr<T> FindSymbol(const std::vector<Ceng::SharedPtr<T>> Scope::*list,const String &name)
		{
			UINT32 k;

			String text = "Scope.FindSymbol : ";
			text += name;

			Log::Print(text);			
	
			const std::vector<Ceng::SharedPtr<T>> &vector = this->*list;

			if (vector.size() > 0)
			{
				text = "last symbol = ";
				text += vector[vector.size()-1]->name;
				Log::Print(text);
			}
	
			// Search local namespace

			for(k=0;k<vector.size();k++)
			{
				text = "comparing to = ";
				text += vector[k]->name;
				Log::Print(text);

				if (vector[k]->name == name)
				{
					Log::Print("Scope.FindSymbol : match found!");
					return vector[k];
				}
			}

			Log::Print("Scope.FindSymbol : not local -> seek parent namespace");

			if (parent != NULL)
			{
				return parent->FindSymbol(list,name);
			}
			else
			{
				Log::Print("Scope.FindSymbol : No parent -> abort");
			}

			return NULL;
		}

		Struct* CreateStruct(const String &name);
		Function* CreateFunction(const String &name);
	
		CRESULT AddAlias(const String &typeName,const String &aliasName);

		Ceng::BOOL IsFreeSymbol(const String &name);

		Ceng::BOOL ConfigureFlags(Ceng::UINT32 *flags);
		
		CRESULT AddFunction(Function *function);
	
		CRESULT DeclareType(Datatype *datatype);
	};
}

#endif