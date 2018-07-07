#ifndef SEARCHTEXT_H
#define SEARCHTEXT_H

#include <vector>
#include <mutex>
#include <thread>
#include <list>

#include "fifu.h"
#include "filesystem.h"

namespace fifu
{

typedef enum
{
	SearchJob_null, //FIXME: Used?
	SearchJob_openDirectory,
	SearchJob_readFile,
} SearchJob_type_t;

class SearchJob
{
	private:
		SearchJob_type_t type;
		std::string path;
		std::string name;
	public:
		SearchJob(SearchJob_type_t type, const std::string & path, const std::string & name);
		~SearchJob();

		SearchJob_type_t getType() const;
		std::string getPath() const;
		std::string getName() const;
		std::string getFullName() const;
};

class SearchJobPack
{
	private:
		std::list<SearchJob> queue;
	public:
		SearchJobPack();
		~SearchJobPack();

		void insertJob(SearchJob & job); //по типу работы, в начало очереди двигаются файлы, в конец каталоги
		const SearchJob * getJob() const; //первая в очереди работа
		void shiftQueue(); // сдвиг очереди
		bool empty() const;
};

class SearchText;
class SearchThread
{
	/*
		Смотрит директорию SearchJobPack, если объектов больше чем N, то может распределить их между дополнительными потоками
		Объекты заворачиваются в SearchJob, и попадают в очередь своего каталога SearchJobPack, при этом у каталогов наименьший приоритет обработки
		Далее поток обрабатывает свою очередь из SearchJobPack, заполняя ее рекурсивно в каждой директории SearchJob либо читая файлы SearchJob
	*/
	private:
		SearchText * base;
		std::list<SearchJobPack> queue;
		std::thread self;

		void init(SearchText * base, SearchJobPack & job);
		void jobDirectory(const SearchJob * job); //парсит каталог, заполняя очередь работ
		void jobFile(const SearchJob * job); //ищет в файле
		void makeJob(const SearchJob * from, SearchJobPack & jp, const FileSystemFile & file);
	public:
		SearchThread(SearchText * base, const std::string & path, const std::string & name); //начинает работу от директории path
		SearchThread(SearchText * base, SearchJobPack & job);//начинает работу job
		~SearchThread();

		void join();
		void doing();
};

class SearchText
{
	private:
		std::vector<FiFuFound> * found;
		std::string text;
		std::list<SearchThread *> threads;
		std::mutex rdwr;

		const unsigned char threads_max = 10;
		const unsigned thread_slice_point = 5;
	public:
		SearchText();
		~SearchText();

		void search(const std::string & text, std::vector<FiFuFound> * found);
		void insertThread(const std::string & path, const std::string & name);
		void insertThread(SearchJobPack & job);
		bool isMaxThreads() const;
		void insertFound(FiFuFound & found);
		unsigned getSlicePoint();
};

}

#endif