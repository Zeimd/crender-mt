/*****************************************************************************
*
* asm-scope.cpp
*
* By Jari Korkala 10/2013
*
*****************************************************************************/

#include "asm-scope.h"

#include "asm-struct.h"
#include "asm-function.h"

#include <ceng/lib/liblog.h>

using namespace Ceng;

Scope::Scope()
{
	parent = NULL;
	options = 0;
}

Scope::Scope(SharedPtr<ProgramType> &programType,Scope *parent)
{
	this->programType = programType;
	this->parent = parent;
}

Scope::~Scope()
{
}

String Scope::LocalPrint(String &indent,const Ceng::UINT32 options) const
{
	String temp;

	UINT32 k;

	for(k=0;k<body.size();k++)
	{
		temp += body[k]->ToSource(indent,PRINT_OPTION::NO_INDENT);
	}

	/*
	for(k=0;k<datatypes.size();k++)
	{
		temp += datatypes[k]->ToString(indentLevel,
										PRINT_OPTION::COMMAND_END |
										PRINT_OPTION::NEW_LINE);
	}

	if (datatypes.size() > 0 && variables.size() > 0 && functions.size() > 0)
	{
		temp += "\n";
	}	

	for(k=0;k<variables.size();k++)
	{
		temp += variables[k]->ToString(indentLevel,
										PRINT_OPTION::COMMAND_END |
										PRINT_OPTION::NEW_LINE);
	}

	if (variables.size() > 0 && functions.size() > 0)
	{
		temp += "\n";
	}

	for(k=0;k<functions.size();k++)
	{
		temp += functions[k]->ToString(indentLevel,PRINT_OPTION::NEW_LINE);
	}
	*/

	return temp;
}

String Scope::ThisToSource(String &indent,const Ceng::UINT32 options) const
{
	return LocalPrint(indent,options);
}

Ceng::BOOL Scope::IsFreeSymbol(const String &name)
{
	Datatype *type = FindSymbol<Datatype>(&Scope::datatypes,name);

	if (type != NULL)
	{
		return false;
	}

	Variable *variable = FindSymbol<Variable>(&Scope::variables,name);

	if (variable != NULL)
	{
		return false;
	}

	Function *function = FindSymbol<Function>(&Scope::functions,name);

	if (function != NULL)
	{
		return false;
	}

	return true;
}

CRESULT Scope::AddAlias(const String &typeName,const String &aliasName)
{
	
	// Check that the type exists
	SharedPtr<Datatype> type = FindSymbol<Datatype>(&Scope::datatypes,typeName);

	if (type == NULL)
	{
		return CE_ERR_FAIL;
	}

	// Check that the alias name is free

	if (IsFreeSymbol(aliasName))
	{
		datatypes.push_back(new Alias(aliasName,type));
		return CE_OK;
	}

	return CE_ERR_FAIL;
}

Ceng::BOOL Scope::ConfigureFlags(Ceng::UINT32 *flags)
{
	// Check that no internal flags are set

	if (*flags & VARIABLE_FLAG::NO_WRITE)
	{
		return false;
	}
	if (*flags & VARIABLE_FLAG::PER_PIXEL)
	{
		return false;
	}

	// Uniform variables are always non-writeable

	if (*flags & VARIABLE_FLAG::UNIFORM)
	{
		*flags |= VARIABLE_FLAG::NO_WRITE;
	}

	// Depending on shader type, mark either ATTRIBUTE or VARYING data
	// as non-writable

	switch(programType->type)
	{
	case PROGRAM_TYPE::VERTEX_SHADER:

		if (*flags & VARIABLE_FLAG::ATTRIBUTE)
		{
			*flags |= VARIABLE_FLAG::NO_WRITE;
		}
		break;
	case PROGRAM_TYPE::PIXEL_SHADER:
		
		if (*flags & VARIABLE_FLAG::VARYING)
		{
			*flags |= VARIABLE_FLAG::NO_WRITE;
		}
		break;
	}

	// Check:
	// NO_INTERPOLATION and NO_PERSPECTIVE are only allowed for VARYING data

	if (!(*flags & VARIABLE_FLAG::VARYING))
	{
		if (*flags & VARIABLE_FLAG::NO_INTERPOLATION)
		{
			return false;
		}
		if (*flags & VARIABLE_FLAG::NO_PERSPECTIVE)
		{
			return false;
		}
	}

	return true;
}

CRESULT Scope::DeclareVariable(Ceng::UINT32 flags,const String &typeName,const String &name,
							   const std::vector<Ceng::UINT32> *arraySizes)
{
	Ceng::BOOL flagTest;

	flagTest = ConfigureFlags(&flags);

	if (flagTest == false)
	{
		return CE_ERR_FAIL;
	}

	Log::Print("Scope.AddVariable : datatype check");
			
	SharedPtr<Datatype> typeCheck = FindSymbol<Datatype>(&Scope::datatypes,typeName);

	if (typeCheck == NULL)
	{
		Log::Print("Scope.AddVariable : Unknown datatype");
		return CE_ERR_FAIL;
	}			

	// Check that variable name is not reserved

	BOOL result = IsFreeSymbol(name);

	if (result == false)
	{
		return CE_ERR_FAIL;
	}

	if (arraySizes != NULL)
	{
		if (arraySizes->size() > programType->maxArrayDim)
		{
			return CE_ERR_NOT_SUPPORTED;
		}
	}

	SharedPtr<Variable> variable = new Variable(flags,typeCheck,name,arraySizes);

	if (variable != NULL)
	{
		variables.push_back(variable);

		UINT32 index = body.size();

		body.push_back(new VariableDeclaration(index,variable));

		return CE_OK;
	}

	return CE_ERR_FAIL;
}

CRESULT Scope::DeclareConstant(Ceng::UINT32 flags,const String &datatype,const String &name,
							   const std::vector<Ceng::UINT32> *arraySizes,
							   const InitializerList *initializer)
{
	

	return CE_OK;
}

Struct* Scope::CreateStruct(const String &name)
{
	Ceng::BOOL result = IsFreeSymbol(name);

	if (result == false)
	{
		return NULL;
	}

	return new Struct(programType,name,this);
}
		
CRESULT Scope::DeclareType(Datatype *datatype)
{
	SharedPtr<Datatype> pointer = datatype;

	datatypes.push_back(pointer);

	UINT32 index = body.size();

	body.push_back(new TypeDeclaration(index,pointer));

	return CE_OK;
}

Function* Scope::CreateFunction(const String &name)
{
	Ceng::BOOL result = IsFreeSymbol(name);

	if (result == false)
	{
		return NULL;
	}

	return new Function(programType,name,this);
}

CRESULT Scope::AddFunction(Function *function)
{
	functions.push_back(function);
	return CE_OK;
}