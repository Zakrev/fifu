#include "source/regexp.h"
#include <iostream>

using namespace std;
using namespace fifu;

int main(int args, char ** argv)
{
	if (args != 3)
	{
		cerr << "Use: {expr} {file}" << endl;
		return 1;
	}

	string exp_str = argv[1];
	string file_str = argv[2];

	RegExpBinary bin = RegExpBinary(exp_str);
	RegExp exp = RegExp(&bin, "\n");

	try
	{
		exp.searchStartInFile(file_str);
		while (!exp.eof())
		{
			exp.searchNext();
		}
	}
	catch (char const * err)
	{
		cerr << "Error: " << err << endl;
	}

	return 0;
}
