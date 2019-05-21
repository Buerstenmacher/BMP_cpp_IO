#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include <iostream>
#include <iomanip>
#include "bmp.h"

//compile and run with:
//g++ test.cpp -o test -std=c++11 -Wno-psabi -Weffc++ -Wall -pedantic -pthread -Os && ./test



int main() {

BMP bmp0("sample.bmp");
bmp0.print();


printf("Generating sample.bmp. . .\n");
BMP bmp(80, 80, "sample.bmp");
bmp.print();
bmp.line(Point(8, 8, 0), Point(52, 8, 0));
bmp.print();
bmp.line(Point(8, 8, 0), Point(8, 52, 0));
bmp.print();
bmp.line(Point(8, 52, 0), Point(52, 52, 0));
bmp.print();
bmp.line(Point(52, 8, 0), Point(52, 52, 0));
bmp.print();
bmp.line(Point(23, 23, 0), Point(23, 67, 0));
bmp.print();
bmp.line(Point(23, 23, 0), Point(67, 23, 0));
bmp.print();
bmp.line(Point(23, 67, 0), Point(67, 67, 0));
bmp.print();
bmp.line(Point(67, 23, 0), Point(67, 67, 0));
bmp.print();
bmp.fill(Point(18, 18, 0xFF0000));
bmp.print();
bmp.fill(Point(40, 40, 0xFFF200));
bmp.print();
bmp.fill(Point(60, 60, RGB(181, 230, 29)));
bmp.print();
bmp.circle(Point(23, 52, 0x000000), 10);
bmp.print();
bmp.write();

BMP bmp2("e.bmp");
bmp2.print();
bmp2.write();

BMP bmp3("cat.bmp");
bmp3.print();
}
