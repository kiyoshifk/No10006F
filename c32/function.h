//		code.cpp
void constant_address_eq_reg(int attr, unsigned int constant, const char *reg);
void mem_eq_reg(int attr, const char *dst_reg, char *offset, const char *src_reg);
void reg_eq_constant_address(int attr, const char *reg, int constant);
void reg_eq_mem(int attr, const char *dst_reg, const char *src_reg, char *offset);

//		c32_expr.cpp
void convert_mode0(struct expr_param *expr_p1, int s_numb);
void convert_mode0_2(struct expr_param *expr_p1, int s_numb);
void convert_mode1(struct expr_param *expr_p1);
void c32_expr(struct expr_param *expr_p1);
//static void factor(struct expr_param *expr_p1);
void c32_factor1(struct expr_param *expr_p1);

//		c32_expr_eq.cpp
int c32_expr_and_eq(struct expr_param *expr_p1, struct expr_param *expr_p2);
int c32_expr_asterisk_eq(struct expr_param *expr_p1, struct expr_param *expr_p2);
int c32_expr_eq(struct expr_param *expr_p1, struct expr_param *expr_p2);
int c32_expr_l_shift_eq(struct expr_param *expr_p1, struct expr_param *expr_p2);
int c32_expr_minus_eq(struct expr_param *expr_p1, struct expr_param *expr_p2);
int c32_expr_or_eq(struct expr_param *expr_p1, struct expr_param *expr_p2);
int c32_expr_percent_eq(struct expr_param *expr_p1, struct expr_param *expr_p2);
int c32_expr_plus_eq(struct expr_param *expr_p1, struct expr_param *expr_p2);
int c32_expr_r_shift_eq(struct expr_param *expr_p1, struct expr_param *expr_p2);
int c32_expr_slush_eq(struct expr_param *expr_p1, struct expr_param *expr_p2);
int c32_expr_xor_eq(struct expr_param *expr_p1, struct expr_param *expr_p2);

//		token.cpp
char c32_spskip();
int c32_is_sym_top();
void c32_getsym();
//int hexcode(int c);
struct symtbl *c32_token_process(struct symtbl *func, int *type);

//		useless.cpp
char *c32_attr_to_string(int attr);
void c32_symtbl_dump_auto(struct symtbl *func);
void c32_symtbl_dump_global();
char *c32_type_to_string(uchar type);

//		utility.cpp
int  c32_attr_to_byte(int attr);
int  c32_attr_to_shift_bit(int attr);
char *c32_malloc(unsigned int size);
uint *c32_malloc4(uint size);
int  c32_char_short_int_unsigned_pointer();
void c32_error_message(int err_no, int line, char *filename);
int  c32_make_attr_from_constant(int constant);
void c32_output_buffer_flush();
int c32_printf(const char *fmt, ...);
void c32_skip_to_R_KAKKO(int cnt);
int  c32_skip_to_type(int type0, int type1, int type2);
void c32_src_ptr_restore(struct src_ptr *src_ptr);
void c32_src_ptr_save(struct src_ptr *src_ptr);
struct symtbl *c32_sym_search_all(struct symtbl *func);
struct symtbl *c32_sym_search_auto(struct symtbl *func);
struct symtbl *c32_sym_search_global();
void c32_symbol_out();
#ifdef __XC32
void c32_fprintf(SYS_FS_HANDLE fd, const char *fmt, ...);
#endif