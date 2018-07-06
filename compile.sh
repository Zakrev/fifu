#!/bin/bash

SOURCES_NAMES="source/fifu.cpp source/searchtext.cpp"

g++ -Wall -o main main.cpp $SOURCES_NAMES -lpthread -Lsource -lpcre -std=c++11
