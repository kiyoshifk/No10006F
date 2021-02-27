#include "stdafx.h"
#include "banana-c32.h"
#include "function.h"


/********************************************************************************/
/*		c32_malloc																*/
/********************************************************************************/
char *c32_malloc(unsigned int size)
{
	char *ptr;
	
	if(c32_buff_ptr+size > MAX_MALLOC_AREA){
		c32_error_message(E_OUT_OF_MEMORY, __LINE__, __FILE__);
	}
	ptr = &(cv->buff[c32_buff_ptr]);
	c32_buff_ptr += size;
	return ptr;
}

uint *c32_malloc4(uint size)
{
	c32_buff_ptr += 3;
	c32_buff_ptr &= 0xfffffffc;
	return (uint*)c32_malloc(size);
}
/********************************************************************************/
/*		c32_error_message															*/
/********************************************************************************/
void c32_error_message(int err_no, int line, char *fname)
{
	char *ptr;
	
	if(err_no==0)
		return;								// no error
	printf("*** file=%s prg=%s line=%d *** %s\n", c32_src_filename, fname, line, c32_err_msg[err_no]);
	printf("%s\n", c32_linebuf);
	ptr = c32_linebuf;
	while(ptr != c32_linebufp){
		printf("%c", *ptr++);
	}
	printf("\n");
#ifdef __XC32
	longjmp(c32_env, 1);
#endif
	exit(1);
}
/********************************************************************************/
/*		c32_src_ptr_save															*/
/*		ソースファイルの現在位置を　保存／復元　する							*/
/********************************************************************************/
void c32_src_ptr_save(struct src_ptr *src_ptr)
{
	src_ptr->line_ptr     = strlen(cv->output_buffer);
	src_ptr->c32_linebuf  = c32_linebuf;
	src_ptr->c32_linebufp = c32_linebufp;
}

void c32_src_ptr_restore(struct src_ptr *src_ptr)
{
//	c32_output_buffer_line_numb = src_ptr->line_numb;
	cv->output_buffer[src_ptr->line_ptr] = 0;
	c32_linebuf  = src_ptr->c32_linebuf;
	c32_linebufp = src_ptr->c32_linebufp;
}
/********************************************************************************/
/*		c32_printf																	*/
/********************************************************************************/
int c32_printf(const char *fmt, ...)
{
//	int line_numb;
	int len1, len2;
	va_list ap;
	
	if(c32_no_printx)
		return 0;
	va_start(ap, fmt);
	vsprintf(cv->tmp, fmt, ap);
	va_end(ap);

	len1 = strlen(cv->tmp);
	if(len1+1 > MAX_BUFF_LEN - (len2 = strlen(cv->output_buffer)))
		c32_error_message(E_OUT_OF_MEMORY, __LINE__, __FILE__);
	strcpy(&cv->output_buffer[len2], cv->tmp);
	
	if(len1+len2+1 > c32_max_output_buffer){
		c32_max_output_buffer = len1+len2+1;
	}
	
//printf(cv->tmp);

	return 0;
}
/********************************************************************************/
/*		c32_output_buffer_flush														*/
/********************************************************************************/
void c32_output_buffer_flush()
{
	int i, len, step;
	char buf[50];

	step = sizeof(buf)-1;
	len = strlen(cv->output_buffer);
	for(i=0; i<len; i+=step){
		memcpy(buf, &cv->output_buffer[i], step);
		buf[step] = 0;
		fprintf(c32_asm_fp, buf);
	}
	cv->output_buffer[0] = 0;

//	・ソースバッファのフラッシュとは
//	現在行をバッファの先頭に移動して c32_linebufp/c32_linebuf も移動する
//	現在行の先頭を検出する
//	バッファー終端の検出、終端とは '\0' が２個連続の後ろである

	char *top, *end, *b_top;
	
	b_top = cv->src_buffer;					// ソースバッファトップ
	if(b_top[0]=='\0' && b_top[1]=='\0'){
		if(c32_linebufp != b_top)
			c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
		return;
	}
	top = c32_linebufp;
	if(*top=='\0')
		top--;
	if(top < b_top)
		c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
	while(b_top < top){
		if(*top==0){
			top++;
			break;
		}
		top--;
	}
	
	for(end=c32_linebufp; !(end[0]==0 && end[1]==0); end++)
		;
	end += 2;
	memmove(b_top, top, end-top);
	c32_linebufp -= top - b_top;
	c32_linebuf   = b_top;


//	memset(cv->src_buffer, 0, 10);
//	c32_linebufp = &cv->src_buffer[1];
}
/********************************************************************************/
/*		c32_attr_to_byte															*/
/*		attr から変数のオブジェクトサイズを求める								*/
/********************************************************************************/
int c32_attr_to_byte(int attr)
{
	if((attr & ATTR_INT) || (attr & ATTR_POINTER))
		return 4;
	else if(attr & ATTR_CHAR)
		return 1;
	else if(attr & ATTR_SHORT)
		return 2;
	else{
		printf("attr=%d ", attr);
		c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
	}
	return 0;
}
/********************************************************************************/
/*		c32_make_attr_from_constant													*/
/********************************************************************************/
int c32_make_attr_from_constant(int constant)
{
	unsigned int value;
	
	if(constant >= 0){
		value = constant;
		if(value < 0x80)
			return ATTR_CHAR;
		else if(value < 0x100)
			return ATTR_CHAR | ATTR_UNSIGNED;
		else if(value < 0x8000)
			return ATTR_SHORT;
		else if(value < 0x10000)
			return ATTR_SHORT | ATTR_UNSIGNED;
		else if(value < 0x80000000)
			return ATTR_INT;
		else
			return ATTR_INT | ATTR_UNSIGNED;
	}
	else{
		value = -constant;
		if(value <= 0x80)
			return ATTR_CHAR;
		else if(value <= 0x8000)
			return ATTR_SHORT;
		else
			return ATTR_INT;
	}
}
/********************************************************************************/
/*		char_int_long_unsigned_pointer											*/
/*		return: attr, エラー時は 0												*/
/*		ソースポインタは型宣言をスキップして型宣言でない token の手前になる		*/
/********************************************************************************/
int c32_char_short_int_unsigned_pointer()
{
	int attr, a, tmp, type;
	struct src_ptr src_ptr;
	
	attr = 0;
	for(;;){
		c32_src_ptr_save(&src_ptr);
		c32_token_process(0, &type);
		
		if(type==TYPE_CHAR)
			a = ATTR_CHAR;
		else if(type==TYPE_SHORT)
			a = ATTR_SHORT;
		else if(type==TYPE_INT)
			a = ATTR_INT;
		else if(type==TYPE_LONG)
			a = ATTR_INT;
		else if(type==TYPE_UNSIGNED)
			a = ATTR_UNSIGNED;
		else if(type==TYPE_UCHAR)
			a = ATTR_CHAR | ATTR_UNSIGNED;
		else if(type==TYPE_USHORT)
			a = ATTR_SHORT | ATTR_UNSIGNED;
		else if(type==TYPE_UINT)
			a = ATTR_INT | ATTR_UNSIGNED;
		else if(type==TYPE_ULONG)
			a = ATTR_INT | ATTR_UNSIGNED;
		else if(type==TYPE_VOID)
			a = ATTR_VOID;
		else if(type==TYPE_CONST)
			a = ATTR_CONST;
		else if(type==TYPE_ASTERISK){
			attr += ATTR_POINTER1;
			if((attr & ATTR_POINTER)==0)
				return 0;
			continue;
		}
		else{
			c32_src_ptr_restore(&src_ptr);
			if(attr==0){
				return 0;
			}
			if((attr & ATTR_UNSIGNED) && (attr & (ATTR_CHAR | ATTR_INT | ATTR_SHORT | ATTR_VOID))==0)
				attr |= ATTR_INT;
			tmp = attr & (ATTR_CHAR | ATTR_INT | ATTR_SHORT | ATTR_VOID);
			if(tmp==ATTR_CHAR || tmp==ATTR_INT || tmp==ATTR_SHORT || tmp==ATTR_VOID)
				return attr;
			return 0;
		}
		if(attr & a)
			c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
		attr |= a;
	}
}
/********************************************************************************/
/*		c32_skip_to_type															*/
/*		指定した type が現れるまでスキップする									*/
/*		type の手前で return する												*/
/*		return 1:EOF, 0:normal													*/
/********************************************************************************/
int c32_skip_to_type(int type0, int type1, int type2)
{
	int type;
	struct src_ptr src_ptr;
	
	for(;;){
		c32_src_ptr_save(&src_ptr);
		c32_token_process(0, &type);
		if(type==type0 || type==type1 || type==type2){
			c32_src_ptr_restore(&src_ptr);
			return 0;						// normal end
		}
		if(type==TYPE_EOF)
			return 1;						// EOF
	}
}
/********************************************************************************/
/*		c32_symbol_out																*/
/*		変数シンボルを EQU で出力する											*/
/********************************************************************************/
void c32_symbol_out()
{
	int i;
	struct symtbl *ptr;
	
	c32_printf(";----------  symbol table  ----------\n");
	for(i=0; i<c32_tbl_ptr_g; i++){
		ptr = &cv->tbl_g[i];
		if(ptr->flag1 & FLAG_04){				// global 変数ならば
			if((ptr->attr & ATTR_CONST)==0){
				c32_printf("%s	equ	0x%08X		; size=%d\n", ptr->symbuf, ptr->value, ptr->size);
			}
//			printf("%s %s addr=%08X size=%d dim=%d x=%d y=%d z=%d\n", c32_attr_to_string(ptr->attr), ptr->c32_symbuf, ptr->value, ptr->size, ptr->xyz_dim, ptr->xyz_size[0], ptr->xyz_size[1], ptr->xyz_size[2]);
		}
		c32_output_buffer_flush();
	}
}
/********************************************************************************/
/*		c32_sym_search_global														*/
/*		return 0:undef, シンボルテーブル										*/
/********************************************************************************/
struct symtbl *c32_sym_search_global()
{
	int i;
	
	for(i=0; i<c32_tbl_ptr_g; i++){
		if(strcmp(c32_symbuf, cv->tbl_g[i].symbuf)==0)
			return &cv->tbl_g[i];			// シンボルテーブル発見
	}
	return 0;								// 未登録
}
/********************************************************************************/
/*		c32_sym_search_auto															*/
/*		return 0:undef, シンボルテーブル										*/
/********************************************************************************/
struct symtbl *c32_sym_search_auto(struct symtbl *func)
{
	int i;
	struct symtbl *ptr;
	
	for(i=0; i<func->func_tbl->variable_tbl_cnt; i++){
		ptr = func->func_tbl->variable_tbl[i];
		if(strcmp(c32_symbuf, ptr->symbuf)==0)
			return ptr;						// シンボルテーブル発見
	}
	return 0;								// 未登録
}
/********************************************************************************/
/*		c32_sym_search_all															*/
/*		return 0:undef, シンボルテーブル										*/
/********************************************************************************/
struct symtbl *c32_sym_search_all(struct symtbl *func)
{
	struct symtbl *ptr;
	
	if((ptr=c32_sym_search_auto(func)))
		return ptr;
	return c32_sym_search_global();
}
/********************************************************************************/
/*		c32_attr_to_shift_bit														*/
/*		ポインタか配列の時はオブジェクトサイズをシフトビットで返す				*/
/*		ポインタや配列でなければ 0 を返す										*/
/********************************************************************************/
int c32_attr_to_shift_bit(int attr)
{
	int attr_tmp;
	
	if(attr & ATTR_ARRAY){
		attr_tmp = attr & ~ATTR_ARRAY;
	}
	else if(attr & ATTR_POINTER){
		attr_tmp = attr - ATTR_POINTER1;
	}
	else{
		return 0;
	}

	if((attr_tmp & ATTR_INT) || (attr_tmp & (ATTR_POINTER | ATTR_ARRAY)))
		return 2;
	else if(attr_tmp & ATTR_CHAR)
		return 0;
	else if(attr_tmp & ATTR_SHORT)
		return 1;
	else{
		printf("attr=%d ", attr);
		c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
	}
	return 0;
}
/********************************************************************************/
/*		c32_skip_to_R_KAKKO															*/
/********************************************************************************/
void c32_skip_to_R_KAKKO(int cnt)
{
	int type;
	
	for(; ; ){
		c32_token_process(0, &type);
		if(type==TYPE_R_KAKKO){
			if(--cnt==0)
				return;
		}
		else if(type==TYPE_L_KAKKO){
			cnt++;
		}
		else if(type==TYPE_EOF){
			c32_error_message(E_UNEXPECTED_EOF, __LINE__, __FILE__);
		}
	}
}

/********************************************************************************/
/*		c32_fprintf																*/
/********************************************************************************/
#ifdef __XC32
void c32_fprintf(SYS_FS_HANDLE fd, const char *fmt, ...)
{
	char buf[256];
	va_list ap;

	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);
	
	if(SYS_FS_FileWrite(fd, buf, strlen(buf)) != strlen(buf)){
		ut_error_msg("write");
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	}
}
#endif
