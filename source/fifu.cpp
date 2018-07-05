#include "fifu.h"
#include "searchtext.h"

using namespace std;
using namespace fifu;

FiFuFoundText::FiFuFoundText(const std::string & text, unsigned long line)
{
	this->text = text;
	this->line = line;
}

FiFuFoundText::~FiFuFoundText()
{

}

std::string & FiFuFoundText::getText()
{
	return this->text;
}

unsigned long FiFuFoundText::getLine()
{
	return this->line;
}




FiFuFound::FiFuFound(const std::string & fpath, const std::string & fname)
{
	this->fpath = fpath;
	this->fname = fname;
}

FiFuFound::~FiFuFound()
{

}

void FiFuFound::insertText(const std::string & text, unsigned long line)
{
	auto it = this->fdata.begin() + this->fdata.size();
	this->fdata.insert(it, FiFuFoundText(text, line));
}

FiFuFoundText & FiFuFound::operator[](unsigned idx)
{
	if (this->fdata.size() <= idx)
		throw "Invalid index";

	return this->fdata[idx];
}

size_t FiFuFound::getSize()
{
	return this->fdata.size();
}




FiFu::FiFu()
{

}

FiFu::~FiFu()
{

}

char FiFu::search(const std::string & data)
{
	SearchText st = SearchText(&this->found);

	st.search(data);

	return (this->getSize() > 0);
}

size_t FiFu::getSize()
{
	return this->found.size();
}

FiFuFound & FiFu::operator[](unsigned idx)
{
	if (this->found.size() <= idx)
		throw "Invalid index";

	return this->found[idx];
}
