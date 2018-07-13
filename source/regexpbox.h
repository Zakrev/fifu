#ifndef REGEXPBOX_H
#define REGEXPBOX_H

#include <string>
#include <list>
#include <map>

namespace fifu
{

#define CONTEXT_BUFFER_MINIMAL 128

typedef struct
{
	size_t global_offset; // Полное смещение от начала
	size_t local_buffer_size; // Количество байт в текущем локальном буфере
	size_t local_offset; // Смещение от начала буфера
	size_t shift_offset; // Считано байт (сместить при shift())
	bool eof; // Конец файла. Это последние файлы, больше не прочитать.
} context_t;

// Записать в buffer length байт начиная с offset
// Если новый размер буфера будет меньше length, значит данные кончились (больше запросов не будет)
// Возвращает: 0 - ок, -1 - ошибка
typedef char (* getBufferFunc)(size_t offset, std::string * buffer, size_t length);

class RegExpContext
{
	private:
		std::string buffer;
		std::list<context_t> context_stack;
		context_t context;
		getBufferFunc * getFunc;
		const size_t minimal_buffer_size = CONTEXT_BUFFER_MINIMAL;
		std::map<std::string,RegExpBoxGroup &> named_base; // Именованные группы

		void replaceBuffer(size_t offset);
		void increaseBuffer(size_t length);
	public:
		RegExpContext(const std::string & buffer);
		RegExpContext(getBufferFunc * func);
		~RegExpContext();

		std::string & getChars(size_t len); // Выдает символы по текущей позиции, каждый вызов переписывает shift_offset на len
		void shift(); // Сдвигает указатель на количество прочитанных символов в getChars()
		void saveContext();
		void restoreContext();
		void deleteRestoredContext();
		bool EOF();
};

class RegExpBox
{
	protected:
		bool fOR; // if true then save/restore context by execute(), else not save
		virtual void execute(RegExpContext & context);
		virtual void compile(RegExpContext & context);
	public:
		RegExpBox();
		~RegExpBox();
};

class RegExpBoxGroup : public RegExpBox
{
	// (child)
	// <name>(child)
	// ^(child)
	private:
		bool fNOT; // if true, then 'non-coincidence' by execute()
		std::string name;
		std::list<RegExpBox> child;
		std::list<size_t> offsets; // ...[start_global_offset][end_global_offset]...
		void execute(RegExpContext & context);
		void compile(RegExpContext & context);
	public:
		RegExpBoxGroup();
		~RegExpBoxGroup();
};

class RegExpBoxString : public RegExpBox
{
	// value
	private:
		std::string value;
		void execute(RegExpContext & context);
		void compile(RegExpContext & context);
	public:
		RegExpBoxString();
		~RegExpBoxString();
};

class RegExpBoxRepeater : public RegExpBox
{
	// child{a}, a >= 0
	// child{a,b}, 0 <= a <= b
	// child{a,} , {1, } equivalent + , {0, } equivalent *
	private:
		RegExpBox child;
		void execute(RegExpContext & context);
		void compile(RegExpContext & context);
	public:
		RegExpBoxGroup();
		~RegExpBoxGroup();
};

class RegExpBoxAny : public RegExpBox
{
	// .
	private:
		void execute(RegExpContext & context);
		void compile(RegExpContext & context);
	public:
		RegExpBoxString();
		~RegExpBoxString();
};

class RegExpBoxArray : public RegExpBox
{
	// [child]
	// [child_range-child_range]
	// [^child]
	private:
		bool fNOT; // if true, then 'non-coincidence' by execute()
		std::list<std::string> child;
		std::list<std::string> child_range;
		void execute(RegExpContext & context);
		void compile(RegExpContext & context);
	public:
		RegExpBoxGroup();
		~RegExpBoxGroup();
};

}

#endif