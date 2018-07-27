#ifndef REGEXP_H
#define REGEXP_H

#include <string>
#include <fstream>
#include <vector>

#include "regexpbox.h"

namespace fifu
{

class RegExpBinary
{
	private:
		RegExpBoxGroup root;
	public:
		RegExpBinary(const std::string & exp);
		~RegExpBinary();

		void execute(RegExpContext & context);
};

class RegExp
{
	private:
		RegExpBuffer * file;
		RegExpContext context;
		RegExpBinary * binary; // Регулярное выражение
		std::string eol; // EndOfLine, символы конца строки
		bool endOfFile;
	public:
		RegExp(RegExpBinary * binary, const std::string & eol = "");
		~RegExp();

		void searchStartInFile(const std::string & path);
		/*void searchStartInBuffer(const std::string & buffer);
		void searchStartInBuffer(getBufferFunc * get_buffer_func);*/
		void searchNext();
		bool eof();
		/*result_t getResult();
		std::list<result_t> & getResult(std::string & name);*/
};

}

#endif