#ifndef REGEXPBOX_H
#define REGEXPBOX_H

#include <string>
#include <list>
#include <map>

namespace fifu
{

enum
{
	minimal_buffer_size = 128,

	f_onlyText = 0x1, // Интерпритация регулярного выражения как обычного текста
};
typedef unsigned int RegExpFlags_t;

class RegExpBuffer
{
	public:
		// Записать в buffer length байт начиная с offset
		// Если новый размер буфера будет меньше length, значит данные кончились (больше запросов не будет)
		// Возвращает: true - ок, false - ошибка
		virtual bool getBuffer(size_t offset, std::string * buffer, size_t length){ return false; }

		RegExpBuffer(){}
		virtual ~RegExpBuffer(){}
};

typedef struct
{
	size_t global_offset; // Полное смещение от начала
	size_t local_buffer_size; // Количество байт в текущем локальном буфере
	size_t local_offset; // Смещение от начала буфера
	size_t shift_offset; // Считано байт (сместить при shift())
	bool eof; // Конец файла. Это последние файлы, больше не прочитать.
} context_t;

typedef struct
{
	size_t len;
	size_t global_offset_start;
} result_t;

class RegExpBoxGroup;
class RegExpContext
{
	friend class RegExpBoxGroup;
	private:
		std::string buffer;
		std::list<context_t> context_stack;
		context_t context;
		RegExpBuffer * buffers;
		std::map<std::string,RegExpBoxGroup *> named_group; // Именованные группы
		bool lastIsSucces;
		result_t result; // Общий результат
		std::map<std::string,std::list<result_t>> results; // Именованные группы, результаты
		RegExpFlags_t regexp_flags;

		void replaceBuffer(size_t offset);
		void increaseBuffer(size_t length);
		void insertResult(std::string & name, result_t result);
	public:
		RegExpContext(const std::string & buffer, RegExpFlags_t regexp_flags = 0);
		RegExpContext(RegExpBuffer * buffers, RegExpFlags_t regexp_flags = 0);
		RegExpContext();
		~RegExpContext();

		std::string getChars(size_t len); // Выдает символы по текущей позиции, каждый вызов переписывает shift_offset на len
		void shift(); // Сдвигает указатель на количество прочитанных символов в getChars()
		void saveContext(); // Сохраняет контекст в стеке
		void restoreContext(); // Восстанавливает последний контекст (но не удаляет из стека)
		void deleteRestoredContext(); // Удаляет из стека последний контекст
		bool eof();
		size_t getGlobalOffset();
		size_t getTotalOffset();
		void setSuccess();
		void setError();
		bool isSuccess();
		bool isSetOnlyText();
		result_t & getResult();
		std::list<result_t> & getResult(std::string & name);

		void dump(); //DEBUG
};

class RegExpBox
{
	protected:

	public:
		bool fOR; // if true then save/restore context by execute(), else not save

		virtual void execute(RegExpContext & context);
		virtual void compile(RegExpContext & context);
		RegExpBox();
		virtual ~RegExpBox();
};

class RegExpBoxGroup : public RegExpBox
{
	// (child)
	// <name>(child)
	// ^(child)
	private:
		bool fNOT; // if true, then 'non-coincidence' by execute()
		std::string name;
		std::list<RegExpBox *> child;
		//std::list<size_t> offsets; // ...[start_global_offset][end_global_offset]... бинарник не должен хранить у себя промежуточные данные
	public:
		void execute(RegExpContext & context);
		void compile(RegExpContext & context);
		RegExpBoxGroup();
		~RegExpBoxGroup();
};

class RegExpBoxString : public RegExpBox
{
	// value
	private:
		std::string value;
	public:
		void execute(RegExpContext & context);
		void compile(RegExpContext & context);
		RegExpBoxString();
		~RegExpBoxString();
};
/*
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
		RegExpBoxRepeater();
		~RegExpBoxRepeater();
};

class RegExpBoxAny : public RegExpBox
{
	// .
	private:
		void execute(RegExpContext & context);
		void compile(RegExpContext & context);
	public:
		RegExpBoxAny();
		~RegExpBoxAny();
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
		RegExpBoxArray();
		~RegExpBoxArray();
};
*/
}

#endif