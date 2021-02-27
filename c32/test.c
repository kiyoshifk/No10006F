#include "stdafx.h"
#include "banana-c32.h"
#include "function.h"

/********************************************************************************/
/*		c32_test																	*/
/********************************************************************************/
void c32_test(struct symtbl *func)
{
	struct expr_param expr_p1;

	expr_p1.func = func;
	sprintf(expr_p1.reg_ans, "t%d", c32_s_numb);
	c32_expr(&expr_p1);
	printf("%s mode=%d value=%d\n", c32_attr_to_string(expr_p1.attr), expr_p1.mode, expr_p1.value);
}
