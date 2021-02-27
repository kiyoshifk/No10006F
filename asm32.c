#include "app.h"

#define fprintf		asm_fprintf

#define VERSION				"asm32 V300"
#define MAX_SYMBUF_LEN		64
#define MAX_SYMBOL_TABLE	1000


#define NO_OPCODE			0x01
#define NO_LINEBUF			0x02
#define ONE_BYTE			0x04


#define RAM_BASE			0xa0010000	// アプリ用のエリアを使用する為


#define TYPE_LABEL			1

#define TYPE_Ij2			100
#define TYPE_Ij3			101
#define TYPE_Ii				102
#define TYPE_Ii2			103
#define TYPE_J				104
#define TYPE_R				105
#define TYPE_Rx				106
#define TYPE_R1				107
#define TYPE_R2				108
#define TYPE_Rs				109
#define TYPE_Rm				110
#define TYPE_NOP			111
#define TYPE_MTHI			112
#define TYPE_MFHI			113
#define TYPE_LI				114
#define TYPE_LA				115
#define TYPE_LS				116
#define TYPE_MOVE			117

#define TYPE_END			200
#define TYPE_ORG			201
#define TYPE_EQU			202
#define TYPE_DW				203
#define TYPE_DH				204
#define TYPE_DB				205


struct symtbl{
	char *symbuf;
	int type;
	unsigned int value;
};

static struct symtbl reg_tbl[]={
	{"zero", TYPE_LABEL , 0},
	{"at",   TYPE_LABEL , 1},
	{"v0",   TYPE_LABEL , 2},
	{"v1",   TYPE_LABEL , 3},
	{"a0",   TYPE_LABEL , 4},
	{"a1",   TYPE_LABEL , 5},
	{"a2",   TYPE_LABEL , 6},
	{"a3",   TYPE_LABEL , 7},
	{"t0",   TYPE_LABEL , 8},
	{"t1",   TYPE_LABEL , 9},
	{"t2",   TYPE_LABEL , 10},
	{"t3",   TYPE_LABEL , 11},
	{"t4",   TYPE_LABEL , 12},
	{"t5",   TYPE_LABEL , 13},
	{"t6",   TYPE_LABEL , 14},
	{"t7",   TYPE_LABEL , 15},
	{"s0",   TYPE_LABEL , 16},
	{"s1",   TYPE_LABEL , 17},
	{"s2",   TYPE_LABEL , 18},
	{"s3",   TYPE_LABEL , 19},
	{"s4",   TYPE_LABEL , 20},
	{"s5",   TYPE_LABEL , 21},
	{"s6",   TYPE_LABEL , 22},
	{"s7",   TYPE_LABEL , 23},
	{"t8",   TYPE_LABEL , 24},
	{"t9",   TYPE_LABEL , 25},
	{"k0",   TYPE_LABEL , 26},
	{"k1",   TYPE_LABEL , 27},
	{"gp",   TYPE_LABEL , 28},
	{"sp",   TYPE_LABEL , 29},
	{"fp",   TYPE_LABEL , 30},
	{"ra",   TYPE_LABEL , 31},
	
	{"end",  TYPE_END, 0},
	{"org",  TYPE_ORG, 0},
	{"equ",  TYPE_EQU, 0},
	{"dw",   TYPE_DW, 0},
	{"dh",   TYPE_DH, 0},
	{"db",   TYPE_DB, 0},
	
	{"add",  TYPE_Rx, 0x20},		// add   $rd,$rs,$rt ... rd = rs+rt
	{"addu", TYPE_Rx, 0x21},		// addu  $rd,$rs,$rt ... rd = rs+rt
	{"addi", TYPE_Ii, 0x20000000},	// addi  $rd,$rs,Imm ... rd = rs+Imm
	{"addiu",TYPE_Ii, 0x24000000},	// addiu $rd,$rs,Imm ... rd = rs+Imm
	{"and",  TYPE_Rx, 0x24},
	{"andi", TYPE_Ii, 0x30000000},
	{"div",  TYPE_Rm, 0x1a},		// div   $rs,$rt
	{"divu", TYPE_Rm, 0x1b},		// divu  $rs,$rt
	{"mult", TYPE_Rm, 0x18},		// mult  $rs,$rt
	{"multu",TYPE_Rm, 0x19},		// multu $rs,$rt
	{"nor",  TYPE_Rx, 0x27},		// nor  $rd,$rs,$rt ... rd = ~(rs | rt)
	{"or",   TYPE_Rx, 0x25},
	{"ori",  TYPE_Ii, 0x34000000},
	{"sll",  TYPE_Rs, 0x00},		// sll  $rd,$rt,shamt ... rd = rt << shamt
	{"sllv", TYPE_R,  0x04},		// sllv $rd,$rt,$rs   ... rd = rt << rs
	{"sra",  TYPE_Rs, 0x03},
	{"srav", TYPE_R,  0x07},
	{"srl",  TYPE_Rs, 0x02},
	{"srlv", TYPE_R,  0x06},
	{"ror",  TYPE_Rs, 0x200002},
	{"rorv", TYPE_R,  0x46},
	{"sub",  TYPE_Rx, 0x22},
	{"subu", TYPE_Rx, 0x23},
	{"xor",  TYPE_Rx, 0x26},
	{"xori", TYPE_Ii, 0x38000000},

	{"lui",  TYPE_Ii2, 0x3c000000},	// lui   $rt,Imm       ... rt[31:16] = Imm
	{"li",   TYPE_LI,  0x24000000},	// li $rd,Imm (addiu $rd,$zero,Imm)
	{"la",   TYPE_LA,0},			// la $rd,Imm (lui $at,Imm上位、ori $rd,$at,Imm下位)
	{"move", TYPE_MOVE,0x21},		// move $rd,$rt (addu $rd,$zero,$rt)

	{"slt",  TYPE_Rx,  0x2a},		// slt   $rd,$rs,$rt   ... rd = rs<rt ? 1 : 0;
	{"sltu", TYPE_Rx,  0x2b},		// sltu  $rd,$rs,$rt   ... rd = rs<rt ? 1 : 0;
	{"slti", TYPE_Ii,  0x28000000},	// slti  $rd,$rs,Imm   ... rd = rs<Imm ? 1 : 0;
	{"sltiu",TYPE_Ii,  0x2c000000},	// sltiu $rd,$rs,Imm   ... rd = rs<Imm ? 1 : 0;

//	{"b"				// b      label         ... branch
	{"beq",  TYPE_Ij3, 0x10000000},	// beq    $rs,$rt,label ... branch if rs==rt
	{"bgez", TYPE_Ij2, 0x04010000},	// bgez   $rs,label     ... branch if rs>= 0
	{"bgezal",TYPE_Ij2,0x04110000},	// bgezal $rs,label     ... bgez + $ra=次命令
	{"bgtz", TYPE_Ij2, 0x1c000000},	// bgtz   $rs,label     ... branch if rs>0
	{"blez", TYPE_Ij2, 0x18000000},	// blez   $rs,label     ... branch if rs<=0
	{"bltz", TYPE_Ij2, 0x04000000},	// bltz   $rs,label     ... branch if rs<0
	{"bltzal",TYPE_Ij2,0x04100000},	// bltzal $rs,label     ... bltz + $ra=次命令
	{"bne",  TYPE_Ij3, 0x14000000},	// bne    $rs,rt,label  ... branch if rs != rt
	{"beqz", TYPE_Ij2, 0x10000000},	// beqz   $rs,label     ... branch if rs==0
	{"bnez", TYPE_Ij2, 0x14000000},	// bnez   $rs,label     ... branch if rs != 0

	{"j",    TYPE_J,   0x08000000},	// j      target        ... jump
	{"jal",  TYPE_J,   0x0c000000},	// jal    target        ... ジャンプ先から jr $sa で復帰できる
	{"jalr", TYPE_R2,  0x09},		// jalr   $rd,$rs       ... rd 次命令格納、rs とび先
	{"jr",   TYPE_R1,  0x08},		// jr     rs            ... jump rs

	{"lb",   TYPE_LS,  0x80000000},	// lb   $rt,offset($rs) ... rt = *((char*)address)
	{"lbu",  TYPE_LS,  0x90000000},
	{"lh",   TYPE_LS,  0x84000000},	// lh   $rt,offset($rs) ... rt = *((short*)address)
	{"lhu",  TYPE_LS,  0x94000000},
	{"lw",   TYPE_LS,  0x8c000000},	// lw   $rt,offset($rs) ... rt = *((short*)address)

	{"sb",   TYPE_LS,  0xa0000000},	// sb   $rt,offset($rs) ... *((char*)address) = rt
	{"sh",   TYPE_LS,  0xa4000000},	// sh   $rt,offset($rs) ... *((short*)address) = rt
	{"sw",   TYPE_LS,  0xac000000},	// sw   $rt,offset($rs) ... *((int*)address) = rt

	{"mfhi", TYPE_MFHI, 0x10},		// mfhi $rd          ... rd = hi
	{"mflo", TYPE_MFHI, 0x12},		// mflo $rd          ... rd = lo
	{"mthi", TYPE_MTHI, 0x11},		// mthi $rs          ... hi = rs
	{"mtlo", TYPE_MTHI, 0x13},		// mtlo $rs          ... lo = rs
//	{"movn"				// movn $rd,$rs,$rt  ... rd=rs if rt!=0
//	{"movz"				// movz $rd,$rs,$rt  ... rd=rs if rt==0

	{"nop",  TYPE_NOP, 0x00000000},

	{0,0,0}
};

static struct asm32v{
	struct symtbl tbl[MAX_SYMBOL_TABLE];
	char buff[0x8000];
	char linebuf[1024];
} *asm32v = (struct asm32v *)RAM_BASE;


static int pass;
static unsigned int prg_counter;
static int line_number;
static int buff_ptr;
static int tbl_ptr;
static char *linebufp;
static char symbuf[MAX_SYMBUF_LEN];
static SYS_FS_HANDLE src_fp, hex_fp, lst_fp;
static jmp_buf env;


static void error_message(int err_no, int line, char *filename);


#define E_NOT_SYMBOL				1
#define E_OUT_OF_MEMORY				2
#define E_TOO_LONG_SYMBOL			3
#define E_MULTIPLE_DEFINITION		4
#define E_INTERNAL_ERROR			5
#define E_END_DETECTED				6
#define E_UNDEFINED_SYMBOL			7
#define E_SYNTAX_ERROR				8
#define E_ADDRESS_OUT				9
#define E_FILE_ERROR                10

static const char *err_msg[]={
	"NO_ERROR",						// 0
	"E_NOT_SYMBOL",					// 1
	"E_OUT_OF_MEMORY",				// 2
	"E_TOO_LONG_SYMBOL",			// 3
	"E_MULTIPLE_DEFINITION",		// 4
	"E_INTERNAL_ERROR",				// 5
	"E_END_DETECTED",				// 6
	"E_UNDEFINED_SYMBOL",			// 7
	"E_SYNTAX_ERROR",				// 8
	"E_ADDRESS_OUT",				// 9
    "E_FILE_ERROR",                 // 10
};

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
	if(*linebufp=='_' || isalpha(*linebufp) || *linebufp=='@')
		return 1;							// シンボルトップである
	return 0;								// シンボルトップではない
}
/********************************************************************************/
/*		asm_fprintf																*/
/********************************************************************************/
static void asm_fprintf(SYS_FS_HANDLE fd, const char *fmt, ...)
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
/********************************************************************************/
/*		is_line_end																*/
/********************************************************************************/
static int is_line_end()
{
	if(*linebufp==';' || *linebufp=='\n' || *linebufp=='\r' || *linebufp=='\0')
		return 1;							// 行末だった
	return 0;								// 行末ではない
}
/********************************************************************************/
/*		test_line_end															*/
/********************************************************************************/
static void test_line_end()
{
	spskip();
	if(is_line_end()==0)
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
}
/********************************************************************************/
/*		getsym																	*/
/*		return 0:success														*/
/********************************************************************************/
static void getsym()
{
	int i;
	
	memset(symbuf, 0, sizeof(symbuf));
	spskip();
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
/*		sym_search																*/
/*		return 0～:tbl 番号、-1:未登録											*/
/********************************************************************************/
static int sym_search()
{
	int i;
	
	for(i=0; i<tbl_ptr; i++){
		if(strcmp(symbuf, asm32v->tbl[i].symbuf)==0)
			return i;						// シンボルテーブル発見
	}
	return -1;								// 未登録
}
/********************************************************************************/
/*		ut_malloc																*/
/********************************************************************************/
static char *ut_malloc(unsigned int size)
{
	char *ptr;
	
	if(buff_ptr+size > 0x8000){
		error_message(E_OUT_OF_MEMORY, __LINE__, __FILE__);
	}
	ptr = &(asm32v->buff[buff_ptr]);
	buff_ptr += size;
	return ptr;
}
/********************************************************************************/
/*		label_process															*/
/*		行の先頭で呼び出される、ラベルなら処理する								*/
/********************************************************************************/
static void label_process()
{
	int num;
	struct symtbl *ptr;
	
	if(is_sym_top()){						// ラベルだった
		getsym();
		prg_counter += 3;
		prg_counter &= 0xfffffffc;

		num = sym_search();
		if(pass==1){						// pass1
			if(num >= 0){					// 2重定義
				error_message(E_MULTIPLE_DEFINITION, __LINE__, __FILE__);
			}
			num = tbl_ptr++;
			if(num >= MAX_SYMBOL_TABLE){				// シンボルテーブルオーバーフロー
				error_message(E_OUT_OF_MEMORY, __LINE__, __FILE__);
			}
			ptr = &(asm32v->tbl[num]);
			ptr->symbuf = ut_malloc(strlen(symbuf)+1);
			strcpy(ptr->symbuf, symbuf);
			ptr->type = TYPE_LABEL;
			ptr->value = prg_counter;
			return;
		}
		else{								// pass2
			if(num < 0){					// undef
				error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
			}
		}
	}
}
/********************************************************************************/
/*		error_message															*/
/********************************************************************************/
static void error_message(int err_no, int line, char *filename)
{
	if(err_no==0)
		return;								// no error
	ut_printf("file=%s line=%d *** %s\n", filename, line, err_msg[err_no]);
	ut_printf("%7d: %s", line_number, asm32v->linebuf);
//	exit(1);
	longjmp(env, 1);
}
/********************************************************************************/
/*		list_out																*/
/*		flag NO_OPCODE:opcode 無し、NO_LINEBUF:linebuf 無し						*/
/*			ONE_BYTE:opcode が１バイト											*/
/********************************************************************************/
static void list_out(int flag, unsigned int addr, unsigned int opcode)
{
	unsigned char *ptr = (unsigned char *)&opcode;
	
	if(pass != 2)
		return;
	fprintf(lst_fp, "%08X ", addr);

	if(flag & NO_OPCODE)
		fprintf(lst_fp, "        ");
	else{
		if(flag & ONE_BYTE)
			fprintf(lst_fp, "      %02X", ptr[0]);
		else
			fprintf(lst_fp, "%02X%02X%02X%02X", ptr[0], ptr[1], ptr[2], ptr[3]);
	}

	fprintf(lst_fp, "%5d: ", line_number);
	
	if((flag & NO_LINEBUF)==0)
		fprintf(lst_fp, "%s", asm32v->linebuf);
	else
		fprintf(lst_fp, "\n");


//	fprintf(lst_fp, "         %02X %02X %02X %02X %02X %02X\n", (opcode>>26)&0x3f, (opcode>>21)&0x1f,
//							 (opcode>>16)&0x1f, (opcode>>11)&0x1f, (opcode>>6)&0x1f, opcode&0x3f);
}
/********************************************************************************/
/*		registration															*/
/********************************************************************************/
static void registration()
{
	int i, num;
	struct symtbl *ptr;
	
	for(i=0; reg_tbl[i].symbuf; i++){
		strcpy(symbuf, reg_tbl[i].symbuf);
		num = sym_search();
		if(num >= 0){						// ２重定義
			error_message(E_MULTIPLE_DEFINITION, __LINE__, __FILE__);
		}
		num = tbl_ptr++;
		if(num >= MAX_SYMBOL_TABLE){					// シンボルテーブルオーバーフロー
			error_message(E_OUT_OF_MEMORY, __LINE__, __FILE__);
		}
		ptr = &(asm32v->tbl[num]);
		ptr->symbuf = ut_malloc(strlen(symbuf)+1);
		strcpy(ptr->symbuf, symbuf);
		ptr->type = reg_tbl[i].type;
		ptr->value = reg_tbl[i].value;
	}
}
/********************************************************************************/
/*		symbol_out																*/
/********************************************************************************/
static void symbol_out()
{
	int i;
	
	fprintf(lst_fp, "\n---------- symbol table ----------\n");
	for(i=0; i<tbl_ptr; i++){
		if(asm32v->tbl[i].type==TYPE_LABEL){
			fprintf(lst_fp, "%08X  %s\n", asm32v->tbl[i].value, asm32v->tbl[i].symbuf);
		}
	}
}
/********************************************************************************/
/*		factor																	*/
/*		シンボル、10進数、16進数(0xabcd)、文字									*/
/********************************************************************************/
static unsigned int factor()
{
	unsigned int value;
	int num;
	struct symtbl *ptr;
	
	spskip();
	if(is_sym_top()){						// シンボル
		getsym();
		if(pass==2){
			num = sym_search();
			if(num < 0){
				error_message(E_UNDEFINED_SYMBOL, __LINE__, __FILE__);
			}
			ptr = &(asm32v->tbl[num]);
			if(ptr->type != TYPE_LABEL){
				error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
			}
			return ptr->value;
		}
		return 0;
	}
	else if(linebufp[0]=='0' && linebufp[1]=='x'){	// 16進数
		linebufp += 2;
		value = 0;
		for(;;){
			if(*linebufp>='0' && *linebufp<='9'){
				value *= 16;
				value += *linebufp++ - '0';
			}
			else if(*linebufp>='A' && *linebufp<='F'){
				value *= 16;
				value += *linebufp++ - 'A' + 10;
			}
			else if(*linebufp>='a' && *linebufp<='f'){
				value *= 16;
				value += *linebufp++ - 'a' + 10;
			}
			else{
				return value;
			}
		}
	}
	else if(isdigit(*linebufp)){			// 10進数
		value = 0;
		while(isdigit(*linebufp)){
			value *= 10;
			value += *linebufp++ - '0';
		}
		return value;
	}
	else if(*linebufp=='\''){				// 文字
		linebufp++;
		value = *linebufp++;
		if(*linebufp++ != '\''){
			error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
		}
		return value;
	}
	else{
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	}
	return 0;
}
/********************************************************************************/
/*		expr3																	*/
/*		expr3 = factor * / factor * / ... factor								*/
/********************************************************************************/
static unsigned int expr3()
{
	unsigned int v1, v2;
	
	v1 = factor();
	while(1){
		spskip();
		if(*linebufp=='*'){
			linebufp++;
			v2 = factor();
			v1 *= v2;
		}
		else if(*linebufp=='/'){
			linebufp++;
			v2 = factor();
			v1 /= v2;
		}
		else{
			break;
		}
	}
	return v1;
}
/********************************************************************************/
/*		expr2																	*/
/*		expr2 = (-)expr3 +- expr3 +- ... expr3									*/
/********************************************************************************/
static unsigned int expr2()
{
	unsigned int v1, v2;
	
	spskip();
	if(*linebufp=='-'){
		linebufp++;
		v1 = expr3();
		v1 = 0-v1;
	}
	else{
		v1 = expr3();
	}
	while(1){
		spskip();
		if(*linebufp=='+'){
			linebufp++;
			v2 = expr3();
			v1 += v2;
		}
		else if(*linebufp=='-'){
			linebufp++;
			v2 = expr3();
			v1 -= v2;
		}
		else{
			break;
		}
	}
	return v1;
}
/********************************************************************************/
/*		expr1																	*/
/*		expr1 = expr2 & expr2 & ... expr2										*/
/********************************************************************************/
static unsigned int expr1()
{
	unsigned int v1, v2;
	
	v1 = expr2();
	spskip();
	while(*linebufp=='&'){
		linebufp++;
		v2 = expr2();
		v1 &= v2;
		spskip();
	}
	return v1;
}
/********************************************************************************/
/*		expr																	*/
/*		expr = expr1 | expr1 | ... expr1										*/
/********************************************************************************/
static unsigned int expr()
{
	unsigned int v1, v2;
	
	v1 = expr1();
	spskip();
	while(*linebufp=='|'){
		linebufp++;
		v2 = expr1();
		v1 |= v2;
		spskip();
	}
	return v1;
}
/********************************************************************************/
/*		hex_out																	*/
/*		byte は max 4															*/
/********************************************************************************/
static unsigned char hex_out_data[16];
static int hex_out_byte;
static unsigned int hex_out_addr;

static void hex_out_flush()
{
	int i, sum;
	
	if (pass != 2){
		hex_out_byte = 0;
		return;
	}
	if(hex_out_byte==0)
		return;
	/***	output	***/
	sum = 0;
	fprintf(hex_fp, "S3%02X", hex_out_byte+4+1);
	sum += hex_out_byte+4+1;
	fprintf(hex_fp, "%02X%02X%02X%02X", (hex_out_addr>>24)&0xff, (hex_out_addr>>16)&0xff, (hex_out_addr>>8)&0xff, hex_out_addr&0xff);
	sum += ((hex_out_addr>>24)&0xff) + ((hex_out_addr>>16)&0xff) + ((hex_out_addr>>8)&0xff) + (hex_out_addr&0xff);
	for(i=0; i<hex_out_byte; i++){
		fprintf(hex_fp, "%02X", hex_out_data[i]);
		sum += hex_out_data[i];
	}
	fprintf(hex_fp, "%02X\n", (~sum)&0xff);
	
	hex_out_byte = 0;
}

static void hex_out(unsigned int addr, unsigned int data, int byte)
{
	int i;
	
	if(hex_out_addr+hex_out_byte != addr){
		hex_out_flush();
		hex_out_addr = addr;
	}
	for(i=0; i<byte; i++){
		hex_out_data[hex_out_byte++] = ((char*)&data)[i];
		if(hex_out_byte >= 16){
			hex_out_flush();
			hex_out_addr = addr;
		}
	}
}

static void hex_out_header()
{
	fprintf(hex_fp, "S00B0000686F67652E686578DE\n");
}

static void hex_out_startaddr(unsigned int addr)
{
	// S7059D0000005D
	int i, sum;
	
	sum = 5;
	fprintf(hex_fp, "S705");
	for(i=0; i<4; i++){
		fprintf(hex_fp, "%02X", (addr>>24)&0xff);
		sum += (addr>>24)&0xff;
		addr <<= 8;
	}
	fprintf(hex_fp, "%02X\n", (~sum)&0xff);
}
/********************************************************************************/
/*		opcode_nop																*/
/********************************************************************************/
static int opcode_nop(struct symtbl *ptr)
{
	unsigned int opcode;
	
	opcode = ptr->value;
	test_line_end();
	
	prg_counter += 3;
	prg_counter &= 0xfffffffc;
	hex_out(prg_counter, opcode, 4);
	list_out(0, prg_counter, opcode);
	prg_counter += 4;
	
	return 0;
}
/********************************************************************************/
/*		opcode_EQU																*/
/********************************************************************************/
static int opcode_EQU(struct symtbl *ptr)
{
	int value, num;
	
	if(pass==1){
		pass = 2;
		value = expr();
		pass = 1;
	}
	else{
		value = expr();
	}
	test_line_end();
	
	linebufp = asm32v->linebuf;
	if(is_sym_top()==0)
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	
	getsym();
	num = sym_search();
	if(num < 0)
		error_message(E_UNDEFINED_SYMBOL, __LINE__, __FILE__);
	if(asm32v->tbl[num].type != TYPE_LABEL)
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	
	asm32v->tbl[num].value = value;
	list_out(0, 0, value);
	
	return 0;
}
/********************************************************************************/
/*		opcode_DW																*/
/*		dw 0x12345678																*/
/********************************************************************************/
static int opcode_DW(struct symtbl *ptr)
{
	unsigned int opcode;
	
	opcode = expr();
	test_line_end();
	
	prg_counter += 3;
	prg_counter &= 0xfffffffc;
	hex_out(prg_counter, opcode, 4);
	list_out(0, prg_counter, opcode);
	prg_counter += 4;
	
	return 0;
}
/********************************************************************************/
/*		opcode_DH																*/
/*		dw 0x1234																*/
/********************************************************************************/
static int opcode_DH(struct symtbl *ptr)
{
	unsigned int opcode;
	
	opcode = expr();
	test_line_end();
	
	prg_counter += 1;
	prg_counter &= 0xfffffffe;
	hex_out(prg_counter, opcode, 2);
	list_out(0, prg_counter, opcode);
	prg_counter += 2;
	
	return 0;
}
/********************************************************************************/
/*		opcode_DB																*/
/*		db 0x12																*/
/********************************************************************************/
static int opcode_DB(struct symtbl *ptr)
{
	unsigned int opcode;
	
	opcode = expr();
	test_line_end();
	
//	prg_counter += 3;
//	prg_counter &= 0xfffffffc;
	hex_out(prg_counter, opcode, 1);
	list_out(0, prg_counter, opcode);
	prg_counter += 1;
	
	return 0;
}
/********************************************************************************/
/*		opcode_Ii																*/
/*		addi  $rd,$rs,Imm														*/
/********************************************************************************/
static int opcode_Ii(struct symtbl *ptr)
{
	unsigned int opcode;
	int rd, rs, Imm;
	
	opcode = ptr->value;
	spskip();
	if(*linebufp++ != '$')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	rd = expr();
	if(*linebufp++ != ',')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	spskip();
	if(*linebufp++ != '$')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	rs = expr();
	if(*linebufp++ != ',')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	Imm = expr();
	test_line_end();

	opcode |= (rs & 0x1f)<<21;
	opcode |= (rd & 0x1f)<<16;
	opcode |= (Imm & 0xffff);
	
	prg_counter += 3;
	prg_counter &= 0xfffffffc;
	hex_out(prg_counter, opcode, 4);
	list_out(0, prg_counter, opcode);
	prg_counter += 4;
	
	return 0;
}
/********************************************************************************/
/*		opcode_LA																*/
/*		la $rd,Imm32    →    lui $at,Imm上位、ori $rd,$at,Imm下位				*/
/********************************************************************************/
static int opcode_LA(struct symtbl *ptr)
{
	unsigned int opcode, Imm;
	int rd;
	
	spskip();
	if(*linebufp++ != '$')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	rd = expr();
	if(*linebufp++ != ',')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	Imm = expr();
	test_line_end();

	opcode = 0x3c000000;					// lui
//	opcode |= (rs & 0x1f)<<21;
	opcode |= (rd & 0x1f)<<16;
	opcode |= ((Imm>>16) & 0xffff);			// Imm 上位
	
	prg_counter += 3;
	prg_counter &= 0xfffffffc;
	hex_out(prg_counter, opcode, 4);
	list_out(0, prg_counter, opcode);
	prg_counter += 4;
	
	opcode = 0x34000000;					// ori
	opcode |= (rd & 0x1f)<<21;
	opcode |= (rd & 0x1f)<<16;
	opcode |= (Imm & 0xffff);				// Imm 下位
	
	hex_out(prg_counter, opcode, 4);
	list_out(NO_LINEBUF, prg_counter, opcode);
	prg_counter += 4;
	
	return 0;
}
/********************************************************************************/
/*		opcode_MOVE																*/
/*		move $rd,$rt  →   addu $rd,$rs,$zero									*/
/********************************************************************************/
static int opcode_MOVE(struct symtbl *ptr)
{
	unsigned int opcode;
	int rd, rs;

	opcode = ptr->value;
	spskip();
	if(*linebufp++ != '$')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	rd = expr();
	if(*linebufp++ != ',')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	spskip();
	if(*linebufp++ != '$')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	rs = expr();
	test_line_end();
	
	opcode |= (rs & 0x1f)<<21;
	opcode |= (rd & 0x1f)<<11;
	
	prg_counter += 3;
	prg_counter &= 0xfffffffc;
	hex_out(prg_counter, opcode, 4);
	list_out(0, prg_counter, opcode);
	prg_counter += 4;
	
	return 0;
}
/********************************************************************************/
/*		opcode_LI																*/
/*		li $rd,Imm    →   addiu $rd,$zero,Imm									*/
/********************************************************************************/
static int opcode_LI(struct symtbl *ptr)
{
	unsigned int opcode, Imm;
	int rd;
	
	opcode = ptr->value;
	spskip();
	if(*linebufp++ != '$')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	rd = expr();
	if(*linebufp++ != ',')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);

	if(pass==1){
		pass = 2;
		Imm = expr();
		pass = 1;
	}
	else{
		Imm = expr();
	}
	test_line_end();

	if((Imm & 0xffff8000)==0xffff8000 || (Imm & 0xffff8000)==0){
		opcode = 0x24000000;				// addiu $rd,$zero,Imm
	}
	else if((Imm & 0xffff0000)==0){
		opcode = 0x34000000;				// ori $rd,$zero,Imm
	}
	else if((Imm & 0xffff)==0){				// lui
		opcode = 0x3c000000;

		opcode |= (rd & 0x1f)<<16;
		opcode |= ((Imm>>16) & 0xffff);		// Imm 上位
		
		prg_counter += 3;
		prg_counter &= 0xfffffffc;
		hex_out(prg_counter, opcode, 4);
		list_out(0, prg_counter, opcode);
		prg_counter += 4;
		
		return 0;
	}
	else{
		opcode = 0x3c000000;					// lui
		opcode |= (rd & 0x1f)<<16;
		opcode |= ((Imm>>16) & 0xffff);			// Imm 上位
		
		prg_counter += 3;
		prg_counter &= 0xfffffffc;
		hex_out(prg_counter, opcode, 4);
		list_out(0, prg_counter, opcode);
		prg_counter += 4;
		
		opcode = 0x34000000;					// ori
		opcode |= (rd & 0x1f)<<21;
		opcode |= (rd & 0x1f)<<16;
		opcode |= (Imm & 0xffff);				// Imm 下位
		
		hex_out(prg_counter, opcode, 4);
		list_out(NO_LINEBUF, prg_counter, opcode);
		prg_counter += 4;
		
		return 0;
	}


	opcode |= (rd & 0x1f)<<16;
	opcode |= (Imm & 0xffff);
	
	prg_counter += 3;
	prg_counter &= 0xfffffffc;
	hex_out(prg_counter, opcode, 4);
	list_out(0, prg_counter, opcode);
	prg_counter += 4;
	
	return 0;
}
/********************************************************************************/
/*		opcode_Ii2																*/
/*		lui   $rt,Imm															*/
/********************************************************************************/
static int opcode_Ii2(struct symtbl *ptr)
{
	unsigned int opcode;
	int rt, Imm;
	
	opcode = ptr->value;
	spskip();
	if(*linebufp++ != '$')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	rt = expr();
	if(*linebufp++ != ',')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	Imm = expr();
	test_line_end();

	opcode |= (rt & 0x1f)<<16;
	opcode |= (Imm & 0xffff);
	
	prg_counter += 3;
	prg_counter &= 0xfffffffc;
	hex_out(prg_counter, opcode, 4);
	list_out(0, prg_counter, opcode);
	prg_counter += 4;
	
	return 0;
}
/********************************************************************************/
/*		opcode_Ij2																*/
/*		bgtz  $rs,label															*/
/********************************************************************************/
static int opcode_Ij2(struct symtbl *ptr)
{
	unsigned int opcode;
	int rs, addr, tmp;
	
	opcode = ptr->value;
	spskip();
	if(*linebufp++ != '$')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	rs = expr();
	if(*linebufp++ != ',')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	addr = expr();
	test_line_end();
	
	addr -= prg_counter+4;
	if(pass==2){
		tmp = addr & 0xfffe0000;
		if(tmp!=0 && (tmp != 0xfffe0000))
			error_message(E_ADDRESS_OUT, __LINE__, __FILE__);
	}
	
	opcode |= (rs & 0x1f)<<21;
	opcode |= (addr>>2)&0xffff;
	
	prg_counter += 3;
	prg_counter &= 0xfffffffc;
	hex_out(prg_counter, opcode, 4);
	list_out(0, prg_counter, opcode);
	prg_counter += 4;
	
	opcode = 0;								// nop
	hex_out(prg_counter, opcode, 4);
	list_out(NO_LINEBUF, prg_counter, opcode);
	prg_counter += 4;
	
	return 0;
}
/********************************************************************************/
/*		opcode_Ij3																*/
/*		beq  $rs,$rt,label														*/
/********************************************************************************/
static int opcode_Ij3(struct symtbl *ptr)
{
	unsigned int opcode;
	int rs, rt, addr, tmp;
	
	opcode = ptr->value;
	spskip();
	if(*linebufp++ != '$')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	rs = expr();
	if(*linebufp++ != ',')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	spskip();
	if(*linebufp++ != '$')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	rt = expr();
	if(*linebufp++ != ',')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	addr = expr();
	test_line_end();
	
	addr -= prg_counter+4;
	if(pass==2){
		tmp = addr & 0xfffe0000;
		if(tmp!=0 && (tmp != 0xfffe0000))
			error_message(E_ADDRESS_OUT, __LINE__, __FILE__);
	}
	
	opcode |= (rs & 0x1f)<<21;
	opcode |= (rt & 0x1f)<<16;
	opcode |= (addr>>2)&0xffff;
	
	prg_counter += 3;
	prg_counter &= 0xfffffffc;
	hex_out(prg_counter, opcode, 4);
	list_out(0, prg_counter, opcode);
	prg_counter += 4;
	
	opcode = 0;								// nop
	hex_out(prg_counter, opcode, 4);
	list_out(NO_LINEBUF, prg_counter, opcode);
	prg_counter += 4;
	
	return 0;
}
/********************************************************************************/
/*		opcde_J																	*/
/*		j target																*/
/********************************************************************************/
static int opcode_J(struct symtbl *ptr)
{
	unsigned int opcode, addr;
	
	opcode = ptr->value;
	addr = expr();
	test_line_end();
	
	if(pass==2){
		if(((prg_counter+4) & 0xf0000000) != (addr & 0xf0000000))
			error_message(E_ADDRESS_OUT, __LINE__, __FILE__);
	}
	opcode |= (addr & 0x0fffffff)>>2;
	
	prg_counter += 3;
	prg_counter &= 0xfffffffc;
	hex_out(prg_counter, opcode, 4);
	list_out(0, prg_counter, opcode);
	prg_counter += 4;
	
	opcode = 0;								// nop
	hex_out(prg_counter, opcode, 4);
	list_out(NO_LINEBUF, prg_counter, opcode);
	prg_counter += 4;
	
	return 0;
}
/********************************************************************************/
/*		opcode_R																*/
/*		sllv  $rd,$rt,$rs														*/
/********************************************************************************/
static int opcode_R(struct symtbl *ptr)
{
	unsigned int opcode;
	int rd, rt, rs;
	
	opcode = ptr->value;
	spskip();
	if(*linebufp++ != '$')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	rd = expr();
	if(*linebufp++ != ',')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	spskip();
	if(*linebufp++ != '$')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	rt = expr();
	if(*linebufp++ != ',')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	spskip();
	if(*linebufp++ != '$')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	rs = expr();
	test_line_end();

	opcode |= (rs & 0x1f)<<21;
	opcode |= (rt & 0x1f)<<16;
	opcode |= (rd & 0x1f)<<11;
	
	prg_counter += 3;
	prg_counter &= 0xfffffffc;
	hex_out(prg_counter, opcode, 4);
	list_out(0, prg_counter, opcode);
	prg_counter += 4;
	
	return 0;
}
/********************************************************************************/
/*		opcode_Rx																*/
/*		sllv  $rd,$rs,$rt														*/
/********************************************************************************/
static int opcode_Rx(struct symtbl *ptr)
{
	unsigned int opcode;
	int rd, rt, rs;
	
	opcode = ptr->value;
	spskip();
	if(*linebufp++ != '$')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	rd = expr();
	if(*linebufp++ != ',')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	spskip();
	if(*linebufp++ != '$')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	rs = expr();
	if(*linebufp++ != ',')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	spskip();
	if(*linebufp++ != '$')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	rt = expr();
	test_line_end();

	opcode |= (rs & 0x1f)<<21;
	opcode |= (rt & 0x1f)<<16;
	opcode |= (rd & 0x1f)<<11;
	
	prg_counter += 3;
	prg_counter &= 0xfffffffc;
	hex_out(prg_counter, opcode, 4);
	list_out(0, prg_counter, opcode);
	prg_counter += 4;
	
	return 0;
}
/********************************************************************************/
/*		opcode_Rm																*/
/*		mult  $rs,$rt														*/
/********************************************************************************/
static int opcode_Rm(struct symtbl *ptr)
{
	unsigned int opcode;
	int rt, rs;
	
	opcode = ptr->value;
	spskip();
	if(*linebufp++ != '$')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	rs = expr();
	if(*linebufp++ != ',')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	spskip();
	if(*linebufp++ != '$')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	rt = expr();
	test_line_end();

	opcode |= (rs & 0x1f)<<21;
	opcode |= (rt & 0x1f)<<16;
	
	prg_counter += 3;
	prg_counter &= 0xfffffffc;
	hex_out(prg_counter, opcode, 4);
	list_out(0, prg_counter, opcode);
	prg_counter += 4;
	
	return 0;
}
/********************************************************************************/
/*		opcode_MTHI																*/
/*		mthi    $rs																*/
/********************************************************************************/
static int opcode_MTHI(struct symtbl *ptr)
{
	unsigned int opcode;
	int rs;
	
	opcode = ptr->value;
	spskip();
	if(*linebufp++ != '$')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	rs = expr();
	test_line_end();
	
	opcode |= (rs & 0x1f)<<21;
	
	prg_counter += 3;
	prg_counter &= 0xfffffffc;
	hex_out(prg_counter, opcode, 4);
	list_out(0, prg_counter, opcode);
	prg_counter += 4;
	return 0;
}
/********************************************************************************/
/*		opcode_MFHI																*/
/*		mthi    $rd																*/
/********************************************************************************/
static int opcode_MFHI(struct symtbl *ptr)
{
	unsigned int opcode;
	int rd;
	
	opcode = ptr->value;
	spskip();
	if(*linebufp++ != '$')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	rd = expr();
	test_line_end();
	
	opcode |= (rd & 0x1f)<<11;
	
	prg_counter += 3;
	prg_counter &= 0xfffffffc;
	hex_out(prg_counter, opcode, 4);
	list_out(0, prg_counter, opcode);
	prg_counter += 4;
	return 0;
}
/********************************************************************************/
/*		opcode_R1																*/
/*		jr    $rs																*/
/********************************************************************************/
static int opcode_R1(struct symtbl *ptr)
{
	unsigned int opcode;
	int rs;
	
	opcode = ptr->value;
	spskip();
	if(*linebufp++ != '$')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	rs = expr();
	test_line_end();
	
	opcode |= (rs & 0x1f)<<21;
	
	prg_counter += 3;
	prg_counter &= 0xfffffffc;
	hex_out(prg_counter, opcode, 4);
	list_out(0, prg_counter, opcode);
	prg_counter += 4;
	
	opcode = 0;								// nop
	hex_out(prg_counter, opcode, 4);
	list_out(NO_LINEBUF, prg_counter, opcode);
	prg_counter += 4;
	
	return 0;
}
/********************************************************************************/
/*		opcode_R2																*/
/*		jalr   $rd,$rs															*/
/********************************************************************************/
static int opcode_R2(struct symtbl *ptr)
{
	unsigned int opcode;
	int rs, rd;
	
	opcode = ptr->value;
	spskip();
	if(*linebufp++ != '$')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	rd = expr();
	if(*linebufp++ != ',')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	spskip();
	if(*linebufp++ != '$')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	rs = expr();
	test_line_end();
	
	opcode |= (rs & 0x1f)<<21;
	opcode |= (rd & 0x1f)<<11;
	
	prg_counter += 3;
	prg_counter &= 0xfffffffc;
	hex_out(prg_counter, opcode, 4);
	list_out(0, prg_counter, opcode);
	prg_counter += 4;
	
	opcode = 0;								// nop
	hex_out(prg_counter, opcode, 4);
	list_out(NO_LINEBUF, prg_counter, opcode);
	prg_counter += 4;
	
	return 0;
}
/********************************************************************************/
/*		opcode_Rs																*/
/*		sll  $rd,$rt,shamt														*/
/********************************************************************************/
static int opcode_Rs(struct symtbl *ptr)
{
	unsigned int opcode;
	int rd, rt, shamt;
	
	opcode = ptr->value;
	spskip();
	if(*linebufp++ != '$')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	rd = expr();
	if(*linebufp++ != ',')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	spskip();
	if(*linebufp++ != '$')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	rt = expr();
	if(*linebufp++ != ',')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	spskip();
	shamt = expr();
	test_line_end();

	opcode |= (shamt & 0x1f)<<6;
	opcode |= (rt & 0x1f)<<16;
	opcode |= (rd & 0x1f)<<11;
	
	prg_counter += 3;
	prg_counter &= 0xfffffffc;
	hex_out(prg_counter, opcode, 4);
	list_out(0, prg_counter, opcode);
	prg_counter += 4;
	
	return 0;
}
/********************************************************************************/
/*		opcode_LS																*/
/*		lb  $rt,offset($rs)														*/
/********************************************************************************/
static int opcode_LS(struct symtbl *ptr)
{
	unsigned int opcode;
	int rt, rs, offset;
	
	opcode = ptr->value;
	spskip();
	if(*linebufp++ != '$')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	rt = expr();
	if(*linebufp++ != ',')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	spskip();
	offset = expr();
	if(*linebufp++ != '(')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	spskip();
	if(*linebufp++ != '$')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	rs = expr();
	if(*linebufp++ != ')')
		error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	test_line_end();
	
	opcode |= (rt & 0x1f)<<16;
	opcode |= (rs & 0x1f)<<21;
	opcode |= offset & 0xffff;
	
	prg_counter += 3;
	prg_counter &= 0xfffffffc;
	hex_out(prg_counter, opcode, 4);
	list_out(0, prg_counter, opcode);
	prg_counter += 4;
	
	return 0;
}
/********************************************************************************/
/*		opcode_END																*/
/********************************************************************************/
static void opcode_END()
{
	unsigned int addr;
	
	addr = expr();
	test_line_end();
	list_out(NO_OPCODE, prg_counter, 0);
	if(pass==2){
		ut_printf("last addr = %08X\n", prg_counter);
		hex_out_flush();
		hex_out_startaddr(addr);
	}
}
/********************************************************************************/
/*		opcode_ORG																*/
/********************************************************************************/
static int opcode_ORG(struct symtbl *ptr)
{
	unsigned int addr;
	
	if(pass==1){
		pass = 2;
		addr = expr();
		pass = 1;
	}
	else{
		addr = expr();
	}
	test_line_end();
	
	prg_counter = addr;
	list_out(NO_OPCODE, prg_counter, 0);
	return 0;
}
/********************************************************************************/
/*		opcode_process															*/
/*		return 0:success, E_END_DETECTED:end 命令検出							*/
/********************************************************************************/
static int opcode_process()
{
	int num;
	struct symtbl *ptr;
	
	getsym();
	num = sym_search();
	if(num < 0){							// undef
		error_message(E_UNDEFINED_SYMBOL, __LINE__, __FILE__);
	}
	ptr = &(asm32v->tbl[num]);
	switch(ptr->type){
		case TYPE_END:
			opcode_END();
			return E_END_DETECTED;
		case TYPE_ORG:
			return opcode_ORG(ptr);
		case TYPE_EQU:
			return opcode_EQU(ptr);
		case TYPE_DW:
			return opcode_DW(ptr);
		case TYPE_DH:
			return opcode_DH(ptr);
		case TYPE_DB:
			return opcode_DB(ptr);
		case TYPE_MOVE:
			return opcode_MOVE(ptr);
		case TYPE_Ii:
			return opcode_Ii(ptr);
		case TYPE_Ii2:
			return opcode_Ii2(ptr);
		case TYPE_Ij2:
			return opcode_Ij2(ptr);
		case TYPE_Ij3:
			return opcode_Ij3(ptr);
		case TYPE_R:
			return opcode_R(ptr);
		case TYPE_Rx:
			return opcode_Rx(ptr);
		case TYPE_Rm:
			return opcode_Rm(ptr);
		case TYPE_R1:
			return opcode_R1(ptr);
		case TYPE_R2:
			return opcode_R2(ptr);
		case TYPE_Rs:
			return opcode_Rs(ptr);
		case TYPE_J:
			return opcode_J(ptr);
		case TYPE_MTHI:
			return opcode_MTHI(ptr);
		case TYPE_MFHI:
			return opcode_MFHI(ptr);
		case TYPE_LI:
			return opcode_LI(ptr);
		case TYPE_LA:
			return opcode_LA(ptr);
		case TYPE_LS:
			return opcode_LS(ptr);
		case TYPE_NOP:
			return opcode_nop(ptr);
		default:
			error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
	}
	return E_INTERNAL_ERROR;
}
/********************************************************************************/
/*		asm_pass																*/
/********************************************************************************/
static void asm_pass()
{
	int ret;
	
	prg_counter = 0;
	line_number = 0;
//	while(fgets(asm32v->linebuf, sizeof(asm32v->linebuf), src_fp)){
	while(SYS_FS_FileStringGet(src_fp, asm32v->linebuf, sizeof(asm32v->linebuf))==SYS_FS_RES_SUCCESS){
		line_number++;
		linebufp = asm32v->linebuf;
		
		label_process();					// 行の先頭がシンボルならラベルとして登録する

		spskip();
		if(is_line_end()){					// ラベルのみの行だった
			list_out(NO_OPCODE, prg_counter, 0);	// リスト出力、オペコード無し
			continue;
		}
		
		ret = opcode_process();				// １行アッセンブル
		
		if(ret==E_END_DETECTED){
			return;
		}
		error_message(ret, __LINE__, __FILE__);
	}
}
/********************************************************************************/
/*		asm32																	*/
/*		filename: main.s → main.s, main.s.hex, main.s.lst						*/
/********************************************************************************/
int asm32(const char *filename)
{
	char *buf = asm32v->linebuf;
	char *buf2 = asm32v->linebuf+512;
	char *ptr;
	
	if(setjmp(env)){
		SYS_FS_FileClose(src_fp);
		SYS_FS_FileClose(hex_fp);
		SYS_FS_FileClose(lst_fp);
		return 1;							// error: longjmp() が実行された
	}
	ut_printf("%s %s %s\n", VERSION, __DATE__, __TIME__);
	memset(asm32v, 0, sizeof(struct asm32v));
	pass = 0;
	prg_counter = 0;
	line_number = 0;
	buff_ptr = 0;
	tbl_ptr = 0;
	
	/***	make file name base	***/
	strcpy(buf2, filename);
	ptr = strrchr(buf2, '.');
	if(ptr){
		*ptr = '\0';
	}
	/***	registration	***/
	registration();
	
	/***	file open	***/
	if((src_fp=SYS_FS_FileOpen(filename, SYS_FS_FILE_OPEN_READ))==SYS_FS_HANDLE_INVALID){
		ut_error_msg("src open");
		return 1;							// error
	}
	strcpy(buf, buf2);
	strcat(buf, ".hex");
	SYS_FS_FileDirectoryRemove(buf);
	if((hex_fp=SYS_FS_FileOpen(buf, SYS_FS_FILE_OPEN_WRITE))==SYS_FS_HANDLE_INVALID){
		SYS_FS_FileClose(src_fp);
		ut_error_msg("hex open");
		return 1;							// error
	}
	strcpy(buf, buf2);
	strcat(buf, ".lst");
	SYS_FS_FileDirectoryRemove(buf);
	if((lst_fp=SYS_FS_FileOpen(buf, SYS_FS_FILE_OPEN_WRITE))==SYS_FS_HANDLE_INVALID){
		SYS_FS_FileClose(src_fp);
		SYS_FS_FileClose(hex_fp);
		ut_error_msg("lst open");
		return 1;							// error
	}

    /***	pass1	***/
	pass = 1;
	ut_printf("[ PASS1 ]\n");
	asm_pass();

	/***	pass2	***/
	if(SYS_FS_FileSeek(src_fp, 0, SYS_FS_SEEK_SET) < 0){
		SYS_FS_FileClose(src_fp);
		SYS_FS_FileClose(hex_fp);
		SYS_FS_FileClose(lst_fp);
		ut_error_msg("src seek");
		return 1;							// error
	}
	pass = 2;
	ut_printf("[ PASS2 ]\n");
	hex_out_byte = 0;
	hex_out_header();
	asm_pass();

	/***	end process	***/
	symbol_out();
	SYS_FS_FileClose(src_fp);
	SYS_FS_FileClose(hex_fp);
	SYS_FS_FileClose(lst_fp);
	
    return 0;
}
