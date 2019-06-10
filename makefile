.PHONY: main
main:main.cpp
	g++  -g -std=c++14 $^ -o $@ -lpthread
