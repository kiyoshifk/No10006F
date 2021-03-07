#include "stdafx.h"
#include "banana-c32.h"
#include "function.h"



/********************************************************************************/
/*		c32_expr_r_shift_eq														*/
/*		return: 求めた 値の attr												*/
/*		mode=0: 計算式属性														*/
/*		mode=1: 計算式アドレス属性、最終オブジェクト有り						*/
/*		mode=2: 定数															*/
/*		mode=6: ラベルアドレス属性、value=label_numb							*/
/*																				*/
/*		value : 定数の時、定数を返すエリア										*/
/********************************************************************************/
int c32_expr_r_shift_eq(struct expr_param *expr_p1, struct expr_param *expr_p2)
{
	if(expr_p1->attr & (ATTR_POINTER | ATTR_ARRAY)){
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	}
	if(expr_p2->attr & (ATTR_POINTER | ATTR_ARRAY))
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);

	reg_eq_mem(expr_p1->attr, "t0", expr_p1->reg_var, expr_p1->off);	// $t0: 第一項
	if(expr_p2->mode==MODE2){
		c32_printf("	srl		$t1, $t0, %d\n", expr_p2->value);
	}
	else{
		convert_mode0(expr_p2, c32_s_numb);
		c32_printf("	srlv	$t1, $t0, $%s\n", expr_p2->reg_var);
	}
	mem_eq_reg(expr_p1->attr, expr_p1->reg_var, expr_p1->off, "t1");
	expr_p1->off = "0";
	expr_p1->value = 0;

	return expr_p1->attr;
}
/********************************************************************************/
/*		c32_expr_l_shift_eq														*/
/*		return: 求めた 値の attr												*/
/*		mode=0: 計算式属性														*/
/*		mode=1: 計算式アドレス属性、最終オブジェクト有り						*/
/*		mode=2: 定数															*/
/*		mode=6: ラベルアドレス属性、value=label_numb							*/
/*																				*/
/*		value : 定数の時、定数を返すエリア										*/
/********************************************************************************/
int c32_expr_l_shift_eq(struct expr_param *expr_p1, struct expr_param *expr_p2)
{
	if(expr_p1->attr & (ATTR_POINTER | ATTR_ARRAY)){
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	}
	if(expr_p2->attr & (ATTR_POINTER | ATTR_ARRAY))
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);

	reg_eq_mem(expr_p1->attr, "t0", expr_p1->reg_var, expr_p1->off);	// $t0: 第一項
	if(expr_p2->mode==MODE2){
		c32_printf("	sll		$t1, $t0, %d\n", expr_p2->value);
	}
	else{
		convert_mode0(expr_p2, c32_s_numb);
		c32_printf("	sllv	$t1, $t0, $%s\n", expr_p2->reg_var);
	}
	mem_eq_reg(expr_p1->attr, expr_p1->reg_var, expr_p1->off, "t1");
	expr_p1->off = "0";
	expr_p1->value = 0;

	return expr_p1->attr;
}
/********************************************************************************/
/*		c32_expr_xor_eq															*/
/*		return: 求めた 値の attr												*/
/*		mode=0: 計算式属性														*/
/*		mode=1: 計算式アドレス属性、最終オブジェクト有り						*/
/*		mode=2: 定数															*/
/*		mode=6: ラベルアドレス属性、value=label_numb							*/
/*																				*/
/*		value : 定数の時、定数を返すエリア										*/
/********************************************************************************/
int c32_expr_xor_eq(struct expr_param *expr_p1, struct expr_param *expr_p2)
{
	if(expr_p1->attr & (ATTR_POINTER | ATTR_ARRAY)){
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	}
	if(expr_p2->attr & (ATTR_POINTER | ATTR_ARRAY))
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);

	reg_eq_mem(expr_p1->attr, "t0", expr_p1->reg_var, expr_p1->off);	// $t0: 第一項
	if(expr_p2->mode==MODE2 && (expr_p2->value & 0xffff0000)==0){
		c32_printf("	xori	$t1, $t0, %d\n", expr_p2->value);
	}
	else{
		convert_mode0(expr_p2, c32_s_numb);
		c32_printf("	xor		$t1, $t0, $%s\n", expr_p2->reg_var);
	}
	mem_eq_reg(expr_p1->attr, expr_p1->reg_var, expr_p1->off, "t1");

	return expr_p1->attr;
}
/********************************************************************************/
/*		c32_expr_or_eq															*/
/*		return: 求めた 値の attr												*/
/*		mode=0: 計算式属性														*/
/*		mode=1: 計算式アドレス属性、最終オブジェクト有り						*/
/*		mode=2: 定数															*/
/*		mode=6: ラベルアドレス属性、value=label_numb							*/
/*																				*/
/*		value : 定数の時、定数を返すエリア										*/
/********************************************************************************/
int c32_expr_or_eq(struct expr_param *expr_p1, struct expr_param *expr_p2)
{
	if(expr_p1->attr & (ATTR_POINTER | ATTR_ARRAY)){
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	}
	if(expr_p2->attr & (ATTR_POINTER | ATTR_ARRAY))
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);

	reg_eq_mem(expr_p1->attr, "t0", expr_p1->reg_var, expr_p1->off);	// $t0: 第一項
	if(expr_p2->mode==MODE2 && (expr_p2->value & 0xffff0000)==0){
		c32_printf("	ori		$t1, $t0, %d\n", expr_p2->value);
	}
	else{
		convert_mode0(expr_p2, c32_s_numb);
		c32_printf("	or		$t1, $t0, $%s\n", expr_p2->reg_var);
	}
	mem_eq_reg(expr_p1->attr, expr_p1->reg_var, expr_p1->off, "t1");

	return expr_p1->attr;
}
/********************************************************************************/
/*		c32_expr_and_eq															*/
/*		return: 求めた 値の attr												*/
/*		mode=0: 計算式属性														*/
/*		mode=1: 計算式アドレス属性、最終オブジェクト有り						*/
/*		mode=2: 定数															*/
/*		mode=6: ラベルアドレス属性、value=label_numb							*/
/*																				*/
/*		value : 定数の時、定数を返すエリア										*/
/********************************************************************************/
int c32_expr_and_eq(struct expr_param *expr_p1, struct expr_param *expr_p2)
{
	if(expr_p1->attr & (ATTR_POINTER | ATTR_ARRAY)){
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	}
	if(expr_p2->attr & (ATTR_POINTER | ATTR_ARRAY))
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);

	reg_eq_mem(expr_p1->attr, "t0", expr_p1->reg_var, expr_p1->off);	// $t0: 第一項
	if(expr_p2->mode==MODE2 && (expr_p2->value & 0xffff0000)==0){
		c32_printf("	andi	$t1, $t0, %d\n", expr_p2->value);
	}
	else{
		convert_mode0(expr_p2, c32_s_numb);
		c32_printf("	and		$t1, $t0, $%s\n", expr_p2->reg_var);
	}
	mem_eq_reg(expr_p1->attr, expr_p1->reg_var, expr_p1->off, "t1");

	return expr_p1->attr;
}
/********************************************************************************/
/*		c32_expr_percent_eq														*/
/*		return: 求めた 値の attr												*/
/*		mode=0: 計算式属性														*/
/*		mode=1: 計算式アドレス属性、最終オブジェクト有り						*/
/*		mode=2: 定数															*/
/*		mode=6: ラベルアドレス属性、value=label_numb							*/
/*																				*/
/*		value : 定数の時、定数を返すエリア										*/
/********************************************************************************/
int c32_expr_percent_eq(struct expr_param *expr_p1, struct expr_param *expr_p2)
{
	if(expr_p1->attr & (ATTR_POINTER | ATTR_ARRAY)){
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	}
	if(expr_p2->attr & (ATTR_POINTER | ATTR_ARRAY))
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);

	convert_mode0(expr_p2, c32_s_numb);
	reg_eq_mem(expr_p1->attr, "t0", expr_p1->reg_var, expr_p1->off);		// $t0: 第一項
	if((expr_p1->attr | expr_p2->attr) & ATTR_UNSIGNED){
		c32_printf("	divu	$t0, $%s\n", expr_p2->reg_var);
		c32_printf("	mfhi	$t1\n");
	}
	else{
		c32_printf("	div		$t0, $%s\n", expr_p2->reg_var);
		c32_printf("	mfhi	$t1\n");
	}
	mem_eq_reg(expr_p1->attr, expr_p1->reg_var, expr_p1->off, "t1");

	return expr_p1->attr;
}
/********************************************************************************/
/*		c32_expr_slush_eq														*/
/*		return: 求めた 値の attr												*/
/*		mode=0: 計算式属性														*/
/*		mode=1: 計算式アドレス属性、最終オブジェクト有り						*/
/*		mode=2: 定数															*/
/*		mode=6: ラベルアドレス属性、value=label_numb							*/
/*																				*/
/*		value : 定数の時、定数を返すエリア										*/
/********************************************************************************/
int c32_expr_slush_eq(struct expr_param *expr_p1, struct expr_param *expr_p2)
{
	if(expr_p1->attr & (ATTR_POINTER | ATTR_ARRAY)){
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	}
	if(expr_p2->attr & (ATTR_POINTER | ATTR_ARRAY))
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);

	convert_mode0(expr_p2, c32_s_numb);
	reg_eq_mem(expr_p1->attr, "t0", expr_p1->reg_var, expr_p1->off);		// $t0: 第一項
	if((expr_p1->attr | expr_p2->attr) & ATTR_UNSIGNED){
		c32_printf("	divu	$t0, $%s\n", expr_p2->reg_var);
		c32_printf("	mflo	$t1\n");
	}
	else{
		c32_printf("	div		$t0, $%s\n", expr_p2->reg_var);
		c32_printf("	mflo	$t1\n");
	}
	mem_eq_reg(expr_p1->attr, expr_p1->reg_var, expr_p1->off, "t1");

	return expr_p1->attr;
}
/********************************************************************************/
/*		c32_expr_asterisk_eq													*/
/*		return: 求めた 値の attr												*/
/*		mode=0: 計算式属性														*/
/*		mode=1: 計算式アドレス属性、最終オブジェクト有り						*/
/*		mode=2: 定数															*/
/*		mode=6: ラベルアドレス属性、value=label_numb							*/
/*																				*/
/*		value : 定数の時、定数を返すエリア										*/
/********************************************************************************/
int c32_expr_asterisk_eq(struct expr_param *expr_p1, struct expr_param *expr_p2)
{
	if(expr_p1->attr & (ATTR_POINTER | ATTR_ARRAY)){
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	}
	if(expr_p2->attr & (ATTR_POINTER | ATTR_ARRAY))
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);

	convert_mode0(expr_p2, c32_s_numb);
	reg_eq_mem(expr_p1->attr, "t0", expr_p1->reg_var, expr_p1->off);		// $t0: 第一項
	if((expr_p1->attr | expr_p2->attr) & ATTR_UNSIGNED){
		c32_printf("	multu	$t0, $%s\n", expr_p2->reg_var);
		c32_printf("	mflo	$t1\n");
	}
	else{
		c32_printf("	mult	$t0, $%s\n", expr_p2->reg_var);
		c32_printf("	mflo	$t1\n");
	}
	mem_eq_reg(expr_p1->attr, expr_p1->reg_var, expr_p1->off, "t1");

	return expr_p1->attr;
}
/********************************************************************************/
/*		c32_expr_minus_eq														*/
/*		return: 求めた 値の attr												*/
/*		mode=0: 計算式属性														*/
/*		mode=1: 計算式アドレス属性、最終オブジェクト有り						*/
/*		mode=2: 定数															*/
/*		mode=6: ラベルアドレス属性、value=label_numb							*/
/*																				*/
/*		value : 定数の時、定数を返すエリア										*/
/********************************************************************************/
int c32_expr_minus_eq(struct expr_param *expr_p1, struct expr_param *expr_p2)
{
	int shift, tmp;

	if(expr_p2->attr & (ATTR_POINTER | ATTR_ARRAY))
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);

	reg_eq_mem(expr_p1->attr, "t0", expr_p1->reg_var, expr_p1->off);
	if(expr_p1->attr & (ATTR_POINTER | ATTR_ARRAY)){
		shift = c32_attr_to_shift_bit(expr_p1->attr);
		tmp = expr_p2->value * shift & 0xffff8000;
		if(expr_p2->mode==MODE2 && (tmp==0 || tmp==0xffff8000)){
			c32_printf("	addiu	$t0, $t0, %d\n", 0-expr_p2->value * shift);
		}
		else{
			convert_mode0(expr_p2, c32_s_numb);
			if(shift){
				c32_printf("	sll		$%s, $%s, %d\n", expr_p2->reg_var, expr_p2->reg_var, shift);
			}
			c32_printf("	subu	$t0, $t0, $%s\n", expr_p2->reg_var);
		}
	}
	else{
		tmp = expr_p2->value & 0xffff8000;
		if(expr_p2->mode==MODE2 && (tmp==0 || tmp==0xffff8000)){
			c32_printf("	addiu	$t0, $t0, %d\n", 0-expr_p2->value);
		}
		else{
			convert_mode0(expr_p2, c32_s_numb);
			c32_printf("	subu	$t0, $t0, $%s\n", expr_p2->reg_var);
		}
	}
	mem_eq_reg(expr_p1->attr, expr_p1->reg_var, expr_p1->off, "t0");

	return expr_p1->attr;
}
/********************************************************************************/
/*		c32_expr_plus_eq														*/
/*		return: 求めた 値の attr												*/
/*		mode=0: 計算式属性														*/
/*		mode=1: 計算式アドレス属性、最終オブジェクト有り						*/
/*		mode=2: 定数															*/
/*		mode=6: ラベルアドレス属性、value=label_numb							*/
/*																				*/
/*		value : 定数の時、定数を返すエリア										*/
/********************************************************************************/
int c32_expr_plus_eq(struct expr_param *expr_p1, struct expr_param *expr_p2)
{
	int shift, tmp;
	
	if(expr_p2->attr & (ATTR_POINTER | ATTR_ARRAY))
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);

	reg_eq_mem(expr_p1->attr, "t0", expr_p1->reg_var, expr_p1->off);
	if(expr_p1->attr & (ATTR_POINTER | ATTR_ARRAY)){
		shift = c32_attr_to_shift_bit(expr_p1->attr);
		tmp = expr_p2->value * shift & 0xffff8000;
		if(expr_p2->mode==MODE2 && (tmp==0 || tmp==0xffff8000)){
			c32_printf("	addiu	$t0, $t0, %d\n", expr_p2->value * shift);
		}
		else{
			convert_mode0(expr_p2, c32_s_numb);
			if(shift){
				c32_printf("	sll		$%s, $%s, %d\n", expr_p2->reg_var, expr_p2->reg_var, shift);
			}
			c32_printf("	addu	$t0, $t0, $%s\n", expr_p2->reg_var);
		}
	}
	else{
		tmp = expr_p2->value & 0xffff8000;
		if(expr_p2->mode==MODE2 && (tmp==0 || tmp==0xffff8000)){
			c32_printf("	addiu	$t0, $t0, %d\n", expr_p2->value);
		}
		else{
			convert_mode0(expr_p2, c32_s_numb);
			c32_printf("	addu	$t0, $t0, $%s\n", expr_p2->reg_var);
		}
	}
	mem_eq_reg(expr_p1->attr, expr_p1->reg_var, expr_p1->off, "t0");

	return expr_p1->attr;
}
/********************************************************************************/
/*		c32_expr_eq																*/
/*		return: 求めた 値の attr												*/
/*		mode=0: 計算式属性														*/
/*		mode=1: 計算式アドレス属性、最終オブジェクト有り						*/
/*		mode=2: 定数															*/
/*		mode=6: ラベルアドレス属性、value=label_numb							*/
/*																				*/
/*		value : 定数の時、定数を返すエリア										*/
/********************************************************************************/
int c32_expr_eq(struct expr_param *expr_p1, struct expr_param *expr_p2)
{
	convert_mode0(expr_p2, c32_s_numb);
	mem_eq_reg(expr_p1->attr, expr_p1->reg_var, expr_p1->off, expr_p2->reg_var);
	return expr_p1->attr;
}
