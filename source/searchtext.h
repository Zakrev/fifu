#ifndef SEARCHTEXT_H
#define SEARCHTEXT_H

#include <vector>
#include <mutex>
#include <thread>

#include "fifu.h"
#include <list>

namespace fifu
{

typedef enum
{
	SearchJob_null,
	SearchJob_openDirectory,
	SearchJob_readFile,
} SearchJob_type_t;

class SearchJob
{
	private:
		SearchJob_type_t type;
		std::string path;
	public:
		SearchJob(SearchJob_type_t type, const std::string & path);
		~SearchJob();
};

class SearchJobPack
{
	private:
		std::list<SearchJob> queue;
	public:
		SearchJobPack(); //пустая, только для хранения очереди
		~SearchJobPack();

		void insertJob(SearchJob & job); //по типу работы, в начало очереди двигаются файлы, в конец каталоги
		SearchJob & getNextJob();
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

		void pmain(); //функция потока
		void jobDirectory(const std::string & path); //парсит каталог, заполняя очередь работ
		void jobFile(const std::string & path); //ищет в файле
	public:
		SearchThread(SearchText * base, const std::string & path); //начинает работу от директории path
		SearchThread(SearchText * base, SearchJobPack job);//начинает работу job
		~SearchThread();
};

class SearchText
{
	private:
		std::vector<FiFuFound> * found;
		std::string text;
		const unsigned char threads_max = 10;
		std::vector<SearchThread> threads;
		std::mutex rdwr;
	public:
		SearchText(std::vector<FiFuFound> * found);
		~SearchText();

		void search(const std::string & text);
		void insertThread(const std::string & path);
		void insertThread(SearchJobPack job);
		void insertFound(FiFuFound & found);
};

}

#endif