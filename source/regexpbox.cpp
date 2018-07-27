#include "regexpbox.h"

#define LOGS_H_LOG_ENABLED
#define LDEBUG 0
#include "logs.h"

using namespace std;
using namespace fifu;

void RegExpContext::replaceBuffer(size_t offset)
{
	if (!this->buffers)
	{
		this->context.eof = true;
		return;
	}

	if (this->context.eof && offset > this->context.global_offset)
	{
		log(LDEBUG,"Can't read buffer from ", (unsigned long)offset, " : eof");
		return;
	}

	if (offset == this->context.global_offset)
	{
		return;
	}

	if (this->buffers->getBuffer(offset, &this->buffer, fifu::minimal_buffer_size))
	{
		this->context.global_offset = offset;
		this->context.local_buffer_size = this->buffer.length();
		this->context.local_offset = 0;
		this->context.shift_offset = 0;
		if (fifu::minimal_buffer_size > this->context.local_buffer_size)
		{
			this->context.eof = true;
		}
	}
	else
		throw("Can't read buffer: getBuffer return false");
}

void RegExpContext::increaseBuffer(size_t length)
{
	if (!this->buffers)
	{
		this->context.eof = true;
		return;
	}

	size_t offset = this->context.global_offset;
	if (this->context.eof)
	{
		log(LDEBUG,"Can't read buffer from ", (unsigned long)offset, " : eof");
		return;
	}

	string inc;
	if (this->buffers->getBuffer(offset + this->context.local_buffer_size, &inc, length))
	{
		this->buffer += inc;
		this->context.local_buffer_size = this->buffer.length();
		if (length > this->context.local_buffer_size)
		{
			this->context.eof = true;
		}
	}
	else
		throw("Can't read buffer: getBuffer return false");
}

void RegExpContext::insertResult(std::string & name, result_t result)
{
	if (name.length() == 0)
	{
		this->result = result;
	}
	else
	{
		auto it = this->results.find(name);

		if (it == this->results.end())
		{
			this->results.insert( pair<string,list<result_t>>(name, list<result_t>()) );
		}

		this->results[name].push_back(result);
	}
}

RegExpContext::RegExpContext(const string & buffer, RegExpFlags_t regexp_flags)
{
	this->buffer = buffer;
	this->context = {
		.global_offset = 0,
		.local_buffer_size = this->buffer.length(),
		.local_offset = 0,
		.shift_offset = 0,
		.eof = true,
	};
	this->buffers = NULL;
	this->regexp_flags = regexp_flags;
}

RegExpContext::RegExpContext(RegExpBuffer * buffers, RegExpFlags_t regexp_flags)
{
	if (!buffers)
		throw "ptr buffers is NULL";

	this->buffers = buffers;
	this->context = {
		.global_offset = 0,
		.local_buffer_size = 0,
		.local_offset = 0,
		.shift_offset = 0,
		.eof = false,
	};
	this->replaceBuffer(0);
	this->regexp_flags = regexp_flags;
}

RegExpContext::RegExpContext()
{

}

RegExpContext::~RegExpContext()
{

}

string RegExpContext::getChars(size_t len)
{
	if (this->eof())
	{
		throw "Can't get chars: EOF";
	}

	if (this->context.local_buffer_size < (this->context.local_offset + len))
	{
		this->increaseBuffer( ((this->context.local_offset + len) - this->context.local_buffer_size) * 2 );
	}

	if (this->context.local_buffer_size <= (this->context.local_offset + len))
	{
		this->context.shift_offset = this->context.local_buffer_size;
		if (this->buffer.length() < this->context.local_offset)
		{
			log(LERROR,"Buffer: ", this->buffer.length(), " < ", this->context.local_offset);
			throw "Invalid local_offset";
		}
		return this->buffer.substr(this->context.local_offset);
	}
	else
	{
		this->context.shift_offset = len;
		if (this->buffer.length() < this->context.local_offset)
		{
			log(LERROR,"Buffer: ", this->buffer.length(), " < ", this->context.local_offset);
			throw "Invalid local_offset";
		}
		if (this->buffer.length() < this->context.local_offset + len)
		{
			log(LERROR,"Buffer: ", this->buffer.length(), " < ", this->context.local_offset + len);
			throw "Invalid len";
		}
		return this->buffer.substr(this->context.local_offset, len);
	}
}

void RegExpContext::shift()
{
	if (this->eof())
	{
		throw "Can't shift: EOF";
	}

	this->context.local_offset += this->context.shift_offset;

	if (this->context.local_buffer_size < this->context.local_offset)
	{
		if (this->context.eof)
		{
			log(LDEBUG,"Can't shift: eof");
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

	this->replaceBuffer(this->context_stack.back().global_offset);
	this->context = this->context_stack.back();
}

void RegExpContext::deleteRestoredContext()
{
	if (this->context_stack.size() < 1)
		throw "Can't remove context: context_stack is empty";

	this->context_stack.pop_back();
}

bool RegExpContext::eof()
{
	return (this->context.eof && (this->context.local_offset >= this->context.local_buffer_size));
}

size_t RegExpContext::getGlobalOffset()
{
	return this->context.global_offset;
}

size_t RegExpContext::getTotalOffset()
{
	return this->context.global_offset + this->context.local_offset;
}

void RegExpContext::setSuccess()
{
	this->lastIsSucces = true;
}

void RegExpContext::setError()
{
	this->lastIsSucces = false;
}

bool RegExpContext::isSuccess()
{
	return this->lastIsSucces;
}

bool RegExpContext::isSetOnlyText()
{
	return (this->regexp_flags & fifu::f_onlyText);
}

result_t & RegExpContext::getResult()
{
	return this->result;
}

std::list<result_t> & RegExpContext::getResult(std::string & name)
{
	auto it = this->results.find(name);
	if (it == this->results.end())
		throw "Name not found";

	return this->results[name];
}

void RegExpContext::dump()
{
	log(LDEBUG,"DUMP:\n\tglobal_offset: ", this->context.global_offset, "\n\tlocal_offset: ",
		this->context.local_offset, "\n\tlocal_buffer_size: ", this->context.local_buffer_size,
		"\n\tshift_offset: ", this->context.shift_offset, "\n\teof: ", this->context.eof);
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
	result_t local_result = {
		.len = 0,
		.global_offset_start = context.getTotalOffset(),
	};
	context.saveContext();
	for (auto it = this->child.begin(); !context.eof() && it != this->child.end(); it++)
	{
		(*it)->execute(context);

		if (context.isSuccess())
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
		if (context.isSuccess())
			context.setError();
		else
			context.setSuccess();
	}
	/*else ненужно
	{
		if (context.isSuccess())
			context.setSuccess();
		else
			context.setError();
	}*/

	if (context.isSuccess())
	{
		local_result.len = context.getTotalOffset() - local_result.global_offset_start;
		context.insertResult(this->name, local_result);
	}
}

void RegExpBoxGroup::compile(RegExpContext & context)
{
	if (context.isSetOnlyText())
	{
		RegExpBoxString * ch;

		ch = new RegExpBoxString();
		if (!ch)
			throw "Failed new RegExpBoxString()";

		this->child.push_back(ch);
		ch->compile(context);

		goto compile_end;
	}
	else // not isSetOnlyText()
		while (!context.eof())
		{
			string text = context.getChars(1);
			if (text.size() == 0)
			{
				log(LDEBUG,"eof on ", context.getTotalOffset());
				goto compile_end;
			}

			log(LDEBUG,"Found: '", text, "' on ", context.getTotalOffset());
			do {
				if (text == RB_SIMBOLS_OR)
				{
					log(LERROR,"Position ", context.getTotalOffset(), ", unexpected '", RB_SIMBOLS_OR "': ", text);
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
						log(LERROR,"Position ", context.getTotalOffset(), ", expected '", RB_SIMBOLS_GROUP_END "': ", text);
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
		log(LDEBUG,"Succesfull: ", context.getTotalOffset());
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

		this->child.pop_front();
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
			log(LDEBUG,"Succesfull: ", context.getTotalOffset(), " '", this->value, "'");
		}
	}
}

void RegExpBoxString::compile(RegExpContext & context)
{
	if (context.isSetOnlyText())
	{
		while (!context.eof())
		{
			this->value = context.getChars(fifu::minimal_buffer_size);
		}
		goto compile_end;
	}
	else // not isSetOnlyText()
		while (!context.eof())
		{
			string text = context.getChars(1);
			if (text.size() == 0)
			{
				log(LDEBUG,"eof on ", context.getTotalOffset());
				goto compile_end;
			}

			log(LDEBUG,"Found: '", text, "' on ", context.getTotalOffset());
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
						log(LERROR,"Position ", context.getTotalOffset(), ", expected any symbol after '" RB_SIMBOLS_ESC "': ", text);
						throw "Failed compile";
					}
				}
				this->value += text;
				context.shift();
				// Add text
			} while (0);
		}
	compile_end:
		log(LDEBUG,"Succesfull: ", context.getTotalOffset(), " '", this->value, "'");
}

RegExpBoxString::RegExpBoxString()
{

}

RegExpBoxString::~RegExpBoxString()
{

}
