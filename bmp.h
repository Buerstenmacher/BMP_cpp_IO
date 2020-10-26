#ifndef BMP_H
#define BMP_H

#include <cstdio>
#include <string.h>	//stl
#include <iostream>	//stl
#include <vector>	//stl
#include <ncurses.h> 	//sudo apt-get install libncurses5-dev libncursesw5-dev
			//compile with -lncurses flag!
			//ncurses will work on linux only
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include "../rom_header/rom_time.h"
#include "../rom_header/rom_rand.h"
#include "../rom_header/rom_matrix.h"


// we will interpret pictures as std::vector<std::vector<color_type>> (vector of rows)
// so first of all we will create some template-functions to deal with this types
////////////////////////////////////////////////////////////////////////////////////////////////////
template <class it>					//iterator to some color_type like RGB24
typename it::value_type at(it begin,it end,float pos) {	//bilinear interpolation inside of range of iterators
size_t sz{size_t(std::distance(begin,end))-1};		//pos is expected to go from 0.0 to 1.0
float at{pos*sz};
if (begin == end) {return typename it::value_type();}
if (pos >=1.0) {return *(end-1);}
if (pos <=0.0) {return *begin;  }
auto left{begin};
std::advance(left, size_t(at));
auto right{ ((left+1)==end)?left:(left+1) };
float distance_left{at-size_t(at)};
float distance_right{float(1.0)-distance_left};
return (*left)*distance_right+(*right)*distance_left;
}

template<class val_t>								//bilinear interpolation for an entire container
std::vector<val_t> bilinear_scale(const std::vector<val_t>& inv,size_t n_elem) {//new container has n_elem elements
std::vector<val_t> ret(n_elem,val_t{});
double step{1.0/(n_elem-1)};
for (size_t i{0};i!=n_elem;++i) {ret.at(i) = at(inv.begin(),inv.end(), i*step );}
return ret;
}

			//test if an 2d vector is rectangular
template<class blt> 	//should work for every built in type
uint8_t is_rectangular(const std::vector<std::vector<blt>>& in) {	//every subvector has to have the same size!
for (auto& elem:in) {if (elem.size()!=in.back().size()) {return 0;}}	//if that's not true we will return 0
return 1;								//return 1 if every subvector has the same size
}

template<class blt> //should work for every built in type
//this should create a vector of columns if input is a vector of rows and vise-versa
std::vector<std::vector<blt>> row_column_mirror(const std::vector<std::vector<blt>>& in) {
if (is_rectangular(in)==0) {rom::error("expected a rectangular 2d vector");}
std::vector<blt> one_row(in.size(),blt{0});                     //temporaray storage
std::vector<std::vector<blt>> ret(in.back().size(),one_row);    //size should be mirror of in
for (size_t a{0};a!=in.size();++a) {
        for (size_t b{0};b!=in.at(a).size();++b) {
                ret.at(b).at(a)=in.at(a).at(b); //mirror every value
                }
        }
return ret;
}

template<class blt> //should work for every built in type
//this should rotate a 2d vector of pixel by 180 degree
std::vector<std::vector<blt>> rot_180(const std::vector<std::vector<blt>>& in) {
if (is_rectangular(in)==0) {rom::error("expected a rectangular 2d vector");}
std::vector<std::vector<blt>> ret{in};    //size should be like that of in
for (size_t a{0};a!=in.size();++a) {
        for (size_t b{0};b!=in.at(a).size();++b) {
                ret.at(in.size()-a-1).at(in.at(a).size()-b-1)=in.at(a).at(b); //mirror every value
                }
        }
return std::move(ret);
}

template<class blt> //should work for every built in type
std::vector<std::vector<blt>> rectangular_me(const std::vector<blt>& in,size_t row_size) {	//function to flaten 2d vector
if (in.size() % row_size)	{rom::error("rectangular_me cannot be done, because input vector size is not divisible by providet row size.");}
size_t columns{row_size};
size_t rows{in.size()/row_size};
std::vector<blt> one_row(columns,blt{0});		//temporaray storage
std::vector<std::vector<blt>> ret(rows,one_row);	//size should be mirror of in
auto it{in.begin()};
for (auto& r: ret) {for (auto& c: r) {c = *it++;}}
if (is_rectangular(ret)==0) {rom::error("expected a rectangular 2d vector");}//check if input is rectangular
return ret;
}

template<class blt> //should work for every built in type
std::vector<blt> flat_me(const std::vector<std::vector<blt>>& in) {	//function to flaten 2d vector
if (is_rectangular(in)==0) {rom::error("expected a rectangular 2d vector");}//check if input is rectangular
std::vector<blt> ret{};
ret.reserve(in.size()*in.at(0).size());
for (auto& r:in) {for (auto& c:r) {ret.push_back(c);}}
return ret;
}

template<class val_t>						//bilinear interpolation for an entire 2d container
std::vector<std::vector<val_t>> bilinear_scale(const std::vector<std::vector<val_t>>& inv,size_t colin, size_t rowin) {//new container has (colin * rowin) elements
if (is_rectangular(inv)==0) {rom::error("expected a rectangular 2d vector");}//check if input is rectangular
std::vector<std::vector<val_t>> tmp1{};
tmp1.reserve(inv.size());
for (auto& row:inv) {tmp1.push_back(bilinear_scale(row,colin));}
auto tmp2{row_column_mirror(tmp1)};
tmp1.clear();	//todo tmp3 is not nesecerry use tmp1 instead
tmp1.reserve(tmp2.size());
for (auto& row:tmp2) {tmp1.push_back(bilinear_scale(row,rowin));}
return row_column_mirror(tmp1);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
class ncurses_window {
private:

 struct _WIN_struct {	//nested class
 int32_t height, width, startx, starty;

  struct _win_border_struct {	//nested nested class
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
else	{for(j = y; j <= y + h; ++j) {for(i = x; i <= x + w; ++i) {mvaddch(j, i, ' ');}}}
refresh();
}

public:
uint32_t terminalheight(void) 	{return LINES;}
uint32_t terminalwidth(void)	{return COLS;}
uint32_t row(void)	 	{return LINES;}
uint32_t col(void)		{return COLS;}

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
class RGB24 {	//24bit colour representation
uint8_t _bl,_gr,_re;
public:
RGB24(uint32_t rgb):_bl{uint8_t((rgb & 0x0000FF) >> 0)},_gr{uint8_t((rgb & 0x00FF00) >> 8)},_re{uint8_t((rgb & 0xFF0000) >> 16)} {}
inline RGB24(uint8_t r, uint8_t g, uint8_t b):_bl{b},_gr{g},_re{r} {}
RGB24(void):_bl{100},_gr{100},_re{100} {}	//default is gray

template <class flt=float>
RGB24(flt in):_bl{},_gr{},_re{} {
if 	(in>=1.0) {_re = _bl = _gr =255;}
else if (in<=0.0) {_re = _bl = _gr =0;}
else 		  {_re = _bl = _gr =in*265.0;}
}

inline RGB24 operator*(float multiplicator) const {
if (multiplicator < 0.0)	{throw std::runtime_error("Multiplication of RGB24-Pixel with negative Number");}
static float tmp;
static uint8_t bl,gr,re;
tmp = round(this->_bl*multiplicator);
bl = (tmp<=255.0)?uint8_t(tmp):255;
tmp = round(this->_gr*multiplicator);
gr = (tmp<=255.0)?uint8_t(tmp):255;
tmp = round(this->_re*multiplicator);
re = (tmp<=255.0)?uint8_t(tmp):255;
return std::move(RGB24(re,gr,bl));
}

inline RGB24 operator+(const RGB24& right) const {
static float re,gr,bl;
static uint8_t red,gre,blu;
re = _re+right._re;
gr = _gr+right._gr;
bl = _bl+right._bl;
red = (re<256)?uint8_t(re):255;
gre = (gr<256)?uint8_t(gr):255;
blu = (bl<256)?uint8_t(bl):255;
return std::move(RGB24(red,gre,blu));
}

bool operator==(const RGB24& b) const {return (_bl==b.blue() && _gr==b.green() && _re==b.red())?true:false;}
uint8_t blue(void) const 	{return _bl;}
uint8_t green(void) const 	{return _gr;}
uint8_t red(void) const		{return _re;}
uint8_t& blue(void) 		{return _bl;}
uint8_t& green(void) 		{return _gr;}
uint8_t& red(void) 		{return _re;}

template <class flt=float>
operator flt() const {	//convert to any floatingPoint type as grayscale
flt ret{};
ret = (flt(_bl) + flt(_re) + flt(_gr))/flt(3.0*256);
return ret;
}

};//class RGB24
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
class BW1 {	//1bit colour representation
uint8_t _bl;
public:

BW1(uint8_t black):_bl{black} {}

BW1(const RGB24& in):_bl{0}	{				//construct from RGB24
static uint8_t remainder{127};
float average{round(float(in.red()+in.green()+in.blue()) / float{3.0})};
uint8_t last{remainder};
remainder += average;		//integer overflow is intentional
_bl = (remainder<last)?1:0;	//colour black every time ther is an overflow
}

uint8_t black(void) const 	{return _bl;}

};//class BW1
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
class gui_fb {	//this allows us to get access to the graphics framebuffer of linux graphics user interface
private:	//only on raspbian
struct fb_var_screeninfo vinfo{};
struct fb_fix_screeninfo finfo{};
int32_t fbfile;
uint8_t* fbp;

public:
gui_fb(std::string filename = "/dev/fb0"):fbfile{0},fbp{nullptr}	{
fbfile = open(filename.c_str(), O_RDWR);                // Open the framebuffer device file for reading and writing
if (fbfile < 0)	{throw std::runtime_error("Error: cannot open framebuffer device.");}
if (ioctl(fbfile, FBIOGET_VSCREENINFO, &vinfo)) {throw std::runtime_error("Error reading variable screen info.");}    // Get variable screen information
if (ioctl(fbfile, FBIOGET_FSCREENINFO, &finfo)) {throw std::runtime_error("Error reading fixed screen info.");}
fbp = reinterpret_cast<uint8_t*>(mmap(0,bufsize(),PROT_READ|PROT_WRITE,MAP_SHARED,fbfile,0));
if(fbp == MAP_FAILED) {throw std::runtime_error("Failed to mmap.");}
}

gui_fb(const gui_fb&)		= delete;	//no copy
gui_fb operator=(const gui_fb&)	= delete;	//no copy

size_t 	 bufsize(void) const 		{return finfo.smem_len;}	//screensize in byte
uint8_t* get_pointer(void) const 	{return fbp;}
uint32_t get_xres(void) const 		{return vinfo.xres;}		//width in pixel
uint32_t get_yres(void) const 		{return vinfo.yres;}		//height in pixel
uint32_t get_bits_per_pixel(void) const	{return vinfo.bits_per_pixel;}	//bit per pixel
uint32_t get_bytes_per_pixel(void) const{return (get_bits_per_pixel()/8);}
uint32_t get_bytes_per_row(void) const	{return get_bytes_per_pixel()*get_xres();}
uint32_t get_bytes_per_screen(void) const	{return get_bytes_per_row()*get_yres();}

~gui_fb(void){
munmap(fbp,bufsize());
close(fbfile);      //close file
}

//export framebuffer as 2d std::vector of pixels
operator std::vector<std::vector<RGB24>>() const {
std::vector<std::vector<RGB24>> ret{};
ret.reserve(get_yres());
for (size_t y{0};y<size_t(get_yres());++y) {
	ret.push_back(std::vector<RGB24>{});
	ret.back().reserve(get_xres());
	for (size_t x{0};x<size_t(get_xres());++x) {
		uint32_t index = x*get_bytes_per_pixel() + y*get_bytes_per_row();
		uint32_t value = *reinterpret_cast<uint32_t *>(get_pointer()+index);
		ret.at(y).push_back(RGB24(value));
		}
	}
if (ret.size()*ret.begin()->size()!=get_xres()*get_yres())
	{throw std::runtime_error("Size is not correct in operator std::vector<std::vector<RGB24>>");}
if (ret.size()*ret.begin()->size()!=(get_bytes_per_screen()/get_bytes_per_pixel()))
	{throw std::runtime_error("Size is not correct in operator std::vector<std::vector<RGB24>>");}
return ret;
}

};//class gui_fb
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
class Point {
int32_t _x,_y;
//RGB24 _color;
public:
int32_t& x(void)	{return _x;}
int32_t& y(void)	{return _y;}
//RGB24& color(void)	{return _color;}
Point(int32_t x, int32_t y/*, RGB24 rgb=RGB24{}*/) : _x(x), _y(y)/*, _color(rgb)*/ {}
Point():_x{0},_y{0}/*,_color{}*/ {}
uint8_t operator <(const Point& r ) const {return (_x<r._x && _y<r._y && _y>=0 && _x>=0);}
uint8_t operator >(const Point& r ) const {return (_x>r._x && _y>r._y && _y>=0 && _x>=0);}
uint8_t operator ==(const Point& r ) const {return (this->_x == r._x) && (this->_y  == r._y);}
uint8_t operator>=(const Point& r ) const {return (*this > r) or (*this == r);}
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
class terminal_viewer {
private:

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
std::cout << std::endl;
}

public:
void print(const std::vector<std::vector<RGB24>>& data_inp) {	//print .BMP File on Terminal :-o
auto row{get_terminal_rows()-4};
auto col{get_terminal_columns()-2};
auto di{bilinear_scale(data_inp,col,row)};
std::cout  << data_inp.begin()->size() << " * "<< data_inp.size()<< std::endl;;
horizontal_line();
for (uint32_t r{0};r<row;r++) {
	std::cout << '|';
	for (uint32_t c{0};c<col;++c) {	printf("\033[48;2;%d;%d;%dm \033[0m",di.at(r).at(c).red(),di.at(r).at(c).green(),di.at(r).at(c).blue());}
	std::cout << '|';
	std::cout << std::endl;	//next line(row)
	}
horizontal_line();
}

};	//class terminal_viewer
////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
class BMP final{	//todo: split of an entire class for terminal output
std::string filename;
//FILE* bmpfile;
BMP_header header;
std::vector<RGB24> data;

void fill_(Point p,const RGB24& color, const RGB24& ori_col) {
Point max{width(),height()};
if (p<max	&& 	at(p.y(),p.x()) == ori_col) {
	this->put_pixel(p,color);
	Point up{p}, down{p}, left{p}, right{p};
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

public:
//bmpfile{fopen(path, "wb")},
//create an empty .BMP file with this constructor
BMP(int32_t w, int32_t h, const char* path = ""):filename{path},header{},data{} {
header.width() = w;
header.height() = h;
header.update_header();
data.resize(w*h);
}

int32_t row_size(void) const 	{return header.row_size();}		//row-size in byte
int32_t width(void) const	{return int32_t(header.width());}	//row-size in pixel
int32_t height(void) const	{return int32_t(header.height());}	//column-size in pixel
RGB24& at(int32_t index)			{return this->data.at(index);}
RGB24& at(int32_t hei,int32_t wid)		{return this->data.at(hei*width()+wid);}
const RGB24& at(int32_t index) const			{return at(index);}
const RGB24& at(int32_t hei,int32_t wid) const		{return at(hei*width()+wid);}

//bmpfile{fopen(path.c_str(), "wb")},
//construct bmp object from 2d std::vector of Pixels
BMP(const std::vector<std::vector<RGB24>>& inp,std::string path = ""):filename{path},header{},data{} {
header.width() = inp.begin()->size();
header.height() = inp.size();
header.update_header();
data = flat_me(inp);
}

//construct bmp object from 2d std::vector of values grayscale
template <class flt>
BMP(const std::vector<std::vector<flt>>& inp):BMP(inp.back().size(),inp.size()) {
data.clear();
for (auto& row:inp)	{for (auto& pix:row)	{data.push_back(pix);}}
}

//open an existing .BMP file with this constructor
BMP(const char* path):filename{path},header{},data{} {
auto bmpfile{fopen(path, "rb")};
header = BMP_header(bmpfile);
if (!header) {
	fprintf(stderr, "This file is not a bmp file!");
	exit(1);
	}
int32_t start = header.start_addr() + row_size()*(header.height() - 1);
fseek(bmpfile, start, SEEK_SET);
data.resize(width()*height());
for (int32_t i{0}; i < height(); i++) {
	for (int32_t j{0}; j < width(); j++) {
		if (fread(&(at(i,j)), 3, 1, bmpfile)!=1) {throw std::runtime_error("File read was not successful");}
		}
	fseek(bmpfile, start - row_size()*(i + 1), SEEK_SET);
	}
fclose(bmpfile);
}

BMP(const std::string& s1):BMP(s1.c_str()) {}	//delegating from std::string to c_string

~BMP() {}			//close file on destruction

BMP(const BMP&) 		= default;	//standard copy
BMP& operator=(const BMP&)	= default;	//standard copy

void print(void) {
terminal_viewer t{};
t.print(std::vector<std::vector<RGB24>>(*this));
}

void save() {	//save.BMP file
if (!filename.size()) {throw std::runtime_error("cannot save a file without filename");}
auto bmpfile = fopen(filename.c_str(), "wb");
header.write_header(bmpfile);
for (int64_t i = height() - 1; i >= 0; i--) {		// raw data!!
	for (int32_t j {0}; j < width(); j++) {fwrite(&(at(i,j)), 3, 1, bmpfile);}
	for (int32_t k {0}; k < (row_size() - width() * 3); k++) {fputc('\0', bmpfile);}
	}// eof
fclose(bmpfile);
}

//export bmp as 2d std::vector of pixels
operator std::vector<std::vector<RGB24>>() const {return std::move(rectangular_me(data,width()));}

//export as 2d vector of grayscale values
template <class flt>
operator std::vector<std::vector<flt>>() const {
std::vector<std::vector<RGB24>> colour {*this};
std::vector<std::vector<flt>> ret{};
for (auto& row:colour) {
	ret.push_back(std::vector<flt>{});
	for (auto& pix:row)	{ret.back().push_back(pix);}
	}
return ret;
}

void black_white(void) {
std::vector<std::vector<float>> bw{*this};
BMP bmp3(bw);
*this = bmp3;
}

void flip_diagonal(void) {
std::vector<std::vector<float>> bw{*this};
auto m1{rom::Matrix<float>(bw)};
m1 = m1.transpose();
bw = m1;
*this = BMP(bw);
}

void put_pixel(Point p, RGB24 color =RGB24{}) {if (p < Point{width(),height()}) {at(p.y(),p.x()) = color;}}

void put_pixel(int32_t x, int32_t y, RGB24 color = RGB24{}) {if (Point{x,y} < Point{width(),height()}) {at(y,x) = color;}}

void line(Point p1, Point p2, RGB24 color) {
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

void circle(Point p, int32_t radius,RGB24 color =RGB24{}) {
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

void fill(Point p,RGB24 color = RGB24{}) {
if (p.x() < width() && p.y() < height()) {
	RGB24 origin_color = at(p.y(),p.x());
	fill_(p,color,origin_color);
	}
}

};//class BMP
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//testfunctions:
void agora(void) { //draw demo-bmp file on terminal and save it as "sample.bmp"
BMP bmp(400,300, "agora.bmp");
bmp.print();
bmp.line(Point(0,299), Point(399,0),RGB24{0});
bmp.print();
bmp.fill(Point(40, 40), RGB24{0x00});
bmp.print();
bmp.save();
}


//testfunctions:
void demo(void) { //draw demo-bmp file on terminal and save it as "sample.bmp"
BMP bmp(77, 77, "sample.bmp");
bmp.print();
bmp.line(Point(8, 8), Point(52, 8),RGB24{0});
bmp.print();
bmp.line(Point(8, 8), Point(8, 52),RGB24{0});
bmp.print();
bmp.line(Point(8, 52), Point(52, 52),RGB24{0});
bmp.print();
bmp.line(Point(52, 8), Point(52, 52),RGB24{0});
bmp.print();
bmp.line(Point(23, 23), Point(23, 67),RGB24{0});
bmp.print();
bmp.line(Point(23, 23), Point(67, 23),RGB24{0});
bmp.print();
bmp.line(Point(23, 67), Point(67, 67),RGB24{0});
bmp.print();
bmp.line(Point(67, 23), Point(67, 67),RGB24{0});
bmp.print();
bmp.fill(Point(18, 18), RGB24{0xFF0000});
bmp.print();
bmp.fill(Point(40, 40), RGB24{0x00FF00});
bmp.print();
bmp.fill(Point(60, 60), RGB24{0, 0, 255});
bmp.print();
bmp.circle(Point(23, 52),10, RGB24{0x000000});
bmp.print();
bmp.save();
}

void gui(void) {

gui_fb screenshot;
//for (uint8_t i{0};i<20;++i) {
	std::vector<std::vector<RGB24>> buf(screenshot);
	buf = bilinear_scale(buf,1920,1080);
	BMP bmp1{buf};
	bmp1.print();
//	}
bmp1.save();


}

#endif // !BMP_H
