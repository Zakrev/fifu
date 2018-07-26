#include "searchtext.h"
#include "logs.h"

#include <thread>

using namespace std;
using namespace fifu;

SearchJob::SearchJob(SearchJob_type_t type, const std::string & path, const std::string & name)
{
	this->type = type;
	this->path = path;
	this->name = name;
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

std::string SearchJob::getName() const
{
	return this->name;
}

std::string SearchJob::getFullName() const
{
	switch (this->type)
	{
		case SearchJob_openDirectory:
			return this->path + this->name + "/";
		default:
			return this->path + this->name;
	}
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


static void SearchThread_main(SearchThread * self) //функция потока
{
	if(!self)
		throw "ptr 'self' is NULL";

	self->doing();
}

void SearchThread::init(SearchText * base, SearchJobPack & job)
{
	this->base = base;
	this->queue.push_front(job);
	this->self = thread(SearchThread_main, this);
}

void SearchThread::makeJob(const SearchJob * from, SearchJobPack & jp, const FileSystemFile & file)
{
	switch (file.getType())
	{
		case FileSystemFile_readFile:
			{
				SearchJob j = SearchJob(SearchJob_readFile, from->getFullName(), file.getName());
				jp.insertJob(j);
			}
			break;
		case FileSystemFile_directory:
			{
				SearchJob j = SearchJob(SearchJob_openDirectory, from->getFullName(), file.getName());
				jp.insertJob(j);
			}
			break;
		default:
			LOG_DBG("skip file: ", file.getName());
			break;
	}
}

void SearchThread::jobDirectory(const SearchJob * job)
{
	FileSystem fs;

	fs.readDir(job->getFullName());
	if (!fs.isReaded())
	{
		LOG_DBG("can't read directory: ", job->getFullName());
		return;
	}

	const list<FileSystemFile> & files = fs.getFiles();
	if (files.size() < 1)
	{
		LOG_DBG("directory is empty: ", job->getFullName());
		return;
	}

	if (files.size() > this->base->getSlicePoint() && !this->base->isMaxThreads())
	{
		auto it = files.begin();

		while (it != files.end())
		{
			SearchJobPack jp;

			for (unsigned fcount = this->base->getSlicePoint(); fcount > 0 && it != files.end(); fcount--, it++)
			{
				this->makeJob(job, jp, *it);
			}

			try
			{
				this->base->insertThread(jp);
			}
			catch (char const * mess)
			{
				LOG_DBG("can't create thread: ", mess);

				for (; it != files.end(); it++)
				{
					this->makeJob(job, jp, *it);
				}

				this->queue.push_back(jp);
				break;
			}
		}
	}
	else
	{
		SearchJobPack jp;

		for (auto it = files.begin(); it != files.end(); it++)
		{
			this->makeJob(job, jp, *it);
		}

		this->queue.push_back(jp);
	}
}

void SearchThread::jobFile(const SearchJob * job)
{

}

SearchThread::SearchThread(SearchText * base, const std::string & path, const std::string & name)
{
	SearchJob job = SearchJob(SearchJob_openDirectory, path, ".");
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




SearchText::SearchText()
{

}

SearchText::~SearchText()
{

}

void SearchText::search(const std::string & text)
{
	this->text = text;

	this->insertThread(FileSystem::getLocalPath(), ".");

	while (this->threads.size() > 0)
	{
		(*this->threads.begin())->join();
		delete (*this->threads.begin());
		this->threads.pop_front();
	}
}

void SearchText::insertThread(const std::string & path, const std::string & name)
{
	this->rdwr.lock();

	if (this->isMaxThreads())
	{
		this->rdwr.unlock();
		throw "Threads is max";
	}

	this->threads.push_back(new SearchThread(this, path, name));

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

	this->rdwr.unlock();
}

bool SearchText::isMaxThreads() const
{
	return (this->config.threads_max <= this->threads.size());
}

unsigned SearchText::getSlicePoint() const
{
	return this->config.thread_slice_point;
}

/*
#include <cstring> // memcpy()
// Linux fork(), wait(), exec()
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
// Linux fork(), wait(), exec()
void SearchText::runExternalRegexp(const std::string & path)
{
	this->out_mutex.lock();
	pid_t pid = fork();

	if (pid == 0)
	{
		char * expr_str = new char [this->text.length() + 1];
		char * patch_str = new char [path.length() + 1];

		memcpy(expr_str, this->text.c_str(), this->text.length());
		expr_str[this->text.length()] = '\0';

		memcpy(patch_str, path.c_str(), path.length());
		patch_str[path.length()] = '\0';

		char * argv[] = { "/bin/grep", "-n", "-H", expr_str, patch_str, NULL };
		execve("/bin/grep", argv, (char * const *)NULL);

		delete [] expr_str;
		delete [] patch_str;
		LOG_ERR("Error: Can't execve()");
		perror("Error: Can't execve()");
		exit(1);
	}
	else if (pid > 0)
	{
		int status;
		waitpid(pid, &status, 0);
		this->out_mutex.unlock();
	}
	else
	{
		this->out_mutex.unlock();
		throw "Can't fork()";
	}
}
*/