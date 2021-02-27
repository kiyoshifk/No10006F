#include "stdafx.h"
#include "banana-c32.h"
#include "function.h"


#if 0
/********************************************************************************/
/*		constant_address_eq_reg													*/
/*		$t9 使用																*/
/********************************************************************************/
void constant_address_eq_reg(int attr, unsigned int constant, const char *reg)
{
	switch(c32_attr_to_byte(attr))
	{
		case 1:
			c32_printf("	li		$t9, 0x%x\n", constant);
			c32_printf("	sb		$%s, 0($t9)\n", reg);
			break;
		case 2:
			c32_printf("	li		$t9, 0x%x\n", constant);
			c32_printf("	sh		$%s, %d($t9)\n", reg);
			break;
		case 4:
			c32_printf("	li		$t9, 0x%x\n", constant);
			c32_printf("	sw		$%s, %d($t9)\n", reg);
			break;
		default:
			c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
			break;
	}
}
#endif
/********************************************************************************/
/*		mem_eq_reg														*/
/********************************************************************************/
void mem_eq_reg(int attr, const char *dst_reg, char *offset, const char *src_reg)
{
	switch(c32_attr_to_byte(attr))
	{
		case 1:
			c32_printf("	sb		$%s, %s($%s)\n", src_reg, offset, dst_reg);
			break;
		case 2:
			c32_printf("	sh		$%s, %s($%s)\n", src_reg, offset, dst_reg);
			break;
		case 4:
			c32_printf("	sw		$%s, %s($%s)\n", src_reg, offset, dst_reg);
			break;
		default:
			c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
			break;
	}
}
/********************************************************************************/
/*		reg_eq_constant_address													*/
/*		$t9 を使用する															*/
/********************************************************************************/
#if 0
void reg_eq_constant_address(int attr, const char *reg, int constant)
{
	switch(c32_attr_to_byte(attr))
	{
		case 1:
			if(attr & ATTR_UNSIGNED){
				c32_printf("	li		$t9, %d\n",	constant);
				c32_printf("	lbu		$%s, 0($t9)\n", reg);
			}
			else{
				c32_printf("	li		$t9, %d\n",	constant);
				c32_printf("	lb		$%s, 0($t9)\n", reg);
			}
			break;
		case 2:
			if(attr & ATTR_UNSIGNED){
				c32_printf("	li		$t9, %d\n",	constant);
				c32_printf("	lhu		$%s, 0($t9)\n", reg);
			}
			else{
				c32_printf("	li		$t9, %d\n",	constant);
				c32_printf("	lh		$%s, 0($t9)\n", reg);
			}
			break;
		case 4:
			c32_printf("	li		$t9, %d\n",	constant);
			c32_printf("	lw		$%s, 0($t9)\n", reg);
			break;
		default:
			c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
			break;
	}
}
#endif
/********************************************************************************/
/*		ret_eq_reg_adress														*/
/********************************************************************************/
void reg_eq_mem(int attr, const char *dst_reg, const char *src_reg, char *offset)
{
	switch(c32_attr_to_byte(attr))
	{
		case 1:
			if(attr & ATTR_UNSIGNED){
				c32_printf("	lbu		$%s, %s($%s)\n", dst_reg, offset, src_reg);
			}
			else{
				c32_printf("	lb		$%s, %s($%s)\n", dst_reg, offset, src_reg);
			}
			break;
		case 2:
			if(attr & ATTR_UNSIGNED){
				c32_printf("	lhu		$%s, %s($%s)\n", dst_reg, offset, src_reg);
			}
			else{
				c32_printf("	lh		$%s, %s($%s)\n", dst_reg, offset, src_reg);
			}
			break;
		case 4:
			c32_printf("	lw		$%s, %s($%s)\n", dst_reg, offset, src_reg);
			break;
		default:
			c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
			break;
	}
}
