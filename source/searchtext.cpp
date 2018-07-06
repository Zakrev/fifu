#include "searchtext.h"
#include <thread>

#include "logs.h"

using namespace std;
using namespace fifu;

SearchJob::SearchJob(SearchJob_type_t type, const std::string & path)
{
	this->type = type;
	this->path = path;
}

SearchJob::~SearchJob()
{

}

SearchJob_type_t SearchJob::getType() const
{
	return this->type;
}

string SearchJob::getPath() const
{
	return this->path;
}




SearchJobPack::SearchJobPack()
{

}

SearchJobPack::~SearchJobPack()
{

}

void SearchJobPack::insertJob(SearchJob & job)
{
	switch (job.getType())
	{
		case SearchJob_openDirectory:
			this->queue.push_back(job);
			break;
		case SearchJob_readFile:
			this->queue.push_front(job);
			break;
		default:
			throw "Invalid job type";
	}
}

const SearchJob * SearchJobPack::getJob() const
{
	if (!this->queue.size())
		throw "Job queue is empty";

	return &(this->queue.front());
}

void SearchJobPack::shiftQueue()
{
	if (!this->queue.size())
		throw "Job queue is empty";

	this->queue.pop_front();
}

bool SearchJobPack::empty() const
{
	return this->queue.empty();
}



static void SearchThread_main(SearchThread * self); //функция потока
void SearchThread::init(SearchText * base, SearchJobPack & job)
{
	this->base = base;
	this->queue.push_front(job);
	this->self = thread(SearchThread_main, this);
}

void SearchThread::jobDirectory(const SearchJob * job)
{
	LOG_DBG("Dirrectory: ", job->getPath());
}

void SearchThread::jobFile(const SearchJob * job)
{
	LOG_DBG("File: ", job->getPath());
}

SearchThread::SearchThread(SearchText * base, const std::string & path)
{
	SearchJob job = SearchJob(SearchJob_openDirectory, path);
	SearchJobPack jobp;
	jobp.insertJob(job);

	this->init(base, jobp);
}

SearchThread::SearchThread(SearchText * base, SearchJobPack & job)
{
	this->init(base, job);
}

SearchThread::~SearchThread()
{

}

void SearchThread::join()
{
	this->self.join();
}

void SearchThread::doing()
{
	if (!this->base->isMaxThreads())
		this->base->insertThread("some path to some dir");

	while (this->queue.size() > 0)
	{
		SearchJobPack * jobp = &(*this->queue.begin());

		while (!jobp->empty())
		{
			const SearchJob * job = jobp->getJob();

			switch (job->getType())
			{
				case SearchJob_openDirectory:
					this->jobDirectory(job);
					break;
				case SearchJob_readFile:
					this->jobFile(job);
					break;
				default:
					throw "Invalid job type";
			}

			jobp->shiftQueue();
		}

		this->queue.pop_front();
	}
}



static void SearchThread_main(SearchThread * self)
{
	if(!self)
		throw "ptr 'self' is NULL";

	self->doing();
}




SearchText::SearchText()
{

}


SearchText::~SearchText()
{

}


void SearchText::search(const std::string & text, std::vector<FiFuFound> * found)
{
	if(!found)
		throw "ptr 'found' is NULL";

	this->found = found;
	this->text = text;

	this->insertThread("some path to some dir");

	while (this->threads.size() > 0)
	{
		(*this->threads.begin())->join();
		delete (*this->threads.begin());
		this->threads.pop_front();
	}
}


void SearchText::insertThread(const std::string & path)
{
	this->rdwr.lock();

	if (this->isMaxThreads())
	{
		this->rdwr.unlock();
		throw "Threads is max";
	}

	this->threads.push_back(new SearchThread(this, path));
	LOG_DBG("Size: ", this->threads.size(), ": ", this->isMaxThreads());

	this->rdwr.unlock();
}

void SearchText::insertThread(SearchJobPack & job)
{
	this->rdwr.lock();

	if (this->isMaxThreads())
	{
		this->rdwr.unlock();
		throw "Threads is max";
	}

	this->threads.push_back(new SearchThread(this, job));
	LOG_DBG("Size: ", this->threads.size(), ": ", this->isMaxThreads());

	this->rdwr.unlock();
}

bool SearchText::isMaxThreads() const
{
	return (this->threads_max <= this->threads.size());
}

void SearchText::insertFound(FiFuFound & found)
{
	this->rdwr.lock();

	this->found->insert(this->found->begin() + this->found->size(), found);

	this->rdwr.unlock();
}

