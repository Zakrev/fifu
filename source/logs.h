#ifndef LOGS_H
#define LOGS_H

#include <iostream>
/*
static void logs_print_log()
{
	cerr << endl;
}
*/
template <typename T> static void logs_print_log(const T& t)
{
	std::cerr << t << std::endl;
}

template <typename First, typename... Rest> static void logs_print_log(const First& first, const Rest&... rest)
{
	std::cerr << first;
	logs_print_log(rest...);
}

#define LOG_DBG(...)
	//logs_print_log("DBG<", __FUNCTION__, "><", __LINE__, ">: ", ## __VA_ARGS__ )

#define LOG_ERR(...)
	//logs_print_log("ERR<", __FUNCTION__, "><", __LINE__, ">: ", ## __VA_ARGS__ )

#endif
