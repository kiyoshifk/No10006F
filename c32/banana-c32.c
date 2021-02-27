#include "stdafx.h"
#define  ERR_MSG_DEFINE
#include "banana-c32.h"
#include "function.h"


struct registration_tbl{
	char *symbuf;
	uchar type;
} registration_tbl[]={
	{"_else"  ,      TYPE_ELSE},
	{"_goto"  ,      TYPE_GOTO},
	{"_unsigned",    TYPE_UNSIGNED},
	{"_char",        TYPE_CHAR},
	{"_short",       TYPE_SHORT},
	{"_int"   ,      TYPE_INT},
	{"_long",        TYPE_LONG},
	{"_if",          TYPE_IF},
	{"_return",      TYPE_RETURN},
	{"_while",       TYPE_WHILE},
	{"_continue",    TYPE_CONTINUE},
	{"_break",       TYPE_BREAK},
	{"_for",         TYPE_FOR},
	{"___interrupt", TYPE_INTERRUPT},
	{"__asm",        TYPE_ASM},
	{"___test__",    TYPE_TEST},
	{"_uchar",       TYPE_UCHAR},
	{"_ushort",      TYPE_USHORT},
	{"_uint",        TYPE_UINT},
	{"_ulong",       TYPE_ULONG},
	{"_sizeof",      TYPE_SIZEOF},
	{"_switch",      TYPE_SWITCH},
	{"_case",        TYPE_CASE},
	{"_default",     TYPE_DEFAULT},
	{"_do",          TYPE_DO},
	{"_void",        TYPE_VOID},
	{"_const",       TYPE_CONST},
	{0,0},
};


struct numb_param{
	int break_numb;				// break ���ł̂Ƃѐ惉�x���ԍ�
	int continue_numb;			// continue ���ł̂Ƃѐ惉�x���ԍ�
	int switch_numb;			// switch ���̌ʂ̔ԍ�
	int serial_numb;			// case ���̒ʂ��ԍ�
	int s_switch;				// switch ���̎��̒l����ꂽ c32_s_numb
	int default_flag;			// �u���b�N���� default ���L��� 1 �ɂȂ�
};


struct cv *cv = (struct cv *)RAM_BASE;
char *c32_linebuf;
char *c32_linebufp;
char c32_symbuf[MAX_SYMBUF_LEN];
char c32_src_filename[MAX_SYMBUF_LEN];
int c32_label_counter;
int c32_function_counter;
//int c32_output_buffer_line_numb;
int c32_tbl_ptr_g;
int c32_buff_ptr;
int c32_no_printx;
int c32_no_label_out;
int c32_pass;
int c32_s_numb, c32_max_s_numb;
int c32_ram_addr = RAM_BASE;
const char *c32_err_msg[];
jmp_buf c32_env;
int c32_max_src_buffer;
int c32_max_output_buffer;
#ifdef __XC32
SYS_FS_HANDLE c32_src_fp, c32_asm_fp;
#else
FILE *c32_src_fp, *c32_asm_fp;
#endif


static void function_pop_reg(struct symtbl *func);
static void parser_block(struct symtbl *func, struct numb_param *numb_param);
static void parser_function_block(struct symtbl *func, struct numb_param *numb_param);
static int parser_function_block_single(struct symtbl *func, struct numb_param *numb_param);
void c32_test(struct symtbl *func);
static void parser_dainyuu(struct symtbl *func);
static void parser_array(struct symtbl *var);


#if 0
	switch(mode){
	case MODE0:							// �v�Z������    c32_s_numb
		break;
	case MODE1:							// �v�Z���A�h���X����  c32_s_numb
		break;
	case MODE2:							// �萔����    value
		break;
//	case MODE3:							// �萔�A�h���X����  value
//		break;
	case MODE4:							// $fp offset �A�h���X����  value
		break;
	case MODE5:							// $s7 offset �A�h���X����  value
		break;
	case MODE6:							// ���x���A�h���X����   value
		break;
	case MODE7:							// ���W�X�^����  (char*)&value
		break;
	default:
		c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
	}
#endif


/********************************************************************************/
/*		variable_init															*/
/********************************************************************************/
static void variable_init()
{
	int i;
	struct symtbl *ptr;
	
	c32_printf(";----------  variable init  ----------\n");
	for(i=0; i<c32_tbl_ptr_g; i++){
		ptr = &cv->tbl_g[i];
		if((ptr->flag1 & (FLAG_04 | FLAG_08))==(FLAG_04|FLAG_08)
					 && (ptr->attr & ATTR_CONST)==0){	// global �ϐ��A����������Ă���ϐ�
			if(ptr->attr & ATTR_ARRAY){			// �z��
				c32_printf("	la		$t0, %s	; dst addr\n", ptr->label);
				c32_printf("	la		$t1, %s@init	; src addr\n", ptr->symbuf);
				c32_printf("	li		$t2, 0x%08X	; byte\n", ptr->size);
				c32_printf("	jal		startup_memcpy\n");
			}
			else{								// �z��ȊO�̕ϐ�
				if((ptr->attr & ATTR_CONST)==0){
					if(ptr->flag1 & FLAG_10){		// �����l�e�[�u���ŏo�Ă���(�����񃉃x��)
						c32_printf("	la		$t1, %s@init	; data\n", ptr->symbuf);
						mem_eq_reg(ptr->attr, "s7", ptr->label, "t1");
					}
					else{							// ptr->init = �����l���̂���
						c32_printf("	li		$t1, 0x%08X	; data\n", ptr->init);
						mem_eq_reg(ptr->attr, "s7", ptr->label, "t1");
					}
				}
			}
		}
		c32_output_buffer_flush();
	}
	c32_printf(";-------------------------------------\n");
}
/********************************************************************************/
/*		startup																	*/
/********************************************************************************/
static void startup()
{
	c32_printf(";----------  startup  ----------\n");
	c32_printf("startup\n");
	c32_printf("	addiu	$sp, $sp, -8\n");		// $s7, $ra �ۑ��p
	c32_printf("	sw		$ra, 0($sp)\n");
	c32_printf("	sw		$s7, 4($sp)\n");
	c32_printf("	li		$s7, 0x%08X		; RAM_BASE\n", RAM_BASE);
	c32_output_buffer_flush();
	
	c32_printf("	la		$t0, 0x%08X		; RAM_BASE\n", RAM_BASE);
	c32_printf("	li		$t1, 0			; clear data\n");
	c32_printf("	li		$t2, 0x%x		; RAM_SIZE\n", RAM_SIZE);
	c32_printf("	jal		startup_memset\n");
	c32_output_buffer_flush();
	
	variable_init();

	c32_printf("	addiu	$sp, $sp, -16\n");
	c32_printf("	jal		_main\n");
	c32_printf("	addiu	$sp, $sp, 16\n");
	
	c32_printf("	lw		$ra, 0($sp)\n");
	c32_printf("	lw		$s7, 4($sp)\n");
	c32_printf("	addiu	$sp, $sp, 8\n");
	c32_printf("	jr		$ra\n");
	c32_output_buffer_flush();
	
	c32_printf("; $t0: dst_addr\n");
	c32_printf("; $t1: src_addr\n");
	c32_printf("; $t2: byte\n");
	c32_printf("startup_memcpy\n");
	c32_printf("	lbu		$t3, 0($t1)		; read src data\n");
	c32_printf("	sb		$t3, 0($t0)		; write to dst\n");
	c32_printf("	addiu	$t0, $t0, 1\n");
	c32_printf("	addiu	$t1, $t1, 1\n");
	c32_printf("	addiu	$t2, $t2, -1\n");
	c32_printf("	bnez	$t2, startup_memcpy\n");
	c32_printf("	jr		$ra\n");
	c32_output_buffer_flush();
	
	c32_printf("; $t0: dst_addr\n");
	c32_printf("; $t1: set data\n");
	c32_printf("; $t2: byte\n");
	c32_printf("startup_memset\n");
	c32_printf("	sb		$t1, 0($t0)		; write to dst\n");
	c32_printf("	addiu	$t0, $t0, 1\n");
	c32_printf("	addiu	$t2, $t2, -1\n");
	c32_printf("	bnez	$t2, startup_memset\n");
	c32_printf("	jr		$ra\n");
	c32_output_buffer_flush();
}
/********************************************************************************/
/*		parser_do																*/
/*		do{...}while(...);														*/
/********************************************************************************/
static void parser_do(struct symtbl *func, struct numb_param *numb_param)
{
	int top_numb = c32_label_counter++;
	int break_numb = c32_label_counter++;
	int continue_numb = c32_label_counter++;
	int type;
	struct numb_param numb_param2;
	struct expr_param expr_p1;
	
	expr_p1.func = func;
	c32_token_process(0, &type);				// do ���m�F����
	if(type != TYPE_DO)
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	numb_param2 = *numb_param;
	numb_param2.break_numb = break_numb;
	numb_param2.continue_numb = continue_numb;
	c32_printf("L%d\n", top_numb);
	
	parser_function_block(func, &numb_param2);
	
	c32_token_process(0, &type);				// while ���m�F����
	if(type != TYPE_WHILE)
		c32_error_message(E_WHILE_MISSING, __LINE__, __FILE__);
	c32_token_process(0, &type);				// '(' ���m�F����
	if(type != TYPE_L_KAKKO)
		c32_error_message(E_L_KAKKO_MISSING, __LINE__, __FILE__);
	
	c32_printf("L%d\n", continue_numb);

	sprintf(expr_p1.reg_ans, "t%d", c32_s_numb);
	c32_expr(&expr_p1);
	c32_output_buffer_flush();
	convert_mode0(&expr_p1);

	c32_token_process(0, &type);				// ')' ���m�F����
	if(type != TYPE_R_KAKKO)
		c32_error_message(E_R_KAKKO_MISSING, __LINE__, __FILE__);
	c32_token_process(0, &type);				// ';' ���m�F����
	if(type != TYPE_SEMIKORON)
		c32_error_message(E_SEMIKORON_MISSING, __LINE__, __FILE__);
	c32_printf("	bnez	$t%d, L%d\n", c32_s_numb, top_numb);
	c32_printf("L%d\n", break_numb);
	c32_output_buffer_flush();
}
/********************************************************************************/
/*		parser_switch															*/
/*		case ���x���́F "Lswitch_numbre@�ʂ��ԍ�"								*/
/*		switch ���̓��� j "case���x��0"											*/
/********************************************************************************/
static void parser_switch(struct symtbl *func, struct numb_param *numb_param)
{
	int break_numb = c32_label_counter++;
	int switch_numb = c32_label_counter++;
	int s_numb_save = c32_s_numb;
	int type;
	struct numb_param numb_param2;
	struct expr_param expr_p1;
	
	c32_printf(";----------  parser switch  ----------\n");
	expr_p1.func = func;
	c32_token_process(0, &type);				// switch ���m�F����
	if(type != TYPE_SWITCH)
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	c32_token_process(0, &type);				// '(' ���m�F����
	if(type != TYPE_L_KAKKO)
		c32_error_message(E_L_KAKKO_MISSING, __LINE__, __FILE__);
	
	sprintf(expr_p1.reg_ans, "t%d", c32_s_numb);
	c32_expr(&expr_p1);
	c32_output_buffer_flush();
	convert_mode0(&expr_p1);
	
	c32_token_process(0, &type);				// ')' ���m�F����
	if(type != TYPE_R_KAKKO)
		c32_error_message(E_R_KAKKO_MISSING, __LINE__, __FILE__);
	c32_s_numb++;
	if(c32_max_s_numb < c32_s_numb)
		c32_max_s_numb = c32_s_numb;
	numb_param2 = *numb_param;
	numb_param2.break_numb = break_numb;
	numb_param2.switch_numb = switch_numb;
	numb_param2.serial_numb = 1;
	numb_param2.s_switch = s_numb_save;
	numb_param2.default_flag = 0;
	
	c32_printf("	j		L%d@%d\n", switch_numb, numb_param2.serial_numb);	// ���� serial_numb �� 0 �ł���
	parser_function_block(func, &numb_param2);
	
	c32_printf("	j		L%d\n", break_numb);
	c32_printf("L%d@%d\n", switch_numb, numb_param2.serial_numb);	// serial_numb �� case ���ŃC���N�������g�����
	if(numb_param2.default_flag)								// default �����L����
		c32_printf("	j		L%d@default\n", switch_numb);
	c32_printf("L%d					; switch end\n", break_numb);
	c32_s_numb = s_numb_save;
	c32_output_buffer_flush();
}
/********************************************************************************/
/*		parser_case																*/
/********************************************************************************/
static void parser_case(struct symtbl *func, struct numb_param *numb_param)
{
	int type;
	struct expr_param expr_p1;
	
	expr_p1.func = func;
	c32_token_process(0, &type);				// case ���m�F����
	if(type != TYPE_CASE)
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	
	sprintf(expr_p1.reg_ans, "t%d", c32_s_numb);
	c32_expr(&expr_p1);
	c32_output_buffer_flush();
	
	if(expr_p1.mode != MODE2)						// �萔�łȂ���΃G���[�ɂ���
		c32_error_message(E_NEED_CONSTANT, __LINE__, __FILE__);
	c32_token_process(0, &type);				// ':' ���m�F����
	if(type != TYPE_KORON)
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	if(numb_param->switch_numb < 0)
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	c32_printf("L%d@%d				; case label\n", numb_param->switch_numb, (numb_param->serial_numb)++);
	c32_printf("	li		$t0, %d\n", expr_p1.value);
	c32_printf("	bne		$t%d, $t0, L%d@%d\n", numb_param->s_switch, numb_param->switch_numb, numb_param->serial_numb);
	c32_output_buffer_flush();
}
/********************************************************************************/
/*		parser_default															*/
/********************************************************************************/
static void parser_default(struct symtbl *func, struct numb_param *numb_param)
{
	int type;
	
	c32_token_process(0, &type);				// default ���m�F����
	if(type != TYPE_DEFAULT)
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	c32_token_process(0, &type);				// ':' ���m�F����
	if(type != TYPE_KORON)
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	if(numb_param->switch_numb < 0)
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	numb_param->default_flag = 1;
	c32_printf("L%d@default			; default label\n", numb_param->switch_numb);
}
/********************************************************************************/
/*		parser_continue															*/
/********************************************************************************/
static void parser_continue(struct symtbl *func, struct numb_param *numb_param)
{
	int type;
	
	c32_printf(";----------  parser_continue  -----------\n");
	c32_token_process(0, &type);				// continue �m�F
	if(type != TYPE_CONTINUE)
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	c32_token_process(0, &type);				// ';' �m�F
	if(type != TYPE_SEMIKORON)
		c32_error_message(E_SEMIKORON_MISSING, __LINE__, __FILE__);
	if(numb_param->continue_numb < 0)
		c32_error_message(E_ILLEGAL_CONTINUE, __LINE__, __FILE__);
	c32_printf("	j		L%d\n", numb_param->continue_numb);
}
/********************************************************************************/
/*		parser_break															*/
/********************************************************************************/
static void parser_break(struct symtbl *func, struct numb_param *numb_param)
{
	int type;
	
	c32_printf(";----------  parser_break  ----------\n");
	c32_token_process(0, &type);				// break �m�F
	if(type != TYPE_BREAK)
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	c32_token_process(0, &type);				// ';' �m�F
	if(type != TYPE_SEMIKORON)
		c32_error_message(E_SEMIKORON_MISSING, __LINE__, __FILE__);
	if(numb_param->break_numb < 0)
		c32_error_message(E_ILLEGAL_BREAK, __LINE__, __FILE__);
	c32_printf("	j		L%d\n", numb_param->break_numb);
}
/********************************************************************************/
/*		parser_for																*/
/*		for(a;b;c){d}															*/
/********************************************************************************/
static void parser_for(struct symtbl *func, struct numb_param *numb_param)
{
	int break_label = c32_label_counter++;
	int continue_label = c32_label_counter++;
	int top_label = c32_label_counter++;
	int true_label = c32_label_counter++;
	struct src_ptr src_ptr;
	int type;
	struct numb_param numb_param2;
	struct expr_param expr_p1;
	
	c32_printf(";---------- parser_for ----------\n");
	expr_p1.func = func;
	numb_param2 = *numb_param;
	numb_param2.break_numb = break_label;
	numb_param2.continue_numb = continue_label;
	numb_param2.default_flag = 0;
	
	c32_token_process(0, &type);				// for ���m�F����
	if(type != TYPE_FOR)
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	c32_token_process(0, &type);				// '(' ���m�F����
	if(type != TYPE_L_KAKKO)
		c32_error_message(E_L_KAKKO_MISSING, __LINE__, __FILE__);
	c32_src_ptr_save(&src_ptr);					// a �̎�O
	c32_token_process(0, &type);
	if(type != TYPE_SEMIKORON){		// a �L��
		c32_src_ptr_restore(&src_ptr);
		
		parser_dainyuu(func);				// a �̖{��
		c32_output_buffer_flush();
	}
	c32_printf("L%d\n", top_label);				// b �̎�O
	c32_src_ptr_save(&src_ptr);
	c32_token_process(0, &type);				// b �L��Ȃ���
	if(type==TYPE_SEMIKORON){				// b ����
		c32_printf("	j		L%d\n", true_label);
	}
	else{									// b �L
		c32_src_ptr_restore(&src_ptr);
		
//		expr_compare(func, true_label, break_label);	// b �̖{��
		sprintf(expr_p1.reg_ans, "t%d", c32_s_numb);
		c32_expr(&expr_p1);
		c32_output_buffer_flush();
		convert_mode0(&expr_p1);
		c32_printf("	beqz	$t%d, L%d\n", c32_s_numb, break_label);
		c32_printf("	j		L%d\n", true_label);
		
		c32_token_process(0, &type);			// ';' �m�F
		if(type != TYPE_SEMIKORON)
			c32_error_message(E_SEMIKORON_MISSING, __LINE__, __FILE__);
	}
	c32_printf("L%d\n", continue_label);		// continue ����
	c32_src_ptr_save(&src_ptr);
	c32_token_process(0, &type);
	if(type != TYPE_R_KAKKO){
		c32_src_ptr_restore(&src_ptr);
		
		sprintf(expr_p1.reg_ans, "t%d", c32_s_numb);
		c32_expr(&expr_p1);						// c �̖{��
		c32_output_buffer_flush();
		convert_mode0(&expr_p1);
		
		c32_token_process(0, &type);			// ')' �m�F
		if(type != TYPE_R_KAKKO)
			c32_error_message(E_R_KAKKO_MISSING, __LINE__, __FILE__);
	}
	c32_printf("	j		L%d\n", top_label);
	c32_printf("L%d\n", true_label);			// {d} �̎�O
	
	parser_block(func, &numb_param2);
	c32_output_buffer_flush();
	
	c32_printf("	j		L%d\n", continue_label);
	
	c32_printf("L%d\n", break_label);
}
/********************************************************************************/
/*		parser_while															*/
/********************************************************************************/
static void parser_while(struct symtbl *func, struct numb_param *numb_param)
{
	int break_label = c32_label_counter++;
	int continue_label = c32_label_counter++;
	int true_label = c32_label_counter++;
	int type;
	struct numb_param numb_param2;
	struct expr_param expr_p1;
	
	c32_printf(";---------- parser_while ----------\n");
	expr_p1.func = func;
	numb_param2 = *numb_param;
	numb_param2.break_numb = break_label;
	numb_param2.continue_numb = continue_label;
	numb_param2.default_flag = 0;
	
	c32_token_process(0, &type);				// while ���m�F����
	if(type != TYPE_WHILE)
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	c32_token_process(0, &type);				// '(' ���m�F����
	if(type != TYPE_L_KAKKO)
		c32_error_message(E_L_KAKKO_MISSING, __LINE__, __FILE__);
	
	c32_printf("L%d\n", continue_label);
//	expr_compare(func, true_label, break_label);
	sprintf(expr_p1.reg_ans, "t%d", c32_s_numb);
	c32_expr(&expr_p1);
	c32_output_buffer_flush();
	convert_mode0(&expr_p1);
	c32_printf("	beqz	$t%d, L%d\n", c32_s_numb, break_label);
	
	c32_token_process(0, &type);				// ')' ���m�F����
	if(type != TYPE_R_KAKKO)
		c32_error_message(E_R_KAKKO_MISSING, __LINE__, __FILE__);
	
	c32_printf("L%d\n", true_label);			// loop top
	parser_block(func, &numb_param2);	// break_label, continue_label �̏�
	c32_output_buffer_flush();
	c32_printf("	j		L%d\n", continue_label);
	
	c32_printf("L%d\n", break_label);
}
/********************************************************************************/
/*		parser_if																*/
/********************************************************************************/
static void parser_if(struct symtbl *func, struct numb_param *numb_param)
{
//	int true_label = c32_label_counter++;
	int false_label = c32_label_counter++;
	int end_label = c32_label_counter++;
	int type;
	struct src_ptr src_ptr;
	struct expr_param expr_p1;
	
	c32_printf(";---------- parser_if ----------\n");
	expr_p1.func = func;
	c32_token_process(0, &type);				// if  ���m�F����
	if(type != TYPE_IF)
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	c32_token_process(0, &type);				// '(' ���m�F����
	if(type != TYPE_L_KAKKO)
		c32_error_message(E_L_KAKKO_MISSING, __LINE__, __FILE__);
	
//	expr_compare(func, true_label, false_label);
	sprintf(expr_p1.reg_ans, "t%d", c32_s_numb);
	c32_expr(&expr_p1);
	c32_output_buffer_flush();
	convert_mode0(&expr_p1);
	c32_printf("	beqz	$t%d, L%d\n", c32_s_numb, false_label);

	c32_token_process(0, &type);				// ')' ���m�F����
	if(type != TYPE_R_KAKKO)
		c32_error_message(E_R_KAKKO_MISSING, __LINE__, __FILE__);
	
	parser_block(func, numb_param);

	c32_printf("	j		L%d\n", end_label);

	c32_printf("L%d\n", false_label);
	
	c32_src_ptr_save(&src_ptr);
	c32_token_process(0, &type);				// else ���L�邩�ǂ������ׂ�
	if(type==TYPE_ELSE){					// else ���L����
		parser_block(func, numb_param);
	}
	else{
		c32_src_ptr_restore(&src_ptr);
	}
	c32_printf("L%d\n", end_label);
}
/********************************************************************************/
/*		parser_asm																*/
/*																				*/
/*		�g�p��																	*/
/*		_asm{																	*/
/*			MOVLW 10															*/
/*		}																		*/
/********************************************************************************/
static void parser_asm()
{
	int type, no_printx_save;
	struct src_ptr src_ptr;
	int end_numb = c32_label_counter++;
	
	c32_printf(";---------- parser_asm ----------\n");
	no_printx_save = c32_no_printx;
	c32_token_process(0, &type);				// _asm ���m�F����
	if(type != TYPE_ASM)
		c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
	c32_token_process(0, &type);				// '{' ���m�F����
	if(type != TYPE_L_NAMIKAKKO)
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	c32_linebufp += strlen(c32_linebufp);			// '\n' �܂ŃX�L�b�v����
	for(;;){
		c32_no_printx = 1;
		c32_spskip();
		c32_no_printx = no_printx_save;
		c32_linebufp = c32_linebuf;
		c32_linebufp = strchr(c32_linebufp, ':');
		if(c32_linebufp==0)
			c32_error_message(E_INPUT_FILE_ERROR, __LINE__, __FILE__);
		c32_linebufp += 2;
		// �s���i���ԍ��X�L�b�v�ς݁j
		c32_src_ptr_save(&src_ptr);
		c32_spskip();
		if(*c32_linebufp=='}'){
			c32_linebufp++;
			break;
		}
		c32_src_ptr_restore(&src_ptr);
		// �s���i���ԍ��X�L�b�v�ς݁j
		c32_printf("%s\n", c32_linebufp);
		c32_linebufp += strlen(c32_linebufp);
	}
	c32_printf("L%d\n", end_numb);
}
/********************************************************************************/
/*		parser_goto																*/
/*		goto ���̐擪���珈������Agoto �ł��邱�Ƃ͊m�F�ς�					*/
/********************************************************************************/
static void parser_goto(struct symtbl *func)
{
	int type;
	
	c32_printf(";----------  parser_goto  ----------\n");
	c32_token_process(0, &type);				// goto �̊m�F
	if(type != TYPE_GOTO)
		c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
	c32_spskip();
	if(c32_is_sym_top()==0)
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	c32_getsym();
	c32_printf("	j	%s@%s\n", func->symbuf, c32_symbuf);
	
	c32_token_process(0, &type);				// ';' �̊m�F
	if(type != TYPE_SEMIKORON)
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
}
/********************************************************************************/
/*		parser_dainyuu															*/
/*		c32_expr() �� goto �̃��x��													*/
/********************************************************************************/
static void parser_dainyuu(struct symtbl *func)
{
	int type;
	char buf[MAX_SYMBUF_LEN];
	struct src_ptr src_ptr;
	struct expr_param expr_p1;
	
	c32_printf(";---------- parser_dainyuu ----------\n");
	expr_p1.func = func;
	c32_src_ptr_save(&src_ptr);					// ���̐擪��ۑ�����
	c32_token_process(0, &type);					// �擪�� symbol ����荞��
	strcpy(buf, c32_symbuf);
	if(type != TYPE_SYMBOL){
		goto next1;
	}
	c32_token_process(0, &type);
	if(type==TYPE_KORON){					// ':' goto label
		if((strlen(buf)+1) >= MAX_SYMBUF_LEN)
			c32_error_message(E_TOO_LONG_SYMBOL, __LINE__, __FILE__);
		c32_printf("%s@%s\n", func->symbuf, c32_symbuf);
		return;
	}
next1:;
	c32_src_ptr_restore(&src_ptr);				// ���̐擪�ɖ߂�
	
	sprintf(expr_p1.reg_ans, "t%d", c32_s_numb);
	c32_expr(&expr_p1);
	c32_output_buffer_flush();
	
	c32_token_process(0, &type);				// ';' �m�F
	if(type != TYPE_SEMIKORON)
		c32_error_message(E_SEMIKORON_MISSING, __LINE__, __FILE__);
}
/********************************************************************************/
/*		parser_return															*/
/********************************************************************************/
static void parser_return(struct symtbl *func)
{
	int type;
	struct src_ptr src_ptr;
	char buf[10];
	struct expr_param expr_p1;
	
	c32_printf(";----------  parser_return  ---------\n");
	expr_p1.func = func;
	sprintf(buf, "t%d", c32_s_numb);
	c32_token_process(0, &type);				// "return" ���m�F����
	if(type != TYPE_RETURN)
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	c32_src_ptr_save(&src_ptr);
	c32_token_process(0, &type);
	if(type==TYPE_SEMIKORON){
		function_pop_reg(func);
		c32_printf("	jr		$ra\n");
		return;
	}
	c32_src_ptr_restore(&src_ptr);
	
//	sprintf(expr_p1.reg_ans, "t%d", c32_s_numb);
	strcpy(expr_p1.reg_ans, "v0");
	c32_expr(&expr_p1);
	c32_output_buffer_flush();
	convert_mode0(&expr_p1);

	function_pop_reg(func);
	c32_printf("	jr		$ra\n");
	
	c32_token_process(0, &type);				// ';' ���m�F����
	if(type != TYPE_SEMIKORON)
		c32_error_message(E_SEMIKORON_MISSING, __LINE__, __FILE__);
}
/********************************************************************************/
/*		parser_auto_variable_reg												*/
/*		c32_symbuf �� auto �ϐ����������Ă���A���o�^�ł��邱�Ƃ͊m�F�ς�			*/
/********************************************************************************/
static struct symtbl *parser_auto_variable_reg(struct symtbl *func, int attr)
{
	int cnt;
	uint cnt_auto;
	struct symtbl *ptr;
	
	ptr = (struct symtbl *)c32_malloc4(sizeof(struct symtbl));
	func->func_tbl->variable_tbl [ cnt=func->func_tbl->variable_tbl_cnt++ ] = ptr;
	cnt_auto = cnt - func->func_tbl->param_numb;
	if(cnt_auto<7 && c32_pass==PASS3 && (ptr->flag1 & FLAG_20)==0){		// ���W�X�^���蓖�Ă�����
		sprintf(ptr->reg, "s%d", cnt_auto);
	}
	if(cnt >= MAX_VARIABLE)
		c32_error_message(E_OUT_OF_MEMORY, __LINE__, __FILE__);
	ptr->symbuf = c32_malloc(strlen(c32_symbuf)+1);
	strcpy(ptr->symbuf, c32_symbuf);
	ptr->label = c32_malloc(strlen(c32_symbuf)+strlen(func->symbuf)+2);
	sprintf(ptr->label, "%s@%s", func->symbuf, c32_symbuf);
	ptr->size = c32_attr_to_byte(attr);	// �P���ϐ��Ƃ��ăT�C�Y���Z�b�g����
	ptr->type = TYPE_SYMBOL;
	ptr->attr = attr;
	return ptr;
}
/********************************************************************************/
/*		parser_auto_variable													*/
/*		���̐擪���珈������													*/
/********************************************************************************/
static void parser_auto_variable(struct symtbl *func)
{
	int attr, type;
	struct symtbl *ptr;
	struct src_ptr src_ptr;
	
	attr = c32_char_short_int_unsigned_pointer();
	if(attr==0)
		c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
	for(;;){
		c32_token_process(0, &type);			// �ϐ����� c32_symbuf[] �֎�荞��
		if(type != TYPE_SYMBOL)
			c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
		ptr = c32_sym_search_auto(func);		// �V���{���e�[�u���T�[�`
		if(c32_pass==PASS3 || c32_pass==PASS2){
			if(ptr==0)
				c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
		}
		if(c32_pass==PASS1){					// auto�ϐ� �o�^�p�X
			if(ptr)
				c32_error_message(E_MULTIPLE_DEFINITION, __LINE__, __FILE__);
			//	ptr �� undefine
			ptr = parser_auto_variable_reg(func, attr);
			ptr->flag1 |= FLAG_02;			// auto �ϐ��t���O
		}
		//	�����ł� ptr �ɃV���{���e�[�u�����o���Ă���
		
		c32_src_ptr_save(&src_ptr);
		c32_token_process(0, &type);				// int sym[  int sym;  int sym=  int sym,
		if(type==TYPE_SEMIKORON){				// int sym;   �P���ϐ�
			return;
		}
		else if(type==TYPE_KANMA){				// int sym,
			continue;
		}
		else if(type==TYPE_L_KAKUKAKKO){		// int sym[
			c32_src_ptr_restore(&src_ptr);
			parser_array(ptr);				// int var[x][y][z]
			
//AAAAA			ptr->attr |= ATTR_ARRAY;
//			parser_auto_array(func, ptr);		// int sym[...]   '[' �̎�O���珈������
			c32_token_process(0, &type);			// ',' ';'
			if(type==TYPE_SEMIKORON)
				return;
			else if(type==TYPE_KANMA)
				continue;
			else
				c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
		}
		else{
			c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
		}
	}
}
/********************************************************************************/
/*		parser_block															*/
/*		{...} �� �P��;   �łȂ���΃G���[�ɂ��邼								*/
/*		'{' �̎�O���珈���J�n�A{} �������ꍇ�i�P���j����������					*/
/********************************************************************************/
static void parser_block(struct symtbl *func, struct numb_param *numb_param)
{
	int ret, type;
	struct src_ptr src_ptr;
	
	c32_printf(";---------- parser_block ----------\n");
	c32_src_ptr_save(&src_ptr);
	c32_token_process(0, &type);				// { ���P���̐擪��荞��
	c32_src_ptr_restore(&src_ptr);
	if(type==TYPE_L_NAMIKAKKO){				// { ��荞��
		parser_function_block(func, numb_param);
		c32_output_buffer_flush();
	}
	else{									// �P������������
		ret = parser_function_block_single(func, numb_param);
		c32_output_buffer_flush();
		if(ret==0){							// �P������ success
			return;
		}
		else								// �P���ł͂Ȃ�����
			c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	}
}
/********************************************************************************/
/*		parser_function_block_single											*/
/*		���P�̏����A���̓��ŌĂяo�����										*/
/*		return 0:success, 1:���ł͂Ȃ������Arestore �ς�						*/
/********************************************************************************/
static int parser_function_block_single(struct symtbl *func, struct numb_param *numb_param)
{
	int attr, type;
	struct src_ptr src_ptr;
	
	c32_src_ptr_save(&src_ptr);
	attr = c32_char_short_int_unsigned_pointer();	// �^�錾��荞��
	c32_src_ptr_restore(&src_ptr);				// ���̓��ɖ߂�
	if(attr){								// success�A�^�錾������
		parser_auto_variable(func);
		return 0;							// success
	}
	else{									// �^�錾�ł͖�������
		c32_token_process(0, &type);
		c32_src_ptr_restore(&src_ptr);			// ���̓��ɖ߂�
		
		if(type==TYPE_GOTO){			// goto ��������
			parser_goto(func);
		}
		else if(type==TYPE_CONTINUE){		// continue ��������
			parser_continue(func, numb_param);
		}
		else if(type==TYPE_BREAK){			// break ��������
			parser_break(func, numb_param);
		}
		else if(type==TYPE_FOR){			// for ��������
			parser_for(func, numb_param);
		}
		else if(type==TYPE_DO){				// do ��������
			parser_do(func, numb_param);
		}
		else if(type==TYPE_WHILE){			// while ��������
			parser_while(func, numb_param);
		}
		else if(type==TYPE_IF){				// if ��������
			parser_if(func, numb_param);
		}
		else if(type==TYPE_ELSE){			// else ��������
			c32_error_message(E_ILLEGAL_ELSE, __LINE__, __FILE__);
		}
		else if(type==TYPE_RETURN){			// return ��������
			parser_return(func);
		}
		else if(type==TYPE_TEST){			// __test__  abc
			c32_token_process(0, &type);			// __test__ �X�L�b�v
			c32_test(func);
		}
		else if(type==TYPE_ASM){			// _asm
			parser_asm();
		}
		else if(type==TYPE_SWITCH){			// switch ��������
			parser_switch(func, numb_param);
		}
		else if(type==TYPE_CASE){			// case ��������
			parser_case(func, numb_param);
		}
		else if(type==TYPE_DEFAULT){		// default ��������
			parser_default(func, numb_param);
		}
		else if(type==TYPE_SEMIKORON){		// ';' �󕶂������A';' �̎�O�ɖ߂��Ă���
			c32_token_process(0, &type);		// ';' �������
		}
		else if(type==TYPE_R_NAMIKAKKO){	// '}' ������
			return 1;
		}
		else{								// �ǂ̕��ł��Ȃ�  ���
			parser_dainyuu(func);
		}
		return 0;
	}
}
/********************************************************************************/
/*		parser_function_block													*/
/*		{...} �̏��� '{' �̎�O���珈������										*/
/********************************************************************************/
static void parser_function_block(struct symtbl *func, struct numb_param *numb_param)
{
	int ret, type;
	
	c32_printf(";---------- parser_function_block ----------\n");
	c32_token_process(0, &type);				// '{' ���m�F����
	if(type != TYPE_L_NAMIKAKKO)
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	for(;;){
		ret = parser_function_block_single(func, numb_param);	// ���P�̏����A0:success�A1:error,restore �ς�
		c32_output_buffer_flush();
		if(ret){							// error �Ȃ��
			c32_token_process(0, &type);
			if(type==TYPE_R_NAMIKAKKO){		// {...} ��������
				return;
			}
			else{
				c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
			}
		}
	}
}
/********************************************************************************/
/*			function_push_reg													*/
/*			$ra, $fp, $s0�`    push ����												*/
/********************************************************************************/
static void function_push_reg(struct symtbl *func)
{
	int auto_size, reserve_s;
	
	reserve_s = func->func_tbl->variable_tbl_cnt;
	reserve_s = reserve_s > 7 ? 7 : reserve_s;
	auto_size = func->func_tbl->auto_size;
	c32_printf("	addiu	$sp, $sp, %d	; $fp,$ra,$sx\n", -(reserve_s+2)*4);
	c32_printf("	sw		$fp, 4($sp)\n");
	c32_printf("	sw		$ra, 0($sp)\n");
//	for(i=0; i<reserve_s; i++){
//		c32_printf("	sw		$s%d, %d($sp)\n", i, (i+2)*4);
//	}
	
	if(auto_size)
		c32_printf("	addiu	$sp, $sp, %d	; auto variable\n", -auto_size);	// auto �ϐ��G���A�쐬
	c32_printf("	move	$fp, $sp\n");
	c32_output_buffer_flush();
}

static void function_pop_reg(struct symtbl *func)
{
	int auto_size, reserve_s;
	
	reserve_s = func->func_tbl->variable_tbl_cnt;
	reserve_s = reserve_s > 7 ? 7 : reserve_s;
	auto_size = func->func_tbl->auto_size;
	if(auto_size)
		c32_printf("	addiu	$sp, $sp, %d	; auto variable\n", auto_size);	// auto �ϐ��G���A�J��

	c32_printf("	lw		$fp, 4($sp)\n");
	c32_printf("	lw		$ra, 0($sp)\n");
//	for(i=0; i<reserve_s; i++){
//		c32_printf("	lw		$s%d, %d($sp)\n", i, (i+2)*4);
//	}
	c32_printf("	addiu	$sp, $sp, %d	; $fp,$ra,$sx\n", (reserve_s /*MAX_RESERVE_S_REG*/ +2)*4);
	c32_output_buffer_flush();
}
/********************************************************************************/
/*		parser_function															*/
/*		int main(...){...}     '(' �̎�O���珈������							*/
/********************************************************************************/
static void parser_function(struct symtbl *func)
{
	int i, type, attr, cnt;
	struct symtbl *ptr;
	struct src_ptr src_ptr;
	struct numb_param numb_param2;
	
	c32_token_process(0, &type);				// '(' ���m�F����
	if(type != TYPE_L_KAKKO)
		c32_error_message(E_L_KAKKO_MISSING, __LINE__, __FILE__);
	c32_src_ptr_save(&src_ptr);
	c32_token_process(0, &type);				// ')' ���m�F����
	
	/***	�p�����[�^�������[�v	***/
	if(type != TYPE_R_KAKKO){				// parameter �L
		c32_src_ptr_restore(&src_ptr);
		for(;;){
			attr = c32_char_short_int_unsigned_pointer();
			if(attr==0){					// �錾����������
				c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
			}
			/***	�錾���L����	***/
			c32_token_process(0, &type);		// param ������
			if(type != TYPE_SYMBOL)
				c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
			ptr = c32_sym_search_auto(func);	// param ����荞��
			if(c32_pass==PASS3 || c32_pass==PASS2){
				if(ptr==0)
					c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
			}
			if(c32_pass==PASS1){				// �ϐ�/�֐� �o�^�p�X
				if(ptr)
					c32_error_message(E_MULTIPLE_DEFINITION, __LINE__, __FILE__);
				//	ptr �� undefine
				ptr = parser_auto_variable_reg(func, attr);
				ptr->flag1 |= FLAG_01;		// param �ϐ��t���O
			}
			c32_output_buffer_flush();
			c32_src_ptr_save(&src_ptr);
			c32_token_process(0, &type);		// '[' ���� ')' ���� ',' ���m�F����
			if(type==TYPE_L_KAKUKAKKO){
				c32_src_ptr_restore(&src_ptr);
				parser_array(ptr);				// [] ����
				ptr->attr |= ATTR_ARRAY_P;
				c32_token_process(0, &type);
			}
			if(type==TYPE_R_KAKKO)
				break;
			if(type==TYPE_KANMA)
				continue;
			c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
		}
	}
	/***	�ϐ��I�t�Z�b�g�� equ �ŏo�͂��Ă���	***/
	cnt = func->func_tbl->variable_tbl_cnt;
	for(i=0; i<cnt; i++){
		ptr = func->func_tbl->variable_tbl[i];
		c32_printf("%s@%s	equ	%d\n", func->symbuf, ptr->symbuf, ptr->value);
		c32_output_buffer_flush();
	}
	/***	�֐��{�̏���	***/
	c32_printf(";---------- function %s ----------\n", func->symbuf);
	c32_printf("%s\n", func->symbuf);			// ���x���f���o��
	c32_s_numb = c32_max_s_numb = 2;				// $s0 ���� c32_expr() �p�Ɏg�p����

	numb_param2.break_numb = -100;
	numb_param2.continue_numb = -100;
	numb_param2.switch_numb = -100;
	numb_param2.serial_numb = -100;
	numb_param2.s_switch = -100;
	numb_param2.default_flag = 0;
	function_push_reg(func);
	c32_output_buffer_flush();
	
	parser_function_block(func, &numb_param2);		// {...} �̏���
	
	function_pop_reg(func);
	c32_output_buffer_flush();

	func->func_tbl->max_s_reg_numb = c32_max_s_numb;
	if(c32_max_s_numb >= 9)						// t8 �܂Ŏg�p��
		c32_error_message(E_EXPRESSION_TOO_COMPLEX, __LINE__, __FILE__);
	c32_printf("	jr		$ra\n");
}
/********************************************************************************/
/*		parser_globa_array_init													*/
/*		{...}  �������e�[�u������												*/
/*		global �̔z��ł̓��x���ԍ��� var->init �ɃZ�b�g����Ă���				*/
/********************************************************************************/
static void parser_global_array_init(struct symtbl *var)
{
	int i, num, type, object_size, no_printx_save;
	struct src_ptr src_ptr, src_ptr2;
	struct expr_param expr_p1;
//	int label_start = c32_label_counter++;
	
	expr_p1.func = 0;
	object_size = c32_attr_to_byte(var->attr);
	//	�T�C�Y�O�̔z��̃T�C�Y���������f�[�^���狁�߂�
	if(var->size==0){
		c32_src_ptr_save(&src_ptr2);
		c32_token_process(0, &type);				// '{' �� string �𒲂ׂ�
		if(type==TYPE_STRING){
			var->size = (ushort)strlen(cv->string_buf)+1;
		}
		else{
			if(type != TYPE_L_NAMIKAKKO)		// '{' ���m�F����
				c32_error_message(E_L_NAMIKAKKO_MISSING, __LINE__, __FILE__);
			for(i=0; ; i++){
				sprintf(expr_p1.reg_ans, "t%d", c32_s_numb);
				
				no_printx_save = c32_no_printx;
				c32_no_printx = 1;
				c32_factor1(&expr_p1);				// ',' ���Z�q�s�g�p
				c32_no_printx = no_printx_save;
				
				c32_token_process(0, &type);
				if(type==TYPE_R_NAMIKAKKO)
					break;
				else if(type==TYPE_KANMA){
					c32_src_ptr_save(&src_ptr);
					c32_token_process(0, &type);
					if(type==TYPE_R_NAMIKAKKO)
						break;
					c32_src_ptr_restore(&src_ptr);
					continue;
				}
				else{
					c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
				}
			}
			var->size = (i+1)*object_size;
		}
		c32_src_ptr_restore(&src_ptr2);
		//	�������f�[�^�G���A�� work �ɍ��
		var->work = (void*)c32_malloc4(var->size);
		if(c32_attr_to_byte(var->attr)==4){	// 4�o�C�g�ϐ��Ȃ� var->work_bitmap �G���A�����
			var->work_bitmap = c32_malloc((expr_p1.value+7)/8);
		}
		var->work_cnt = var->size;
	}
	num = var->size / object_size;
	c32_token_process(0, &type);					// '{' �� string �𒲂ׂ�

	if(type==TYPE_STRING){
		if(var->size < strlen(cv->string_buf)+1)
			c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
		strcpy((char*)var->work, cv->string_buf);
	}
	else{
		if(type != TYPE_L_NAMIKAKKO)			// '{' ���m�F����
			c32_error_message(E_L_NAMIKAKKO_MISSING, __LINE__, __FILE__);
		//
		//	�����l�e�[�u���� var->work �ɍ��
		for(i=0; i<num; i++){
			sprintf(expr_p1.reg_ans, "t%d", c32_s_numb);
			c32_factor1(&expr_p1);					// ',' ���Z�q�s�g�p
			c32_output_buffer_flush();			//AAAAA
			if(expr_p1.mode==MODE2){			// �萔����
				switch(object_size){
					case 1:
						((uchar*)var->work)[i] = expr_p1.value;
						break;
					case 2:
						((ushort*)var->work)[i] = expr_p1.value;
						break;
					case 4:
						((uint*)var->work)[i] = expr_p1.value;
						break;
					default:
						c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
				}
			}
			else if(expr_p1.mode==MODE6){		// ���x���A�h���X����
				if(c32_attr_to_byte(var->attr) != 4)
					c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
				((uint*)var->work)[i] = expr_p1.value;
				(var->work_bitmap)[i/8] |= 1<<i%8;	// ���x���ł��邱�Ƃ������t���O�Z�b�g
			}
			else{
				c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
			}
			
			c32_token_process(0, &type);
			if(type==TYPE_R_NAMIKAKKO)
				break;
			else if(type==TYPE_KANMA){
				c32_src_ptr_save(&src_ptr);
				c32_token_process(0, &type);
				if(type==TYPE_R_NAMIKAKKO)
					break;
				c32_src_ptr_restore(&src_ptr);
				continue;
			}
			else{
				c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
			}
		}
		if(i >= num)
			c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	}
	/***	var->work ��f���o��	***/
//	var->init = label_start;
//	c32_printf("L%d\n", label_start);
	if(var->attr & ATTR_CONST){					// const �Ȃ��
		c32_printf("%s\n", var->symbuf);
	}
	else{
		c32_printf("%s@init\n", var->symbuf);
	}
	if(object_size==4){
		for(i=0; i<num; i++){
			if((var->work_bitmap)[i/8] & (1<<i%8))	// ���x���ԍ��ł���
				c32_printf("	dw		L%d\n", ((uint*)var->work)[i]);
			else
				c32_printf("	dw		%d\n", ((uint*)var->work)[i]);
			c32_output_buffer_flush();
		}
	}
	else{
		for(i=0; i<var->size; i++){
			c32_printf("	db		%d\n", ((uchar*)var->work)[i]);
			c32_output_buffer_flush();
		}
	}
	
	var->flag1 |= FLAG_08|FLAG_10;				// ����������Ă���ϐ�, init �̓��x���ԍ�
}
/********************************************************************************/
/*		parser_global_array														*/
/*		int var[...]={...}   '=' �̎�O���珈������								*/
/*		var->size, var->work, var->work_bitmap, var->work_cnt					*/
/*		parser_global_array_init() �Ăяo��										*/
/********************************************************************************/
static void parser_global_array(struct symtbl *var)
{
	int type;
	struct src_ptr src_ptr;

	c32_src_ptr_save(&src_ptr);
	c32_token_process(0, &type);				// ',' ���� ';' ���� '='
	if(type==TYPE_SEMIKORON || type==TYPE_KANMA){
		c32_src_ptr_restore(&src_ptr);			// ',' ';' �̎�O�ɖ߂�
		return;
	}
	else if(type==TYPE_EQ){					// '='
		if(var->size){
			//	�������f�[�^�G���A�� work �ɍ��
			var->work = (void*)c32_malloc4(var->size);
			if(c32_attr_to_byte(var->attr)==4){	// 4�o�C�g�ϐ��Ȃ� var->work_bitmap �G���A�����
				var->work_bitmap = c32_malloc((var->size/4+7)/8);
			}
			var->work_cnt = var->size;
		}
		
		parser_global_array_init(var);		// {...}
		return;
	}
	else{
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	}
}
/********************************************************************************/
/*		parser_array															*/
/*		var->size, var->xyz_size[MAX_ARRAY_DIM], var->xyz_dim, var->attr ATTR_ARRAY			*/
/********************************************************************************/
static void parser_array(struct symtbl *var)
{
	int i, type;
	struct expr_param expr_p1;
	struct src_ptr src_ptr;
	
	expr_p1.func = 0;
	for(i=0; i<MAX_ARRAY_DIM; i++){				// max 7�����z��
		c32_src_ptr_save(&src_ptr);
		c32_token_process(0, &type);			// '[' �̏���
		if(type != TYPE_L_KAKUKAKKO){
			c32_src_ptr_restore(&src_ptr);
			break;
		}
		c32_src_ptr_save(&src_ptr);
		c32_token_process(0, &type);			// [] �̃e�X�g
		if(type==TYPE_R_KAKUKAKKO){
			var->xyz_size[i] = 0;
			continue;
		}
		c32_src_ptr_restore(&src_ptr);
		sprintf(expr_p1.reg_ans, "t%d", c32_s_numb);
		c32_expr(&expr_p1);						// �Y�����T�C�Y�����߂�
		c32_output_buffer_flush();
		if(expr_p1.mode != MODE2)			// �萔�ł��鎖
			c32_error_message(E_NEED_CONSTANT, __LINE__, __FILE__);
		if(expr_p1.value==0)
			c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
		var->xyz_size[i] = expr_p1.value;	// �Y�����T�C�Y��������
		c32_token_process(0, &type);			// ']' ���m�F����
		if(type != TYPE_R_KAKUKAKKO)
			c32_error_message(E_R_KAKUKAKKO_MISSING, __LINE__, __FILE__);
	}
	var->xyz_dim = i;
	var->attr |= i*ATTR_ARRAY1;
	var->size = c32_attr_to_byte(var->attr);
	for(i=0; i<var->xyz_dim; i++){
		var->size *= var->xyz_size[i];
	}
}
/********************************************************************************/
/*		variable_addr_global_cal												*/
/*		global �ϐ��̃A�h���X�v�Z												*/
/********************************************************************************/
static void variable_addr_global_cal()
{
	int i, j, t;
	struct symtbl tmp, *tbli, *tblj;
	
	/***	global �ϐ����בւ�		***/
	for(i=0; i<c32_tbl_ptr_g; i++){
		tbli = &cv->tbl_g[i];
		if((tbli->attr & ATTR_ARRAY) && (tbli->flag1 & FLAG_04)){	// FLAG_04: global �ϐ�
			for(j=i+1; j<c32_tbl_ptr_g; j++){
				tblj = &cv->tbl_g[j];
				if((tblj->attr & ATTR_ARRAY)==0 && (tblj->flag1 & FLAG_04)){	// FLAG_04: global �ϐ�
					tmp = *tbli;
					*tbli = *tblj;
					*tblj = tmp;
					break;
				}
			}
			if(j==c32_tbl_ptr_g)
				break;
		}
	}
	/***	�z��Ȃ�ϐ��A�h���X�A�ϐ��Ȃ�I�t�Z�b�g�� value �ɃZ�b�g����	***/
	for(i=0; i<c32_tbl_ptr_g; i++){
		tbli = &cv->tbl_g[i];
		if((tbli->flag1 & FLAG_04)==0 || (tbli->attr & ATTR_CONST))		// FLAG_04: global �ϐ�
			continue;
		switch(c32_attr_to_byte(tbli->attr)){
			case 1:
				break;
			case 2:
				c32_ram_addr += 1;
				c32_ram_addr &= 0xfffffffe;
				break;
			case 4:
				c32_ram_addr += 3;
				c32_ram_addr &= 0xfffffffc;
				break;
			default:
				c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
		}
		t = c32_ram_addr;
		if((tbli->attr & ATTR_ARRAY)==0)
			t -= RAM_BASE;
		tbli->value = t;
		c32_ram_addr += tbli->size;
	}
	c32_ram_addr += 3;
	c32_ram_addr &= 0xfffffffc;
}
/********************************************************************************/
/*		variable_addr_auto_cal													*/
/*		auto �ϐ��̃I�t�Z�b�g�v�Z												*/
/*		int sub(int a, int b, int c, int d, int e, int f){ int g;}				*/
/*		stack:	f																*/
/*				e																*/
/*				-	d $a3 ������												*/
/*				-	c $a2 ������												*/
/*				-	b $a1 ������												*/
/*				-	a $a0 ������												*/
/*			......................... call ���̃X�^�b�N							*/
/*				$s-		max_s_reg_numb �G���A�L��								*/
/*				$fp																*/
/*				$ra																*/
/*				g																*/
/********************************************************************************/
static void variable_addr_auto_cal(struct symtbl *func)
{
	int i, j, size, cnt, auto_top, reserve_s;
	struct symtbl **tbl, *tmp;
	
	reserve_s = func->func_tbl->variable_tbl_cnt;
	reserve_s = reserve_s > 7 ? 7 : reserve_s;
	cnt = func->func_tbl->variable_tbl_cnt;
	tbl = func->func_tbl->variable_tbl;
	/***	auto_top �쐬	***/
	for(auto_top=0; auto_top<cnt; auto_top++){
		if(tbl[auto_top]->flag1 & FLAG_02)	// auto �ϐ�
			break;
	}
	/***	auto �ϐ����בւ�	***/
	for(i=auto_top; i<cnt; i++){
		if((tbl[i]->attr & ATTR_CHAR)==0){	// i �Ԗڂ� char �łȂ�
			for(j=i+1; j<cnt; j++){
				if(tbl[j]->attr & ATTR_CHAR){
					tmp = tbl[i];
					tbl[i] = tbl[j];
					tbl[j] = tmp;
					break;
				}
			}
			if(j==cnt)
				break;
		}
	}
	for( ; i<cnt; i++){
		if((tbl[i]->attr & ATTR_SHORT)==0){	// i �Ԗڂ� short �łȂ�
			for(j=i+1; j<cnt; j++){
				if(tbl[j]->attr & ATTR_SHORT){
					tmp = tbl[i];
					tbl[i] = tbl[j];
					tbl[j] = tmp;
					break;
				}
			}
			if(j==cnt)
				break;
		}
	}
	/***	auto �ϐ��A�h���X/���o�C�g���v�Z		***/
	size = 0;
	for(i=auto_top; i<cnt; i++){
		switch(c32_attr_to_byte(tbl[i]->attr)){
			case 1:
				break;
			case 2:
				size += 1;
				size &= 0xfffffffe;
				break;
			case 4:
				size += 3;
				size &= 0xfffffffc;
				break;
			default:
				c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
		}
		tbl[i]->value = size;				// �ϐ��I�t�Z�b�g
		size += tbl[i]->size;
	}
	size += 3;
	size &= 0xfffffffc;
	func->func_tbl->auto_size = size;		// auto ���o�C�g���Z�[�u
	func->func_tbl->param_numb = auto_top;	// �p�����[�^���Z�[�u
	/***	param �A�h���X�v�Z		***/
	for(i=0; i<auto_top; i++){
		tbl[i]->value = size + (reserve_s /*MAX_RESERVE_S_REG*/ +2)*4 + i*4;	// �p�����[�^�I�t�Z�b�g
	}
}
/********************************************************************************/
/*		variable_addr_cal														*/
/*		�ϐ��̃A�h���X/�I�t�Z�b�g�v�Z�Aglobal �͕��בւ��i�P��/�z��j			*/
/********************************************************************************/
static void variable_addr_cal()
{
	int i;
	struct symtbl *func;
	
	/***	auto �ϐ�	***/
	for(i=0; i<c32_tbl_ptr_g; i++){
		func = &cv->tbl_g[i];
		if(func->attr & ATTR_FUNC){			// �֐�������
			variable_addr_auto_cal(func);	// auto �ϐ��̃A�h���X���蓖��
		}
	}
	/***	global �ϐ�	***/
	variable_addr_global_cal();				// global �ϐ��̃A�h���X���蓖��
}
/********************************************************************************/
/*		parser_char_short_int													*/
/*		attr: ATTR_CHAR, ATTR_INT, ATTR_LONG, ATTR_UNSIGNED, ATTR_POINTER		*/
/*		�^�錾����荞��ŌĂяo�����											*/
/*		global �ϐ��̎��́A�֐��錾/����										*/
/********************************************************************************/
static void parser_char_short_int(int attr)
{
	int type;
	struct symtbl *ptr;
	struct src_ptr src_ptr;
	struct expr_param expr_p1;
	
	expr_p1.func = 0;
	for(;;){
		ptr = c32_token_process(0, &type);			// global �ϐ�/�֐�������荞��
		if(type != TYPE_SYMBOL)
			c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
		if(c32_pass==PASS3 || c32_pass==PASS2){
			if(ptr==0)
				c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
		}
		if(c32_pass==PASS1){						// �ϐ�/�֐� �o�^�p�X
			if(ptr)
				c32_error_message(E_MULTIPLE_DEFINITION, __LINE__, __FILE__);
			//	ptr �� undefine
			ptr = &cv->tbl_g[c32_tbl_ptr_g++];
			if(c32_tbl_ptr_g > MAX_SYMBOL_TABLE_G)
				c32_error_message(E_OUT_OF_MEMORY, __LINE__, __FILE__);
			ptr->symbuf = ptr->label = c32_malloc(strlen(c32_symbuf)+1);
			strcpy(ptr->symbuf, c32_symbuf);
			if((attr & ATTR_VOID)==0)
				ptr->size = c32_attr_to_byte(attr);	// �P���ϐ��Ƃ��ăT�C�Y���Z�b�g����
			ptr->type = TYPE_SYMBOL;
			ptr->attr = attr;
			ptr->flag1 |= FLAG_04;			// global �ϐ��t���O
		}
		//	�����ł� ptr �ɃV���{���e�[�u�����o���Ă���
		
		c32_src_ptr_save(&src_ptr);
		c32_token_process(0, &type);				// int sym(  int sym[  int sym;  int sym=  int sym,
		if(type==TYPE_L_KAKKO){					// int sym(
			c32_src_ptr_restore(&src_ptr);
			if(c32_pass==PASS1)
				ptr->func_tbl = (struct func_tbl *)c32_malloc4(sizeof(struct func_tbl));
			ptr->attr |= ATTR_FUNC;
			ptr->flag1 = 0;
			parser_function(ptr);				// int sym(...){...}   '(' �̎�O���珈������
			c32_output_buffer_flush();
			return;
		}
		else if(type==TYPE_SEMIKORON){			// int sym;   �P���ϐ�
			if(attr & ATTR_VOID)
				c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
			return;
		}
		else if(type==TYPE_KANMA){				// int sym,
			if(attr & ATTR_VOID)
				c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
			continue;
		}
		else if(type==TYPE_EQ){					// int sym=
			if(attr & ATTR_VOID)
				c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
			sprintf(expr_p1.reg_ans, "t%d", c32_s_numb);
			c32_expr(&expr_p1);
			c32_output_buffer_flush();
			ptr->init = expr_p1.value;
			ptr->flag1 |= FLAG_08;				// ����������Ă���ϐ��ł���
			c32_token_process(0, &type);			// ';' ���m�F����
			if(type != TYPE_SEMIKORON)
				c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);

			if(expr_p1.mode==MODE2){			// �萔
				if(ptr->attr & ATTR_CONST){			// const �Ȃ��
					c32_printf("%s	dw	0x%x\n", ptr->symbuf, expr_p1.value);
				}
				return;
			}
			else if(expr_p1.mode==MODE6){		// ���x���A�h���X����
				if(ptr->attr & ATTR_CONST){
					c32_printf("%s	equ	L%d\n", ptr->symbuf, expr_p1.value);
				}
				else{
					c32_printf("%s@init	equ	L%d\n", ptr->symbuf, expr_p1.value);
				}
				ptr->flag1 |= FLAG_10;			// ptr->init �̓��x���ł���
				return;
			}
			else{
				c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
			}
		}
		else if(type==TYPE_L_KAKUKAKKO){		// int sym[
			if(attr & ATTR_VOID)
				c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
			c32_src_ptr_restore(&src_ptr);
			parser_array(ptr);				// int var[x][y][z]
			
//AAAAA			ptr->attr |= ATTR_ARRAY;
			parser_global_array(ptr);		// int sym[...]={...}   '=' �̎�O���珈������
			c32_token_process(0, &type);			// ',' ';'
			if(type==TYPE_SEMIKORON)
				return;
			else if(type==TYPE_KANMA)
				continue;
			else
				c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
		}
		else{
			c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
		}
	}
}
/********************************************************************************/
/*		parser																	*/
/********************************************************************************/
static void parser()
{
	int attr, type;
//	struct symtbl *ptr;
	struct src_ptr src_ptr;
	
	for(;;){
		c32_src_ptr_save(&src_ptr);
		attr = c32_char_short_int_unsigned_pointer();	// ���̐擪���^�錾���H
		if(attr){							// success
			parser_char_short_int(attr);
		}
		else{								// error
			c32_src_ptr_restore(&src_ptr);
			c32_token_process(0, &type);		// ���̐擪�� token ��荞��
			if(type==TYPE_EOF){
				return;
			}
			else{
				c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
			}
		}
	}
}
/********************************************************************************/
/*		registration															*/
/********************************************************************************/
static void registration()
{
	int i;
	struct symtbl *ptr;
	
	for(i=0; registration_tbl[i].symbuf; i++){
		strcpy(c32_symbuf, registration_tbl[i].symbuf);
		ptr = c32_sym_search_global();
		if(ptr){
			c32_error_message(E_MULTIPLE_DEFINITION, __LINE__, __FILE__);
		}
		ptr = &cv->tbl_g[c32_tbl_ptr_g++];
		ptr->symbuf = c32_malloc(strlen(c32_symbuf)+1);
		strcpy(ptr->symbuf, c32_symbuf);
		ptr->type = registration_tbl[i].type;
	}
}
/********************************************************************************/
/*		reg																		*/
/*		����V���{���̐錾														*/
/********************************************************************************/
//static void reg1()
//{
//	struct symtbl *ptr;
//	
//	ptr = &cv->tbl_g[c32_tbl_ptr_g++];
//	ptr->symbuf = c32_malloc(strlen("__ram_end")+1);
//	ptr->label = ptr->symbuf;
//	strcpy(ptr->symbuf, "__ram_end");
//	ptr->size = c32_attr_to_byte(ATTR_INT);
//	ptr->type = TYPE_SYMBOL;
//	ptr->attr = ATTR_INT;
//	ptr->flag1 |= FLAG_04 | FLAG_08;		// global �ϐ��t���O�A�������ϐ�
//	ptr->init = c32_ram_addr;
//}
//
//static void reg2()
//{
//	struct symtbl *ptr;
//	
//	strcpy(c32_symbuf, "__ram_end");
//	ptr = c32_sym_search_global();
//	if(ptr==0){
//		printf("*** %s undefined symbol\n", c32_symbuf);
//		exit(1);
//	}
//	ptr->init = c32_ram_addr;
//}
/********************************************************************************/
/*		main																	*/
/*		major update �\��														*/
/*			1. static �̃T�|�[�g												*/
/*			2. const  �̃T�|�[�g												*/
/********************************************************************************/
#ifndef __XC32
int main(int argc, char *argv[])
{
	char *ptr;
	
	printf("%s %s %s\n", VERSION, __DATE__, __TIME__);
	cv = (struct cv *)malloc(sizeof(struct cv));
	if(cv==0){
		printf("*** out of memory\n");
		return 1;
	}
	memset(cv, 0, sizeof(struct cv));
	if(argc!=2 && argc!=3){
		printf("*** error command line\n");
		printf("$ compiler <src_file>\n");
		return 1;
	}

	/***	registration	***/
	registration();

	/***	file open	***/
	if((c32_src_fp = fopen(argv[1], "r"))==0){
		printf("*** src file open error, %s\n", argv[1]);
		return 1;
	}
	strcpy(cv->string_buf, argv[1]);
	ptr = strrchr(cv->string_buf, '.');
	if(ptr){
		*ptr = '\0';
	}
	strcat(cv->string_buf, ".s");
	if((c32_asm_fp = fopen(cv->string_buf, "w"))==0){
		printf("*** asm file open error, %s\n", cv->string_buf);
		return 1;
	}
	
	/***	c32_pass 1		***/
	//	�V���{���e�[�u���쐬�A�ϐ��T�C�Y�m��A�ϐ������l�� ptr->init �ɃZ�b�g����Aused_s_reg_numb �쐬
	//	�z�񏉊��l�͂��̏�œf���o�� ���̃��x���ԍ��� ptr->init �� �o�C�g���� ptr->size �ɃZ�b�g����
//	reg1();									// ����V���{���̐錾

	printf("[ PASS1 ]\n");
	c32_pass = PASS1;
	c32_no_printx = 1;
	memset(cv->src_buffer, 0, 10);
	c32_linebufp = cv->src_buffer;
	c32_output_buffer_flush();
	
	parser();
	
	/***	�ϐ��A�h���X/�I�t�Z�b�g �v�Z �P���ϐ��� low address �� ���̏�ɔz���z�u����	***/
	variable_addr_cal();
	
	c32_no_printx = 0;
	if(argc==3 && strcmp(argv[2], "-ram")==0)
		c32_printf("	org		0x%X\n", RAM_TOP);
	else
		c32_printf("	org		0x%X\n", FLUSH_TOP);
	c32_printf("	j		startup\n");
	
	/***	c32_pass 3		***/
	//	�R�[�h�W�F�l���[�g	***/
	printf("[ PASS3 ]\n");
	c32_pass = PASS3;
	c32_no_printx = 0;
	memset(cv->src_buffer, 0, 10);
	c32_linebufp = cv->src_buffer;
	c32_output_buffer_flush();
	rewind(c32_src_fp);
	
	parser();
	
//	reg2();									// ����V���{���̐錾
	/***	�ϐ������l�Z�b�g�v���O�����̓f���o�� startup �̍쐬	***/
	startup();
	c32_symbol_out();
	c32_printf("	end		startup\n");
	
printf("RAM addr = %08X %d\n", c32_ram_addr, c32_ram_addr-RAM_BASE);
	
printf("buff �g�p��            %dbyte\n", c32_buff_ptr);
printf("�V���{���e�[�u���g�p�� %d�V���{��\n", c32_tbl_ptr_g);
printf("src_buffer �g�p��      %dbyte\n", c32_max_src_buffer);
printf("output_bffer �g�p��    %dbyte\n", c32_max_output_buffer);
	
	c32_output_buffer_flush();
	
	fclose(c32_asm_fp);
	fclose(c32_src_fp);
	return 0;
}
#else
/********************************************************************************/
/*		c32																		*/
/*		major update �\��														*/
/*			1. static �̃T�|�[�g												*/
/*			2. const  �̃T�|�[�g												*/
/********************************************************************************/
int c32(const char *fname)
{
	char *ptr;
	
	if(setjmp(c32_env)){
		SYS_FS_FileClose(c32_src_fp);
		SYS_FS_FileClose(c32_asm_fp);
		return 1;							// error: longjmp() �����s���ꂽ
	}
	printf("%s %s %s\n", VERSION, __DATE__, __TIME__);
	memset(cv, 0, sizeof(struct cv));
	c32_label_counter = 0;
//	c32_output_buffer_line_numb = 0;
	c32_tbl_ptr_g = 0;
	c32_buff_ptr = 0;
	c32_no_printx = 0;
	c32_no_label_out = 0;
	c32_s_numb = c32_max_s_numb = 0;
	c32_ram_addr = RAM_BASE;
	
	/***	registration	***/
	registration();

	/***	file open	***/
	if((c32_src_fp=SYS_FS_FileOpen(fname, SYS_FS_FILE_OPEN_READ))==SYS_FS_HANDLE_INVALID){
		ut_error_msg("src open");
		return 1;							// error
	}
	strcpy(cv->string_buf, fname);
	ptr = strrchr(cv->string_buf, '.');
	if(ptr){
		*ptr = '\0';
	}
	strcat(cv->string_buf, ".s");
	if((c32_asm_fp=SYS_FS_FileOpen(cv->string_buf, SYS_FS_FILE_OPEN_WRITE))==SYS_FS_HANDLE_INVALID){
		ut_error_msg("asm open");
		SYS_FS_FileClose(c32_src_fp);
		return 1;							// error
	}
	
	/***	c32_pass 1		***/
	//	�V���{���e�[�u���쐬�A�ϐ��T�C�Y�m��A�ϐ������l�� ptr->init �ɃZ�b�g����Aused_s_reg_numb �쐬
	//	�z�񏉊��l�͂��̏�œf���o�� ���̃��x���ԍ��� ptr->init �� �o�C�g���� ptr->size �ɃZ�b�g����
//	reg1();									// ����V���{���̐錾

	printf("[ PASS1 ]\n");
	c32_pass = PASS1;
	c32_no_printx = 1;
	memset(cv->src_buffer, 0, 10);
	c32_linebufp = cv->src_buffer;
	c32_output_buffer_flush();
	
	parser();
	
	/***	�ϐ��A�h���X/�I�t�Z�b�g �v�Z �P���ϐ��� low address �� ���̏�ɔz���z�u����	***/
	variable_addr_cal();
	
	c32_no_printx = 0;
	c32_printf("	org		0x%X\n", RAM_TOP);
	c32_printf("	j		startup\n");
	
	if(SYS_FS_FileSeek(c32_src_fp, 0, SYS_FS_SEEK_SET) < 0){
		ut_error_msg("src seek");
		SYS_FS_FileClose(c32_src_fp);
		SYS_FS_FileClose(c32_asm_fp);
		return 1;							// error
	}
	/***	c32_pass 3		***/
	//	�R�[�h�W�F�l���[�g	***/
	printf("[ PASS3 ]\n");
	c32_pass = PASS3;
	c32_no_printx = 0;
	memset(cv->src_buffer, 0, 10);
	c32_linebufp = cv->src_buffer;
	c32_output_buffer_flush();
	
	parser();
	
//	reg2();									// ����V���{���̐錾
	/***	�ϐ������l�Z�b�g�v���O�����̓f���o�� startup �̍쐬	***/
	startup();
	c32_symbol_out();
	c32_printf("	end		startup\n");
	printf("RAM addr=%08X\n", c32_ram_addr);
	c32_output_buffer_flush();
	
	SYS_FS_FileClose(c32_src_fp);
	SYS_FS_FileClose(c32_asm_fp);
	return 0;
}
#endif