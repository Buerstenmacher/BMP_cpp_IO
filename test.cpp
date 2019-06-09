#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include <iostream>
#include <iomanip>
#include "bmp.h"

//you shuld be on an linx computer with a terminal that has truecolor capabilities
//ubuntu mate with mate terminal is recommendet
//compile with:
//	g++ test.cpp -o test -std=c++11 -Wno-psabi -Weffc++ -Wall -pedantic -pthread -Os
//set your terminal to a very small font size and maximize the window
//run with:
//	./test cat.bmp
//if it works you will see an image of an cat on a couch

std::vector<std::string> commandl_args(int argc, char* argv[]) {//helps to deal with comand line parameters
std::vector<std::string> ret{};
for (int32_t i{0};i<argc;i++) {ret.push_back(std::string(argv[i]));}
return ret;
}

int main (int argc, char* argv[]) {
auto commands{commandl_args(argc,argv)};
std::string key (".bmp");
while (commands.size()<2) {commands.push_back("error");}
std::string com{commands.at(1)};
std::size_t found = com.find(key);
if (com == "demo") {
	demo();
	}
else if (com == "window") {
//	window();
	}
else if (found!=std::string::npos) {
	BMP bmp2(com);
	bmp2.print();
	}
else{
	std::cout << "Yo should run this program with one parameter containing the name of a bmp-file or \"demo\"!";
	std::cout << std::endl;
	}
}//main
