#ifndef REGEXP_H
#define REGEXP_H

#include <string>
#include <fstream>
#include <vector>

namespace fifu
{

class RegExpBox;
class RegExpBinary
{
	private:
		RegExpBox * bin;
	public:
		RegExpBinary(const std::string & exp);
		~RegExpBinary();
};

class RegExp;
class RegExpResult
{
	friend RegExp;
	private:
		std::vector<std::string> texts;
		std::vector<size_t> positions; // Координаты результатов: ...[номер строки][смещение начала][смещение конца]...[номер строки][смещение начала][смещение конца]...

		void insert(size_t line, size_t start, size_t end);
		void insert(size_t line, size_t start, size_t end, const char * text);
	public:
		RegExpResult();
		~RegExpResult();

		size_t getSize() const;
		size_t getLine(size_t idx) const;
		size_t getStartOffset(size_t idx) const;
		size_t getEndOffset(size_t idx) const;
		const std::string & getText(size_t idx) const;
};

class RegExpContext;
class RegExp
{
	private:
		RegExpContext context;
		const RegExpBinary * binary; // Регулярное выражение
		std::string eol; // EndOfLine, символы конца строки
		size_t lines; // Количество прочитанных строк (текущая строка)
		std::ifstream input;
	public:
		RegExp(const RegExpBinary * binary, const std::string & eol);
		~RegExp();

		void initSearch(const std::string & path);
		RegExpResult nextResult();
		RegExpResult nextResultAsText();
		void resetSearch();
};

}

#endif