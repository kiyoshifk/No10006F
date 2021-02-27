#include "app.h"


#define X_MAX   448
#define Y_MAX   320


char cur_x;							// 0〜76
char cur_y;							// 0〜31
char cur_data;						// カーソル置き換えデータ


extern uint32_t video[320][20];				// video RAM


void Pset(int x, int y, int c);
int Pget(int x, int y);

/********************************************************/
/*      charactor generator                             */
/********************************************************/
const unsigned char chara_gene[] = {  // 6x8 charactor, bit0 が上
0,0x00,0x00,0x00,0x00,0x00,	// ' '
0,0x00,0x00,0x5f,0x00,0x00,	// '!'
0,0x00,0x07,0x00,0x07,0x00,	// '"'
0,0x14,0x7f,0x14,0x7f,0x14,	// '#'
0,0x24,0x2a,0x7f,0x2a,0x12,	// '$'
0,0x23,0x13,0x08,0x64,0x62,	// '%'
0,0x36,0x49,0x55,0x22,0x50,	// '&'
0,0x00,0x05,0x03,0x00,0x00,	// '''
0,0x00,0x1c,0x22,0x41,0x00,	// '('
0,0x00,0x41,0x22,0x1c,0x00,	// ')'
0,0x14,0x08,0x3e,0x08,0x14,	// '*'
0,0x08,0x08,0x3e,0x08,0x08,	// '+'
0,0x00,0x50,0x30,0x00,0x00,	// ','
0,0x08,0x08,0x08,0x08,0x08,	// '-'
0,0x00,0x60,0x60,0x00,0x00,	// '.'
0,0x20,0x10,0x08,0x04,0x02,	// '/'

0,0x3e,0x51,0x49,0x45,0x3e,	// '0'
0,0x00,0x42,0x7f,0x40,0x00,	// '1'
0,0x42,0x61,0x51,0x49,0x46,	// '2'
0,0x21,0x41,0x45,0x4b,0x31,	// '3'
0,0x18,0x14,0x12,0x7f,0x10,	// '4'
0,0x27,0x45,0x45,0x45,0x39,	// '5'
0,0x3c,0x4a,0x49,0x49,0x30,	// '6'
0,0x01,0x71,0x09,0x05,0x03,	// '7'
0,0x36,0x49,0x49,0x49,0x36,	// '8'
0,0x06,0x49,0x49,0x29,0x1e,	// '9'
0,0x00,0x36,0x36,0x00,0x00,	// ':'
0,0x00,0x56,0x36,0x00,0x00,	// ';'
0,0x08,0x14,0x22,0x41,0x00,	// '<'
0,0x14,0x14,0x14,0x14,0x14,	// '='
0,0x00,0x41,0x22,0x14,0x08,	// '>'
0,0x02,0x01,0x51,0x09,0x06,	// '?'

0,0x32,0x49,0x79,0x41,0x3e,	// '@'
0,0x7e,0x11,0x11,0x11,0x7e,	// 'A'
0,0x7f,0x49,0x49,0x49,0x36,	// 'B'
0,0x3e,0x41,0x41,0x41,0x22,	// 'C'
0,0x7f,0x41,0x41,0x22,0x1c,	// 'D'
0,0x7f,0x49,0x49,0x49,0x41,	// 'E'
0,0x7f,0x09,0x09,0x09,0x01,	// 'F'
0,0x3e,0x41,0x49,0x49,0x7a,	// 'G'
0,0x7f,0x08,0x08,0x08,0x7f,	// 'H'
0,0x00,0x41,0x7f,0x41,0x00,	// 'I'
0,0x20,0x40,0x41,0x3f,0x01,	// 'J'
0,0x7f,0x08,0x14,0x22,0x41,	// 'K'
0,0x7f,0x40,0x40,0x40,0x40,	// 'L'
0,0x7f,0x02,0x0c,0x02,0x7f,	// 'M'
0,0x7f,0x04,0x08,0x10,0x7f,	// 'N'
0,0x3e,0x41,0x41,0x41,0x3e,	// 'O'

0,0x7f,0x09,0x09,0x09,0x06,	// 'P'
0,0x3e,0x41,0x51,0x21,0x5e,	// 'Q'
0,0x7f,0x09,0x19,0x29,0x46,	// 'R'
0,0x46,0x49,0x49,0x49,0x31,	// 'S'
0,0x01,0x01,0x7f,0x01,0x01,	// 'T'
0,0x3f,0x40,0x40,0x40,0x3f,	// 'U'
0,0x1f,0x20,0x40,0x20,0x1f,	// 'V'
0,0x3f,0x40,0x38,0x40,0x3f,	// 'W'
0,0x63,0x14,0x08,0x14,0x63,	// 'X'
0,0x07,0x08,0x70,0x08,0x07,	// 'Y'
0,0x61,0x51,0x49,0x45,0x43,	// 'Z'
0,0x00,0x7f,0x41,0x41,0x00,	// '['
0,0x15,0x16,0x3c,0x16,0x15,	// '\'
0,0x00,0x41,0x41,0x7f,0x00,	// ']'
0,0x04,0x02,0x01,0x02,0x04,	// '^'
0,0x40,0x40,0x40,0x40,0x40,	// '_'

0,0x00,0x01,0x02,0x04,0x00,	// '`'
0,0x20,0x54,0x54,0x54,0x78,	// 'a'
0,0x7f,0x48,0x44,0x44,0x38,	// 'b'
0,0x38,0x44,0x44,0x44,0x20,	// 'c'
0,0x38,0x44,0x44,0x48,0x7f,	// 'd'
0,0x38,0x54,0x54,0x54,0x18,	// 'e'
0,0x08,0x7e,0x09,0x01,0x02,	// 'f'
0,0x0c,0x52,0x52,0x52,0x3e,	// 'g'
0,0x7f,0x08,0x04,0x04,0x78,	// 'h'
0,0x00,0x44,0x7d,0x40,0x00,	// 'i'
0,0x20,0x40,0x44,0x3d,0x00,	// 'j'
0,0x7f,0x10,0x28,0x44,0x00,	// 'k'
0,0x00,0x00,0x7f,0x00,0x00,	// 'l'
0,0x7c,0x04,0x18,0x04,0x78,	// 'm'
0,0x7c,0x08,0x04,0x04,0x78,	// 'n'
0,0x38,0x44,0x44,0x44,0x38,	// 'o'

0,0x7c,0x14,0x14,0x14,0x08,	// 'p'
0,0x08,0x14,0x14,0x18,0x7c,	// 'q'
0,0x7c,0x08,0x04,0x04,0x08,	// 'r'
0,0x48,0x54,0x54,0x54,0x20,	// 's'
0,0x04,0x3f,0x44,0x40,0x20,	// 't'
0,0x3c,0x40,0x40,0x20,0x7c,	// 'u'
0,0x1c,0x20,0x40,0x20,0x1c,	// 'v'
0,0x3c,0x40,0x30,0x40,0x3c,	// 'w'
0,0x44,0x28,0x10,0x28,0x44,	// 'x'
0,0x0c,0x50,0x50,0x50,0x3c,	// 'y'
0,0x44,0x64,0x54,0x4c,0x44,	// 'z'
0,0x00,0x08,0x36,0x41,0x00,	// '{'
0,0x00,0x00,0x7f,0x00,0x00,	// '|'
0,0x00,0x41,0x36,0x08,0x00,	// '}'
0,0x08,0x04,0x08,0x10,0x08, // '~'
0,0x44,0x44,0x5f,0x44,0x44, // '±'

0x00,0x00,0x00,0x00,0x00,0x00,	// 0x80
0x00,0x00,0x00,0x00,0x00,0x00,	// 0x81
0x00,0x00,0x00,0x00,0x00,0x00,	// 0x82
0x00,0x00,0x00,0x00,0x00,0x00,	// 0x83
0x00,0x00,0x00,0x00,0x00,0x00,	// 0x84
0x00,0x00,0x00,0x00,0x00,0x00,	// 0x85
0x00,0x00,0x00,0x00,0x00,0x00,	// 0x86
0x00,0x00,0x00,0x00,0x00,0x00,	// 0x87
0x00,0x00,0x00,0x00,0x00,0x00,	// 0x88
0x00,0x00,0x00,0x00,0x00,0x00,	// 0x89
0x00,0x00,0x00,0x00,0x00,0x00,	// 0x8a
0x00,0x00,0x00,0x00,0x00,0x00,	// 0x8b
0x00,0x00,0x00,0x00,0x00,0x00,	// 0x8c
0x00,0x00,0x00,0x00,0x00,0x00,	// 0x8d
0x00,0x00,0x00,0x00,0x00,0x00,	// 0x8e
0x00,0x00,0x00,0x00,0x00,0x00,	// 0x8f

0x00,0x00,0x00,0x00,0x00,0x00,	// 0x90
0x00,0x00,0x00,0x00,0x00,0x00,	// 0x91
0x00,0x00,0x00,0x00,0x00,0x00,	// 0x92
0x00,0x00,0x00,0x00,0x00,0x00,	// 0x93
0x00,0x00,0x00,0x00,0x00,0x00,	// 0x94
0x00,0x00,0x00,0x00,0x00,0x00,	// 0x95
0x00,0x00,0x00,0x00,0x00,0x00,	// 0x96
0x00,0x00,0x00,0x00,0x00,0x00,	// 0x97
0x00,0x00,0x00,0x00,0x00,0x00,	// 0x98
0x00,0x00,0x00,0x00,0x00,0x00,	// 0x99
0x00,0x00,0x00,0x00,0x00,0x00,	// 0x9a
0x00,0x00,0x00,0x00,0x00,0x00,	// 0x9b
0x00,0x00,0x00,0x00,0x00,0x00,	// 0x9c
0x00,0x00,0x00,0x00,0x00,0x00,	// 0x9d
0x00,0x00,0x00,0x00,0x00,0x00,	// 0x9e
0x00,0x00,0x00,0x00,0x00,0x00,	// 0x9f

0x00,0x00,0x00,0x00,0x00,0x00,	// 0xa0
0x00,0x70,0x50,0x70,0x00,0x00,	// 0xa1  '｡'
0x00,0x00,0x00,0x0f,0x01,0x01,	// 0xa2  '｢'
0x00,0x40,0x40,0x78,0x00,0x00,	// 0xa3  '｣'
0x00,0x10,0x20,0x40,0x00,0x00,	// 0xa4  '､'
0x00,0x00,0x18,0x18,0x00,0x00,	// 0xa5  '･'
0x00,0x0a,0x0a,0x4a,0x2a,0x1e,	// 0xa6  'ｦ'
0x00,0x04,0x44,0x34,0x14,0x0c,	// 0xa7  'ｧ'
0x00,0x20,0x10,0x78,0x04,0x00,	// 0xa8  'ｨ'
0x00,0x18,0x08,0x4c,0x48,0x38,	// 0xa9  'ｩ'
0x00,0x48,0x48,0x78,0x48,0x48,	// 0xaa  'ｪ'
0x00,0x48,0x28,0x18,0x7c,0x08,	// 0xab  'ｫ'
0x00,0x08,0x7c,0x08,0x28,0x18,	// 0xac  'ｬ'
0x00,0x40,0x48,0x48,0x78,0x40,	// 0xad  'ｭ'
0x00,0x54,0x54,0x54,0x7c,0x00,	// 0xae  'ｮ'
0x00,0x18,0x00,0x58,0x40,0x38,	// 0xaf  'ｯ'

0x00,0x08,0x08,0x08,0x08,0x08,	// 0xb0  'ｰ'
0x00,0x01,0x41,0x3d,0x09,0x07,	// 0xb1  'ｱ'
0x00,0x10,0x08,0x7c,0x02,0x01,	// 0xb2  'ｲ'
0x00,0x0e,0x02,0x43,0x22,0x1e,	// 0xb3  'ｳ'
0x00,0x42,0x42,0x7e,0x42,0x42,	// 0xb4  'ｴ'
0x00,0x22,0x12,0x0a,0x7f,0x02,	// 0xb5  'ｵ'
0x00,0x42,0x3f,0x02,0x42,0x3e,	// 0xb6  'ｶ'
0x00,0x0a,0x0a,0x7f,0x0a,0x0a,	// 0xb7  'ｷ'
0x00,0x08,0x46,0x42,0x22,0x1e,	// 0xb8  'ｸ'
0x00,0x04,0x03,0x42,0x3e,0x02,	// 0xb9  'ｹ'
0x00,0x42,0x42,0x42,0x42,0x7e,	// 0xba  'ｺ'
0x00,0x02,0x4f,0x22,0x1f,0x02,	// 0xbb  'ｻ'
0x00,0x4a,0x4a,0x40,0x20,0x1c,	// 0xbc  'ｼ'
0x00,0x42,0x22,0x12,0x2a,0x46,	// 0xbd  'ｽ'
0x00,0x02,0x3f,0x42,0x4a,0x46,	// 0xbe  'ｾ'
0x00,0x06,0x48,0x40,0x20,0x1e,	// 0xbf  'ｿ'

0x00,0x08,0x46,0x4a,0x32,0x1e,	// 0xc0  'ﾀ'
0x00,0x0a,0x4a,0x3e,0x09,0x08,	// 0xc1  'ﾁ'
0x00,0x0e,0x00,0x4e,0x20,0x1e,	// 0xc2  'ﾂ'
0x00,0x04,0x45,0x3d,0x05,0x04,	// 0xc3  'ﾃ'
0x00,0x00,0x7f,0x08,0x10,0x00,	// 0xc4  'ﾄ'
0x00,0x44,0x24,0x1f,0x04,0x04,	// 0xc5  'ﾅ'
0x00,0x40,0x42,0x42,0x42,0x40,	// 0xc6  'ﾆ'
0x00,0x42,0x2a,0x12,0x2a,0x06,	// 0xc7  'ﾇ'
0x00,0x22,0x12,0x7b,0x16,0x22,	// 0xc8  'ﾈ'
0x00,0x00,0x40,0x20,0x1f,0x00,	// 0xc9  'ﾉ'
0x00,0x78,0x00,0x02,0x04,0x78,	// 0xca  'ﾊ'
0x00,0x3f,0x44,0x44,0x44,0x44,	// 0xcb  'ﾋ'
0x00,0x02,0x42,0x42,0x22,0x1e,	// 0xcc  'ﾌ'
0x00,0x04,0x02,0x04,0x08,0x30,	// 0xcd  'ﾍ'
0x00,0x32,0x02,0x7f,0x02,0x32,	// 0xce  'ﾎ'
0x00,0x02,0x12,0x22,0x52,0x0e,	// 0xcf  'ﾏ'

0x00,0x00,0x2a,0x2a,0x2a,0x40,	// 0xd0  'ﾐ'
0x00,0x38,0x24,0x22,0x20,0x70,	// 0xd1  'ﾑ'
0x00,0x40,0x28,0x10,0x28,0x06,	// 0xd2  'ﾒ'
0x00,0x0a,0x3e,0x4a,0x4a,0x4a,	// 0xd3  'ﾓ'
0x00,0x04,0x7f,0x04,0x14,0x0c,	// 0xd4  'ﾔ'
0x00,0x40,0x42,0x42,0x7e,0x40,	// 0xd5  'ﾕ'
0x00,0x4a,0x4a,0x4a,0x4a,0x7e,	// 0xd6  'ﾖ'
0x00,0x04,0x05,0x45,0x25,0x1c,	// 0xd7  'ﾗ'
0x00,0x0f,0x40,0x20,0x1f,0x00,	// 0xd8  'ﾘ'
0x00,0x7c,0x00,0x7e,0x40,0x20,	// 0xd9  'ﾙ'
0x00,0x7e,0x40,0x20,0x10,0x08,	// 0xda  'ﾚ'
0x00,0x7e,0x42,0x42,0x42,0x7e,	// 0xdb  'ﾛ'
0x00,0x0e,0x02,0x42,0x22,0x1e,	// 0xdc  'ﾜ'
0x00,0x42,0x42,0x40,0x20,0x18,	// 0xdd  'ﾝ'
0x00,0x02,0x04,0x01,0x02,0x00,	// 0xde  'ﾞ'
0x00,0x07,0x05,0x07,0x00,0x00,	// 0xdf  'ﾟ'

0x00,0x00,0x00,0x00,0x00,0x00,	// 0xe0
0x00,0x00,0x00,0x00,0x00,0x00,	// 0xe1
0x00,0x00,0x00,0x00,0x00,0x00,	// 0xe2
0x00,0x00,0x00,0x00,0x00,0x00,	// 0xe3
0x00,0x00,0x00,0x00,0x00,0x00,	// 0xe4
0x00,0x00,0x00,0x00,0x00,0x00,	// 0xe5
0x00,0x00,0x00,0x00,0x00,0x00,	// 0xe6
0x00,0x00,0x00,0x00,0x00,0x00,	// 0xe7
0x00,0x00,0x00,0x00,0x00,0x00,	// 0xe8
0x00,0x00,0x00,0x00,0x00,0x00,	// 0xe9
0x00,0x00,0x00,0x00,0x00,0x00,	// 0xea
0x00,0x00,0x00,0x00,0x00,0x00,	// 0xeb
0x00,0x00,0x00,0x00,0x00,0x00,	// 0xec
0x00,0x00,0x00,0x00,0x00,0x00,	// 0xed
0x00,0x00,0x00,0x00,0x00,0x00,	// 0xee
0x00,0x00,0x00,0x00,0x00,0x00,	// 0xef

0x00,0x00,0x00,0x00,0x00,0x00,	// 0xf0
0x00,0x00,0x00,0x00,0x00,0x00,	// 0xf1
0x00,0x00,0x00,0x00,0x00,0x00,	// 0xf2
0x00,0x00,0x00,0x00,0x00,0x00,	// 0xf3
0x00,0x00,0x00,0x00,0x00,0x00,	// 0xf4
0x00,0x00,0x00,0x00,0x00,0x00,	// 0xf5
0x00,0x00,0x00,0x00,0x00,0x00,	// 0xf6
0x00,0x00,0x00,0x00,0x00,0x00,	// 0xf7
0x00,0x00,0x00,0x00,0x00,0x00,	// 0xf8
0x00,0x00,0x00,0x00,0x00,0x00,	// 0xf9
0x00,0x00,0x00,0x00,0x00,0x00,	// 0xfa
0x00,0x00,0x00,0x00,0x00,0x00,	// 0xfb
0x00,0x00,0x00,0x00,0x00,0x00,	// 0xfc
0x00,0x00,0x00,0x00,0x00,0x00,	// 0xfd
0x00,0x00,0x00,0x00,0x00,0x00,	// 0xfe
0x00,0x00,0x00,0x00,0x00,0x00,	// 0xff
};
/********************************************************************************/
/*		fatal																	*/
/********************************************************************************/
void fatal(const char *fmt, ...)
{
	char buf[50];
	va_list ap;

	memset(video, 0, sizeof(video));		// clear
	display_xn(0, 10, "FATAL error", 2);

	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);
	
	display_xn(0, 30, buf, 2);
	for(;;) ;
}
/********************************************************************************/
/*		g_pset																	*/
/********************************************************************************/
void g_pset(int x, int y, int color)
{
//	if(color){
//		Pset(x,y);
//    }
//	else{
//		PCLR1(x,y);
//    }
	Pset(x, y, color);
}
/********************************************************************************/
/*		line1																	*/
/********************************************************************************/
void line1(int x1, int y1, int x2, int y2, int color)
{
	int sx,sy,dx,dy,i;
	int e;

	if(x2>x1){
		dx=x2-x1;
		sx=1;
	}
	else{
		dx=x1-x2;
		sx=-1;
	}
	if(y2>y1){
		dy=y2-y1;
		sy=1;
	}
	else{
		dy=y1-y2;
		sy=-1;
	}
	if(dx>=dy){
		e=-dx;
		for(i=0;i<=dx;i++){
			g_pset(x1,y1,color);
			x1+=sx;
			e+=dy*2;
			if(e>=0){
				y1+=sy;
				e-=dx*2;
			}
		}
	}
	else{
		e=-dy;
		for(i=0;i<=dy;i++){
			g_pset(x1,y1,color);
			y1+=sy;
			e+=dx*2;
			if(e>=0){
				x1+=sx;
				e-=dy*2;
			}
		}
	}
}
/********************************************************************************/
/*		circle1																	*/
/*		(x0,y0) を中心に、半径 r の円を描く										*/
/********************************************************************************/
void circle1(int x0, int y0, int r, int color)
{
	int x,y,f;
	x=r;
	y=0;
	f=-2*r+3;
	while(x>=y){
		g_pset(x0-x,y0-y,color);
		g_pset(x0-x,y0+y,color);
		g_pset(x0+x,y0-y,color);
		g_pset(x0+x,y0+y,color);
		g_pset(x0-y,y0-x,color);
		g_pset(x0-y,y0+x,color);
		g_pset(x0+y,y0-x,color);
		g_pset(x0+y,y0+x,color);
		if(f>=0){
			x--;
			f-=x*4;
		}
		y++;
		f+=y*4+2;
	}
}
/********************************************************************************/
/*		circle2																	*/
/*		(x0,y0) を中心に、半径 r の円を描く										*/
/********************************************************************************/
void circle2(int x0, int y0, int rx, int ry, int color)
{
	int x,y,f;
	x=rx;
	y=0;
	f=(3-rx-ry)*rx*ry;
//    f = -(rx+ry)*rx*ry/2;
	while(x*ry*ry >= y*rx*rx){
		g_pset(x0-x,y0-y,color);
		g_pset(x0-x,y0+y,color);
		g_pset(x0+x,y0-y,color);
		g_pset(x0+x,y0+y,color);
		if(f>=0){
			x--;
			f-=x*4*ry*ry;
		}
		y++;
		f+=y*4*rx*rx;
	}
	g_pset(x0-x,y0-y,color);
	g_pset(x0-x,y0+y,color);
	g_pset(x0+x,y0-y,color);
	g_pset(x0+x,y0+y,color);

	y=ry;
	x=0;
	f=(3-rx-ry)*rx*ry;
//    f = -(rx+ry)*rx*ry/2;
	while(y*rx*rx >= x*ry*ry){
		g_pset(x0-x,y0-y,color);
		g_pset(x0-x,y0+y,color);
		g_pset(x0+x,y0-y,color);
		g_pset(x0+x,y0+y,color);
		if(f>=0){
			y--;
			f-=y*4*rx*rx;
		}
		x++;
		f+=x*4*ry*ry;
	}
	g_pset(x0-x,y0-y,color);
	g_pset(x0-x,y0+y,color);
	g_pset(x0+x,y0-y,color);
	g_pset(x0+x,y0+y,color);
}
/********************************************************************************/
/*		boxfill																	*/
/*		(x1,y1),(x2,y2)を対角線とするカラーcで塗られた長方形を描画				*/
/********************************************************************************/
void boxfill(int x1, int y1, int x2, int y2, int c)
{
	int temp;
	if(x1>x2){
		temp=x1;
		x1=x2;
		x2=temp;
	}
	if(x2<0 || x1>=X_MAX) return;
	if(y1>y2){
		temp=y1;
		y1=y2;
		y2=temp;
	}
	if(y2<0 || y1>=Y_MAX) return;
	if(y1<0) y1=0;
	if(y2>=Y_MAX) y2=Y_MAX-1;
	while(y1<=y2){
		line1(x1,y1,x2,y1,c);
        y1++;
	}
}
/********************************************************************************/
/*		circlefill																*/
/*		(x0,y0)を中心に、半径r、カラーcで塗られた円を描画						*/
/********************************************************************************/
void circlefill(int x0, int y0, int r, int c)
{
	int x,y,f;
	x=r;
	y=0;
	f=-2*r+3;
	while(x>=y){
		line1(x0-x,y0-y,x0+x,y0-y,c);
		line1(x0-x,y0+y,x0+x,y0+y,c);
		line1(x0-y,y0-x,x0+y,y0-x,c);
		line1(x0-y,y0+x,x0+y,y0+x,c);
		if(f>=0){
			x--;
			f-=x*4;
		}
		y++;
		f+=y*4+2;
	}
}
/********************************************************************************/
/*		circlefill																*/
/*		(x0,y0)を中心に、半径r、カラーcで塗られた円を描画						*/
/********************************************************************************/
void circlefill2(int x0, int y0, int rx, int ry, int c)
{
	int x,y,f;
	x=rx;
	y=0;
//	f=(3-rx-ry)*rx*ry;
    f = -(rx+ry)*rx*ry/2;
	while(x*ry*ry >= y*rx*rx){
		line1(x0-x,y0-y,x0+x,y0-y,c);
		line1(x0-x,y0+y,x0+x,y0+y,c);
		if(f>=0){
			x--;
			f-=x*4*ry*ry;
		}
		y++;
		f+=y*4*rx*rx;
	}
	line1(x0-x,y0-y,x0+x,y0-y,c);
	line1(x0-x,y0+y,x0+x,y0+y,c);

	y=ry;
	x=0;
//	f=(3-rx-ry)*rx*ry;
    f = -(rx+ry)*rx*ry/2;
	while(y*rx*rx >= x*ry*ry){
		line1(x0-x,y0-y,x0+x,y0-y,c);
		line1(x0-x,y0+y,x0+x,y0+y,c);
		if(f>=0){
			y--;
			f-=y*4*rx*rx;
		}
		x++;
		f+=x*4*ry*ry;
	}
	line1(x0-x,y0-y,x0+x,y0-y,c);
	line1(x0-x,y0+y,x0+x,y0+y,c);
}
