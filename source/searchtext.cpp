#include "searchtext.h"
#include <thread>

using namespace std;
using namespace fifu;

SearchText::SearchText(std::vector<FiFuFound> * found)
{
	this->found = found;
}

SearchText::~SearchText()
{

}

void SearchText::SearchTextsearch(const std::string & text)
{
	this->text = text;
}
