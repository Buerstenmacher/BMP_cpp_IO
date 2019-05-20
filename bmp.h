#ifndef BMP_H
#define BMP_H

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>

class RGB {
uint8_t _bl,_gr,_re;
public:
RGB(uint32_t rgb):_bl{uint8_t((rgb & 0x0000FF) >> 0)},_gr{uint8_t((rgb & 0x00FF00) >> 8)},_re{uint8_t((rgb & 0xFF0000) >> 16)} {}
RGB(uint8_t r, uint8_t g, uint8_t b):_bl{b},_gr{g},_re{r} {}
RGB(void):_bl{255},_gr{255},_re{255} {}	//default is white
bool operator==(const RGB& b) {return (_bl==b.blue() && _gr==b.green() && _re==b.red())?true:false;}
const uint8_t& blue(void) const 	{return _bl;}
const uint8_t& green(void) const 	{return _gr;}
const uint8_t& red(void) const		{return _re;}
uint8_t& blue(void) 	{return _bl;}
uint8_t& green(void) 	{return _gr;}
uint8_t& red(void) 	{return _re;}
};//class RGB

struct Point {
uint32_t x,y;
RGB color;
Point(uint32_t x, uint32_t y, RGB rgb) : x(x), y(y), color(rgb) {}
Point():x{0},y{0},color{} {}
};//struct Point

struct DIB_header {		// DIB header 40 byte in size
uint32_t header_size = 40; 	// DIB_header's size is determined by its type. 0x0E/14, 32 bits.
int32_t width{0}; 		// the bitmap width in pixels (signed integer). 0x12/18, 32 bits.
int32_t height{0}; 		// the bitmap height in pixels (signed integer). 0x16/22, 32 bits.
uint16_t color_planes = 1; 	// the number of color planes (must be 1). 0x1A/26, 16 bits.
uint16_t bits_per_pixel = 24; 	// the number of bits per pixel, which is the color depth of the image. Typical values are 1, 4, 8, 16, 24 and 32. 0x1C/28, 16 bits.
uint32_t compression_method = 0;// the compression method being used. 0x1E/30, 32 bits.
uint32_t raw_data_size = 0; 	// the image size. This is the size of the raw bitmap data; a dummy 0 can be given for BI_RGB bitmaps. 0x22/34, 32 bits.
int32_t h_reso = 5669; 		// the horizontal resolution of the image. (pixel per metre, signed integer) 0x26/38, 32 bits.
int32_t v_reso = 5669; 		// the vertical resolution of the image. (pixel per metre, signed integer) 0x2A/42, 32 bits.
uint32_t colors = 0; 		// the number of colors in the color palette, or 0 to default to 2^n. 0x2E/46, 32 bits.
uint32_t important_colors = 0; 	// the number of important colors used, or 0 when every color is important; generally ignored. 0x32/50, 32 bits.
DIB_header(void) {}
};//struct DIB_header

class BMP_header {		// BMP header's size is fixed, which equals to 14 byte
public:
std::string head{"BM"};
uint32_t file_size{0};
uint32_t reserved_field = 0;
uint32_t starting_addr = 54;
uint32_t DIB_header_size = 40;
DIB_header DIB;

operator bool() {return (head=="BM" || head=="BA" || head=="CI" || head=="CP" || head=="IC" || head=="PT")?true:false;}

BMP_header(void):DIB{} {}

BMP_header(FILE* bmpfile):DIB{} {
BMP_header ret{};
char tmp;
fread(&tmp, 1, 1, bmpfile);
ret.head.at(0) = tmp;
fread(&tmp, 1, 1, bmpfile);
ret.head.at(1) = tmp;
fread(&ret.file_size, 4, 1, bmpfile);
fread(&ret.reserved_field, 4, 1, bmpfile);
fread(&ret.starting_addr, 4, 1, bmpfile);
fread(&ret.DIB_header_size, 4, 1, bmpfile);
if (ret.DIB_header_size == 40) {
	ret.DIB.header_size = ret.DIB_header_size;
	fread(&ret.DIB.width, 4, 1, bmpfile);
	fread(&ret.DIB.height, 4, 1, bmpfile);
	fread(&ret.DIB.color_planes, 2, 1, bmpfile);
	fread(&ret.DIB.bits_per_pixel, 2, 1, bmpfile);
	fread(&ret.DIB.compression_method, 4, 1, bmpfile);
	fread(&ret.DIB.raw_data_size, 4, 1, bmpfile);
	fread(&ret.DIB.h_reso, 4, 1, bmpfile);
	fread(&ret.DIB.v_reso, 4, 1, bmpfile);
	fread(&ret.DIB.colors, 4, 1, bmpfile);
	fread(&ret.DIB.important_colors, 4, 1, bmpfile);
	}
rewind(bmpfile);
(*this) = ret;
}

uint32_t get_row_size(void) 	{return ((DIB.bits_per_pixel * DIB.width + 31) / 32) * 4;}
uint32_t get_data_size(void) 	{return get_row_size()*abs(DIB.height);}

};//class BMP_header


class BMP{
FILE* bmpfile;
BMP_header header;
uint32_t row_size(void) 	{return ((24 * width() + 31) / 32) * 4;}
uint32_t width(void) 		{return header.DIB.width;}
uint32_t height(void) 		{return header.DIB.height;}
std::vector<RGB> data;

void fill_(Point p, const RGB& origin) {
if (p.x < width() && p.y < height() && data[p.y * width() + p.x] == origin) {
	this->put_pixel(p);
	Point up, down, left, right;
	up = down = left = right =p;
	up.y--;
	down.y++;
	left.x--;
	right.x++;
	if (up.x < width() && up.y < height() && up.y >= 0 && up.x >= 0 && data[up.y * width() + up.x] 			== origin)	{fill_(up, origin);}
	if (down.x < width() && down.y < height() && down.y >= 0 && down.x >= 0 && data[down.y * width() + down.x] 	== origin) 	{fill_(down, origin);}
	if (left.x < width() && left.y < height() && left.y >= 0 && left.x >= 0 && data[left.y * width() + left.x] 	== origin) 	{fill_(left, origin);}
	if (right.x < width() && right.y < height() && right.y >= 0 && right.x >= 0 && data[right.y * width() + right.x]== origin) 	{fill_(right, origin);}
	}
}

public:
BMP(int32_t w, int32_t h, const char* path):bmpfile{fopen(path, "wb")},header{},data{} {
header.DIB.width = w;
header.DIB.height = h;
header.DIB.raw_data_size = h * row_size();
header.file_size = 54 + header.DIB.raw_data_size;
data.resize(w*h);
}

BMP(const char* path):bmpfile{fopen(path, "rb")},header{bmpfile},data{} {
if (!header) {
	fprintf(stderr, "This file is not a bmp file!");
	exit(1);
	}
int32_t start = header.starting_addr + row_size()*(header.DIB.height - 1);
fseek(bmpfile, start, SEEK_SET);
data.resize(width()*height());
for (uint32_t i{0}; i < height(); i++) {
	for (uint32_t j{0}; j < width(); j++) {fread(&(data[i*width()+j]), 3, 1, bmpfile);}
	fseek(bmpfile, start - row_size()*(i + 1), SEEK_SET);
	}
}

~BMP() {fclose(bmpfile);}

BMP(const BMP&) = delete;		//no copy
BMP operator=(const BMP&)=delete;	//no copy

void print() {
for (uint32_t i{0}; i<height(); i++) {
	for (uint32_t j{0}; j<width(); j++) {printf("\033[48;2;%d;%d;%dm  \033[0m", data[i*width() + j].red(), data[i*width() + j].green(), data[i*width() + j].blue());}
	std::cout << std::endl;
	}
}

void write() {	// header
fwrite(&header.head.at(0), 1, 1, bmpfile);
fwrite(&header.head.at(1), 1, 1, bmpfile);
fwrite(&header.file_size, 4, 1, bmpfile);
fwrite(&header.reserved_field, 4, 1, bmpfile);
fwrite(&header.starting_addr, 4, 1, bmpfile);
fwrite(&header.DIB_header_size, 4, 1, bmpfile);
fwrite(&header.DIB.width, 4, 1, bmpfile);
fwrite(&header.DIB.height, 4, 1, bmpfile);
fwrite(&header.DIB.color_planes, 2, 1, bmpfile);
fwrite(&header.DIB.bits_per_pixel, 2, 1, bmpfile);
fwrite(&header.DIB.compression_method, 4, 1, bmpfile);
fwrite(&header.DIB.raw_data_size, 4, 1, bmpfile);
fwrite(&header.DIB.h_reso, 4, 1, bmpfile);
fwrite(&header.DIB.v_reso, 4, 1, bmpfile);
fwrite(&header.DIB.colors, 4, 1, bmpfile);
fwrite(&header.DIB.important_colors, 4, 1, bmpfile);
for (int64_t i = height() - 1; i >= 0; i--) {		// raw data!!
	for (uint32_t j {0}; j < width(); j++) {fwrite(&(data[i*width() + j]), 3, 1, bmpfile);}
	for (uint32_t k {0}; k < (row_size() - width() * 3); k++) {fputc('\0', bmpfile);}
	}// eof
}

RGB* operator[](int32_t sub){return &data[sub*width()];}

void put_pixel(Point p) {if (p.x < width() && p.y < height() && p.x >= 0 && p.y >= 0) {data[p.x + p.y*width()] = p.color;}}

void put_pixel(uint32_t x, uint32_t y, RGB color = RGB()) {if (x < width() && y < height() && x >= 0 && y >= 0) {data[x + y*width()] = color;}}

void line(Point p1, Point p2) {
double dx = p2.x - p1.x;
double dy = p2.y - p1.y;
double dr = p2.color.red() - p1.color.red();
double dg = p2.color.green() - p1.color.green();
double db = p2.color.blue() - p1.color.blue();
int32_t step = int((abs(dx) >= abs(dy)) ? abs(dx) : abs(dy));
dx /= step;
dy /= step;
dr /= step;
dg /= step;
db /= step;
double x = p1.x;
double y = p1.y;
double r = p1.color.red();
double g = p1.color.green();
double b = p1.color.blue();
for (int32_t i = 0; i <= step; i++) {
	put_pixel(x, y, RGB(r, g, b));
	x = x + dx;
	y = y + dy;
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
	this->put_pixel(p.x + x, p.y + y, p.color);
	this->put_pixel(p.x + y, p.y + x, p.color);
	this->put_pixel(p.x - y, p.y + x, p.color);
	this->put_pixel(p.x - x, p.y + y, p.color);
	this->put_pixel(p.x - x, p.y - y, p.color);
	this->put_pixel(p.x - y, p.y - x, p.color);
	this->put_pixel(p.x + y, p.y - x, p.color);
	this->put_pixel(p.x + x, p.y - y, p.color);
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
if (p.x < width() && p.y < height()) {
	RGB origin = (*this)[p.y][p.x];
	fill_(p, origin);
	}
}

};//class BMP

#endif // !BMP_H
