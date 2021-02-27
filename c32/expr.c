#include "stdafx.h"
#include "banana-c32.h"
#include "function.h"


void c32_factor1(struct expr_param *expr_p1);


/********************************************************************************/
/*		c32_expr_function_push													*/
/********************************************************************************/
void c32_expr_function_push()
{
	int i, size;
	
	size = 4*(c32_s_numb-2);
	if(size <= 0)
		return;
	c32_printf("	addiu	$sp, $sp, %d\n", -size);
	for(i=2; i<c32_s_numb; i++){
		c32_printf("	sw		$t%d, %d($sp)\n", i, (i-2)*4);
	}
}

void c32_expr_function_pop()
{
	int i, size;
	
	size = 4*(c32_s_numb-2);
	if(size <= 0)
		return;
	for(i=2; i<c32_s_numb; i++){
		c32_printf("	lw		$t%d, %d($sp)\n", i, (i-2)*4);
	}
	c32_printf("	addiu	$sp, $sp, %d\n", size);
}
/********************************************************************************/
/*		expr_function_call														*/
/*		int sub(int a, int b, int c, int d, int e, int f){ int g;}				*/
/*		stack:	f																*/
/*				e																*/
/*				d $a3 を入れる													*/
/*				c $a2 を入れる													*/
/*				b $a1 を入れる													*/
/*				a $a0 を入れる													*/
/*			......................... call 時のスタック							*/
/*				$s-		max_s_reg_numb 個エリア有り								*/
/*				$fp																*/
/*				$ra																*/
/*				g																*/
/********************************************************************************/
static void expr_function_call(struct expr_param *expr_p1, struct symtbl *ptr)
{
	int i, type, param_numb;
	char reg[4];
//	struct expr_param expr_p2;
	
//	s_numb_save = c32_s_numb;
	strcpy(reg, expr_p1->reg_ans);
	c32_token_process(0, &type);				// '(' を確認する
	if(type != TYPE_L_KAKKO)
		c32_error_message(E_L_KAKKO_MISSING, __LINE__, __FILE__);
	
	c32_expr_function_push();
	param_numb = ptr->func_tbl->param_numb;
	if(param_numb)
		c32_printf("	addiu	$sp, $sp, %d\n", -param_numb*4);
	if(param_numb==0){
		c32_token_process(0, &type);
		if(type != TYPE_R_KAKKO)
			c32_error_message(E_R_KAKKO_MISSING, __LINE__, __FILE__);
	}
	else{
		for(i=0; i<param_numb; i++){
			if(i<4)
				sprintf(expr_p1->reg_ans, "a%d", i);
			else
				sprintf(expr_p1->reg_ans, "t%d", c32_s_numb);
			c32_factor1(expr_p1);				// c32_expr() では ',' 演算子処理が入ってしまう
											// 関数の引数の計算
			convert_mode0(expr_p1);
			c32_printf("	sw		$%s, %d($sp)\n", expr_p1->reg_ans, i*4);
			c32_token_process(0, &type);			// ',' 又は ')'
			if(type==TYPE_KANMA)
				continue;
			else if(type==TYPE_R_KAKKO)
				break;
			else
				c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
		}
		if(i != param_numb-1)
			c32_error_message(E_PARAM_NUMBER_ERROR, __LINE__, __FILE__);
//		for(i=0; i<(param_numb<4?param_numb:4); i++){
//			c32_printf("	lw		$a%d, %d($sp)\n", i, i*4);
//		}
		strcpy(expr_p1->reg_ans, reg);
	}
	c32_printf("	jal		%s\n", ptr->symbuf);
	c32_printf("	move	$%s, $v0\n", expr_p1->reg_ans);
	if(param_numb)
		c32_printf("	addiu	$sp, $sp, %d\n", param_numb*4);
	c32_expr_function_pop();
}

static void expr_function_call_pass1(struct expr_param *expr_p1)
{
	int type;
	struct src_ptr src_ptr;
	struct expr_param expr_p2;
	
	c32_token_process(0, &type);				// '(' を確認する
	if(type != TYPE_L_KAKKO)
		c32_error_message(E_L_KAKKO_MISSING, __LINE__, __FILE__);
	c32_src_ptr_save(&src_ptr);
	c32_token_process(0, &type);				// ')' を調べる
	if(type != TYPE_R_KAKKO){
		c32_src_ptr_restore(&src_ptr);
		expr_p2 = *expr_p1;
		for(;;){
			sprintf(expr_p2.reg_ans, "t%d", c32_s_numb);
			c32_factor1(&expr_p2);				// 関数の引数の計算
			convert_mode0(&expr_p2);
			c32_token_process(0, &type);
			if(type==TYPE_R_KAKKO)
				break;
			if(type != TYPE_KANMA)
				c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
		}
	}
}
/********************************************************************************/
/*		expr_function_call_pointer												*/
/*		関数ポインタによる関数呼び出し											*/
/********************************************************************************/
static void expr_function_call_pointer(struct expr_param *expr_p1, struct symtbl *ptr)
{
	int i, type, param_numb;
	struct src_ptr src_ptr;
	char reg[4];

	strcpy(reg, expr_p1->reg_ans);
	c32_token_process(0, &type);				// '(' を確認する
	if(type != TYPE_L_KAKKO)
		c32_error_message(E_L_KAKKO_MISSING, __LINE__, __FILE__);
	c32_src_ptr_save(&src_ptr);
	c32_token_process(0, &type);				// ')' を調べる
	if(type==TYPE_R_KAKKO){
		param_numb = 0;
	}
	else{
		c32_src_ptr_restore(&src_ptr);
		for(i=0; ; i++){
			sprintf(expr_p1->reg_ans, "t%d", c32_s_numb);
			c32_factor1(expr_p1);
			c32_token_process(0, &type);
			if(type==TYPE_KANMA)
				continue;
			else if(type==TYPE_R_KAKKO)
				break;
			else
				c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
		}
		param_numb = i+1;
	}
	
	c32_src_ptr_restore(&src_ptr);
	c32_expr_function_push();
	if(param_numb)
		c32_printf("	addiu	$sp, $sp, %d\n", -(param_numb*4>16?param_numb*4:16));
	
	if(param_numb==0){
		c32_token_process(0, &type);
		if(type != TYPE_R_KAKKO)
			c32_error_message(E_R_KAKKO_MISSING, __LINE__, __FILE__);
	}
	else{
		for(i=0; i<param_numb; i++){
			if(i<4)
				sprintf(expr_p1->reg_ans, "a%d", i);
			else
				sprintf(expr_p1->reg_ans, "t%d", c32_s_numb);
			c32_factor1(expr_p1);		// c32_expr() では ',' 演算子処理が入ってしまう
			convert_mode0(expr_p1);
			c32_printf("	sw		$%s, %d($sp)\n", expr_p1->reg_ans, i*4);
			c32_token_process(0, &type);			// ',' 又は ')'
			if(type==TYPE_KANMA)
				continue;
			else if(type==TYPE_R_KAKKO)
				break;
			else
				c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
		}
		if(i != param_numb-1)
			c32_error_message(E_PARAM_NUMBER_ERROR, __LINE__, __FILE__);
		for(i=0; i<(param_numb<4?param_numb:4); i++){
			c32_printf("	lw		$a%d, %d($sp)\n", i, i*4);
		}
	}
	if(ptr->flag1 & (FLAG_01 | FLAG_02))	// auto / param 変数
		c32_printf("	lw		$t1, %s($fp)\n", ptr->label);
	else if(ptr->flag1 & (FLAG_04 | FLAG_08)){	// global 変数
		c32_printf("	lw		$t1, %s($s7)\n", ptr->label);
	}
	strcpy(expr_p1->reg_ans, reg);
	c32_printf("	lw		$t1, 0($t1)\n");
	c32_printf("	jalr	$ra, $t1\n");
	c32_printf("	move	$%s, $v0\n", expr_p1->reg_ans);
	if(param_numb)
		c32_printf("	addiu	$sp, $sp, %d\n", param_numb*4>16?param_numb*4:16);
	c32_expr_function_pop();
}
/********************************************************************************/
/*		factor																	*/
/*		return: 求めた 値の attr												*/
/*		mode=0: 計算式属性														*/
/*		mode=1: 計算式アドレス属性、最終オブジェクト有り						*/
/*		mode=2: 定数															*/
/*	   (mode=3: 定数アドレス属性、最終オブジェクトのアドレス)					*/
/*		mode=4; $fp offset アドレス属性											*/
/*		mode=5: $s7 offset アドレス属性											*/
/*		mode=6: ラベルアドレス属性、value=label_numb							*/
/*		mode=7: レジスタ属性、(char*)&value   "s0" 等が入る						*/
/*																				*/
/*		value : 定数の時、定数を返すエリア										*/
/*		func が 0 の場合は global table のみ検索する							*/
/********************************************************************************/
static void factor(struct expr_param *expr_p1)
{
	int i, type;
	struct symtbl *ptr;
	struct src_ptr src_ptr;
	
//	c32_printf(";---------- factor ----------\n");
	expr_p1->ptr = 0;
	expr_p1->attr = 0;
	expr_p1->mode = 0;
	expr_p1->value = 0;
	memset(expr_p1->xyz_size, 0, sizeof(expr_p1->xyz_size));
	expr_p1->xyz_dim = 0;

	ptr = c32_token_process(expr_p1->func, &type);
	if(type==TYPE_DIGIT){					// 定数 ====================
		expr_p1->mode = MODE2;				// 定数属性
		expr_p1->value = *((uint*)cv->string_buf);
		expr_p1->attr = c32_make_attr_from_constant(*((uint*)cv->string_buf));
		return;
	}
	else if(type==TYPE_STRING){				// 文字列 ====================
		int v_numb = c32_label_counter;
		int p_numb = c32_label_counter+1;
		c32_label_counter += 2;
		if(c32_no_label_out==0){
			c32_printf("	j		L%d\n", p_numb);
			c32_printf("L%d\n", v_numb);
			for(i=0; i<(int)strlen(cv->string_buf)+1; i++){
				c32_printf("	db		0x%x\n", (uchar)cv->string_buf[i]);
			}
			c32_printf("L%d\n", p_numb);
		}
//		c32_printf("	la		$%s, L%d\n", expr_p1->reg_ans, v_numb);
		expr_p1->value = v_numb;
		expr_p1->mode = MODE6;				// ラベル
		expr_p1->attr = ATTR_CHAR | ATTR_ARRAY;
		return;
	}
	else if(type==TYPE_L_KAKKO){			// '('  ====================	(c32_expr) のみ
		c32_expr(expr_p1);
		
		c32_token_process(0, &type);
		if(type==TYPE_R_KAKKO){				// ')'
			return;
		}
		c32_error_message(E_R_KAKKO_MISSING, __LINE__, __FILE__);
	}
	else if(type==TYPE_SYMBOL){				// symbol  ====================   関数/変数
		expr_p1->ptr = ptr;
		if(ptr==0){							// 関数は PASS1 で undef になることが有る
			if(c32_pass==PASS3)
				c32_error_message(E_UNDEFINED_SYMBOL, __LINE__, __FILE__);
			c32_src_ptr_save(&src_ptr);
			c32_token_process(0, &type);
			if(type != TYPE_L_KAKKO)
				c32_error_message(E_UNDEFINED_SYMBOL, __LINE__, __FILE__);
			c32_src_ptr_restore(&src_ptr);
			expr_function_call_pass1(expr_p1);
			expr_p1->mode = MODE0;			// 計算式属性, acc
			expr_p1->attr = ATTR_INT;
			return;
		}
		if(ptr->attr & ATTR_FUNC){			// 関数だった  --------------------
			c32_printf(";---------- call %s ----------\n", ptr->symbuf);
			if(c32_pass==PASS1){
				expr_function_call_pass1(expr_p1);
				expr_p1->mode = MODE0;		// 計算式属性, acc
				expr_p1->attr = ATTR_INT;
				return;
			}
			expr_function_call(expr_p1, ptr);	// 関数呼び出し
			expr_p1->mode = MODE0;			// 計算式属性, acc
			expr_p1->attr = ptr->attr;
			return;
		}
		else if(ptr->attr & ATTR_ARRAY){	// array  --------------------
			expr_p1->xyz_dim = ptr->xyz_dim;
			memcpy(expr_p1->xyz_size, ptr->xyz_size, sizeof(expr_p1->xyz_size));
			expr_p1->off = ptr->label;
			if(ptr->flag1 & FLAG_04){		// global
				c32_printf("	la		$%s, %s\n", expr_p1->reg_ans, expr_p1->off);
				expr_p1->mode = MODE0;		// 計算式属性
			}
			else{							// auto/param
				strcpy(expr_p1->reg_var, "fp");
				expr_p1->mode = MODE1;		// レジスタ相対
			}
//			expr_p1->value = ptr->value;
			expr_p1->attr = ptr->attr;
			return;
		}
		else{								// 単純変数  --------------------
			expr_p1->xyz_dim = ptr->xyz_dim;
			memcpy(expr_p1->xyz_size, ptr->xyz_size, sizeof(expr_p1->xyz_size));
			c32_src_ptr_save(&src_ptr);
			c32_token_process(0, &type);
			c32_src_ptr_restore(&src_ptr);
			if(type==TYPE_L_KAKKO){			// 関数ポインタ
				expr_function_call_pointer(expr_p1, ptr);
				expr_p1->mode = MODE0;
				expr_p1->attr = ptr->attr;
				return;
			}
//			expr_p1->value = ptr->value;
			if(ptr->flag1 & FLAG_04){		// global
				strcpy(expr_p1->reg_var, "s7");
				expr_p1->mode = MODE1;		// $s7 レジスタ相対
			}
			else{							// auto/param
//				if(ptr->reg[0]){
//					expr_p1->value = 0;
//					strcpy(expr_p1->reg_var, ptr->reg);
//					expr_p1->mode = MODE0;	// レジスタ
//				}
//				else{
					strcpy(expr_p1->reg_var, "fp");
					expr_p1->mode = MODE1;	// $fp レジスタ相対
//				}
			}
			expr_p1->off = ptr->label;
			expr_p1->attr = ptr->attr;
			return;
		}
	}
	else{
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	}
}
/********************************************************************************/
/*		factor15																*/
/*		return: 求めた 値の attr												*/
/*		mode=0: 計算式属性														*/
/*		mode=1: 計算式アドレス属性、最終オブジェクト有り						*/
/*		mode=2: 定数															*/
/*	   (mode=3: 定数アドレス属性、最終オブジェクトのアドレス)					*/
/*		mode=4; $fp offset アドレス属性											*/
/*		mode=5: $s7 offset アドレス属性											*/
/*		mode=6: ラベルアドレス属性、value=label_numb							*/
/*		mode=7: レジスタ属性、(char*)&value   "s0" 等が入る						*/
/*																				*/
/*		value : 定数の時、定数を返すエリア										*/
/*		func が 0 の場合は global table のみ検索する							*/
/********************************************************************************/
static void factor15(struct expr_param *expr_p1)
{
	return factor(expr_p1);
}
/********************************************************************************/
/*		factor_array_param														*/
/*		[][][] 計算: 最初の [] の処理は終わっている→c32_s_numb					*/
/*		配列名: expr_p1,s_numb_save,  第一引数: c32_s_numb→s_numb_save			*/
/*		配列アドレス：s_numb_save, offset:										*/
/********************************************************************************/
static void factor_array_param(struct expr_param *expr_p1)
{
	struct expr_param expr_p2;
	struct src_ptr src_ptr;
	int i, type, dim, s_numb_save;
	
	s_numb_save = c32_s_numb;
	dim = expr_p1->xyz_dim;
	--expr_p1->xyz_dim;
	expr_p1->attr -= ATTR_ARRAY1;
	c32_src_ptr_save(&src_ptr);
	c32_token_process(0, &type);				// '[' をテストする
	c32_src_ptr_restore(&src_ptr);
	if(type != TYPE_L_KAKUKAKKO){
		if(expr_p1->xyz_dim >= 2)
			c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
		if(expr_p1->xyz_dim==1){
			c32_printf("	li		$t0, %d\n", expr_p1->xyz_size[dim-1]);
			c32_printf("	multu	$t%d, $t0\n", c32_s_numb);
			c32_printf("	mflo	$t%d\n", c32_s_numb);
		}
		return;
	}
	s_numb_save = c32_s_numb++;
	if(c32_s_numb > c32_max_s_numb)
		c32_max_s_numb = c32_s_numb;
	for(i=1; i<dim; i++){
		c32_src_ptr_save(&src_ptr);
		c32_token_process(0, &type);			// '[' をテストする
		if(type != TYPE_L_KAKUKAKKO){
			c32_src_ptr_restore(&src_ptr);
			break;
		}
		if(((expr_p1->attr -= ATTR_ARRAY1) & ATTR_ARRAY)==ATTR_ARRAY)
			c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
		--expr_p1->xyz_dim;
		//	dim=3: xyz_size[i] を s_numb_save に掛けてから c32_expr() を加算する
		//  dim=2: xyz_size[i] を s_numb_save に掛けてから c32_expr() を加算する
		//  dim=1: ここには来ない
		c32_printf("	li		$t0, %d\n", expr_p1->xyz_size[i]);
		c32_printf("	multu	$t%d, $t0\n", s_numb_save);
		c32_printf("	mflo	$t%d\n", s_numb_save);
		
		expr_p2 = *expr_p1;
		sprintf(expr_p2.reg_ans, "t%d", c32_s_numb);
		c32_expr(&expr_p2);							// 引数を求める
		convert_mode0(&expr_p2);
		c32_printf("	addu	$t%d, $t%d, $t%d\n", s_numb_save, s_numb_save, c32_s_numb);
		
		c32_token_process(0, &type);			// ']' を確認する
		if(type != TYPE_R_KAKUKAKKO)
			c32_error_message(E_R_KAKUKAKKO_MISSING, __LINE__, __FILE__);
	}
	if(expr_p1->xyz_dim >= 2)
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	//	dim==1 なら xyz_size[i] を掛ける
	if(expr_p1->xyz_dim==1){
		c32_printf("	li		$t0, %d\n", expr_p1->xyz_size[dim-1]);
		c32_printf("	multu	$t%d, $t0\n", s_numb_save);
		c32_printf("	mflo	$t%d\n", s_numb_save);
	}
	
	c32_s_numb = s_numb_save;
}
/********************************************************************************/
/*		factor_array_element													*/
/*		配列要素を求める														*/
/*		expr_p1 と s_numb_save : 配列開始アドレス								*/
/*		c32_s_numb             : offset											*/
/********************************************************************************/
static void factor_array_element(struct expr_param *expr_p1, int s_numb_save, int c32_s_numb)
{
	
	switch(c32_attr_to_byte(expr_p1->attr)){
	case 1:
		break;
	case 2:
		c32_printf("	sll		$t%d, $t%d, 1\n", c32_s_numb, c32_s_numb);
		break;
	case 4:
		c32_printf("	sll		$t%d, $t%d, 2\n", c32_s_numb, c32_s_numb);
		break;
	default:
		c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
	}
	
	switch(expr_p1->mode){
	case MODE0:							// レジスタ属性    c32_s_numb
		if(expr_p1->attr & ATTR_ARRAY_P){		// 配列へのポインタならば（通常は関数引数）
			c32_printf("	lw		$%s, 0($%s)\n", expr_p1->reg_ans, expr_p1->reg_ans);
		}
		c32_printf("	addu	$%s, $%s, $t%d\n", expr_p1->reg_ans, expr_p1->reg_ans, c32_s_numb);
		strcpy(expr_p1->reg_var, expr_p1->reg_ans);
		expr_p1->off = "0";
		expr_p1->mode = MODE1;					// アドレス相対
		break;
	case MODE1:								// レジスタ相対  c32_s_numb
		c32_printf("	addiu	$%s, $%s, %s\n", expr_p1->reg_ans, expr_p1->reg_var, expr_p1->off);
		if(expr_p1->attr & ATTR_ARRAY_P){		// 配列へのポインタならば（通常は関数引数）
			c32_printf("	lw		$%s, 0($%s)\n", expr_p1->reg_ans, expr_p1->reg_ans);
		}
		c32_printf("	addu	$%s, $%s, $t%d\n", expr_p1->reg_ans, expr_p1->reg_ans, c32_s_numb);
		strcpy(expr_p1->reg_var, expr_p1->reg_ans);
		expr_p1->off = "0";
		expr_p1->mode = MODE1;					// 計算式アドレス属性
		break;
	case MODE2:							// 定数属性    
		c32_printf("	la		$%s, %d\n", expr_p1->reg_ans, expr_p1->value);
next_mode2:;
		if(expr_p1->attr & ATTR_ARRAY_P){		// 配列へのポインタならば（通常は関数引数）
			c32_printf("	lw		$%s, 0($%s)\n", expr_p1->reg_ans, expr_p1->reg_ans);
		}
		c32_printf("	addu	$%s, $%s, $t%d\n", expr_p1->reg_ans, expr_p1->reg_ans, c32_s_numb);
		strcpy(expr_p1->reg_var, expr_p1->reg_ans);
		expr_p1->off = "0";
		expr_p1->mode = MODE1;					// 計算式アドレス属性
		break;
	case MODE6:									// ラベル
		c32_printf("	la		$%s, L%d\n", expr_p1->reg_ans, expr_p1->value);
		goto next_mode2;
	default:
		c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
	}
}
/********************************************************************************/
/*		factor_pointer_element													*/
/********************************************************************************/
static void factor_pointer_element(struct expr_param *expr_p1, int s_numb_save, int c32_s_numb)
{
	switch(c32_attr_to_byte(expr_p1->attr)){
	case 1:
		break;
	case 2:
		c32_printf("	sll		$t%d, $t%d, %d\n", c32_s_numb, c32_s_numb, 1);
		break;
	case 4:
		c32_printf("	sll		$t%d, $t%d, %d\n", c32_s_numb, c32_s_numb, 2);
		break;
	default:
		c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
	}
	
	switch(expr_p1->mode){
	case MODE0:							// レジスタ属性    c32_s_numb
		c32_printf("	addu	$%s, $%s, $t%d\n", expr_p1->reg_ans, expr_p1->reg_ans, c32_s_numb);
		strcpy(expr_p1->reg_var, expr_p1->reg_ans);
		expr_p1->off = "0";
		break;
	case MODE1:							// レジスタ相対  c32_s_numb
		reg_eq_mem(ATTR_INT, expr_p1->reg_ans, expr_p1->reg_var, expr_p1->off);
		c32_printf("	addu	$%s, $%s, $t%d\n", expr_p1->reg_ans, expr_p1->reg_ans, c32_s_numb);
		strcpy(expr_p1->reg_var, expr_p1->reg_ans);
		expr_p1->off = "0";
		break;
	case MODE2:							// 定数属性
		c32_printf("	la		$%s, %d\n", expr_p1->reg_ans, expr_p1->value);
next_mode2:;
		c32_printf("	addu	$%s, $%s, $t%d\n", expr_p1->reg_ans, expr_p1->reg_ans, c32_s_numb);
		strcpy(expr_p1->reg_var, expr_p1->reg_ans);
		expr_p1->off = "0";
		break;
	case MODE6:							// ラベル
		c32_printf("	la		$%s, L%d\n", expr_p1->reg_ans, expr_p1->value);
		goto next_mode2;
	default:
		c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
	}

	expr_p1->mode = MODE1;					// 計算式アドレス属性
}
/********************************************************************************/
/*		factor14																*/
/*		return: 求めた 値の attr												*/
/*		mode=0: 計算式属性														*/
/*		mode=1: 計算式アドレス属性、最終オブジェクト有り						*/
/*		mode=2: 定数															*/
/*	   (mode=3: 定数アドレス属性、最終オブジェクトのアドレス)					*/
/*		mode=4; $fp offset アドレス属性											*/
/*		mode=5: $s7 offset アドレス属性											*/
/*		mode=6: ラベルアドレス属性、value=label_numb							*/
/*		mode=7: レジスタ属性、(char*)&value   "s0" 等が入る						*/
/*																				*/
/*		value : 定数の時、定数を返すエリア										*/
/*		func が 0 の場合は global table のみ検索する							*/
/*		factor15 []																*/
/********************************************************************************/
static void factor14(struct expr_param *expr_p1)
{
//	return factor15(func, mode, value);

	int type, s_numb_save;
	struct src_ptr src_ptr;
	struct expr_param expr_p2;
	
	s_numb_save = c32_s_numb;
	factor15(expr_p1);
	c32_src_ptr_save(&src_ptr);

	c32_token_process(0, &type);				// '[' をテストする
	if(type != TYPE_L_KAKUKAKKO){
		c32_src_ptr_restore(&src_ptr);
		return;
	}
	/***	'[' だった		***/
	s_numb_save = c32_s_numb++;
	if(c32_s_numb > c32_max_s_numb)
		c32_max_s_numb = c32_s_numb;
	
	expr_p2 = *expr_p1;
	sprintf(expr_p2.reg_ans, "t%d", c32_s_numb);
	c32_expr(&expr_p2);							// 配列の第一引数
	convert_mode0(&expr_p2);
	
	c32_token_process(0, &type);				// ']' を確認する
	if(type != TYPE_R_KAKUKAKKO)
		c32_error_message(E_R_KAKUKAKKO_MISSING, __LINE__, __FILE__);

	if(expr_p2.attr & (ATTR_POINTER | ATTR_ARRAY))
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	
	if(expr_p1->attr & ATTR_ARRAY){
		factor_array_param(expr_p1);		// 配列名: expr_p1,s_numb_save,  offset を c32_s_numb に入れる
		factor_array_element(expr_p1, s_numb_save, c32_s_numb);
	}
	else if(expr_p1->attr & ATTR_POINTER){
		expr_p1->attr -= ATTR_POINTER1;
		factor_pointer_element(expr_p1, s_numb_save, c32_s_numb);
	}
	else{
		c32_error_message(E_NEED_POINTER, __LINE__, __FILE__);
	}
	
	c32_s_numb = s_numb_save;
}
/********************************************************************************/
/*		factor13																*/
/*		return: 求めた 値の attr												*/
/*		mode=0: 計算式属性														*/
/*		mode=1: 計算式アドレス属性、最終オブジェクト有り						*/
/*		mode=2: 定数															*/
/*	   (mode=3: 定数アドレス属性、最終オブジェクトのアドレス)					*/
/*		mode=4; $fp offset アドレス属性											*/
/*		mode=5: $s7 offset アドレス属性											*/
/*		mode=6: ラベルアドレス属性、value=label_numb							*/
/*		mode=7: レジスタ属性、(char*)&value   "s0" 等が入る						*/
/*																				*/
/*		value : 定数の時、定数を返すエリア										*/
/*		func が 0 の場合は global table のみ検索する							*/
/*		! ~ ++ -- - * & (type) sizeof											*/
/********************************************************************************/
static void factor13(struct expr_param *expr_p1)
{
//	return factor14(func, mode, value);

	int type, attr2;
	struct src_ptr src_ptr;
	struct symtbl *ptr;
	int top_numb = c32_label_counter++;
	int end_numb = c32_label_counter++;
	
	c32_src_ptr_save(&src_ptr);
	c32_token_process(0, &type);
	if(type==TYPE_L_KAKKO){					// (type) 処理
		attr2 = c32_char_short_int_unsigned_pointer();
		if(attr2==0){
			// if を抜けて factor14 ++ -- 処理を行う
//			c32_src_ptr_restore(&src_ptr);
//			return factor14(func, mode1, value1);	// factor13() には出来ない
		}
		else {
			c32_token_process(0, &type);			// ')' を確認する
			if (type != TYPE_R_KAKKO)
				c32_error_message(E_R_KAKKO_MISSING, __LINE__, __FILE__);
			factor13(expr_p1);
			convert_mode0(expr_p1);
			expr_p1->mode = MODE0;
			expr_p1->attr = attr2;
			return;
		}
	}
	else if(type==TYPE_ASTERISK){			// '*'
		factor13(expr_p1);
		
		if(expr_p1->attr & ATTR_ARRAY)
			expr_p1->attr &= ~ATTR_ARRAY;
		else if(expr_p1->attr & ATTR_POINTER)
			expr_p1->attr -= ATTR_POINTER1;
		else
			c32_error_message(E_NEED_POINTER, __LINE__, __FILE__);

		switch(expr_p1->mode){
		case MODE0:							// レジスタ属性    c32_s_numb
			strcpy(expr_p1->reg_var, expr_p1->reg_ans);
			expr_p1->off = "0";
			break;
		case MODE1:							// 計算式アドレス属性  c32_s_numb
			c32_printf("	lw		$%s, %s($%s)\n", expr_p1->reg_ans, expr_p1->off, expr_p1->reg_var);
			strcpy(expr_p1->reg_var, expr_p1->reg_ans);
			expr_p1->off = "0";
			break;
		case MODE2:							// 定数属性
			c32_printf("	la		$%s, %d\n", expr_p1->reg_ans, expr_p1->value);
			strcpy(expr_p1->reg_var, expr_p1->reg_ans);
			expr_p1->off = "0";
			break;
		case MODE6:							// ラベル
			c32_printf("	la		$%s, L%d\n", expr_p1->reg_ans, expr_p1->value);
			strcpy(expr_p1->reg_var, expr_p1->reg_ans);
			expr_p1->off = "0";
			break;
		default:
			c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
		}
		expr_p1->mode = MODE1;					// 計算式アドレス属性
		return;
	}
	else if(type==TYPE_AND){				// '&'
		factor13(expr_p1);
		
		expr_p1->attr += ATTR_POINTER1;
		if((expr_p1->attr & ATTR_POINTER)==0)
			c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
		
		switch(expr_p1->mode){
		case MODE1:							// 計算式アドレス属性  c32_s_numb
			if(expr_p1->ptr){
				expr_p1->ptr->flag1 |= FLAG_20;	// & 演算子使用
			}
			c32_printf("	addiu	$%s, $%s, %s\n", expr_p1->reg_ans, expr_p1->reg_var, expr_p1->off);
			expr_p1->mode = MODE0;			// 計算式属性
			return;
		case MODE2:							// 定数属性
		case MODE6:							// ラベル
		case MODE0:							// 計算式属性    c32_s_numb
			c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
		default:
			c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
		}
		return;
	}
	else if(type==TYPE_PLUS_PLUS){			// ++
		int tmp;
		factor13(expr_p1);
		
		tmp = c32_attr_to_shift_bit(expr_p1->attr);
		if(tmp==0)
			tmp = 1;
		else if(tmp==1)
			tmp = 2;
		else
			tmp = 4;
		
		switch(expr_p1->mode){
		case MODE1:							// レジスタ相対
			reg_eq_mem(expr_p1->attr, "t0", expr_p1->reg_var, expr_p1->off);
			c32_printf("	addiu	$t1, $t0, %d\n", tmp);
			mem_eq_reg(expr_p1->attr, expr_p1->reg_var, expr_p1->off, "t1");
//			expr_p1->value = 0;
//			expr_p1->mode = MODE0;			// レジスタ属性
			return;
		case MODE2:							// 定数属性
		case MODE6:							// ラベル
		case MODE0:							// 計算式属性    c32_s_numb
			c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
		default:
			c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
		}
		return;
	}
	else if(type==TYPE_MINUS_MINUS){		// --
		int tmp;
		factor13(expr_p1);
		
		tmp = c32_attr_to_shift_bit(expr_p1->attr);
		if(tmp==0)
			tmp = 1;
		else if(tmp==1)
			tmp = 2;
		else
			tmp = 4;
		
		switch(expr_p1->mode){
		case MODE1:							// 計算式アドレス属性  c32_s_numb
			reg_eq_mem(expr_p1->attr, "t0", expr_p1->reg_var, expr_p1->off);
			c32_printf("	addiu	$t1, $t0, %d\n", -tmp);
			mem_eq_reg(expr_p1->attr, expr_p1->reg_var, expr_p1->off, "t1");
//			expr_p1->value = 0;
//			expr_p1->mode = MODE0;			// レジスタ属性
			return;
		case MODE2:							// 定数属性
		case MODE6:							// ラベル
		case MODE0:							// 計算式属性    c32_s_numb
			c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
		default:
			c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
		}
		return;
	}
	else if(type==TYPE_MINUS){				// '-'
		factor13(expr_p1);
		switch(expr_p1->mode){
		case MODE0:							// レジスタ属性
			c32_printf("	subu	$%s, $zero, $%s\n", expr_p1->reg_ans, expr_p1->reg_ans);
			break;
		case MODE1:							// レジスタ相対
			reg_eq_mem(expr_p1->attr, expr_p1->reg_ans, expr_p1->reg_var, expr_p1->off);
			c32_printf("	subu	$%s, $zero, $%s\n", expr_p1->reg_ans, expr_p1->reg_ans);
			expr_p1->mode = MODE0;
			break;
		case MODE2:							// 定数
			expr_p1->value = 0-(expr_p1->value);
			break;
		case MODE6:							// ラベル
			c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
		default:
			c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
		}
		return;
	}
	else if(type==TYPE_TILDE){				// '~'
		factor13(expr_p1);
		switch(expr_p1->mode){
		case MODE0:							// レジスタ属性
			c32_printf("	nor		$%s, $%s, $zero\n", expr_p1->reg_ans, expr_p1->reg_ans);
			break;
		case MODE1:							// レジスタ相対
			reg_eq_mem(expr_p1->attr, expr_p1->reg_ans, expr_p1->reg_var, expr_p1->off);
			c32_printf("	nor		$%s, $%s, $zero\n", expr_p1->reg_ans, expr_p1->reg_ans);
			expr_p1->mode = MODE0;
			break;
		case MODE2:							// 定数
			expr_p1->value = ~(expr_p1->value);
			break;
		case MODE6:							// ラベル
			c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
		default:
			c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
		}
		return;
	}
	else if(type==TYPE_NOT){				// '!'
		factor13(expr_p1);
		convert_mode0(expr_p1);
		c32_printf("	beqz	$%s, L%d\n", expr_p1->reg_ans, top_numb);
		c32_printf("	li		$%s, 0\n", expr_p1->reg_ans);
		c32_printf("	j		L%d\n", end_numb);
		c32_printf("L%d\n", top_numb);
		c32_printf("	li		$%s, 1\n", expr_p1->reg_ans);
		c32_printf("L%d\n", end_numb);
		expr_p1->mode = MODE0;				// 計算式属性
		expr_p1->attr = ATTR_INT;
		return;
	}
	else if(type==TYPE_SIZEOF){				// sizeof a
		c32_src_ptr_save(&src_ptr);
		attr2 = c32_char_short_int_unsigned_pointer();
		if(attr2==0){
			c32_src_ptr_restore(&src_ptr);
			c32_spskip();
			if(*c32_linebufp=='('){
				c32_linebufp++;
				c32_src_ptr_save(&src_ptr);
				attr2 = c32_char_short_int_unsigned_pointer();
				if(attr2){
					c32_token_process(0, &type);
					if(type != TYPE_R_KAKKO){
						c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
					}
					expr_p1->value = c32_attr_to_byte(attr2);
					expr_p1->mode = MODE2;		// 定数属性
					expr_p1->attr = ATTR_INT;
					return;
				}
				c32_src_ptr_restore(&src_ptr);
				c32_getsym();						// symbol でなければエラーにする
				c32_spskip();
				if(*c32_linebufp++ != ')')
					c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
			}
			else{
				c32_getsym();
			}
			ptr = c32_sym_search_all(expr_p1->func);
			if(ptr==0)
				c32_error_message(E_UNDEFINED_SYMBOL, __LINE__, __FILE__);
			if(ptr->flag1 & (FLAG_01 | FLAG_02 | FLAG_04)){	// 変数ならば
				expr_p1->value = ptr->size;
				expr_p1->mode = MODE2;		// 定数属性
				expr_p1->attr = ATTR_INT;
				return;
			}
			// 変数ではない
			c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
		}
		expr_p1->value = c32_attr_to_byte(attr2);
		expr_p1->mode = MODE2;				// 定数属性
		expr_p1->attr = ATTR_INT;
		return;
	}
	c32_src_ptr_restore(&src_ptr);
	factor14(expr_p1);						// factor13() には出来ない
	c32_src_ptr_save(&src_ptr);
	c32_token_process(0, &type);
	if(type==TYPE_PLUS_PLUS){
		int tmp;
		
		tmp = c32_attr_to_shift_bit(expr_p1->attr);
		if(tmp==0)
			tmp = 1;
		else if(tmp==1)
			tmp = 2;
		else
			tmp = 4;
		
		switch(expr_p1->mode){
		case MODE1:							// レジスタ相対
			reg_eq_mem(expr_p1->attr, "t0", expr_p1->reg_var, expr_p1->off);
			c32_printf("	addiu	$t1, $t0, %d\n", tmp);
			mem_eq_reg(expr_p1->attr, expr_p1->reg_var, expr_p1->off, "t1");
			c32_printf("	move	$%s, $t0\n", expr_p1->reg_ans);
			expr_p1->mode = MODE0;			// レジスタ属性
			return;
		case MODE2:							// 定数属性    value
		case MODE6:							// ラベル
		case MODE0:							// 計算式属性    c32_s_numb
			c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
		default:
			c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
		}
	}
	else if(type==TYPE_MINUS_MINUS){
		int tmp;
		
		tmp = c32_attr_to_shift_bit(expr_p1->attr);
		if(tmp==0)
			tmp = 1;
		else if(tmp==1)
			tmp = 2;
		else
			tmp = 4;
		
		switch(expr_p1->mode){
		case MODE1:							// レジスタ相対
			reg_eq_mem(expr_p1->attr, "t0", expr_p1->reg_var, expr_p1->off);
			c32_printf("	addiu	$t1, $t0, %d\n", -tmp);
			mem_eq_reg(expr_p1->attr, expr_p1->reg_var, expr_p1->off, "t1");
			c32_printf("	move	$%s, $t0\n", expr_p1->reg_ans);
			expr_p1->mode = MODE0;			// レジスタ属性
			return;
		case MODE2:							// 定数属性    value
		case MODE6:							// ラベル
		case MODE0:							// 計算式属性    c32_s_numb
			c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
		default:
			c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
		}
	}
	else{
		c32_src_ptr_restore(&src_ptr);
		return;
	}
}
/********************************************************************************/
/*		factor12																*/
/*		return: 求めた 値の attr												*/
/*		mode=0: 計算式属性														*/
/*		mode=1: 計算式アドレス属性、最終オブジェクト有り						*/
/*		mode=2: 定数															*/
/*	   (mode=3: 定数アドレス属性、最終オブジェクトのアドレス)					*/
/*		mode=4; $fp offset アドレス属性											*/
/*		mode=5: $s7 offset アドレス属性											*/
/*		mode=6: ラベルアドレス属性、value=label_numb							*/
/*		mode=7: レジスタ属性、(char*)&value   "s0" 等が入る						*/
/*																				*/
/*		value : 定数の時、定数を返すエリア										*/
/*		func が 0 の場合は global table のみ検索する							*/
/*		factor13 * / % factor13													*/
/********************************************************************************/
static void factor12(struct expr_param *expr_p1)
{
//	return factor13(func, mode, value);

	int type, s_numb_save;
	struct src_ptr src_ptr;
	struct expr_param expr_p2;
	
	s_numb_save = c32_s_numb;
	expr_p2 = *expr_p1;
	factor13(expr_p1);
	c32_src_ptr_save(&src_ptr);
	
	c32_token_process(0, &type);
	if(type==TYPE_ASTERISK || type==TYPE_SLUSH || type==TYPE_PERCENT){
		/***	& だった	***/
		convert_mode0_2(expr_p1);
			
		s_numb_save = c32_s_numb++;
		if(c32_max_s_numb < c32_s_numb)
			c32_max_s_numb = c32_s_numb;
		for(;;){
			sprintf(expr_p2.reg_ans, "t%d", c32_s_numb);
			factor13(&expr_p2);
			convert_mode0_2(&expr_p2);
			
			if(expr_p1->attr & (ATTR_POINTER | ATTR_ARRAY))
				c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
			if(expr_p2.attr & (ATTR_POINTER | ATTR_ARRAY))
				c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
			
			if(type==TYPE_ASTERISK){
				if(expr_p1->mode==MODE2 && expr_p2.mode==MODE2){
					expr_p1->value *= expr_p2.value;
				}
				else{
					convert_mode0(expr_p1);
					convert_mode0(&expr_p2);
					expr_p1->mode = MODE0;
					if((expr_p1->attr | expr_p2.attr) & ATTR_UNSIGNED){	// unsigned
						c32_printf("	multu	$%s, $t%d\n", expr_p1->reg_ans, c32_s_numb);
						c32_printf("	mflo	$%s\n", expr_p1->reg_ans);
					}
					else{									// signed
						c32_printf("	mult	$%s, $t%d\n", expr_p1->reg_ans, c32_s_numb);
						c32_printf("	mflo	$%s\n", expr_p1->reg_ans);
					}
				}
			}
			else if(type==TYPE_SLUSH){
				if(expr_p1->mode==MODE2 && expr_p2.mode==MODE2){
					expr_p1->value /= expr_p2.value;
				}
				else{
					convert_mode0(expr_p1);
					convert_mode0(&expr_p2);
					expr_p1->mode = MODE0;
					if((expr_p1->attr | expr_p2.attr) & ATTR_UNSIGNED){	// unsigned
						c32_printf("	divu	$%s, $t%d\n", expr_p1->reg_ans, c32_s_numb);
						c32_printf("	mflo	$%s\n", expr_p1->reg_ans);
					}
					else{									// signed
						c32_printf("	div		$%s, $t%d\n", expr_p1->reg_ans, c32_s_numb);
						c32_printf("	mflo	$%s\n", expr_p1->reg_ans);
					}
				}
			}
			else if(type==TYPE_PERCENT){
				if(expr_p1->mode==MODE2 && expr_p2.mode==MODE2){
					expr_p1->value %= expr_p2.value;
				}
				else{
					convert_mode0(expr_p1);
					convert_mode0(&expr_p2);
					expr_p1->mode = MODE0;
					if((expr_p1->attr | expr_p2.attr) & ATTR_UNSIGNED){	// unsigned
						c32_printf("	divu	$%s, $t%d\n", expr_p1->reg_ans, c32_s_numb);
						c32_printf("	mfhi	$%s\n", expr_p1->reg_ans);
					}
					else{									// signed
						c32_printf("	div		$%s, $t%d\n", expr_p1->reg_ans, c32_s_numb);
						c32_printf("	mfhi	$%s\n", expr_p1->reg_ans);
					}
				}
			}
			else{
				c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
			}
			expr_p1->attr = ((expr_p1->attr | expr_p2.attr) & ATTR_UNSIGNED) | ATTR_INT;
			
			c32_src_ptr_save(&src_ptr);
			c32_token_process(0, &type);			// & を調べる
			if(type==TYPE_ASTERISK || type==TYPE_SLUSH || type==TYPE_PERCENT)
				continue;
			
			c32_src_ptr_restore(&src_ptr);
			c32_s_numb = s_numb_save;
			return;
		}
	}
	c32_src_ptr_restore(&src_ptr);
}
/********************************************************************************/
/*		factor11																*/
/*		return: 求めた 値の attr												*/
/*		mode=0: 計算式属性														*/
/*		mode=1: 計算式アドレス属性、最終オブジェクト有り						*/
/*		mode=2: 定数															*/
/*	   (mode=3: 定数アドレス属性、最終オブジェクトのアドレス)					*/
/*		mode=4; $fp offset アドレス属性											*/
/*		mode=5: $s7 offset アドレス属性											*/
/*		mode=6: ラベルアドレス属性、value=label_numb							*/
/*		mode=7: レジスタ属性、(char*)&value   "s0" 等が入る						*/
/*																				*/
/*		value : 定数の時、定数を返すエリア										*/
/*		func が 0 の場合は global table のみ検索する							*/
/*		factor12 + - factor12													*/
/********************************************************************************/
static void factor11(struct expr_param *expr_p1)
{
//	return factor12(func, mode, value);

	int type, s_numb_save, shift;
	struct src_ptr src_ptr;
	struct expr_param expr_p2;
	
	s_numb_save = c32_s_numb;
	expr_p2 = *expr_p1;
	factor12(expr_p1);
	c32_src_ptr_save(&src_ptr);
	
	c32_token_process(0, &type);
	if(type==TYPE_PLUS || type==TYPE_MINUS){
		/***	& だった	***/
		convert_mode0_2(expr_p1);
//		expr_p1->mode = MODE0;
			
		s_numb_save = c32_s_numb++;
		if(c32_max_s_numb < c32_s_numb)
			c32_max_s_numb = c32_s_numb;
		for(;;){
			sprintf(expr_p2.reg_ans, "t%d", c32_s_numb);
			factor12(&expr_p2);
			convert_mode0_2(&expr_p2);
			if(type==TYPE_PLUS){
				if(expr_p1->attr & (ATTR_POINTER | ATTR_ARRAY)){
					convert_mode0(expr_p1);
					convert_mode0(&expr_p2);
					expr_p1->mode = MODE0;
					shift = c32_attr_to_shift_bit(expr_p1->attr);
					if(expr_p2.attr & (ATTR_POINTER | ATTR_ARRAY))
						c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
					if(shift){
						c32_printf("	sll		$t%d, $t%d, %d\n", c32_s_numb, c32_s_numb, shift);
					}
					c32_printf("	addu	$%s, $%s, $t%d\n", expr_p1->reg_ans, expr_p1->reg_ans, c32_s_numb);
				}
				else{
					if(expr_p2.attr & (ATTR_POINTER | ATTR_ARRAY))
						c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
					if(expr_p1->mode==MODE2 && expr_p2.mode==MODE2){
						expr_p1->value += expr_p2.value;
					}
					else{
						convert_mode0(expr_p1);
						convert_mode0(&expr_p2);
						expr_p1->mode = MODE0;
						c32_printf("	addu	$%s, $%s, $t%d\n", expr_p1->reg_ans, expr_p1->reg_ans, c32_s_numb);
					}
					expr_p1->attr = ((expr_p1->attr | expr_p2.attr) & ATTR_UNSIGNED) |  ATTR_INT;
				}
			}
			else if(type==TYPE_MINUS){
				if(expr_p1->attr & (ATTR_POINTER | ATTR_ARRAY)){
					convert_mode0(expr_p1);
					convert_mode0(&expr_p2);
					expr_p1->mode = MODE0;
					shift = c32_attr_to_shift_bit(expr_p1->attr);
					if(expr_p2.attr & (ATTR_POINTER | ATTR_ARRAY)){
						if(expr_p1->attr != expr_p2.attr)
							c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
						if(shift){
							c32_printf("	srl		$%s, $%s, %d\n", expr_p1->reg_ans, expr_p1->reg_ans, shift);
						}
						c32_printf("	subu	$%s, $%s, $t%d\n", expr_p1->reg_ans, expr_p1->reg_ans, c32_s_numb);
						expr_p1->attr = ATTR_INT;
					}
					else{
						if(shift){
							c32_printf("	sll		$t%d, $t%d, %d\n", c32_s_numb, c32_s_numb, shift);
						}
						c32_printf("	subu	$%s, $%s, $t%d\n", expr_p1->reg_ans, expr_p1->reg_ans, c32_s_numb);
					}
				}
				else{
					if(expr_p2.attr & (ATTR_POINTER | ATTR_ARRAY))
						c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
					if(expr_p1->mode==MODE2 && expr_p2.mode==MODE2){
						expr_p1->value -= expr_p2.value;
					}
					else{
						convert_mode0(expr_p1);
						convert_mode0(&expr_p2);
						expr_p1->mode = MODE0;
						c32_printf("	subu	$%s, $%s, $t%d\n", expr_p1->reg_ans, expr_p1->reg_ans, c32_s_numb);
					}
					expr_p1->attr = ((expr_p1->attr | expr_p2.attr) & ATTR_UNSIGNED) |  ATTR_INT;
				}
			}
			else{
				c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
			}
			
			c32_src_ptr_save(&src_ptr);
			c32_token_process(0, &type);			// & を調べる
			if(type==TYPE_PLUS || type==TYPE_MINUS)
				continue;
			
			c32_src_ptr_restore(&src_ptr);
			c32_s_numb = s_numb_save;
			return;
		}
	}
	c32_src_ptr_restore(&src_ptr);
}
/********************************************************************************/
/*		factor10																*/
/*		return: 求めた 値の attr												*/
/*		mode=0: 計算式属性														*/
/*		mode=1: 計算式アドレス属性、最終オブジェクト有り						*/
/*		mode=2: 定数															*/
/*	   (mode=3: 定数アドレス属性、最終オブジェクトのアドレス)					*/
/*		mode=4; $fp offset アドレス属性											*/
/*		mode=5: $s7 offset アドレス属性											*/
/*		mode=6: ラベルアドレス属性、value=label_numb							*/
/*		mode=7: レジスタ属性、(char*)&value   "s0" 等が入る						*/
/*																				*/
/*		value : 定数の時、定数を返すエリア										*/
/*		func が 0 の場合は global table のみ検索する							*/
/*		factor11 << >> factor11													*/
/********************************************************************************/
static void factor10(struct expr_param *expr_p1)
{
//	return factor11(func, mode, value);

	int type, s_numb_save;
	struct src_ptr src_ptr;
	struct expr_param expr_p2;
	
	s_numb_save = c32_s_numb;
	expr_p2 = *expr_p1;
	factor11(expr_p1);
	c32_src_ptr_save(&src_ptr);
	
	c32_token_process(0, &type);				// << >> をテストする
	if(type==TYPE_L_SHIFT || type==TYPE_R_SHIFT){
		convert_mode0(expr_p1);
		expr_p1->mode = MODE0;
		s_numb_save = c32_s_numb++;
		if(c32_max_s_numb < c32_s_numb)
			c32_max_s_numb = c32_s_numb;
		
		sprintf(expr_p2.reg_ans, "t%d", c32_s_numb);
		factor11(&expr_p2);
		convert_mode0(&expr_p2);
		expr_p2.mode = MODE0;
		if(type==TYPE_L_SHIFT){
			c32_printf("	sllv	$%s, $%s, $t%d\n", expr_p1->reg_ans, expr_p1->reg_ans, c32_s_numb);
			c32_s_numb = s_numb_save;
			return;
		}
		else if(type==TYPE_R_SHIFT){
			c32_printf("	srlv	$%s, $%s, $t%d\n", expr_p1->reg_ans, expr_p1->reg_ans, c32_s_numb);
			c32_s_numb = s_numb_save;
			return;
		}
		else{
			c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
		}
	}
	c32_src_ptr_restore(&src_ptr);
}
/********************************************************************************/
/*		factor9																	*/
/*		return: 求めた 値の attr												*/
/*		mode=0: 計算式属性														*/
/*		mode=1: 計算式アドレス属性、最終オブジェクト有り						*/
/*		mode=2: 定数															*/
/*	   (mode=3: 定数アドレス属性、最終オブジェクトのアドレス)					*/
/*		mode=4; $fp offset アドレス属性											*/
/*		mode=5: $s7 offset アドレス属性											*/
/*		mode=6: ラベルアドレス属性、value=label_numb							*/
/*		mode=7: レジスタ属性、(char*)&value   "s0" 等が入る						*/
/*																				*/
/*		value : 定数の時、定数を返すエリア										*/
/*		func が 0 の場合は global table のみ検索する							*/
/*		factor10 < <= > >= factor10												*/
/********************************************************************************/
static void factor9(struct expr_param *expr_p1)
{
//	return factor10(func, mode, value);

	int type, s_numb_save;
	struct src_ptr src_ptr;
	struct expr_param expr_p2;
	
	s_numb_save = c32_s_numb;
	expr_p2 = *expr_p1;
	factor10(expr_p1);
	c32_src_ptr_save(&src_ptr);
	
	c32_token_process(0, &type);
	if(type==TYPE_LT || type==TYPE_LE || type==TYPE_GT || type==TYPE_GE){
		convert_mode0(expr_p1);
		expr_p1->mode = MODE0;
		s_numb_save = c32_s_numb++;
		if(c32_max_s_numb < c32_s_numb)
			c32_max_s_numb = c32_s_numb;
		
		sprintf(expr_p2.reg_ans, "t%d", c32_s_numb);
		factor10(&expr_p2);
		convert_mode0(&expr_p2);
		expr_p2.mode = MODE0;
		if(type==TYPE_LT){					// < 処理
			if((expr_p1->attr | expr_p2.attr) & ATTR_UNSIGNED){	// unsigned
				c32_printf("	sltu	$%s, $%s, $t%d\n", expr_p1->reg_ans, expr_p1->reg_ans, c32_s_numb);	// x = x < y ? 1 : 0
			}
			else{									// signed
				c32_printf("	slt		$%s, $%s, $t%d\n", expr_p1->reg_ans, expr_p1->reg_ans, c32_s_numb);	// x = x < y ? 1 : 0
			}
			c32_s_numb = s_numb_save;
			expr_p1->attr = ATTR_INT;
			return;
		}
		else if(type==TYPE_LE){				// <= 処理
			if((expr_p1->attr | expr_p2.attr) & ATTR_UNSIGNED){	// unsigned
				c32_printf("	sltu	$%s, $t%d, $%s\n", expr_p1->reg_ans, c32_s_numb, expr_p1->reg_ans);	// x = x <= y ? 0 : 1
				c32_printf("	li		$t0, 1\n");
				c32_printf("	subu	$%s, $t0, $%s\n", expr_p1->reg_ans, expr_p1->reg_ans);
			}
			else{									// signed
				c32_printf("	slt		$%s, $t%d, $%s\n", expr_p1->reg_ans, c32_s_numb, expr_p1->reg_ans);	// x = x <= y ? 0 : 1
				c32_printf("	li		$t0, 1\n");
				c32_printf("	subu	$%s, $t0, $%s\n", expr_p1->reg_ans, expr_p1->reg_ans);
			}
			c32_s_numb = s_numb_save;
			expr_p1->attr = ATTR_INT;
			return;
		}
		else if(type==TYPE_GT){				// > 処理
			if((expr_p1->attr | expr_p2.attr) & ATTR_UNSIGNED){	// unsigned
				c32_printf("	sltu	$%s, $t%d, $%s\n", expr_p1->reg_ans, c32_s_numb, expr_p1->reg_ans);	// x = x > y ? 1 : 0
			}
			else{									// signed
				c32_printf("	slt		$%s, $t%d, $%s\n", expr_p1->reg_ans, c32_s_numb, expr_p1->reg_ans);	// x = x > y ? 1 : 0
			}
			c32_s_numb = s_numb_save;
			expr_p1->attr = ATTR_INT;
			return;
		}
		else if(type==TYPE_GE){				// >= 処理
			if((expr_p1->attr | expr_p2.attr) & ATTR_UNSIGNED){	// unsigned
				c32_printf("	sltu	$%s, $%s, $t%d\n", expr_p1->reg_ans, expr_p1->reg_ans, c32_s_numb);	// x = x >= y ? 0 : 1
				c32_printf("	li		$t0, 1\n");
				c32_printf("	subu	$%s, $t0, $%s\n", expr_p1->reg_ans, expr_p1->reg_ans);
			}
			else{									// signed
				c32_printf("	slt		$%s, $%s, $t%d\n", expr_p1->reg_ans, expr_p1->reg_ans, c32_s_numb);	// x = x >= y ? 0 : 1
				c32_printf("	li		$t0, 1\n");
				c32_printf("	subu	$%s, $t0, $%s\n", expr_p1->reg_ans, expr_p1->reg_ans);
			}
			c32_s_numb = s_numb_save;
			expr_p1->attr = ATTR_INT;
			return;
		}
		else{
			c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
		}
	}
	c32_src_ptr_restore(&src_ptr);
}
/********************************************************************************/
/*		factor8																	*/
/*		return: 求めた 値の attr												*/
/*		mode=0: 計算式属性														*/
/*		mode=1: 計算式アドレス属性、最終オブジェクト有り						*/
/*		mode=2: 定数															*/
/*	   (mode=3: 定数アドレス属性、最終オブジェクトのアドレス)					*/
/*		mode=4; $fp offset アドレス属性											*/
/*		mode=5: $s7 offset アドレス属性											*/
/*		mode=6: ラベルアドレス属性、value=label_numb							*/
/*		mode=7: レジスタ属性、(char*)&value   "s0" 等が入る						*/
/*																				*/
/*		value : 定数の時、定数を返すエリア										*/
/*		func が 0 の場合は global table のみ検索する							*/
/*		factor9 ==  != factor9													*/
/********************************************************************************/
static void factor8(struct expr_param *expr_p1)
{
//	return factor9(func, mode, value);

	int type, s_numb_save;
	struct src_ptr src_ptr;
	struct expr_param expr_p2;
	int end_numb = c32_label_counter++;
	
	s_numb_save = c32_s_numb;
	expr_p2 = *expr_p1;
	factor9(expr_p1);
	c32_src_ptr_save(&src_ptr);
	
	c32_token_process(0, &type);				// == != を調べる
	if(type==TYPE_EQEQ || type==TYPE_NOTEQ){
		convert_mode0(expr_p1);
		expr_p1->mode = MODE0;
		s_numb_save = c32_s_numb++;
		if(c32_max_s_numb < c32_s_numb)
			c32_max_s_numb = c32_s_numb;
		
		sprintf(expr_p2.reg_ans, "t%d", c32_s_numb);
		factor9(&expr_p2);
		convert_mode0(&expr_p2);
		expr_p2.mode = MODE0;
		if(type==TYPE_EQEQ){				// == 処理
			c32_printf("	subu	$%s, $%s, $t%d\n", expr_p1->reg_ans, expr_p1->reg_ans, c32_s_numb);
			c32_printf("	beqz	$%s, L%d\n", expr_p1->reg_ans, end_numb);
			c32_printf("	li		$%s, -1\n", expr_p1->reg_ans);
			c32_printf("L%d\n", end_numb);
			c32_printf("	addiu	$%s, $%s, 1\n", expr_p1->reg_ans, expr_p1->reg_ans);
			c32_s_numb = s_numb_save;
			expr_p1->attr = ATTR_INT;
			return;
		}
		else{								// != 処理
			c32_printf("	subu	$%s, $%s, $t%d\n", expr_p1->reg_ans, expr_p1->reg_ans, c32_s_numb);
			c32_printf("	beqz	$%s, L%d\n", expr_p1->reg_ans, end_numb);
			c32_printf("	li		$%s, 1\n", expr_p1->reg_ans);
			c32_printf("L%d\n", end_numb);
			c32_s_numb = s_numb_save;
			expr_p1->attr = ATTR_INT;
			return;
		}
	}
	c32_src_ptr_restore(&src_ptr);
}
/********************************************************************************/
/*		factor7																	*/
/*		return: 求めた 値の attr												*/
/*		mode=0: 計算式属性														*/
/*		mode=1: 計算式アドレス属性、最終オブジェクト有り						*/
/*		mode=2: 定数															*/
/*	   (mode=3: 定数アドレス属性、最終オブジェクトのアドレス)					*/
/*		mode=4; $fp offset アドレス属性											*/
/*		mode=5: $s7 offset アドレス属性											*/
/*		mode=6: ラベルアドレス属性、value=label_numb							*/
/*		mode=7: レジスタ属性、(char*)&value   "s0" 等が入る						*/
/*																				*/
/*		value : 定数の時、定数を返すエリア										*/
/*		func が 0 の場合は global table のみ検索する							*/
/*		factor8 & factor8														*/
/********************************************************************************/
static void factor7(struct expr_param *expr_p1)
{
//	return factor8(func, mode, value);

	int type, s_numb_save;
	struct src_ptr src_ptr;
	struct expr_param expr_p2;
	
	s_numb_save = c32_s_numb;
	factor8(expr_p1);
	c32_src_ptr_save(&src_ptr);
	
	c32_token_process(0, &type);				// & を調べる
	if(type != TYPE_AND){
		c32_src_ptr_restore(&src_ptr);
		return;
	}
	/***	& だった	***/
	convert_mode0_2(expr_p1);
	expr_p2 = *expr_p1;
	s_numb_save = c32_s_numb++;
	if(c32_max_s_numb < c32_s_numb)
		c32_max_s_numb = c32_s_numb;
	sprintf(expr_p2.reg_ans, "t%d", c32_s_numb);
	for(;;){
		factor8(&expr_p2);
		convert_mode0_2(&expr_p2);
		
		if(expr_p1->mode==MODE2 && expr_p2.mode==MODE2){
			expr_p1->value &= expr_p2.value;
		}
		else if(expr_p1->mode==MODE0 && expr_p2.mode==MODE2){
			if((expr_p2.value & 0xffff0000)==0){
				c32_printf("	andi	$%s, $%s, 0x%x\n", expr_p1->reg_ans, expr_p1->reg_ans, expr_p2.value);
			}
			else{
				convert_mode0(&expr_p2);
				c32_printf("	and		$%s, $%s, $t%d\n", expr_p1->reg_ans, expr_p1->reg_ans, c32_s_numb);
			}
		}
		else{
			convert_mode0(expr_p1);
			convert_mode0(&expr_p2);
			expr_p1->mode = MODE0;
			c32_printf("	and		$%s, $%s, $t%d\n", expr_p1->reg_ans, expr_p1->reg_ans, c32_s_numb);
		}
		
		c32_src_ptr_save(&src_ptr);
		c32_token_process(0, &type);			// & を調べる
		if(type==TYPE_AND)
			continue;
		
		c32_src_ptr_restore(&src_ptr);
		c32_s_numb = s_numb_save;
		expr_p1->attr = ((expr_p1->attr | expr_p2.attr) & ATTR_UNSIGNED) | ATTR_INT;
		return;
	}
}
/********************************************************************************/
/*		factor6																	*/
/*		return: 求めた 値の attr												*/
/*		mode=0: 計算式属性														*/
/*		mode=1: 計算式アドレス属性、最終オブジェクト有り						*/
/*		mode=2: 定数															*/
/*	   (mode=3: 定数アドレス属性、最終オブジェクトのアドレス)					*/
/*		mode=4; $fp offset アドレス属性											*/
/*		mode=5: $s7 offset アドレス属性											*/
/*		mode=6: ラベルアドレス属性、value=label_numb							*/
/*		mode=7: レジスタ属性、(char*)&value   "s0" 等が入る						*/
/*																				*/
/*		value : 定数の時、定数を返すエリア										*/
/*		func が 0 の場合は global table のみ検索する							*/
/*		factor7 ^ factor7														*/
/********************************************************************************/
static void factor6(struct expr_param *expr_p1)
{
//	return factor7(func, mode, value);

	int type, s_numb_save;
	struct src_ptr src_ptr;
	struct expr_param expr_p2;
	
	s_numb_save = c32_s_numb;
	factor7(expr_p1);
	c32_src_ptr_save(&src_ptr);
	
	c32_token_process(0, &type);				// ^ を調べる
	if(type != TYPE_XOR){
		c32_src_ptr_restore(&src_ptr);
		return;
	}
	/***	^ だった	***/
	convert_mode0_2(expr_p1);
	expr_p2 = *expr_p1;
	s_numb_save = c32_s_numb++;
	if(c32_max_s_numb < c32_s_numb)
		c32_max_s_numb = c32_s_numb;
	sprintf(expr_p2.reg_ans, "t%d", c32_s_numb);
	for(;;){
		factor7(&expr_p2);
		convert_mode0_2(&expr_p2);
		
		if(expr_p1->mode==MODE2 && expr_p2.mode==MODE2){
			expr_p1->value ^= expr_p2.value;
		}
		else if(expr_p1->mode==MODE0 && expr_p2.mode==MODE2){
			if((expr_p2.value & 0xffff0000)==0){
				c32_printf("	xori	$%s, $%s, 0x%x\n", expr_p1->reg_ans, expr_p1->reg_ans, expr_p2.value);
			}
			else{
				convert_mode0(&expr_p2);
				c32_printf("	xor		$%s, $%s, $t%d\n", expr_p1->reg_ans, expr_p1->reg_ans, c32_s_numb);
			}
		}
		else{
			convert_mode0(expr_p1);
			convert_mode0(&expr_p2);
			expr_p1->mode = MODE0;
			c32_printf("	xor		$%s, $%s, $t%d\n", expr_p1->reg_ans, expr_p1->reg_ans, c32_s_numb);
		}
		
		c32_src_ptr_save(&src_ptr);
		c32_token_process(0, &type);			// ^ を調べる
		if(type==TYPE_XOR)
			continue;
		
		c32_src_ptr_restore(&src_ptr);
		c32_s_numb = s_numb_save;
		expr_p1->attr = ((expr_p1->attr | expr_p2.attr) & ATTR_UNSIGNED) | ATTR_INT;
		return;
	}
}
/********************************************************************************/
/*		factor5																	*/
/*		return: 求めた 値の attr												*/
/*		mode=0: 計算式属性														*/
/*		mode=1: 計算式アドレス属性、最終オブジェクト有り						*/
/*		mode=2: 定数															*/
/*	   (mode=3: 定数アドレス属性、最終オブジェクトのアドレス)					*/
/*		mode=4; $fp offset アドレス属性											*/
/*		mode=5: $s7 offset アドレス属性											*/
/*		mode=6: ラベルアドレス属性、value=label_numb							*/
/*		mode=7: レジスタ属性、(char*)&value   "s0" 等が入る						*/
/*																				*/
/*		value : 定数の時、定数を返すエリア										*/
/*		func が 0 の場合は global table のみ検索する							*/
/*		factor6 | factor6														*/
/********************************************************************************/
static void factor5(struct expr_param *expr_p1)
{
//	return factor6(func, mode, value);

	int type, s_numb_save;
	struct src_ptr src_ptr;
	struct expr_param expr_p2;
	
	s_numb_save = c32_s_numb;
	factor6(expr_p1);
	c32_src_ptr_save(&src_ptr);
	
	c32_token_process(0, &type);				// | を調べる
	if(type != TYPE_OR){
		c32_src_ptr_restore(&src_ptr);
		return;
	}
	/***	| だった	***/
	convert_mode0_2(expr_p1);
	expr_p2 = *expr_p1;
	s_numb_save = c32_s_numb++;
	if(c32_max_s_numb < c32_s_numb)
		c32_max_s_numb = c32_s_numb;
	sprintf(expr_p2.reg_ans, "t%d", c32_s_numb);
	for(;;){
		factor6(&expr_p2);
		convert_mode0_2(&expr_p2);

		if(expr_p1->mode==MODE2 && expr_p2.mode==MODE2){
			expr_p1->value |= expr_p2.value;
		}
		else if(expr_p1->mode==MODE0 && expr_p2.mode==MODE2){
			if((expr_p2.value & 0xffff0000)==0){
				c32_printf("	ori		$%s, $%s, 0x%x\n", expr_p1->reg_ans, expr_p1->reg_ans, expr_p2.value);
			}
			else{
				convert_mode0(&expr_p2);
				c32_printf("	or		$%s, $%s, $t%d\n", expr_p1->reg_ans, expr_p1->reg_ans, c32_s_numb);
			}
		}
		else{
			convert_mode0(expr_p1);
			convert_mode0(&expr_p2);
			expr_p1->mode = MODE0;
			c32_printf("	or		$%s, $%s, $t%d\n", expr_p1->reg_ans, expr_p1->reg_ans, c32_s_numb);
		}
		
		c32_src_ptr_save(&src_ptr);
		c32_token_process(0, &type);			// | を調べる
		if(type==TYPE_OR)
			continue;
		
		c32_src_ptr_restore(&src_ptr);
		c32_s_numb = s_numb_save;
		expr_p1->attr = ((expr_p1->attr | expr_p2.attr) & ATTR_UNSIGNED) | ATTR_INT;
		return;
	}
}
/********************************************************************************/
/*		factor4																	*/
/*		return: 求めた 値の attr												*/
/*		mode=0: 計算式属性														*/
/*		mode=1: 計算式アドレス属性、最終オブジェクト有り						*/
/*		mode=2: 定数															*/
/*	   (mode=3: 定数アドレス属性、最終オブジェクトのアドレス)					*/
/*		mode=4; $fp offset アドレス属性											*/
/*		mode=5: $s7 offset アドレス属性											*/
/*		mode=6: ラベルアドレス属性、value=label_numb							*/
/*		mode=7: レジスタ属性、(char*)&value   "s0" 等が入る						*/
/*																				*/
/*		value : 定数の時、定数を返すエリア										*/
/*		func が 0 の場合は global table のみ検索する							*/
/*		factor5 && factor4														*/
/********************************************************************************/
static void factor4(struct expr_param *expr_p1)
{
//	return factor5(func, mode, value);

	int type;
	struct src_ptr src_ptr;
	int end_numb = c32_label_counter++;
	
	factor5(expr_p1);
	c32_src_ptr_save(&src_ptr);
	
	c32_token_process(0, &type);				// && を調べる
	if(type != TYPE_AND_AND){
		c32_src_ptr_restore(&src_ptr);
		return;
	}
	/***	&& だった	***/
	convert_mode0(expr_p1);
	c32_printf("	beqz	$t%d, L%d\n", c32_s_numb, end_numb);
	factor4(expr_p1);
	
	convert_mode0(expr_p1);
	c32_printf("L%d\n", end_numb);
	expr_p1->mode = MODE0;					// 計算式属性
}
/********************************************************************************/
/*		factor3																	*/
/*		return: 求めた 値の attr												*/
/*		mode=0: 計算式属性														*/
/*		mode=1: 計算式アドレス属性、最終オブジェクト有り						*/
/*		mode=2: 定数															*/
/*	   (mode=3: 定数アドレス属性、最終オブジェクトのアドレス)					*/
/*		mode=4; $fp offset アドレス属性											*/
/*		mode=5: $s7 offset アドレス属性											*/
/*		mode=6: ラベルアドレス属性、value=label_numb							*/
/*		mode=7: レジスタ属性、(char*)&value   "s0" 等が入る						*/
/*																				*/
/*		value : 定数の時、定数を返すエリア										*/
/*		func が 0 の場合は global table のみ検索する							*/
/*		factor4 || factor3														*/
/********************************************************************************/
static void factor3(struct expr_param *expr_p1)
{
//	return factor4(func, mode, value);

	int type;
	struct src_ptr src_ptr;
	int end_numb = c32_label_counter++;
	
	factor4(expr_p1);
	c32_src_ptr_save(&src_ptr);
	
	c32_token_process(0, &type);				// || を調べる
	if(type != TYPE_OR_OR){
		c32_src_ptr_restore(&src_ptr);
		return;
	}
	/***	|| だった	***/
	convert_mode0(expr_p1);
	c32_printf("	bnez	$t%d, L%d\n", c32_s_numb, end_numb);
	factor3(expr_p1);
	
	convert_mode0(expr_p1);
	c32_printf("L%d\n", end_numb);
	expr_p1->mode = MODE0;					// 計算式属性
}
/********************************************************************************/
/*		convert_mode0_2															*/
/*		計算式属性/定数属性  にする												*/
/*		c32_s_numb は expr_p1 で計算した時の c32_s_numb を指定しなければならない		*/
/********************************************************************************/
void convert_mode0_2(struct expr_param *expr_p1)
{
	switch(expr_p1->mode){
	case MODE0:							// 計算式属性    c32_s_numb
		break;
	case MODE1:							// 計算式アドレス属性  c32_s_numb
		if(expr_p1->attr & ATTR_ARRAY){
			c32_printf("	addiu	$%s, $%s, %s\n", expr_p1->reg_ans, expr_p1->reg_var, expr_p1->off);
		}
		else{
			reg_eq_mem(expr_p1->attr, expr_p1->reg_ans, expr_p1->reg_var, expr_p1->off);
		}
		break;
	case MODE2:							// 定数属性    value
		return;
	case MODE6:							// ラベル
		c32_printf("	la		$%s, L%d\n", expr_p1->reg_ans, expr_p1->value);
		break;
	default:
		c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
	}
	
	expr_p1->mode = MODE0;				// 計算式属性    c32_s_numb
}
/********************************************************************************/
/*		convert_mode0															*/
/*		計算式属性にする														*/
/*		c32_s_numb は expr_p1 で計算した時の c32_s_numb を指定しなければならない		*/
/********************************************************************************/
void convert_mode0(struct expr_param *expr_p1)
{
	switch(expr_p1->mode){
	case MODE0:							// 計算式属性    c32_s_numb
		break;
	case MODE1:							// 計算式アドレス属性  c32_s_numb
		if(expr_p1->attr & ATTR_ARRAY){
			c32_printf("	addiu	$%s, $%s, %s\n", expr_p1->reg_ans, expr_p1->reg_var, expr_p1->off);
		}
		else{
			reg_eq_mem(expr_p1->attr, expr_p1->reg_ans, expr_p1->reg_var, expr_p1->off);
		}
		break;
	case MODE2:							// 定数属性    value
		c32_printf("	la		$%s, %d\n", expr_p1->reg_ans, expr_p1->value);
		break;
	case MODE6:							// ラベル
		c32_printf("	la		$%s, L%d\n", expr_p1->reg_ans, expr_p1->value);
		break;
	default:
		c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
	}
	expr_p1->mode = MODE0;
}
/********************************************************************************/
/*		convert_mode1															*/
/*		計算式アドレス属性にする												*/
/********************************************************************************/
void convert_mode1(struct expr_param *expr_p1)
{
	switch(expr_p1->mode){
	case MODE1:							// レジスタ相対
		break;
	case MODE2:							// 定数属性    value
	case MODE6:							// ラベル
	case MODE0:							// 計算式属性    c32_s_numb
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	default:
		c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
	}
}
/********************************************************************************/
/*		factor2																	*/
/*		return: 求めた 値の attr												*/
/*		mode=0: 計算式属性														*/
/*		mode=1: 計算式アドレス属性、最終オブジェクト有り						*/
/*		mode=2: 定数															*/
/*	   (mode=3: 定数アドレス属性、最終オブジェクトのアドレス)					*/
/*		mode=4; $fp offset アドレス属性											*/
/*		mode=5: $s7 offset アドレス属性											*/
/*		mode=6: ラベルアドレス属性、value=label_numb							*/
/*		mode=7: レジスタ属性、(char*)&value   "s0" 等が入る						*/
/*																				*/
/*		value : 定数の時、定数を返すエリア										*/
/*		func が 0 の場合は global table のみ検索する							*/
/*		factor3 ? factor3 : factor3												*/
/********************************************************************************/
static void factor2(struct expr_param *expr_p1)
{
//	return factor3(func, mode, value);
	
	int type;
	int false_numb = c32_label_counter++;
	int end_numb = c32_label_counter++;
	struct src_ptr src_ptr;
	
	factor3(expr_p1);
	c32_src_ptr_save(&src_ptr);
	
	c32_token_process(0, &type);				// '?' テスト
	if(type != TYPE_QUESTION){				// '?' ではない
		c32_src_ptr_restore(&src_ptr);
		return;
	}
	/***	? :  処理	***/
	convert_mode0(expr_p1);			// 計算式属性にする → $s*
	c32_printf("	beqz	$t%d, L%d\n", c32_s_numb, false_numb);
	
	factor3(expr_p1);						// true 計算式
	convert_mode0(expr_p1);
	c32_printf("	j		L%d\n", end_numb);
	
	c32_token_process(0, &type);				// ':' テスト
	if(type != TYPE_KORON)
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	
	c32_printf("L%d\n", false_numb);
	factor3(expr_p1);						// false 計算式
	convert_mode0(expr_p1);
	
	c32_printf("L%d\n", end_numb);
	expr_p1->mode = MODE0;					// 計算式属性にする
	if(c32_attr_to_byte(expr_p1->attr)==4){
		return;
	}
	expr_p1->attr = ATTR_INT;
}
/********************************************************************************/
/*		c32_factor1																*/
/*		return: 求めた 値の attr												*/
/*		mode=0: 計算式属性														*/
/*		mode=1: 計算式アドレス属性、最終オブジェクト有り						*/
/*		mode=2: 定数															*/
/*	   (mode=3: 定数アドレス属性、最終オブジェクトのアドレス)					*/
/*		mode=4; $fp offset アドレス属性											*/
/*		mode=5: $s7 offset アドレス属性											*/
/*		mode=6: ラベルアドレス属性、value=label_numb							*/
/*		mode=7: レジスタ属性、(char*)&value   "s0" 等が入る						*/
/*																				*/
/*		value : 定数の時、定数を返すエリア										*/
/*		func が 0 の場合は global table のみ検索する							*/
/*		=  +=  -=  *=  /=  %=  &=  ^=  |=  <<=  >>=								*/
/********************************************************************************/
void c32_factor1(struct expr_param *expr_p1)
{
//	return factor2(func, mode, value);

	int type, s_numb_save;
	struct src_ptr src_ptr;
	struct expr_param expr_p2;
	
	s_numb_save = c32_s_numb;
	expr_p2 = *expr_p1;
	/***	第一項取り込み	***/
	s_numb_save = c32_s_numb;
	factor2(expr_p1);
	c32_src_ptr_save(&src_ptr);
	
	/***	演算子チェック		***/
	c32_token_process(0, &type);
	switch(type){
	case TYPE_EQ:
	case TYPE_PLUS_EQ:
	case TYPE_MINUS_EQ:
	case TYPE_ASTERISK_EQ:
	case TYPE_SLUSH_EQ:
	case TYPE_PERSENT_EQ:
	case TYPE_OR_EQ:
	case TYPE_AND_EQ:
	case TYPE_L_SHIFT_EQ:
	case TYPE_R_SHIFT_EQ:
	case TYPE_XOR_EQ:
		break;
	default:
		c32_src_ptr_restore(&src_ptr);
		return;
	}
	
	convert_mode1(expr_p1);
	/***	c32_s_numb update	***/
	c32_s_numb++;
	if(c32_s_numb > c32_max_s_numb)
		c32_max_s_numb = c32_s_numb;
	
	/***	第二項取り込み		***/
	sprintf(expr_p2.reg_ans, "t%d", c32_s_numb);
	c32_factor1(&expr_p2);
//	convert_mode0(&expr_p2);
//	expr_p2.mode = MODE0;
	
	/***	演算実行	***/
	switch(type){
	case TYPE_EQ:
		c32_expr_eq(expr_p1, &expr_p2);
		break;
	case TYPE_PLUS_EQ:
		c32_expr_plus_eq(expr_p1, &expr_p2);
		break;
	case TYPE_MINUS_EQ:
		c32_expr_minus_eq(expr_p1, &expr_p2);
		break;
	case TYPE_ASTERISK_EQ:
		c32_expr_asterisk_eq(expr_p1, &expr_p2);
		break;
	case TYPE_SLUSH_EQ:
		c32_expr_slush_eq(expr_p1, &expr_p2);
		break;
	case TYPE_PERSENT_EQ:
		c32_expr_percent_eq(expr_p1, &expr_p2);
		break;
	case TYPE_OR_EQ:
		c32_expr_or_eq(expr_p1, &expr_p2);
		break;
	case TYPE_AND_EQ:
		c32_expr_and_eq(expr_p1, &expr_p2);
		break;
	case TYPE_XOR_EQ:
		c32_expr_xor_eq(expr_p1, &expr_p2);
		break;
	case TYPE_L_SHIFT_EQ:
		c32_expr_l_shift_eq(expr_p1, &expr_p2);
		break;
	case TYPE_R_SHIFT_EQ:
		c32_expr_r_shift_eq(expr_p1, &expr_p2);
		break;
	default:
		c32_error_message(E_TO_BE_DEFINE, __LINE__, __FILE__);
	}
	
	c32_s_numb = s_numb_save;
}
/********************************************************************************/
/*		c32_expr																*/
/*		return: 求めた 値の attr												*/
/*		mode=0: 計算式属性														*/
/*		mode=1: 計算式アドレス属性、最終オブジェクト有り						*/
/*		mode=2: 定数															*/
/*	   (mode=3: 定数アドレス属性、最終オブジェクトのアドレス)					*/
/*		mode=4; $fp offset アドレス属性											*/
/*		mode=5: $s7 offset アドレス属性											*/
/*		mode=6: ラベルアドレス属性、value=label_numb							*/
/*		mode=7: レジスタ属性、(char*)&value   "s0" 等が入る						*/
/*																				*/
/*		value : 定数の時、定数を返すエリア										*/
/*		func が 0 の場合は global table のみ検索する							*/
/*		c32_factor1 , c32_factor1 , ... , c32_factor1							*/
/********************************************************************************/
void c32_expr(struct expr_param *expr_p1)
{
	int type;
	struct src_ptr src_ptr;
	
	c32_printf(";---------- c32_expr \"%s\" ----------\n", expr_p1->reg_ans);
	for(;;){
		c32_factor1(expr_p1);
		c32_src_ptr_save(&src_ptr);
		c32_token_process(0, &type);				// ',' テスト
		if(type==TYPE_KANMA){				// ',' だった
			continue;
		}
		c32_src_ptr_restore(&src_ptr);
		return;
	}
}
