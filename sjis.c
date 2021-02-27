#include "app.h"
#include "font_lib.h"


extern const uint8_t KanjiFont12b[7896][18];
extern const short font[256][16];
extern const unsigned char chara_gene[];
extern char cur_x;					// 0～76
extern char cur_y;					// 0～31
extern char cur_data;						// カーソル置き換えデータ
extern uint32_t __attribute__((section(".mySection1"))) video[320][20];	// video RAM




/********************************************************************************/
/*		sjis_parse																*/
/*		文字列から１文字（1/2 バイトコード）取り出し							*/
/*		return: キャラゼネコード												*/
/********************************************************************************/
int sjis_parse(const char *str, int *byte)
{
	int c, ku, ten;
	
	c = *str & 0xff;
	if(c>=0x81 && c<=0x9f){
		ku = (c - 0x81)*2 + 1;				// 1～62
	}
	else if(c>=0xe0 && c<=0xef){			// 63～94
		ku = (c - 0xe0)*2 + 63;
	}
	else{
		*byte = 1;							// 1 バイトコード
		return c;
	}
	
	c = str[1] & 0xff;
	if(c>=0x40 && c<=0x7e){					// 1～63
		ten = c - 0x40 + 1;
	}
	else if(c>=0x80 && c<=0x9e){			// 64～94
		ten = c - 0x80 + 64;
	}
	else if(c>=0x9f && c<=0xfc){			// 1～94
		ku++;
		ten = c - 0x9f + 1;
	}
	else{									// 1 バイトコード  エラー
		*byte = 1;
		return c;
	}
	
    *byte = 2;
	return (ku-1)*94 + ten /* - 1 */;
}
/********************************************************************************/
/*		sjis_strlen																*/
/********************************************************************************/
int sjis_strlen(const char *str)
{
	int i, byte;
	
	for(i=0; ; ){
		if(sjis_parse(str, &byte)==0)
			return i;
		i++;
        str += byte;
	}
}
/********************************************************************************/
/*		char_disp																*/
/********************************************************************************/
void char_disp(int x, int y, int c, int rev_flag)
{
	int x1, y1, c1;
	
    c &= 0xff;
	if(c<0x20 || c>0xff)
		return;
	for(x1=1; x1<6; x1++){
		c1 = chara_gene[(c-0x20)*6+x1];
		for(y1=0; y1<8; y1++){
			if(rev_flag==0){
				Pset(x+x1, y+y1, c1 & (1<<y1));
			}
			else{
				Pset(x+x1, y+y1, (c1 & (1<<y1))==0);
			}
		}
	}
}

//  c: 句点コード 0?
void sjis_disp(int x, int y, int c, int rev_flag)
{
	int i, j, bit, x1, y1, c1;
	
	for(i=0; i<12*16; i++){
		x1 = i%12;
		y1 = i/12;
		j = (i-24)/8;
		bit = 0x80 >> ((i-24)%8);
		c1 = KanjiFont12b[c][j];
		if((c1 & bit) && (i>=24 && i<12*14)){
			Pset(x+x1, y+y1, !rev_flag);
		}
		else{
			Pset(x+x1, y+y1, rev_flag);
		}
	}
}

void char_disp_xn(int x, int y, int c, int rev_flag, int n)
{
	int x1, y1, c1;
	
    c &= 0xff;
	if(c<0x20 || c>0xff)
		return;
	for(x1=n; x1<6*n; x1++){
		c1 = chara_gene[(c-0x20)*6+x1/n];
		for(y1=0; y1<8*n; y1++){
			if(rev_flag==0){
				Pset(x+x1, y+y1, c1 & (1<<y1/n));
			}
			else{
				Pset(x+x1, y+y1, (c1 & (1<<y1/n))==0);
			}
		}
	}
}

void display_xn(int x, int y, const char * str, int n)
{
	while(*str){
		char_disp_xn(x, y, *str++, 0, n);
		x += 6*n;
	}
}

void rev_display_xn(int x, int y, const char * str, int n)
{
	while(*str){
		char_disp_xn(x, y, *str++, 1, n);
		x += 6*n;
	}
}
/********************************************************************************/

//	char cur_x: 0～76
//	char cur_y: 0～19
//	char cur_data: カーソル置き換えデータ

//	カーソル位置に元のグラフィックデータを書き込む
void clear_cur()
{
	int i, x, y, tmp;
	
	x = cur_x * 6;
	y = cur_y * 16 + 14;
	tmp = cur_data;
	for(i=0; i<6; i++){
		Pset(x++, y, tmp & 0x20);
		tmp <<= 1;
	}
}

//	カーソル位置のグラフィックデータを読み取る
//void disp_change_data_get()
//{
//	int i, x, y, tmp;
//	
//	x = cur_x * 6;
//	y = cur_y * 16 + 14;
//	tmp = 0;
//	for(i=0; i<6; i++){
//		tmp = (tmp<<1) + Pget(x++, y);
//	}
//	cur_data = tmp;
//}

void disp_cur()
{
	int i, x, y;
	
	x = cur_x * 6;
	y = cur_y * 16 + 14;
	for(i=0; i<6; i++){
		Pset(x++, y, 1);
	}
}

void disp_U()
{
	clear_cur();
	--cur_y;
	if(cur_y < 0){
		cur_y = 0;
		memmove(video[320], video[320-16], sizeof(video[0])*(320-16));
		memset(video[0], 0, sizeof(video[0])*16);
//        memset(video[161], 0, sizeof(video[0])*5);
	}
//	disp_change_data_get();
	disp_cur();
}

void disp_D()
{
	clear_cur();
	++ cur_y;
	if(cur_y > 19){
		cur_y = 19;
		memmove(video[0], video[16], sizeof(video[0])*(320-16));
		memset(video[320-16], 0, sizeof(video[0])*16);
//        memset(video[160+1-5], 0, sizeof(video[0])*5);
	}
//	disp_change_data_get();
	disp_cur();
}

void disp_R()
{
	clear_cur();
	++cur_x;
	if(cur_x >= 74){
		cur_x = 0;
//		disp_change_data_get();
		disp_D();
		return;
	}
//	disp_change_data_get();
	disp_cur();
}

void disp_L()
{
	clear_cur();
	--cur_x;
	if(cur_x < 0){
		cur_x = 76;
//		disp_change_data_get();
		disp_U();
		return;
	}
//	disp_change_data_get();
	disp_cur();
}

void disp_1char(unsigned char c)
{
	int x, y;
	
	x = cur_x * 6;
	y = cur_y * 16+4;
	if(c<0x20){
		if(c=='\n'){
			clear_cur();
			cur_x = 0;
//			disp_change_data_get();
			disp_D();
			return;
		}
		else if(c=='\t'){					// 4文字毎のタブ
			disp_1char(' ');
			while(cur_x & 0x03){
				disp_1char(' ');
			}
		}
		else if(c=='\b'){
			disp_L();
		}
	}
	else{
		char_disp(x, y, c, 0);
		disp_R();
	}
}

void disp_str(const char *str)
{
	int byte, c, x, y;
	
	for(;;){
		c = sjis_parse(str, &byte);
		if(c==0)
			return;
		if(byte==1){
            disp_1char(c);
//			x = cur_x * 6;
//			y = cur_y * 16+4;
//			char_disp(x, y, c, 0);
//			disp_R();
			str++;
		}
		else{
			if(cur_x <= 72){
				x = cur_x * 6;
				y = cur_y * 16;
				sjis_disp(x, y, c-1, 0);
			}
			else{
				disp_1char('\n');
				x = cur_x * 6;
				y = cur_y * 16;
				sjis_disp(x, y, c-1, 0);
			}
			disp_R();
			disp_R();
			str += 2;
		}
	}
	
//	while(*str) disp_1char(*str++);
}

void ut_printf(const char *fmt, ...)
{
	char buf[256];
	va_list ap;

	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);
	
    disp_str(buf);
}

void cursor_set(int x, int y)
{
	clear_cur();
	if(x<0)
		x = 0;
	if(x>76)
		x = 76;
	if(y<0)
		y = 0;
	if(y>19)
		y = 19;
	cur_x = x;
	cur_y = y;
//	disp_change_data_get();
	disp_cur();
}
