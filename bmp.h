#ifndef BMP_H
#define BMP_H

#include <stdio.h>
#include <string.h>	//stl
#include <iostream>	//stl
#include <vector>	//stl
#include <ncurses.h> 	//sudo apt-get install libncurses5-dev libncursesw5-dev
			//compile with -lncurses flag!
			//ncurses will work on linux only

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
class ncurses_window {
private:

class _WIN_struct {	//nested class
public:
int32_t height, width, startx, starty;

class _win_border_struct {	//nested nested class
public:
char ls, rs, ts, bs, tl, tr, bl, br;

_win_border_struct(void):ls{'|'},rs{'|'},ts{'-'},bs{'-'},tl{'+'},tr{'+'},bl{'+'},br{'+'} {}
}  border;

_WIN_struct(void):height{10},width{10},startx {0},starty{0},border{} {}
} window;

void print_win_params() {
#ifdef _DEBUG
mvprintw(25, 0, "%d %d %d %d", window.startx, window.starty,window.width, window.height);
refresh();
#endif
}

void create_box(bool flag) {
int i, j;
int x, y, w, h;
x = window.startx;
y = window.starty;
w = window.width;
h = window.height;
if(flag == TRUE) {
	mvaddch(y, x, window.border.tl);
	mvaddch(y, x + w, window.border.tr);
	mvaddch(y + h, x, window.border.bl);
	mvaddch(y + h, x + w, window.border.br);
	mvhline(y, x + 1, window.border.ts, w - 1);
	mvhline(y + h, x + 1, window.border.bs, w - 1);
	mvvline(y + 1, x, window.border.ls, h - 1);
	mvvline(y + 1, x + w, window.border.rs, h - 1);
	}
else	{
	for(j = y; j <= y + h; ++j) {
		for(i = x; i <= x + w; ++i) {mvaddch(j, i, ' ');}
		}
	}
refresh();
}

public:
uint32_t terminalheight(void) 	{return LINES;}
uint32_t terminalwidth(void)	{return COLS;}
uint32_t row(void) 	{return LINES;}
uint32_t col(void)	{return COLS;}


ncurses_window(std::string name = "Press ESC to exit"):window{} {
initscr();			// Start curses mode
cbreak();			// Line buffering disabled, Pass on everty thing to me
keypad(stdscr, TRUE);    	// I need that nifty F1
noecho();
print_win_params();
printw(name.c_str());
refresh();
window.startx=0;
window.starty=1;
window.width = terminalwidth() - 1;  	//reserve one column for borders
window.height = terminalheight() -3;	//
create_box(TRUE);
//char ch;
//while((ch = getch()) != 27) {}  //until ESC is pressed
}

~ncurses_window() {
create_box(FALSE);
endwin();
}

};	//class ncurses_window
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////
class RGB {
uint8_t _bl,_gr,_re;
public:
RGB(uint32_t rgb):_bl{uint8_t((rgb & 0x0000FF) >> 0)},_gr{uint8_t((rgb & 0x00FF00) >> 8)},_re{uint8_t((rgb & 0xFF0000) >> 16)} {}
RGB(uint8_t r, uint8_t g, uint8_t b):_bl{b},_gr{g},_re{r} {}
RGB(void):_bl{100},_gr{100},_re{100} {}	//default is gray
bool operator==(const RGB& b) const {return (_bl==b.blue() && _gr==b.green() && _re==b.red())?true:false;}
uint8_t blue(void) const 	{return _bl;}
uint8_t green(void) const 	{return _gr;}
uint8_t red(void) const		{return _re;}
uint8_t& blue(void) 	{return _bl;}
uint8_t& green(void) 	{return _gr;}
uint8_t& red(void) 	{return _re;}
};//class RGB
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
class Point {
int32_t _x,_y;
//RGB _color;
public:
int32_t& x(void)	{return _x;}
int32_t& y(void)	{return _y;}
//RGB& color(void)	{return _color;}
Point(int32_t x, int32_t y/*, RGB rgb=RGB{}*/) : _x(x), _y(y)/*, _color(rgb)*/ {}
Point():_x{0},_y{0}/*,_color{}*/ {}
uint8_t operator <(const Point& r ) const {return ((_x<r._x && _y<r._y && _y>=0 && _x>=0)?1:0);}
};//class Point
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
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
void update_header(void) {
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
if (fread(&head.at(0), 1, 1, bmpfile)!=1)	{throw std::runtime_error("File read was not successful");}
if (fread(&head.at(1), 1, 1, bmpfile)!=1)	{throw std::runtime_error("File read was not successful");}
if (fread(&file_size, 4, 1, bmpfile)!=1)	{throw std::runtime_error("File read was not successful");}
if (fread(&reserved_field, 4, 1, bmpfile)!=1)	{throw std::runtime_error("File read was not successful");}
if (fread(&starting_addr, 4, 1, bmpfile)!=1)	{throw std::runtime_error("File read was not successful");}
if (fread(&DIB_header_size, 4, 1, bmpfile)!=1)	{throw std::runtime_error("File read was not successful");}
if (DIB_header_size == 40) {
	DIB.header_size = DIB_header_size;
	if (fread(&DIB.width, 4, 1, bmpfile)!=1)		{throw std::runtime_error("File read was not successful");}
	if (fread(&DIB.height, 4, 1, bmpfile)!=1)		{throw std::runtime_error("File read was not successful");}
	if (fread(&DIB.color_planes, 2, 1, bmpfile)!=1)		{throw std::runtime_error("File read was not successful");}
	if (fread(&DIB.bits_per_pixel, 2, 1, bmpfile)!=1)	{throw std::runtime_error("File read was not successful");}
	if (fread(&DIB.compression_method, 4, 1, bmpfile)!=1)	{throw std::runtime_error("File read was not successful");}
	if (fread(&DIB.raw_data_size, 4, 1, bmpfile)!=1)	{throw std::runtime_error("File read was not successful");}
	if (fread(&DIB.h_reso, 4, 1, bmpfile)!=1)		{throw std::runtime_error("File read was not successful");}
	if (fread(&DIB.v_reso, 4, 1, bmpfile)!=1)		{throw std::runtime_error("File read was not successful");}
	if (fread(&DIB.colors, 4, 1, bmpfile)!=1)		{throw std::runtime_error("File read was not successful");}
	if (fread(&DIB.important_colors, 4, 1, bmpfile)!=1)	{throw std::runtime_error("File read was not successful");}
	}
rewind(bmpfile);
}

BMP_header(FILE* bmpfile):DIB{} {read_header(bmpfile);}
BMP_header(void):DIB{} {}

int32_t get_row_size(void) const 	{return ((DIB.bits_per_pixel*DIB.width + 31) / 32) * 4;}//every row has to have a multiple of 4 bytes
int32_t row_size(void) 	const 		{return get_row_size();}
int32_t get_data_size(void) const 	{return get_row_size()*abs(DIB.height);}
int32_t height(void) const		{return DIB.height;}
int32_t width(void) const		{return DIB.width;}
uint32_t start_addr(void) const	{return starting_addr;}
int32_t& height(void)		{return DIB.height;}
int32_t& width(void)		{return DIB.width;}
};//class BMP_header
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
class BMP final{
std::string filename;
FILE* bmpfile;
BMP_header header;
std::vector<RGB> data;

void fill_(Point p,const RGB& color, const RGB& ori_col) {
Point max{width(),height()};
if (p<max	&& 	at(p.y(),p.x()) == ori_col) {
	this->put_pixel(p,color);
	Point up, down, left, right;
	up = down = left = right =p;
	up.y()--;
	down.y()++;
	left.x()--;
	right.x()++;
	if (up < max	&& 	at(up.y(),up.x())	== ori_col)	{fill_(up,color, ori_col);}
	if (down < max	&&	at(down.y(),down.x()) 	== ori_col) 	{fill_(down,color, ori_col);}
	if (left < max	&& 	at(left.y(),left.x()) 	== ori_col) 	{fill_(left,color, ori_col);}
	if (right < max &&	at(right.y(),right.x())	== ori_col) 	{fill_(right,color, ori_col);}
	}
}

uint32_t get_terminal_columns(void) {
ncurses_window w1{};
return w1.col();
}

uint32_t get_terminal_rows(void) {
ncurses_window w1{};
return w1.row();
}

void horizontal_line(void) {
auto columns{get_terminal_columns()};
for (uint32_t i{0};i<columns;i++) {std::cout << '-';}
}

public:
int32_t row_size(void) const 	{return header.row_size();}
int32_t width(void) const	{return int32_t(header.width());}
int32_t height(void) const	{return int32_t(header.height());}
RGB& at(int32_t index)			{return this->data.at(index);}
RGB& at(int32_t hei,int32_t wid)	{return this->data.at(hei*width()+wid);}
const RGB& at(int32_t index) const			{return at(index);}
const RGB& at(int32_t hei,int32_t wid) const		{return at(hei*width()+wid);}

//create an empty .BMP file with this constructor
BMP(int32_t w, int32_t h, const char* path):filename{path},bmpfile{fopen(path, "wb")},header{},data{} {
header.width() = w;
header.height() = h;
header.update_header();
data.resize(w*h);
}

//open an existing .BMP file with this constructor
BMP(const char* path):filename{path},bmpfile{fopen(path, "rb")},header{bmpfile},data{} {
if (!header) {
	fprintf(stderr, "This file is not a bmp file!");
	exit(1);
	}
int32_t start = header.start_addr() + row_size()*(header.height() - 1);
fseek(bmpfile, start, SEEK_SET);
data.resize(width()*height());
for (int32_t i{0}; i < height(); i++) {
	for (int32_t j{0}; j < width(); j++) {
		if (fread(&(at(i,j)), 3, 1, bmpfile)!=1) {
			throw std::runtime_error("File read was not successful");
			}
	}
	fseek(bmpfile, start - row_size()*(i + 1), SEEK_SET);
	}
}
BMP(const std::string& s1):BMP(s1.c_str()) {}	//delegating from std::string to c_string
~BMP() {fclose(bmpfile);}		//close file on destruction
BMP(const BMP&) = delete;		//no copy
BMP operator=(const BMP&)=delete;	//no copy

void print() {	//print .BMP File on Terminal :-o
auto row{get_terminal_rows()-4};
auto col{get_terminal_columns()-2};
std::cout << filename << "\t" << width() << " * "<< height()<< std::endl;;
horizontal_line();
for (uint32_t r{0};r<row;r++) {
	std::cout << '|';
	for (uint32_t c{0};c<col;++c) {	//loop  (from 0.0 to 1.0)*(last index)
		uint32_t colpos =	uint32_t((c/float(col-1))*(width()-1));
		uint32_t rowpos =	uint32_t((r/float(row-1))*(height() -1));
		//this is the magic printf command from Howard Zorn;
		//it will print one color Char (pixel) to the terminal (if your terminal can handle truecolor)	
		printf("\033[48;2;%d;%d;%dm \033[0m",at(rowpos,colpos).red(),at(rowpos,colpos).green(),at(rowpos,colpos).blue());
		}
	std::cout << '|';
	std::cout << std::endl;	//next line(row)
	}
horizontal_line();
}

void save() {	//save.BMP file
header.write_header(bmpfile);
for (int64_t i = height() - 1; i >= 0; i--) {		// raw data!!
	for (int32_t j {0}; j < width(); j++) {fwrite(&(at(i,j)), 3, 1, bmpfile);}
	for (int32_t k {0}; k < (row_size() - width() * 3); k++) {fputc('\0', bmpfile);}
	}// eof
}

void put_pixel(Point p, RGB color =RGB{}) {if (p < Point{width(),height()}) {at(p.y(),p.x()) = color;}}

void put_pixel(int32_t x, int32_t y, RGB color = RGB{}) {if (Point{x,y} < Point{width(),height()}) {at(y,x) = color;}}

void line(Point p1, Point p2, RGB color) {
double dx = p2.x() - p1.x();
double dy = p2.y() - p1.y();
uint32_t step = uint32_t(std::max(dx,dy));
dx /= step;
dy /= step;
double x = p1.x();
double y = p1.y();
for (uint32_t i{0}; i <= step; i++) {
	put_pixel(x, y, color);
	x += dx;
	y += dy;
	}
}

void circle(Point p, int32_t radius,RGB color =RGB{}) {
int32_t x = radius - 1;
int32_t y = 0;
int32_t dx = 1;
int32_t dy = 1;
int32_t err = dx - (radius << 1);
while (x >= y)	{
	this->put_pixel(p.x() + x, p.y() + y, color);
	this->put_pixel(p.x() + y, p.y() + x, color);
	this->put_pixel(p.x() - y, p.y() + x, color);
	this->put_pixel(p.x() - x, p.y() + y, color);
	this->put_pixel(p.x() - x, p.y() - y, color);
	this->put_pixel(p.x() - y, p.y() - x, color);
	this->put_pixel(p.x() + y, p.y() - x, color);
	this->put_pixel(p.x() + x, p.y() - y, color);
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

void fill(Point p,RGB color = RGB{}) {
if (p.x() < width() && p.y() < height()) {
	RGB origin_color = at(p.y(),p.x());
	fill_(p,color,origin_color);
	}
}

};//class BMP
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////




void demo(void) { //draw demo-bmp file on terminal and save it as "sample.bmp"
BMP bmp(77, 77, "sample.bmp");
bmp.print();
bmp.line(Point(8, 8), Point(52, 8),RGB{0});
bmp.print();
bmp.line(Point(8, 8), Point(8, 52),RGB{0});
bmp.print();
bmp.line(Point(8, 52), Point(52, 52),RGB{0});
bmp.print();
bmp.line(Point(52, 8), Point(52, 52),RGB{0});
bmp.print();
bmp.line(Point(23, 23), Point(23, 67),RGB{0});
bmp.print();
bmp.line(Point(23, 23), Point(67, 23),RGB{0});
bmp.print();
bmp.line(Point(23, 67), Point(67, 67),RGB{0});
bmp.print();
bmp.line(Point(67, 23), Point(67, 67),RGB{0});
bmp.print();
bmp.fill(Point(18, 18), RGB{0xFF0000});
bmp.print();
bmp.fill(Point(40, 40), RGB{0x00FF00});
bmp.print();
bmp.fill(Point(60, 60), RGB{0, 0, 255});
bmp.print();
bmp.circle(Point(23, 52),10, RGB{0x000000});
bmp.print();
bmp.save();
}

#endif // !BMP_H
