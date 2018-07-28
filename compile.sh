#!/bin/bash

case "$1" in
	fifu)
		SOURCES_NAMES="source/searchtext.cpp source/filesystem.cpp source/regexp.cpp source/regexpbox.cpp"
		g++ -Wall -o main fifu.cpp $SOURCES_NAMES -lpthread -Lsource -lpcre -std=c++11
		;;
	regexp)
		SOURCES_NAMES="source/regexp.cpp source/regexpbox.cpp"
		g++ -Wall -o main regexp.cpp $SOURCES_NAMES -Lsource -std=c++11
		;;
	*)
		exit 1;
		;;
esac
