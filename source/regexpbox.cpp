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

size_t RegExpContext::getGlobalOffset()
{
	return this->context.global_offset;
}

void RegExpContext::setSuccess()
{
	this->lastIsSucces = true;
}

void RegExpContext::setError()
{
	this->lastIsSucces = false;
}

bool RegExpContext::isLastSuccess()
{
	return this->lastIsSucces;
}

// RegExpBox
void RegExpBox::execute(RegExpContext & context)
{

}

void RegExpBox::compile(RegExpContext & context)
{

}

RegExpBox::RegExpBox()
{
	this->fOR = false;
}

RegExpBox::~RegExpBox()
{

}

#define RB_SIMBOLS_ESC "\\"
#define RB_SIMBOLS_OR "|"
#define RB_SIMBOLS_GROUP_START "("
#define RB_SIMBOLS_GROUP_END ")"
#define RB_SIMBOLS_GROUP_NOT "^"

// RegExpBoxGroup
void RegExpBoxGroup::execute(RegExpContext & context)
{
	context.saveContext();
	for (auto it = this->child.begin(); !context.EOF() && it != this->child.end(); it++)
	{
		(*it)->execute(context);

		if (context.isLastSuccess())
		{
			if ((*it)->fOR)
			{
				break;
			}
		}
		else
		{
			for (; it != this->child.end(); it++)
			{
				if ((*it)->fOR)
				{
					context.restoreContext();
					break;
				}
			}
			if (it == this->child.end())
				break;
		}
	}
	context.deleteRestoredContext();

	if (this->fNOT)
	{
		if (context.isLastSuccess())
			context.setError();
		else
			context.setSuccess();
	}
	/*else ненужно
	{
		if (context.isLastSuccess())
			context.setSuccess();
		else
			context.setError();
	}*/
}

void RegExpBoxGroup::compile(RegExpContext & context)
{
	while (!context.EOF())
	{
		string text = context.getChars(1);
		if (text.size() == 0)
		{
			LOG_DBG("EOF on ", context.getGlobalOffset());
			goto compile_end;
		}

		LOG_DBG("Found: '", text, "' on ", context.getGlobalOffset());
		do {
			if (text == RB_SIMBOLS_OR)
			{
				LOG_ERR("Position ", context.getGlobalOffset(), ", unexpected '", RB_SIMBOLS_OR "': ", text);
				throw "Failed compile";
			}

			// End self
			if (text == RB_SIMBOLS_GROUP_END)
			{
				goto compile_end;
			}
			// End self

			// Create RegExpBoxGroup
			if (text == RB_SIMBOLS_GROUP_START)
			{
				context.shift();

				RegExpBoxGroup * ch;

				ch = new RegExpBoxGroup();
				if (!ch)
					throw "Failed new RegExpBoxGroup()";

				this->child.push_back(ch);
				ch->compile(context);

				text = context.getChars(1);
				if (text != RB_SIMBOLS_GROUP_END)
				{
					LOG_ERR("Position ", context.getGlobalOffset(), ", expected '", RB_SIMBOLS_GROUP_END "': ", text);
					throw "Failed compile";
				}
				else
				{
					context.shift();

					text = context.getChars(1);
					if (text == RB_SIMBOLS_OR)
					{
						context.shift();
						ch->fOR = true;
					}
				}
				break;
			}
			// Create RegExpBoxGroup

			//Create RegExpBoxString
			if (text == RB_SIMBOLS_ESC)
			{
				context.shift();
			}
			RegExpBoxString * ch;

			ch = new RegExpBoxString();
			if (!ch)
				throw "Failed new RegExpBoxString()";

			this->child.push_back(ch);
			ch->compile(context);
			//Create RegExpBoxString

		} while (0);
	}
	compile_end:
		LOG_DBG("Succesfull: ", context.getGlobalOffset());
}

RegExpBoxGroup::RegExpBoxGroup() : RegExpBox()
{
	this->fNOT = false;
}

RegExpBoxGroup::~RegExpBoxGroup()
{
	while (this->child.size())
	{
		auto it = this->child.begin();
		RegExpBox * box = (*it);

		this->child.pop_begin();
		delete box;
	}
}

// RegExpBoxString
void RegExpBoxString::execute(RegExpContext & context)
{
	string text = context.getChars(this->value.size());
	context.shift();

	if (text.size() != this->value.size())
	{
		context.setError();
	}
	else
	{
		if (text != this->value)
		{
			context.setError();
		}
		else
		{
			context.setSuccess();
		}
	}
}

void RegExpBoxString::compile(RegExpContext & context)
{
	while (!context.EOF())
	{
		string text = context.getChars(1);
		if (text.size() == 0)
		{
			LOG_DBG("EOF on ", context.getGlobalOffset());
			goto compile_end;
		}

		LOG_DBG("Found: '", text, "' on ", context.getGlobalOffset());
		do {
			// End self
			if (text == RB_SIMBOLS_OR)
			{
				context.shift();
				this->fOR = true;
				goto compile_end;
			}
			if (text == RB_SIMBOLS_GROUP_END)
			{
				goto compile_end;
			}
			if (text == RB_SIMBOLS_GROUP_START)
			{
				goto compile_end;
			}
			// End self

			// Add text
			if (text == RB_SIMBOLS_ESC)
			{
				context.shift();
				text = context.getChars(1);
				if (text.size() == 0)
				{
					LOG_ERR("Position ", context.getGlobalOffset(), ", expected any symbol after '" RB_SIMBOLS_ESC "': ", text);
					throw "Failed compile";
				}
			}
			this->value += text;
			context.shift();
			// Add text
		} while (0);
	}
	compile_end:
		LOG_DBG("Succesfull: ", context.getGlobalOffset());
}

RegExpBoxString::RegExpBoxString()
{

}

RegExpBoxString::~RegExpBoxString()
{

}
