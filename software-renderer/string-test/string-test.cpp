// string-test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include <ceng/datatypes/ce-string.h>

int _tmain(int argc, _TCHAR* argv[])
{
	Ceng::StringUtf8 test = "brickwall.bmp";

	auto iter = test.FindLast('.',test.ConstBeginIterator());

	std::cout << iter.Index() << std::endl;

	Ceng::StringUtf8 temp;

	while (++iter != test.ConstEndIterator())
	{
		auto test = *iter;

		Ceng::CHAR32 symbol = test;

		temp += symbol;
	}

	std::cout << temp << std::endl;

	return 0;
}

