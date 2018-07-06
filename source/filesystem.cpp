#include "filesystem.h"

#include <cerrno>
#include <unistd.h>

using namespace std;
using namespace fifu;

FileSystem::FileSystem()
{

}

FileSystem::~FileSystem()
{

}

string FileSystem::getLocalPath()
{
	char * path = getcwd(NULL, 0);

	if (!path)
	{
		switch (errno)
		{
			case EACCES:
				throw "getcwd: EACCES (Permission denied)";
			default:
				throw "getcwd: Unknown error";
		}
	}

	string buff = path;
	free(path);

	buff += "/";

	return buff;
}
