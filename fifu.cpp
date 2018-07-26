#include "source/searchtext.h"
#include <iostream>

using namespace std;
using namespace fifu;

int main(int argc, char ** argv)
{
	if (argc != 2)
	{
		cerr << "Use: {regexpr}" << endl;
		exit(1);
	}

	SearchText st;

	st.search(argv[1]);

	return 0;
}
