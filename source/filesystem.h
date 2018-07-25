#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>
#include <list>

namespace fifu
{

typedef enum
{
	FileSystemFile_unknown,
	FileSystemFile_readFile,
	FileSystemFile_directory,
	FileSystemFile_notReadFile
} FileSystemFile_type_t;

class FileSystem;
class FileSystemFile
{
	friend class FileSystem;
	private:
		std::string name;
		FileSystemFile_type_t type;
	public:
		FileSystemFile();
		~FileSystemFile();

		const std::string & getName() const;
		FileSystemFile_type_t getType() const;
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
		bool isReaded() const;
		size_t getFilesCount() const;
		const std::list<FileSystemFile> & getFiles() const;

		static std::string getLocalPath();
};

}

#endif