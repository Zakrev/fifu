#ifndef FIFU_H
#define FIFU_H

#include <vector>
#include <string>
#include <cstdlib>

namespace fifu
{

class FiFuFoundText
{
	private:
		std::string text;
		unsigned long line;
	public:

		FiFuFoundText(const std::string & text, unsigned long line);
		~FiFuFoundText();

		std::string & getText();
		unsigned long getLine();
};

class FiFuFound
{
	private:
		std::string fpath;
		std::string fname;
		std::vector<FiFuFoundText> fdata;
	public:

		FiFuFound(const std::string & fpath, const std::string & fname);
		~FiFuFound();

		void insertText(const std::string & text, unsigned long line);
		FiFuFoundText & operator[](unsigned idx);
		size_t getSize();
};

class FiFu
{
	private:
		std::vector<FiFuFound> found;
	public:

		FiFu();
		~FiFu();

		char search(const std::string & data);
		size_t getSize();
		FiFuFound & operator[](unsigned idx);
};

}

#endif