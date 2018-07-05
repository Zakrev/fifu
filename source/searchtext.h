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
	SearchJob_openDirectory,
	SearchJob_readFile,
} SearchJob_type_t;

class SearchJob
{
	private:
		SearchJob_type_t type;
	public:
		SearchJob(SearchJob_type_t type);
		~SearchJob();
};

class SearchJobPack
{
	private:
		std::list<SearchJob> queue;
	public:

		SearchJobPack();
		~SearchJobPack();

		void insert(SearchJob & job);
};

class SearchThread
{
	private:
		std::list<SearchJobPack> queue;
	public:
		SearchThread();
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
};

}

#endif