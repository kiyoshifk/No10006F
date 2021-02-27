	#include "stdafx.h"
#ifndef __XC32
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <ctype.h>
	#include <setjmp.h>
#else
	#include "app.h"
	#define fprintf			prep_fprintf
#endif


#define VERSION				"prep V101"
#define MAX_SYMBUF_LEN		64
#define MAX_INFILE			4
#define MAX_DEFINE			200
#define MAX_PARAMETER		10

#define RAM_BASE		0xa0010000

struct include_data{
#ifndef __XC32
	FILE *in_fp;
#else
	SYS_FS_HANDLE in_fp;
#endif
	char filename[MAX_SYMBUF_LEN];
	int line_numb;
};

struct define_data{
	char *symbuf;
	char *text;
	char *param[MAX_PARAMETER];
};

static struct prepv{
	char buff[0x8000];
	char linebuf[1024];
	char str[1024];
	struct include_data inc_data[MAX_INFILE];
	struct define_data define_data[MAX_DEFINE];
} *prepv = (struct prepv *)RAM_BASE;

static int include_ptr;
static int define_ptr;
static int buff_ptr;
static int in_fp_ptr;
static int line_numb;
static char symbuf[MAX_SYMBUF_LEN];
static char *linebufp;
static jmp_buf env;
#ifndef __XC32
static FILE *out_fp;
#else
static SYS_FS_HANDLE out_fp;
#endif


#define E_NOT_SYMBOL				1
#define E_TOO_LONG_SYMBOL			2
#define E_SYNTAX_ERROR				3
#define E_OUT_OF_MEMORY				4
#define E_MULTIPLE_DEFINITION		5
#define E_FILE_ERROR				6
#define E_PARAM_NUMB_ERROR			7

static const char *err_msg[]={
	"NO_ERROR",						// 0
	"E_NOT_SYMBOL",					// 1
	"E_TOO_LONG_SYMBOL",			// 2
	"E_SYNTAX_ERROR",				// 3
	"E_OUT_OF_MEMORY",				// 4
	"E_MULTIPLE_DEFINITION",		// 5
	"E_FILE_ERROR",					// 6
	"E_PARAM_NUMB_ERROR",			// 7
};

/********************************************************************************/
/*		error_message															*/
/********************************************************************************/
static void error_message(int err_no, int line, char *filename)
{
	if(err_no==0)
		return;								// no error
	printf("file=%s line=%d *** %s\n", filename, line, err_msg[err_no]);
	printf("%7d: %s", line_numb, prepv->linebuf);
#ifndef __XC32
	exit(1);
#else
	longjmp(env, 1);
#endif
}
/********************************************************************************/
/*		prep_fprintf																*/
/********************************************************************************/
#ifdef __XC32
static void prep_fprintf(SYS_FS_HANDLE fd, const char *fmt, ...)
{
	char buf[256];
	va_list ap;

	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);
	
	if(SYS_FS_FileWrite(fd, buf, strlen(buf)) != strlen(buf)){
		ut_error_msg("write");
		error_message(E_FILE_ERROR, __LINE__, __FILE__);
	}
}
#endif
/********************************************************************************/
/*		prep_malloc																*/
/********************************************************************************/
static char *prep_malloc(unsigned int size)
{
	char *ptr;
	
	if(buff_ptr+size > 0x8000){
		error_message(E_OUT_OF_MEMORY, __LINE__, __FILE__);
	}
	ptr = &(prepv->buff[buff_ptr]);
	buff_ptr += size;
	return ptr;
}
/********************************************************************************/
/*		spskip																	*/
/********************************************************************************/
static void spskip()
{
	while(*linebufp==' ' || *linebufp=='\t')
		linebufp++;
}
/********************************************************************************/
/*		is_sym_top																*/
/********************************************************************************/
static int is_sym_top()
{
	spskip();
	if(*linebufp=='_' || isalpha(*linebufp) || *linebufp=='@')
		return 1;							// シンボルトップである
	return 0;								// シンボルトップではない
}
/********************************************************************************/
/*		getsym																	*/
/*		return 0:success														*/
/********************************************************************************/
static void getsym()
{
	int i;
	
	memset(symbuf, 0, sizeof(symbuf));
//	spskip();
	if(is_sym_top()==0)
		error_message(E_NOT_SYMBOL, __LINE__, __FILE__);
	for(i=0; i<MAX_SYMBUF_LEN-1; i++){
		if(*linebufp=='_' || isalnum(*linebufp) || *linebufp=='@')
			symbuf[i] = *linebufp++;
		else
			break;
	}
	if(i==MAX_SYMBUF_LEN-1)
		error_message(E_TOO_LONG_SYMBOL, __LINE__, __FILE__);
}
/********************************************************************************/
/*		get_bun																	*/
/*		linebufp の位置から ',' 又は ')' まで取り込み							*/
/********************************************************************************/
void get_bun(char buf[MAX_SYMBUF_LEN])
{
	int i;
	
	memset(buf, 0, MAX_SYMBUF_LEN);
	for(i=0; i<MAX_SYMBUF_LEN-1; i++){
		if(*linebufp==',' || *linebufp==')'){
			return;
		}
		buf[i] = *linebufp++;
	}
}
/********************************************************************************/
/*		sym_change_exec															*/
/*		シンボルが define_data に登録されていた、def_ptr->symbuf				*/
/*		linebufp_save: ソースのシンボルの頭										*/
/********************************************************************************/
void sym_change_exec(char *linebufp_save, struct define_data *def_ptr)
{
	int i, text_len, sym_len;
	char *linebufp_sv, new_ptr[MAX_SYMBUF_LEN];
	
//AAAAA	spskip();
	strcpy(prepv->str, def_ptr->text);
	if(def_ptr->param[0] && *linebufp=='('){					// 関数型置き換え
		linebufp++;
		spskip();
		for(i=0; i<MAX_PARAMETER; i++){
			if(def_ptr->param[i]==0)
				break;
			spskip();
			get_bun(new_ptr);			// new_ptr = new 文(',' か ')' まで)
			linebufp_sv = linebufp;		// ソースのポインタ
			// old: def_ptr->param[i]
			linebufp = prepv->str;		// 置き換えテキストストリング
			for(;;){
				if(is_sym_top()){
					getsym();
					if(strcmp(symbuf, def_ptr->param[i])==0){	// 一致した
						sym_len = strlen(def_ptr->param[i]);
						text_len = strlen(new_ptr);
						memmove(linebufp-sym_len+text_len, linebufp, strlen(linebufp)+1);
						memmove(linebufp-sym_len, new_ptr, text_len);
						linebufp = linebufp-sym_len+text_len;
					}
					// 不一致
				}
				else if(*linebufp=='\0'){
					linebufp = linebufp_sv;
					break;
				}
				else{
					linebufp++;
				}
			}
			spskip();
			if(*linebufp==','){
				linebufp++;
			}
			else{
				break;
			}
		}
		if(*linebufp++ != ')'){
			error_message(E_PARAM_NUMB_ERROR, __LINE__, __FILE__);
		}
	}
	sym_len = strlen(def_ptr->symbuf);
	text_len = strlen(prepv->str);
	memmove(linebufp_save+text_len, linebufp, strlen(linebufp)+1);
	memmove(linebufp_save, prepv->str, text_len);
	linebufp = linebufp_save+text_len;
}
/********************************************************************************/
/*		sym_change																*/
/*		ソースのシンボルの頭です、define のテキスト置き換えチェック				*/
/********************************************************************************/
static int sym_change()
{
	int i;
	char *linebufp_save;
	struct define_data *def_ptr;
	
	linebufp_save = linebufp;
	getsym();								// symbuf: 置き換えるべきシンボルかもしれない
	for(i=0; i<define_ptr; i++){
		def_ptr = &prepv->define_data[i];
		if(strcmp(symbuf, def_ptr->symbuf)==0){	// 発見：置き換えるべき
			sym_change_exec(linebufp_save, def_ptr);
			linebufp = linebufp_save;
			
//			sym_len = strlen(symbuf);
//			text_len = strlen(def_ptr->text);
//			memmove(linebufp_save+text_len, linebufp, strlen(linebufp)+1);
//			memmove(linebufp_save, def_ptr->text, text_len);
//			linebufp = linebufp_save+text_len;
			return 1;						// 置き換えをやった
		}
	}
	return 0;								// 置き換えなかった
}
/********************************************************************************/
/*		parser_normal_line														*/
/********************************************************************************/
static int parser_normal_line()
{
	for(;;){
		if(is_sym_top()){
			sym_change();	// シンボルだから置き換えテストをする
		}
		else if(*linebufp=='\n' || *linebufp=='\r'){
			return 0;						// normal
		}
		else if(linebufp[0]=='/' && linebufp[1]=='/'){	// "//"
			return 0;						// normal
		}
		else if(linebufp[0]=='/' && linebufp[1]=='*'){	// "/*"
			linebufp += 2;
			for(;;){
				if(linebufp[0]=='*' && linebufp[1]=='/'){	// "*/"
					linebufp += 2;
					break;	// この行の処理は継続
				}
				else if(*linebufp=='\n' || *linebufp=='\r'){
					return 1;				// マルチラインコメント中
				}
				else{
					linebufp++;
				}
			}
		}
		else if(*linebufp=='\"'){	// 文字列スキップ
			linebufp++;
			while(1){
				if(*linebufp=='\n' || *linebufp=='\r'){
					return 0;
				}
				else if(*linebufp=='\"'){
					linebufp++;
					break;
				}
				linebufp++;
			}
		}
		else{
			linebufp++;
		}
	}
}
/********************************************************************************/
/*		include_process															*/
/********************************************************************************/
static void include_process()
{
	int ptr;
	struct include_data *inc_ptr;
	
	ptr = 0;
	memset(symbuf, 0, sizeof(symbuf));
	spskip();
	if(*linebufp++ != '\"'){
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	}
	for(;;){
		if(*linebufp=='\n' || *linebufp=='\r'){
			error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
		}
		if(*linebufp=='\"'){				// ファイル名取り込み完了 → symbuf
			break;
		}
		if(ptr >= MAX_SYMBUF_LEN-1){
			error_message(E_OUT_OF_MEMORY, __LINE__, __FILE__);
		}
		symbuf[ptr++] = *linebufp++;
	}
	if(in_fp_ptr >= MAX_INFILE-1){
		error_message(E_OUT_OF_MEMORY, __LINE__, __FILE__);
	}
	prepv->inc_data[in_fp_ptr].line_numb = line_numb;
	in_fp_ptr++;
	inc_ptr = &prepv->inc_data[in_fp_ptr];

#ifndef __XC32
	inc_ptr->in_fp = fopen(symbuf, "r");
	if(inc_ptr->in_fp==0){
		error_message(E_FILE_ERROR, __LINE__, __FILE__);
	}
#else
	inc_ptr->in_fp = SYS_FS_FileOpen(symbuf, SYS_FS_FILE_OPEN_READ);
	if(inc_ptr->in_fp==SYS_FS_HANDLE_INVALID){
		ut_error_msg("open");
		error_message(E_FILE_ERROR, __LINE__, __FILE__);
	}
#endif

	strcpy(inc_ptr->filename, symbuf);
//	inc_ptr->line_numb = line_numb;
	line_numb = 0;
	fprintf(out_fp, "%4d:   /*#$& %s */\n", 0, inc_ptr->filename);
}
/********************************************************************************/
/*		define_process															*/
/********************************************************************************/
static void define_process()
{
	int i, param_numb;
	struct define_data *def_ptr;
	
	getsym();
	if(define_ptr >= MAX_DEFINE){
		error_message(E_OUT_OF_MEMORY, __LINE__, __FILE__);
	}
	for(i=0; i<define_ptr; i++){
		if(strcmp(symbuf, prepv->define_data[i].symbuf)==0){
			error_message(E_MULTIPLE_DEFINITION, __LINE__, __FILE__);
		}
	}
	def_ptr = &prepv->define_data[define_ptr];
	def_ptr->symbuf = prep_malloc(strlen(symbuf)+1);
	strcpy(def_ptr->symbuf, symbuf);

	if(*linebufp=='('){
		linebufp++;
		for(param_numb=0; param_numb<MAX_PARAMETER; param_numb++){
			spskip();
			if(*linebufp==')'){
				linebufp++;
				break;
			}
			getsym();
			def_ptr->param[param_numb] = prep_malloc(strlen(symbuf)+1);
			strcpy(def_ptr->param[param_numb], symbuf);
			
			spskip();
			if(*linebufp==')'){
				linebufp++;
				break;
			}
			else if(*linebufp==','){
				linebufp++;
			}
			else{
				error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
			}
		}
	}
	spskip();
	for(i=0; ; i++){
		if(linebufp[0]=='/' && linebufp[1]=='/'){	// "//"
			break;
		}
		if(*linebufp=='\n' || *linebufp=='\r'){
			break;
		}
		prepv->linebuf[i] = *linebufp++;
	}
	prepv->linebuf[i] = '\0';
	def_ptr->text = prep_malloc(strlen(prepv->linebuf)+1);
	strcpy(def_ptr->text, prepv->linebuf);
	define_ptr++;
}
/********************************************************************************/
/*		parser																	*/
/********************************************************************************/
static void parser()
{
	int in_comment_flag;
	struct include_data *inc_ptr;
	
	in_comment_flag = 0;
	for(;;){
#ifndef __XC32
		while(fgets(prepv->linebuf, sizeof(prepv->linebuf), prepv->inc_data[in_fp_ptr].in_fp)){
#else
		while(SYS_FS_FileStringGet(prepv->inc_data[in_fp_ptr].in_fp, prepv->linebuf, sizeof(prepv->linebuf))==SYS_FS_RES_SUCCESS){
#endif
			line_numb++;
			linebufp = prepv->linebuf;
			if(in_comment_flag==0){			// 通常の状態
				spskip();
				if(*linebufp=='#'){			// #include か #define の処理
					linebufp++;
					getsym();
					if(strcmp(symbuf, "include")==0){
						include_process();
					}
					else if(strcmp(symbuf, "define")==0){
						define_process();
					}
					else{
						error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
					}
				}
				else{						// C言語の通常の行
					in_comment_flag = parser_normal_line();
					fprintf(out_fp, "%4d: %s", line_numb, prepv->linebuf);
				}
			}
			else{							// multi line comment 内
				for(;;){
					if(linebufp[0]=='*' && linebufp[1]=='/'){
						linebufp += 2;
						break;
					}
					else{
						linebufp++;
					}
				}
				in_comment_flag = parser_normal_line();
				fprintf(out_fp, "%4d: %s", line_numb, prepv->linebuf);
			}
		}
#ifndef __XC32
		fclose(prepv->inc_data[in_fp_ptr].in_fp);
#else
		SYS_FS_FileClose(prepv->inc_data[in_fp_ptr].in_fp);
#endif
		if(in_fp_ptr){						// #include のネスト内
			in_fp_ptr--;
			inc_ptr = &prepv->inc_data[in_fp_ptr];
			line_numb = inc_ptr->line_numb;
			fprintf(out_fp, "%4d:   /*#$& %s */\n", 0, inc_ptr->filename);
			continue;
		}
		return;
	}
}
/********************************************************************************/
/*		main																	*/
/********************************************************************************/
#ifndef __XC32
int main(int argc, char *argv[])
{
	struct include_data *inc_ptr;
	
	printf("%s %s %s\n", VERSION, __DATE__, __TIME__);
	prepv = (struct prepv *)malloc(sizeof(struct prepv));
	if(prepv==0){
		printf("*** out of memory\n");
		return 1;
	}
	memset(prepv, 0, sizeof(struct prepv));
	if(argc != 3){
		printf("*** error command line\n");
		printf("$ prep <in_file> <out_file>\n");
		return 1;
	}
	/***	file open	***/
	inc_ptr = &prepv->inc_data[0];
	if((inc_ptr->in_fp = fopen(argv[1], "r"))==0){
		printf("*** in file open error %s\n", argv[1]);
		return 1;
	}
	if((out_fp=fopen(argv[2], "w"))==0){
		printf("*** out file open error %s\n", argv[2]);
		return 1;
	}
	
	inc_ptr->line_numb = 0;
	strcpy(inc_ptr->filename, argv[1]);
	fprintf(out_fp, "%4d:   /*#$& %s */\n", 0, inc_ptr->filename);
	
	parser();
	
	fclose(inc_ptr->in_fp);
	fclose(out_fp);
    return 0;
}
#else
int prep(const char *filename)
{
	char *buf = prepv->linebuf;
	char *ptr;
	struct include_data *inc_ptr;
	
	if(setjmp(env)){
		SYS_FS_FileClose(prepv->inc_data[0].in_fp);
		SYS_FS_FileClose(out_fp);
		return 1;
	}
	printf("%s %s %s\n", VERSION, __DATE__, __TIME__);
	memset(prepv, 0, sizeof(struct prepv));
	include_ptr = 0;
	define_ptr = 0;
	buff_ptr = 0;
	in_fp_ptr = 0;
	line_numb = 0;
	
	/***	make file name base	***/
	strcpy(buf, filename);
	ptr = strrchr(buf, '.');
	if(ptr){
		*ptr = '\0';
	}
	
	/***	file open	***/
	inc_ptr = &prepv->inc_data[0];
	if((inc_ptr->in_fp = SYS_FS_FileOpen(filename, SYS_FS_FILE_OPEN_READ))==SYS_FS_HANDLE_INVALID){
		ut_error_msg("src open");
		return 1;							// error
	}
	strcat(buf, ".prep");
	SYS_FS_FileDirectoryRemove(buf);
	if((out_fp=SYS_FS_FileOpen(buf, SYS_FS_FILE_OPEN_WRITE))==SYS_FS_HANDLE_INVALID){
		SYS_FS_FileClose(inc_ptr->in_fp);
		ut_error_msg("hex open");
		return 1;							// error
	}
	
	inc_ptr->line_numb = 0;
	strcpy(inc_ptr->filename, filename);
	fprintf(out_fp, "%4d:   /*#$& %s */\n", 0, inc_ptr->filename);
	
	parser();
	
//	SYS_FS_FileClose(inc_ptr->in_fp);
	SYS_FS_FileClose(out_fp);
	return 0;
}
#endif
