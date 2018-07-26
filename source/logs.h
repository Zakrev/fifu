#ifndef LOGS_H
#define LOGS_H

#define LERROR 1

#ifdef LOGS_H_LOG_ENABLED

#include <iostream>

template <typename T> static void logs_print_log(const T& t)
{
	std::cerr << t << std::endl;
}

template <typename First, typename... Rest> static void logs_print_log(const First& first, const Rest&... rest)
{
	std::cerr << first;
	logs_print_log(rest...);
}

#define log(level, ...) \
	if (level) {logs_print_log("\t<", __FUNCTION__, "><", __LINE__, "><" #level ">\n", ## __VA_ARGS__ );}

#else //LOGS_H_LOG_ENABLED
#define log(...)
#endif

#endif //LOGS_H
