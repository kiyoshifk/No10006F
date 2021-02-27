#include "stdafx.h"
#include "banana-c32.h"
#include "function.h"


/********************************************************************************/
/*		c32_spskip																	*/
/*		テキスト中の '\n' は '\0' に置き替えられている、'\0' が２つで終了		*/
/********************************************************************************/
char c32_spskip()
{
	uint len;
	char *pt;
	
	while(*c32_linebufp==' ' || *c32_linebufp=='\t' || *c32_linebufp=='\0'){
		if(*c32_linebufp=='\0'){
			c32_linebufp++;
			if(*c32_linebufp=='\0'){
#ifdef __XC32
                if(SYS_FS_FileStringGet(c32_src_fp, cv->tmp, MAX_STRING_LEN)==SYS_FS_RES_SUCCESS){
#else
				if(fgets(cv->tmp, MAX_STRING_LEN, c32_src_fp)){
#endif
					if(strlen(cv->tmp) + (c32_linebufp - cv->src_buffer) >= MAX_SRC_BUFFER_LEN-2){
						c32_error_message(E_OUT_OF_MEMORY, __LINE__, __FILE__);
					}
					if((int)(strlen(cv->tmp) + (c32_linebufp - cv->src_buffer)) > c32_max_src_buffer){
						c32_max_src_buffer = strlen(cv->tmp) + (c32_linebufp - cv->src_buffer);
					}

					strcpy(c32_linebufp, cv->tmp);
					if(c32_linebufp[strlen(c32_linebufp)-1] != '\n'){
						printf("%02x ", c32_linebufp[strlen(c32_linebufp)-1]);
						c32_error_message(E_INTERNAL_ERROR, __LINE__, __FILE__);
					}
					c32_linebufp[strlen(c32_linebufp)-1] = '\0';	// '\n' を'\0' にする

					pt = strstr(c32_linebufp, "#$&");
					if(pt){
						pt += 4;
						memcpy(c32_src_filename, pt, MAX_SYMBUF_LEN);
						len = strlen(pt)-3;
						if(len >= MAX_SYMBUF_LEN)
							len = MAX_SYMBUF_LEN-1;
						c32_src_filename[len] = 0;
					}
				}
				else{
					c32_linebufp--;
					break;
				}
			}
			c32_linebuf = c32_linebufp;
//			c32_printf(";%s\n", c32_linebuf);
			c32_printf(";");c32_printf(c32_linebuf);c32_printf("\n");
			c32_linebufp = strchr(c32_linebufp, ':');
			if(c32_linebufp==0){
				c32_error_message(E_INPUT_FILE_ERROR, __LINE__, __FILE__);
			}
		}
		c32_linebufp++;
	}
	return *c32_linebufp;
}
/********************************************************************************/
/*		c32_is_sym_top																*/
/********************************************************************************/
int c32_is_sym_top()
{
	c32_spskip();
	if(*c32_linebufp=='_' || *c32_linebufp=='@' || isalpha(*c32_linebufp))
		return 1;
	return 0;
}
/********************************************************************************/
/*		c32_getsym																	*/
/*		次の先頭が '_' かアルファベットであることを確認してから呼び出すこと		*/
/********************************************************************************/
void c32_getsym()
{
	int i;
	
	memset(c32_symbuf, 0, sizeof(c32_symbuf));
//	c32_spskip();
//	if(*c32_linebufp!='_' && *c32_linebufp!='@' && isalpha(*c32_linebufp)==0)
	if(c32_is_sym_top()==0)
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);	// error
	c32_symbuf[0] = '_';
	for(i=1; i<MAX_SYMBUF_LEN-1; i++){
		if(*c32_linebufp=='_' || *c32_linebufp=='@' || isalnum(*c32_linebufp))
			c32_symbuf[i] = *c32_linebufp++;	// toupper(*c32_linebufp++);
		else
			break;
	}
	if(i==MAX_SYMBUF_LEN-1)
		c32_error_message(E_TOO_LONG_SYMBOL, __LINE__, __FILE__);	// error
}
/********************************************************************************/
/*		sym_search																*/
/*		return 0～:tbl 番号、-1:未登録											*/
/********************************************************************************/
//int sym_search()
//{
//	int i;
//	
//	for(i=0; i<tbl_ptr; i++){
//		if(strcmp(c32_symbuf, cv->tbl[i].c32_symbuf)==0)
//			return i;						// シンボルテーブル発見
//	}
//	return -1;								// 未登録
//}
/********************************************************************************/
/*		hexcode																	*/
/********************************************************************************/
static int hexcode(int c)
{
	if(isdigit(c)){
		return c-'0';
	}
	else if(c>='A' && c<='F'){
		return c - 'A' + 10;
	}
	else if(c>='a' && c<='f'){
		return c - 'a' + 10;
	}
	else{
		c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
	}
	return 0;
}
/********************************************************************************/
/*		token																	*/
/*		return 0:未登録 symbol 又はシンボルでない								*/
/*		type=TYPE_STRING の時は cv->string_buf[] に文字列を入れる				*/
/*		type=TYPE_DIGIT  の時は *((uint*)cv->string_buf) に数値を入れる			*/
/*		シンボルテーブルサーチは初めに func->func_tbl をサーチする				*/
/*		  func=0 なら global テーブルのみサーチする								*/
/********************************************************************************/
struct symtbl *c32_token_process(struct symtbl *func, int *type)
{
	int value;
	struct symtbl *ptr;
	
		c32_spskip();
		for(;;){
			/***	シンボル処理	***/
			if(c32_is_sym_top()){				// symbol
				c32_getsym();
				if(func)
					ptr = c32_sym_search_all(func);
				else
					ptr = c32_sym_search_global();
				if(ptr){					// シンボルテーブルに登録されていた
					if(ptr->type==TYPE_SYMBOL){
						/***	シンボル	***/
						*type = TYPE_SYMBOL;
						return ptr;			// success
					}
					else{
						/***	予約語	***/
						*type = ptr->type;
						return 0;
					}
				}
				// undefined symbol
				*type = TYPE_SYMBOL;
				return 0;						// success
			}
			else if(*c32_linebufp=='{'){			// {
				c32_linebufp++;
				*type = TYPE_L_NAMIKAKKO;
				return 0;						// success
			}
			else if(*c32_linebufp=='}'){			// }
				c32_linebufp++;
				*type = TYPE_R_NAMIKAKKO;
				return 0;						// success
			}
			else if(*c32_linebufp=='['){			// [
				c32_linebufp++;
				*type = TYPE_L_KAKUKAKKO;
				return 0;						// success
			}
			else if(*c32_linebufp==']'){			// ]
				c32_linebufp++;
				*type = TYPE_R_KAKUKAKKO;
				return 0;						// success
			}
			else if(*c32_linebufp==';'){			// ;
				c32_linebufp++;
				*type = TYPE_SEMIKORON;
				return 0;						// success
			}
			else if(*c32_linebufp==':'){			// :
				c32_linebufp++;
				*type = TYPE_KORON;
				return 0;						// success
			}
			else if(*c32_linebufp=='='){			// =
				c32_linebufp++;
				if(*c32_linebufp=='='){				// ==
					c32_linebufp++;
					*type = TYPE_EQEQ;
					return 0;
				}
				*type = TYPE_EQ;
				return 0;						// success
			}
			else if(*c32_linebufp=='!'){			// !
				c32_linebufp++;
				if(*c32_linebufp=='='){				// !=
					c32_linebufp++;
					*type = TYPE_NOTEQ;
					return 0;
				}
				else{
					*type = TYPE_NOT;
					return 0;
				}
			}
			else if(c32_linebufp[0]=='<' &&			// <<=
					c32_linebufp[1]=='<' &&
					c32_linebufp[2]=='='){
				c32_linebufp += 3;
				*type = TYPE_L_SHIFT_EQ;
				return 0;
			}
			else if(c32_linebufp[0]=='<' &&			// <<
					c32_linebufp[1]=='<'){
				c32_linebufp += 2;
				*type = TYPE_L_SHIFT;
				return 0;
			}
			else if(*c32_linebufp=='<'){			// <
				c32_linebufp++;
				if(*c32_linebufp=='='){				// <=
					c32_linebufp++;
					*type = TYPE_LE;
					return 0;
				}
				*type = TYPE_LT;
				return 0;
			}
			else if(c32_linebufp[0]=='>' &&			// >>=
					c32_linebufp[1]=='>' &&
					c32_linebufp[2]=='='){
				c32_linebufp += 3;
				*type = TYPE_R_SHIFT_EQ;
				return 0;
			}
			else if(c32_linebufp[0]=='>' &&			// >>
					c32_linebufp[1]=='>'){
				c32_linebufp += 2;
				*type = TYPE_R_SHIFT;
				return 0;
			}
			else if(*c32_linebufp=='>'){			// >
				c32_linebufp++;
				if(*c32_linebufp=='='){				// >=
					c32_linebufp++;
					*type = TYPE_GE;
					return 0;
				}
				*type = TYPE_GT;
				return 0;
			}
			else if(*c32_linebufp=='+' && *(c32_linebufp+1)=='='){	// +=
				c32_linebufp += 2;
				*type = TYPE_PLUS_EQ;
				return 0;
			}
			else if(*c32_linebufp=='+' && *(c32_linebufp+1)=='+'){	// ++
				c32_linebufp += 2;
				*type = TYPE_PLUS_PLUS;
				return 0;
			}
			else if(*c32_linebufp=='+'){						// +
				c32_linebufp++;
				*type = TYPE_PLUS;
				return 0;									// success
			}
			else if(*c32_linebufp=='-' && *(c32_linebufp+1)=='='){	// -=
				c32_linebufp += 2;
				*type = TYPE_MINUS_EQ;
				return 0;
			}
			else if(*c32_linebufp=='-' && *(c32_linebufp+1)=='-'){	// --
				c32_linebufp += 2;
				*type = TYPE_MINUS_MINUS;
				return 0;
			}
			else if(*c32_linebufp=='-'){						// -
				c32_linebufp++;
				*type = TYPE_MINUS;
				return 0;									// success
			}
			else if(c32_linebufp[0]=='^' &&						// ^=
					c32_linebufp[1]=='='){
				c32_linebufp += 2;
				*type = TYPE_XOR_EQ;
				return 0;
			}
			else if(c32_linebufp[0]=='^'){						// ^
				c32_linebufp++;
				*type = TYPE_XOR;
				return 0;
			}
			else if(*c32_linebufp=='&' && *(c32_linebufp+1)=='='){	// &=
				c32_linebufp += 2;
				*type = TYPE_AND_EQ;
				return 0;
			}
			else if(c32_linebufp[0]=='&' && c32_linebufp[1]=='&'){	// &&
				c32_linebufp += 2;
				*type = TYPE_AND_AND;
				return 0;
			}
			else if(*c32_linebufp=='&'){						// &
				c32_linebufp++;
				*type = TYPE_AND;
				return 0;									// success
			}
			else if(*c32_linebufp=='|' && *(c32_linebufp+1)=='='){	// |=
				c32_linebufp += 2;
				*type = TYPE_OR_EQ;
				return 0;
			}
			else if(c32_linebufp[0]=='|' && c32_linebufp[1]=='|'){	// ||
				c32_linebufp += 2;
				*type = TYPE_OR_OR;
				return 0;
			}
			else if(*c32_linebufp=='|'){						// |
				c32_linebufp++;
				*type = TYPE_OR;
				return 0;									// success
			}
			else if(*c32_linebufp=='?'){						// ?
				c32_linebufp++;
				*type = TYPE_QUESTION;
				return 0;
			}
			else if(*c32_linebufp=='~'){						// ~
				c32_linebufp++;
				*type = TYPE_TILDE;
				return 0;
			}
			else if(*c32_linebufp=='*' && *(c32_linebufp+1)=='='){	// *=
				c32_linebufp += 2;
				*type = TYPE_ASTERISK_EQ;
				return 0;
			}
			else if(*c32_linebufp=='*'){						// *
				c32_linebufp++;
				*type = TYPE_ASTERISK;
				return 0;									// success
			}
			else if(*c32_linebufp=='/' && *(c32_linebufp+1)=='='){	//  /=
				c32_linebufp += 2;
				*type = TYPE_SLUSH_EQ;
				return 0;
			}
			else if(*c32_linebufp=='/'){						// /
				c32_linebufp++;
				if(*c32_linebufp=='*'){							// /*     コメント行なのでスキップする
					c32_linebufp++;
					for(;;){
						if(c32_linebufp[0]=='\0'){
							c32_spskip();
							if(c32_linebufp[0]=='\0'){			// EOF エラー
								*type = TYPE_ERROR;
								return 0;
							}
							continue;
						}
						else if(c32_linebufp[0]=='*' && c32_linebufp[1]=='/'){
							c32_linebufp += 2;
							break;
						}
						c32_linebufp++;
					}
					continue;								// 次の token を取り込む
				}
				else if(*c32_linebufp=='/'){					// "//"
//					read_flag = 0;
					c32_linebufp += strlen(c32_linebufp);
					continue;
				}
				*type = TYPE_SLUSH;
				return 0;									// success
			}
			else if(*c32_linebufp=='%' && *(c32_linebufp+1)=='='){	// %=
				c32_linebufp += 2;
				*type = TYPE_PERSENT_EQ;
				return 0;
			}
			else if(*c32_linebufp=='%'){						// %
				c32_linebufp++;
				*type = TYPE_PERCENT;
				return 0;									// success
			}
			else if(*c32_linebufp=='('){						// (
				c32_linebufp++;
				*type = TYPE_L_KAKKO;
				return 0;									// success
			}
			else if(*c32_linebufp==')'){						// )
				c32_linebufp++;
				*type = TYPE_R_KAKKO;
				return 0;									// success
			}
			else if(*c32_linebufp==','){						// ,
				c32_linebufp++;
				*type = TYPE_KANMA;
				return 0;									// success
			}
			else if(*c32_linebufp=='\"'){						// "
				int cnt;
				
				c32_linebufp++;
				*type = TYPE_STRING;
				for(cnt=0; cnt<0x2000; cnt++){
					if(*c32_linebufp=='\"'){
						cv->string_buf[cnt] = '\0';
						cnt++;
						c32_linebufp++;
						break;
					}
					else if(*c32_linebufp=='\\'){
						c32_linebufp++;
						if(*c32_linebufp=='b'){
							cv->string_buf[cnt] = '\b';
							c32_linebufp++;
						}
						else if(*c32_linebufp=='n'){
							cv->string_buf[cnt] = '\n';
							c32_linebufp++;
						}
						else if(*c32_linebufp=='r'){
							cv->string_buf[cnt] = '\r';
							c32_linebufp++;
						}
						else if(*c32_linebufp=='t'){
							cv->string_buf[cnt] = '\t';
							c32_linebufp++;
						}
						else if(*c32_linebufp=='\\'){
							cv->string_buf[cnt] = '\\';
							c32_linebufp++;
						}
						else if(*c32_linebufp=='\''){
							cv->string_buf[cnt] = '\'';
							c32_linebufp++;
						}
						else if(*c32_linebufp=='\"'){
							cv->string_buf[cnt] = '\"';
							c32_linebufp++;
						}
						else if(*c32_linebufp=='x'){
							int tmp;
							c32_linebufp++;
							tmp = hexcode(*c32_linebufp++) * 16;
							tmp += hexcode(*c32_linebufp++);
							cv->string_buf[cnt] = tmp;
						}
						else{
error1:;
							*type = TYPE_ERROR;
							return 0;
							c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
						}
					}
					else{
						cv->string_buf[cnt] = *c32_linebufp++;
					}
				}
//				token->work = (void*)cv->string_buf;
//				token->work_cnt = cnt;
				return 0;
			}
			/***	定数	***/
			else if(isdigit(*c32_linebufp)){								// 0
//				linebufp_save = c32_linebufp;
				if(*c32_linebufp=='0' && *(c32_linebufp+1)=='x'){
					c32_linebufp+=2;
					value = 0;
					for(;;){
						if(isdigit(*c32_linebufp)){
							value = value * 16 + *c32_linebufp - '0';
						}
						else if(*c32_linebufp>='A' && *c32_linebufp<='F'){
							value = value * 16 + *c32_linebufp - 'A' + 10;
						}
						else if(*c32_linebufp>='a' && *c32_linebufp<='f'){
							value = value * 16 + *c32_linebufp - 'a' + 10;
						}
						else{
							break;
						}
						c32_linebufp++;
					}
					*((uint*)cv->string_buf) = value;
//					token->value = value;
					*type = TYPE_DIGIT;
				}
				else{
					value = 0;
					for(;;){
						if(isdigit(*c32_linebufp)){
							value = value * 10 + *c32_linebufp - '0';
						}
						else{
							break;
						}
						c32_linebufp++;
					}
					*((uint*)cv->string_buf) = value;
//					token->value = value;
					*type = TYPE_DIGIT;
				}
//				for(i=0; linebufp_save != c32_linebufp; i++){
//					token->c32_symbuf[i] = *linebufp_save++;
//				}
//				token->c32_symbuf[i] = '\0';
				return 0;
			}
			/***	文字定数	***/
			else if(*c32_linebufp=='\''){			// '
				c32_linebufp++;
				if(*c32_linebufp=='\\'){
					c32_linebufp++;
					if(*c32_linebufp=='b'){
						*((uint*)cv->string_buf) = '\b';
						c32_linebufp++;
					}
					else if(*c32_linebufp=='n'){
						*((uint*)cv->string_buf) = '\n';
						c32_linebufp++;
					}
					else if(*c32_linebufp=='r'){
						*((uint*)cv->string_buf) = '\r';
						c32_linebufp++;
					}
					else if(*c32_linebufp=='t'){
						*((uint*)cv->string_buf) = '\t';
						c32_linebufp++;
					}
					else if(*c32_linebufp=='\\'){
						*((uint*)cv->string_buf) = '\\';
						c32_linebufp++;
					}
					else if(*c32_linebufp=='\''){
						*((uint*)cv->string_buf) = '\'';
						c32_linebufp++;
					}
					else if(*c32_linebufp=='\"'){
						*((uint*)cv->string_buf) = '\"';
						c32_linebufp++;
					}
					else if(*c32_linebufp=='x'){
						int tmp;
						c32_linebufp++;
						tmp = hexcode(*c32_linebufp++) * 16;
						tmp += hexcode(*c32_linebufp++);
						*((uint*)cv->string_buf) = tmp;
					}
					else{
						goto error1;
						c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
					}
				}
				else{
					*((uint*)cv->string_buf) = *c32_linebufp++;
				}
				*type = TYPE_DIGIT;
				if(*c32_linebufp++ != '\''){
					goto error1;
					c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
				}
				return 0;
			}
			else if(*c32_linebufp=='\r'){
				c32_linebufp++;
			}
			else{
				if(*c32_linebufp=='\0'){
					*type = TYPE_EOF;
					return 0;
				}
				c32_linebufp++;
				goto error1;
				c32_error_message(E_SYNTAX_ERROR, __LINE__, __FILE__);
			}
		}
}
