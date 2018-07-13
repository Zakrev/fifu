#include "regexpbox.h"
#include "logs.h"

using namespace std;
using namespace fifu;

void RegExpContext::replaceBuffer(size_t offset)
{
	if (!this->getFunc)
	{
		this->context.eof = true;
		return;
	}

	if (this->context.eof && offset > this->context.global_offset)
	{
		LOG_DBG("Can't read buffer from %lu: EOF", (unsigned long)offset);
		return;
	}

	switch (this->getFunc(offset, &this->buffer, this->minimal_buffer_size))
	{
		case 0:
			this->context.global_offset = offset;
			this->context.local_buffer_size = this->buffer.size();
			this->context.local_offset = 0;
			this->context.shift_offset = 0;
			if (this->minimal_buffer_size > this->context.local_buffer_size)
			{
				this->context.eof = true;
			}
			break;
		case -1:
			throw("Can't read buffer: getFunc return -1");
		default:
			throw "Invalid getBufferFunc() return result";
	}
}

void RegExpContext::increaseBuffer(size_t length)
{
	if (!this->getFunc)
	{
		this->context.eof = true;
		return;
	}

	if (this->context.eof)
	{
		LOG_DBG("Can't read buffer from %lu: EOF", (unsigned long)offset);
		return;
	}

	string inc;
	switch (this->getFunc(offset, &inc, length))
	{
		case 0:
			this->buffer += inc;
			this->context.local_buffer_size = this->buffer.size();
			if (length > this->context.local_buffer_size)
			{
				this->context.eof = true;
			}
			break;
		case -1:
			throw("Can't read buffer: getFunc return -1");
		default:
			throw "Invalid getBufferFunc() return result";
	}
}

RegExpContext::RegExpContext(const string & buffer)
{
	this->buffer = buffer;
	this->context = {
		.global_offset = 0,
		.local_buffer_size = this->buffer.length(),
		.local_offset = 0,
		.shift_offset = 0,
		.eof = true,
	};
	this->getFunc = NULL;
}

RegExpContext::RegExpContext(getBufferFunc * func)
{
	this->getFunc = func;
	this->context = {
		.global_offset = 0,
		.local_buffer_size = 0,
		.local_offset = 0,
		.shift_offset = 0,
		.eof = false,
	};
	this->replaceBuffer();
}

RegExpContext::~RegExpContext()
{

}

string & RegExpContext::getChars(size_t len)
{
	if (this->context.local_buffer_size < (this->context.local_offset + len))
	{
		this->increaseBuffer( ((this->context.local_offset + len) - this->context.local_buffer_size) * 2 );
	}

	if (this->context.local_buffer_size <= (this->context.local_offset + len))
	{
		this->context.shift_offset = this->context.local_buffer_size;
		return this->buffer.substr(this->context.local_offset);
	}
	else
	{
		this->context.shift_offset += len;
		return this->buffer.substr(this->context.local_offset, len);
	}
}

void RegExpContext::shift()
{
	this->context.local_offset += this->context.shift_offset;

	if (this->context.local_buffer_size <= this->context.local_offset)
	{
		if (this->context.eof)
		{
			LOG_DBG("Can't shift: EOF");
			return;
		}

		this->replaceBuffer(this->context.global_offset + this->context.local_buffer_size);
	}
}

void RegExpContext::saveContext()
{
	this->context_stack.push_back(this->context);
}

void RegExpContext::restoreContext()
{
	if (this->context_stack.size() < 1)
		throw "Can't restore context: context_stack is empty";

	this->context = *(this->context_stack.back());
	this->context_stack.pop_back();
	this->replaceBuffer(this->context.global_offset);
}

void RegExpContext::deleteRestoredContext()
{
	if (this->context_stack.size() < 1)
		throw "Can't remove context: context_stack is empty";

	this->context_stack.pop_back();
}

bool RegExpContext::EOF()
{
	return (this->context.eof && (this->context.local_offset >= this->context.local_buffer_size));
}


// RegExpBox

virtual void RegExpBox::execute(RegExpContext & context)
{

}

virtual void RegExpBox::compile(RegExpContext & context)
{

}

RegExpBox::RegExpBox()
{
	this->fOR = false;
}

RegExpBox::~RegExpBox()
{

}
