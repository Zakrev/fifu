#include "regexp.h"

using namespace std;
using namespace fifu;


void RegExpResult::insert(size_t line, size_t start, size_t end)
{
	this->insert(line, start, end, (const char * )NULL);
}

void RegExpResult::insert(size_t line, size_t start, size_t end, const char * text)
{
	auto it = this->positions.begin() + this->positions.size();

	this->positions.insert(it, end);
	this->positions.insert(it, start);
	this->positions.insert(it, line);

	if (text)
	{
		auto it2 = this->texts.begin() + this->texts.size();
		this->texts.insert(it2, string(text));
	}
}

RegExpResult::RegExpResult()
{

}

RegExpResult::~RegExpResult()
{

}

size_t RegExpResult::getSize() const
{
	return this->positions.size() / (size_t)3;
}

size_t RegExpResult::getLine(size_t idx) const
{
	if (id >= this->positions.size())
		throw "Invalid position id";

	return this->positions[id * 3];
}

size_t RegExpResult::getStartOffset(size_t idx) const
{
	if (id >= this->positions.size())
		throw "Invalid position id";

	return this->positions[id * 3 + 1];
}

size_t RegExpResult::getEndOffset(size_t idx) const
{
	if (id >= this->positions.size())
		throw "Invalid position id";

	return this->positions[id * 3 + 2];
}

const std::string & RegExpResult::getText(size_t idx) const
{
	if (this->positions.size() != this->texts.size())
		throw "Text not created";

	if (id >= this->positions.size())
		throw "Invalid position id";

	return this->positions[id * 3 + 2];
}
