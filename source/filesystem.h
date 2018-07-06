#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>
#include <list>

#include <dirent.h>

namespace fifu
{

typedef enum
{
	FileSystemFile_simple,
	FileSystemFile_directory
} FileSystemFile_type_t;

class FileSystemFile
{
	private:
		std::string name;
		FileSystemFile_type_t type;
	public:
		FileSystemFile();
		~FileSystemFile();

		const std::string & getName();
		FileSystemFile_type_t getType();
};

class FileSystem
{
	private:
		bool readed;
		std::list<FileSystemFile> files;
	public:
		FileSystem();
		~FileSystem();

		void readDir(const std::string & path);
		bool isReaded();
		size_t getFilesCount();
		const std::list<FileSystemFile> & getFiles();

		static std::string getLocalPath();
};

}

#endif