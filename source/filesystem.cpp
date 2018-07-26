#include "filesystem.h"

#include <cerrno>
#include <mutex>

#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#define LOGS_H_LOG_ENABLED
#define LDEBUG 1
#include "logs.h"

using namespace std;
using namespace fifu;

FileSystemFile::FileSystemFile()
{

}

FileSystemFile::~FileSystemFile()
{

}


const string & FileSystemFile::getName() const
{
	return this->name;
}

FileSystemFile_type_t FileSystemFile::getType() const
{
	return this->type;
}




FileSystem::FileSystem()
{

}

FileSystem::~FileSystem()
{

}

void FileSystem::readDir(const std::string & path)
{
	DIR * dir = NULL;
	struct dirent * entry = NULL;

	dir = opendir(path.c_str());

	if (!dir)
	{
		log(LDEBUG,"opendir: ", strerror(errno));
		return;
	}

	for (errno = 0, entry = readdir(dir); entry; entry = readdir(dir))
	{
		FileSystemFile file;

		file.name = entry->d_name;

#ifdef _DIRENT_HAVE_D_TYPE
		switch (entry->d_type)
		{
			case DT_DIR:
				if (file.name == "." || file.name == "..")
					continue;
				file.type = FileSystemFile_directory;
				break;
			case DT_REG:
				file.type = FileSystemFile_readFile;
				break;
			case DT_UNKNOWN:
				file.type = FileSystemFile_unknown;
				break;
			default:
				file.type = FileSystemFile_notReadFile;
				break;
		}
#else
#error Use stat()
#endif

		this->files.push_back(file);
	}
	if (errno)
	{
		log(LDEBUG,"readdir: ", strerror(errno));
	}

	closedir(dir);
	this->readed = true;
}

bool FileSystem::isReaded() const
{
	return this->readed;
}

size_t FileSystem::getFilesCount() const
{
	return this->files.size();
}

const list<FileSystemFile> & FileSystem::getFiles() const
{
	return this->files;
}


string FileSystem::getLocalPath()
{
	char * path = getcwd(NULL, 0);

	if (!path)
	{
		log(LDEBUG,"getcwd: ", strerror(errno));
	}

	string buff = path;
	free(path);

	buff += "/";

	return buff;
}
