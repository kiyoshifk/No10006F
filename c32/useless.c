#include "stdafx.h"
#include "banana-c32.h"
#include "function.h"

/********************************************************************************/
/*		c32_type_to_string															*/
/********************************************************************************/
char *c32_type_to_string(uchar type)
{
	switch(type){
		case TYPE_SYMBOL:
			return "SYMBOL";
		case TYPE_L_KAKKO:
			return "(";
		case TYPE_R_KAKKO:
			return ")";
		case TYPE_L_NAMIKAKKO:
			return "{";
		case TYPE_R_NAMIKAKKO:
			return "}";
		case TYPE_L_KAKUKAKKO:
			return "[";
		case TYPE_R_KAKUKAKKO:
			return "]";
		case TYPE_SEMIKORON:
			return ";";
		case TYPE_KORON:
			return ":";
		case TYPE_PLUS:
			return "+";
		case TYPE_MINUS:
			return "-";
		case TYPE_PLUS_PLUS:
			return "++";
		case TYPE_MINUS_MINUS:
			return "--";
		case TYPE_ASTERISK:
			return "*";
		case TYPE_SLUSH:
			return "/";
		case TYPE_PERCENT:
			return "%";
		case TYPE_AND:
			return "&";
		case TYPE_OR:
			return "|";
		case TYPE_KANMA:
			return ",";
		case TYPE_EQ:
			return "=";
		case TYPE_EQEQ:
			return "==";
		case TYPE_NOTEQ:
			return "!=";
		case TYPE_GT:
			return ">";
		case TYPE_GE:
			return ">=";
		case TYPE_LT:
			return "<";
		case TYPE_LE:
			return "<=";
		case TYPE_PLUS_EQ:
			return "+=";
		case TYPE_MINUS_EQ:
			return "-=";
		case TYPE_ASTERISK_EQ:
			return "*=";
		case TYPE_SLUSH_EQ:
			return "/=";
		case TYPE_PERSENT_EQ:
			return "%=";
		case TYPE_OR_EQ:
			return "|=";
		case TYPE_AND_EQ:
			return "&=";
		case TYPE_L_SHIFT:
			return "<<";
		case TYPE_R_SHIFT:
			return ">>";
		case TYPE_XOR:
			return "^";
		case TYPE_L_SHIFT_EQ:
			return "<<=";
		case TYPE_R_SHIFT_EQ:
			return ">>=";
		case TYPE_XOR_EQ:
			return "^=";
		case TYPE_OR_OR:
			return "||";
		case TYPE_AND_AND:
			return "&&";
		case TYPE_QUESTION:
			return "?";
		case TYPE_UNSIGNED:
			return "unsigned";
		case TYPE_CHAR:
			return "char";
		case TYPE_INT:
			return "int";
		case TYPE_LONG:
			return "long";
		case TYPE_IF:
			return "if";
		case TYPE_ELSE:
			return "else";
		case TYPE_RETURN:
			return "return";
		case TYPE_GOTO:
			return "goto";
		case TYPE_WHILE:
			return "while";
		case TYPE_CONTINUE:
			return "continue";
		case TYPE_BREAK:
			return "break";
		case TYPE_STRING:
			return "string";
		case TYPE_FOR:
			return "for";
		case TYPE_INTERRUPT:
			return "interrupt";
		case TYPE_ASM:
			return "asm";
		case TYPE_TEST:
			return "c32_test";
		case TYPE_UCHAR:
			return "uchar";
		case TYPE_UINT:
			return "uint";
		case TYPE_ULONG:
			return "ulong";
		case TYPE_DIGIT:
			return "digit";
		case TYPE_EOF:
			return "EOF";
		case TYPE_ERROR:
			return "error";
		default:
			return "=== default ===";
	}
}
/********************************************************************************/
/*		c32_attr_to_string															*/
/********************************************************************************/
char *c32_attr_to_string(int attr)
{
	static char buf[128];
	
	buf[0] = '\0';
	if(attr & ATTR_UNSIGNED)
		strcat(buf, "unsinged ");
	if(attr & ATTR_CHAR)
		strcat(buf, "char ");
	if(attr & ATTR_SHORT)
		strcat(buf, "short ");
	if(attr & ATTR_INT)
		strcat(buf, "int ");
	if((attr&ATTR_POINTER)==0x10)
		strcat(buf, "* ");
	if((attr&ATTR_POINTER)==0x20)
		strcat(buf, "** ");
	if((attr&ATTR_POINTER)==0x30)
		strcat(buf, "*** ");
//	if(attr & ATTR_ARRAY)
//		strcat(buf, "[]");
	while(attr & ATTR_ARRAY){
		strcat(buf, "[]");
		attr -= ATTR_ARRAY1;
	}
	if(attr & ATTR_FUNC)
		strcat(buf, "()");
	return buf;
}
/********************************************************************************/
/*		c32_symtbl_dump_auto														*/
/********************************************************************************/
void c32_symtbl_dump_auto(struct symtbl *func)
{
	int i, cnt;
	struct symtbl *ptr;
	
	printf("---------- auto symtbl %s ----------\n", func->symbuf);fflush(stdout);
	cnt = func->func_tbl->variable_tbl_cnt;
	if(cnt==0){
		printf("<<< empty >>>\n");fflush(stdout);
		return;
	}
	for(i=0; i<cnt; i++){
		ptr = func->func_tbl->variable_tbl[i];
		if(ptr==0){
			printf("*** error, ptr==0\n");
			return;
		}
		if(ptr->symbuf==0){
			printf("*** c32_symbuf==0\n");
			return;
		}
		printf("%s %s addr=%08X size=%d init=%d\n", c32_attr_to_string(ptr->attr), ptr->symbuf, ptr->value, ptr->size, ptr->init);fflush(stdout);
	}
}
/********************************************************************************/
/*		c32_symtbl_dump_global														*/
/********************************************************************************/
void c32_symtbl_dump_global()
{
	int i;
	struct symtbl *ptr;
	
	printf("---------- global symtbl ----------\n");fflush(stdout);
	if(c32_tbl_ptr_g==0){
		printf("<<< empty >>>\n");fflush(stdout);
		return;
	}
	for(i=0; i<c32_tbl_ptr_g; i++){
		ptr = &cv->tbl_g[i];
		if(ptr->type==TYPE_SYMBOL)
			printf("%s %s addr=%08X size=%d init=%d\n", c32_attr_to_string(ptr->attr), ptr->symbuf, ptr->value, ptr->size, ptr->init);fflush(stdout);
	}
}
