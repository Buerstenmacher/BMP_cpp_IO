#ifndef BMP_H
#define BMP_H

#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <iostream>
#include <vector>
#include <unistd.h>

class terminalWindow {	//special for linux
struct winsize w;
void refresh(void) {ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);}
public:
terminalWindow(void):w{} {refresh();}
void print(void) {std::cout << "This window has: "<< w.ws_row << " rows, and  " << w.ws_col << " columns. " <<std::endl;}
uint32_t rows(void) {refresh(); return w.ws_row;}
uint32_t columns(void) {refresh(); return w.ws_col;}
void clrscr(void) 	{for(uint32_t i{0};i<=rows();++i)	{std::cout << std::endl;}}
};//class terminalWindow

class RGB {
uint8_t _bl,_gr,_re;
public:
RGB(uint32_t rgb):_bl{uint8_t((rgb & 0x0000FF) >> 0)},_gr{uint8_t((rgb & 0x00FF00) >> 8)},_re{uint8_t((rgb & 0xFF0000) >> 16)} {}
RGB(uint8_t r, uint8_t g, uint8_t b):_bl{b},_gr{g},_re{r} {}
RGB(void):_bl{255},_gr{255},_re{255} {}	//default is white
bool operator==(const RGB& b) const {return (_bl==b.blue() && _gr==b.green() && _re==b.red())?true:false;}
const uint8_t& blue(void) const 	{return _bl;}
const uint8_t& green(void) const 	{return _gr;}
const uint8_t& red(void) const		{return _re;}
uint8_t& blue(void) 	{return _bl;}
uint8_t& green(void) 	{return _gr;}
uint8_t& red(void) 	{return _re;}
};//class RGB

class Point {
uint32_t _x,_y;
RGB _color;
public:
uint32_t& x(void)	{return _x;}
uint32_t& y(void)	{return _y;}
RGB& color(void)	{return _color;}
Point(uint32_t x, uint32_t y, RGB rgb=RGB{}) : _x(x), _y(y), _color(rgb) {}
Point():_x{0},_y{0},_color{} {}
uint8_t operator <(const Point& r ) const {return ((_x<r._x && _y<r._y && _y>=0 && _x>=0)?1:0);}
};//class Point

class BMP_header {		// BMP header's size is fixed, which equals to 14 byte
std::string head{"BM"};		// 2 char
uint32_t file_size{0};		// 4 byte
uint32_t reserved_field{0};	// 4 byte
uint32_t starting_addr{54};	// 4 byte
uint32_t DIB_header_size{40};	// 4 byte
	struct DIB_header {		// DIB header 40 byte in size
	uint32_t header_size{40}; 	// DIB_header's size is determined by its type. 0x0E/14, 32 bits.
	int32_t width{0}; 		// the bitmap width in pixels (signed integer). 0x12/18, 32 bits.
	int32_t height{0}; 		// the bitmap height in pixels (signed integer). 0x16/22, 32 bits.
	uint16_t color_planes{1}; 	// the number of color planes (must be 1). 0x1A/26, 16 bits.
	uint16_t bits_per_pixel{24}; 	// the number of bits per pixel, which is the color depth of the image. Typical values are 1, 4, 8, 16, 24 and 32. 0x1C/28, 16 bits.
	uint32_t compression_method{0};	// the compression method being used. 0x1E/30, 32 bits.
	uint32_t raw_data_size{0}; 	// the image size. This is the size of the raw bitmap data; a dummy 0 can be given for BI_RGB bitmaps. 0x22/34, 32 bits.
	int32_t h_reso{5669}; 		// the horizontal resolution of the image. (pixel per metre, signed integer) 0x26/38, 32 bits.
	int32_t v_reso{5669}; 		// the vertical resolution of the image. (pixel per metre, signed integer) 0x2A/42, 32 bits.
	uint32_t colors{0}; 		// the number of colors in the color palette, or 0 to default to 2^n. 0x2E/46, 32 bits.
	uint32_t important_colors{0}; 	// the number of important colors used, or 0 when every color is important; generally ignored. 0x32/50, 32 bits.
	DIB_header(void) {}
	};//struct DIB_header
DIB_header DIB;			// + 40 Byte

public:
void correct_size(void) {
DIB.raw_data_size = height() * row_size();
file_size = 54 + DIB.raw_data_size;
}

operator bool() {return (head=="BM" || head=="BA" || head=="CI" || head=="CP" || head=="IC" || head=="PT")?true:false;}

void write_header(FILE* bmpfile) {
fwrite(&head.at(0), 1, 1, bmpfile);
fwrite(&head.at(1), 1, 1, bmpfile);
fwrite(&file_size, 4, 1, bmpfile);
fwrite(&reserved_field, 4, 1, bmpfile);
fwrite(&starting_addr, 4, 1, bmpfile);
fwrite(&DIB_header_size, 4, 1, bmpfile);
fwrite(&DIB.width, 4, 1, bmpfile);
fwrite(&DIB.height, 4, 1, bmpfile);
fwrite(&DIB.color_planes, 2, 1, bmpfile);
fwrite(&DIB.bits_per_pixel, 2, 1, bmpfile);
fwrite(&DIB.compression_method, 4, 1, bmpfile);
fwrite(&DIB.raw_data_size, 4, 1, bmpfile);
fwrite(&DIB.h_reso, 4, 1, bmpfile);
fwrite(&DIB.v_reso, 4, 1, bmpfile);
fwrite(&DIB.colors, 4, 1, bmpfile);
fwrite(&DIB.important_colors, 4, 1, bmpfile);
}

void read_header(FILE* bmpfile) {
fread(&head.at(0), 1, 1, bmpfile);
fread(&head.at(1), 1, 1, bmpfile);
fread(&file_size, 4, 1, bmpfile);
fread(&reserved_field, 4, 1, bmpfile);
fread(&starting_addr, 4, 1, bmpfile);
fread(&DIB_header_size, 4, 1, bmpfile);
if (DIB_header_size == 40) {
	DIB.header_size = DIB_header_size;
	fread(&DIB.width, 4, 1, bmpfile);
	fread(&DIB.height, 4, 1, bmpfile);
	fread(&DIB.color_planes, 2, 1, bmpfile);
	fread(&DIB.bits_per_pixel, 2, 1, bmpfile);
	fread(&DIB.compression_method, 4, 1, bmpfile);
	fread(&DIB.raw_data_size, 4, 1, bmpfile);
	fread(&DIB.h_reso, 4, 1, bmpfile);
	fread(&DIB.v_reso, 4, 1, bmpfile);
	fread(&DIB.colors, 4, 1, bmpfile);
	fread(&DIB.important_colors, 4, 1, bmpfile);
	}
rewind(bmpfile);
}

BMP_header(FILE* bmpfile):DIB{} {read_header(bmpfile);}
BMP_header(void):DIB{} {}

uint32_t get_row_size(void) const 	{return ((DIB.bits_per_pixel*DIB.width + 31) / 32) * 4;}//every row has to have a multiple of 4 bytes
uint32_t get_data_size(void) const 	{return get_row_size()*abs(DIB.height);}
int32_t height(void) const		{return DIB.height;}
int32_t width(void) const		{return DIB.width;}
uint32_t start_addr(void) const	{return starting_addr;}
uint32_t row_size(void) const 	{return ((24 * width() + 31) / 32) * 4;}
int32_t& height(void)		{return DIB.height;}
int32_t& width(void)		{return DIB.width;}
};//class BMP_header

class BMP final{
FILE* bmpfile;
BMP_header header;
std::vector<RGB> data;

uint32_t row_size(void) const 	{return header.row_size();}
uint32_t width(void) const	{return header.width();}
uint32_t height(void) const	{return header.height();}

void fill_(Point p, const RGB& origin) {
Point max{width(),height()};
if (p<max	&& 	at(p.y(),p.x()) == origin) {
	this->put_pixel(p);
	Point up, down, left, right;
	up = down = left = right =p;
	up.y()--;
	down.y()++;
	left.x()--;
	right.x()++;
	if (up < max	&& 	at(up.y(),up.x())	== origin)	{fill_(up, origin);}
	if (down < max	&&	at(down.y(),down.x()) 	== origin) 	{fill_(down, origin);}
	if (left < max	&& 	at(left.y(),left.x()) 	== origin) 	{fill_(left, origin);}
	if (right < max &&	at(right.y(),right.x())	== origin) 	{fill_(right, origin);}
	}
}

public:
RGB& at(int32_t index)			{return this->data.at(index);}
RGB& at(int32_t hei,int32_t wid)	{return this->data.at(hei*width()+wid);}
const RGB& at(int32_t index) const			{return at(index);}
const RGB& at(int32_t hei,int32_t wid) const		{return at(hei*width()+wid);}

BMP(int32_t w, int32_t h, const char* path):bmpfile{fopen(path, "wb")},header{},data{} {
header.width() = w;
header.height() = h;
header.correct_size();
data.resize(w*h);
}

BMP(const char* path):bmpfile{fopen(path, "rb")},header{bmpfile},data{} {
if (!header) {
	fprintf(stderr, "This file is not a bmp file!");
	exit(1);
	}
int32_t start = header.start_addr() + row_size()*(header.height() - 1);
fseek(bmpfile, start, SEEK_SET);
data.resize(width()*height());
for (uint32_t i{0}; i < height(); i++) {
	for (uint32_t j{0}; j < width(); j++) {fread(&(at(i,j)), 3, 1, bmpfile);}
	fseek(bmpfile, start - row_size()*(i + 1), SEEK_SET);
	}
}

~BMP() {fclose(bmpfile);}		//close file on destruction
BMP(const BMP&) = delete;		//no copy
BMP operator=(const BMP&)=delete;	//no copy

/*void print() {
for (uint32_t i{0}; i<height(); i++) {
	for (uint32_t j{0}; j<width(); j++) {printf("\033[48;2;%d;%d;%dm  \033[0m",at(i,j).red(),at(i,j).green(),at(i,j).blue());}
	std::cout << std::endl;
	}
}*/

void print() {
terminalWindow t{};
uint32_t col = t.columns()/2;
uint32_t row = t.rows()-1;
t.clrscr();
for (uint32_t r{0};r<row;r++) {
	for (uint32_t c{0};c<col;++c) {
		uint32_t colpos =	uint32_t(c*width()/col);
		uint32_t rowpos =	uint32_t(r*height()/row);
		printf("\033[48;2;%d;%d;%dm  \033[0m",at(rowpos,colpos).red(),at(rowpos,colpos).green(),at(rowpos,colpos).blue());
		}
	std::cout << std::endl;
	}
}


void write() {
header.write_header(bmpfile);
for (int64_t i = height() - 1; i >= 0; i--) {		// raw data!!
	for (uint32_t j {0}; j < width(); j++) {fwrite(&(at(i,j)), 3, 1, bmpfile);}
	for (uint32_t k {0}; k < (row_size() - width() * 3); k++) {fputc('\0', bmpfile);}
	}// eof
}

void put_pixel(Point p) {if (p < Point{width(),height()}) {at(p.y(),p.x()) = p.color();}}

void put_pixel(uint32_t x, uint32_t y, RGB color = RGB{}) {if (Point{x,y} < Point{width(),height()}) {at(y,x) = color;}}

void line(Point p1, Point p2) {
double dx = p2.x() - p1.x();
double dy = p2.y() - p1.y();
double dr = p2.color().red() - p1.color().red();
double dg = p2.color().green() - p1.color().green();
double db = p2.color().blue() - p1.color().blue();
uint32_t step = uint32_t(std::max(dx,dy));
dx /= step;
dy /= step;
dr /= step;
dg /= step;
db /= step;
double x = p1.x();
double y = p1.y();
double r = p1.color().red();
double g = p1.color().green();
double b = p1.color().blue();
for (uint32_t i{0}; i <= step; i++) {
	put_pixel(x, y, RGB(r, g, b));
	x += dx;
	y += dy;
	r += dr;
	g += dg;
	b += db;
	}
}

void circle(Point p, int32_t radius) {
int32_t x = radius - 1;
int32_t y = 0;
int32_t dx = 1;
int32_t dy = 1;
int32_t err = dx - (radius << 1);
while (x >= y)	{
	this->put_pixel(p.x() + x, p.y() + y, p.color());
	this->put_pixel(p.x() + y, p.y() + x, p.color());
	this->put_pixel(p.x() - y, p.y() + x, p.color());
	this->put_pixel(p.x() - x, p.y() + y, p.color());
	this->put_pixel(p.x() - x, p.y() - y, p.color());
	this->put_pixel(p.x() - y, p.y() - x, p.color());
	this->put_pixel(p.x() + y, p.y() - x, p.color());
	this->put_pixel(p.x() + x, p.y() - y, p.color());
	if (err <= 0) {
		y++;
		err += dy;
		dy += 2;
		}
	if (err > 0) {
		x--;
		dx += 2;
		err += dx - (radius << 1);
		}
	}
}

void fill(Point p) {
if (p.x() < width() && p.y() < height()) {
	RGB origin = at(p.y(),p.x());
	fill_(p, origin);
	}
}

};//class BMP

#endif // !BMP_H
