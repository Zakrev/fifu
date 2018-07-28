#include "regexp.h"

#define LOGS_H_LOG_ENABLED
#define LDEBUG 1
#include "logs.h"

using namespace std;
using namespace fifu;

RegExpBinary::RegExpBinary(const std::string & exp, RegExpFlags_t flags)
{
	RegExpContext context(exp, flags);

	this->root.compile(context);
}

RegExpBinary::~RegExpBinary()
{

}

void RegExpBinary::execute(RegExpContext & context)
{
	this->root.execute(context);
}



RegExp::RegExp(RegExpBinary * binary, const std::string & eol)
{
	if (!binary)
		throw "binary is NULL";

	this->eol = eol;
	this->binary = binary;
	this->file = NULL;
	this->endOfFile = false;
}

RegExp::~RegExp()
{
	if (this->file)
		delete this->file;
}

class StaticBuffer : public RegExpBuffer
{
	private:
		string path; // DEBUG
		ifstream input;
	public:
		bool getBuffer(size_t offset, std::string * buffer, size_t length)
		{
			if (!this->input.is_open())
			{
				log(LDEBUG,"input not open");
				return false;
			}

			this->input.clear();
			this->input.seekg(offset, ios::beg);

			char * tmp = new char [length];
			if (!tmp)
			{
				log(LDEBUG,"failed new char");
				return false;
			}

			this->input.read(tmp, length);
			buffer->clear();
			buffer->insert(0, tmp, input.gcount());
			delete[] tmp;

			return true;
		}

		StaticBuffer() { }
		~StaticBuffer()
		{
			this->close();
		}

		void open(const std::string & path)
		{
			this->path = path;
			ifstream & inp = this->input;
			inp.open(path, ios_base::binary);

			if (!inp.is_open())
			{
				log(LDEBUG,"can't open file: ", path);
				throw "can't open file";
			}
		}
		void close()
		{
			if (this->input.is_open())
				this->input.close();
		}
		void print(result_t & result)
		{
			if (!this->input.is_open())
			{
				log(LDEBUG,"input not open");
				return;
			}

			this->input.clear();
			this->input.seekg(result.global_offset_start, ios::beg);

			char * tmp = new char [1024];
			if (!tmp)
			{
				log(LDEBUG,"failed new char");
				return;
			}
			this->input.getline(tmp, 1024);

			cout << path << "\n (" << result.global_offset_start << "): " << tmp << endl;
		}
};

void RegExp::searchStartInFile(const std::string & path)
{
	this->endOfFile = false;
	if (!this->file)
	{
		this->file = new StaticBuffer();

		if (!this->file)
			throw "failed new StaticBuffer";
	}

	StaticBuffer * file = (StaticBuffer *)this->file;
	file->close();
	file->open(path);

	this->context = RegExpContext(this->file);
}

void RegExp::searchNext()
{
	while (!this->context.eof())
	{
		this->context.saveContext();
		this->binary->execute(this->context);

		if (!this->context.isSuccess())
		{
			context.restoreContext();
			context.deleteRestoredContext();

			if (this->context.eof())
				break;

			this->context.getChars(1);
			this->context.shift();
		}
		else
		{
			context.deleteRestoredContext();

			StaticBuffer * file = (StaticBuffer *)this->file;
			file->print(this->context.getResult());

		}
	}

	if (this->context.eof())
	{
		this->endOfFile = true;
	}
}

bool RegExp::eof()
{
	return this->endOfFile;
}
/*
result_t RegExp::getResult()
{
	throw "TODO";
}

std::list<result_t> & RegExp::getResult(std::string group_name)
{
	throw "TODO";
}*/
