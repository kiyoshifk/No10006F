#include "app.h"

#define MAX_SYMBUF_LEN		64


#define RAM_BASE			0xa0010000	// アプリ用のエリアを使用する為


char linebuf[128];
char *linebufp;
char symbuf[MAX_SYMBUF_LEN];


char *buffer = (char*)RAM_BASE;


const char *fs_error_msg[]={
	"OK",
	"DISK_ERR",
	"INT_ERR",
	"NOT_READY",
	"NO_FILE",
	"NO_PATH",
	"INVALID_NAME",
	"DENIED",
	"ERROR_EXIST",
	"INVALID_OBJECT",
	"WRITE_PROTECTED",
	"INVALID_DRIVE",
	"NOT_ENABLED",
	"NO_FILESYSTEM",
	"FORMAT_ABORTED",
	"TIMEOUT",
	"LOCKED",
	"NOT_ENOUGH_CORE",
	"TOO_MANY_OPEN_FILES",
	"INVALID_PARAMETER",
	"NOT_ENOUGH_FREE_VOLUME",
	"FS_NOT_SUPPORTED",
	"FS_NOT_MATCH_WITH_VOLUME",
	"NOT_SUPPORTED_IN_NATIVE_FS",
};


int edit_cur_line;
int on_ram_flag;

extern int APP_ch;
extern int APP_chA;

void SYS_Tasks ( void );

void dma_start();
void dma_stop();
void f_mgr_help();
int c32(const char *fname);

/********************************************************************************/
/*		ut_fs_error_msg															*/
//		?o??
/********************************************************************************/
void ut_error_msg(const char *head)
{
	ut_printf("%s: *** エラー %s\n", head, fs_error_msg[SYS_FS_Error()]);
}
/********************************************************************************/
/*		get_APP_chA																*/
/********************************************************************************/
int get_APP_chA()
{
    SYS_Tasks();
	return APP_chA;
}
/********************************************************************************/
/*		ut_getc																	*/
/*		fd: 1000=keyin, ??????:file input										*/
/*		return: EOF==-1, error=-2												*/
/********************************************************************************/
int ut_getc()
{
	int ch;

	while((ch=APP_ch)==0)
		SYS_Tasks();
	APP_ch = 0;
    buzzer(1000, 50);
	return ch;
}
/********************************************************************************/
/*		ut_gets																	*/
//		?o??
/********************************************************************************/
void ut_gets(char *buf, int maxlen)
{
	int ptr = 0;
	int ch;
	
	for(;;){
		ch = ut_getc();
		
        if(ch=='\n'){
			disp_1char(ch);
			buf[ptr++] = ch;
			buf[ptr] = '\0';
			return;
        }
		else if(ch=='\b'){
			if(ptr){
				ptr--;
                disp_L();
            }
		}
		else if(ptr < maxlen-2){
			if((ch>=0x20 && ch<0x100) || ch=='\t' || ch=='\n'){
				disp_1char(ch);
				buf[ptr++] = ch;
			}
		}
	}
}
/********************************************************************************/
/*		f_mgr_gets																*/
/*		通常: return 0,  F1～F12,ESC  は return でコードを返す					*/
/********************************************************************************/
int f_mgr_gets(char *buf, int maxlen)
{
	int ptr = 0;
	int ch;
	
	for(;;){
		ch = ut_getc();
		
        if(ch=='\n'){
			disp_1char(ch);
			buf[ptr++] = ch;
			buf[ptr] = '\0';
			return 0;
        }
		else if(ch=='\b'){
			if(ptr){
				ptr--;
                disp_L();
            }
		}
		else if(ch==ESC || ch>=0x100){
			buf[0] = '\0';
			return ch;
		}
		else if(ptr < maxlen-2){
			if((ch>=0x20 && ch<0x100) || ch=='\t' || ch=='\n'){
				disp_1char(ch);
				buf[ptr++] = ch;
				if(ch=='\n'){
					buf[ptr] = '\0';
					return 0;
				}
			}
		}
	}
}
/********************************************************************************/
/*      ut_putc                                                                 */
/********************************************************************************/
void ut_putc(char c)
{
    disp_1char(c);
}
/********************************************************************************/
/*		ut_puts																	*/
/********************************************************************************/
void ut_puts(const char *str)
{
	disp_str(str);
}
/********************************************************************************/
/*		spskip																	*/
/********************************************************************************/
static char spskip()
{
	while(1){
		if(*linebufp==' ' || *linebufp=='\t'){
			linebufp++;
		}
		else{
			return *linebufp;
		}
	}
}
/********************************************************************************/
/*		getsym																	*/
/********************************************************************************/
static void getsym()
{
	int i;
	
	memset(symbuf, 0, sizeof(symbuf));
	spskip();
	for(i=0; i<MAX_SYMBUF_LEN-1; i++){
		if(*linebufp==' ' || *linebufp=='\t' || *linebufp=='\0' || *linebufp=='\n'){
			return;
		}
		symbuf[i] = *linebufp++;
	}
}
/********************************************************************************/
/*		f_mgr_help																*/
/********************************************************************************/
void f_mgr_help()
{
	ut_printf("\n");
	ut_printf("F1                         ... ヘルプメッセージ表\示\n");
	ut_printf("ls <directory name>        ... ディレクトリリスト\n");
	ut_printf("cd <directory name>        ... ディレクトリ変更\n");
	ut_printf("rm <file name>             ... ファイル削除\n");
	ut_printf("dump <address> <byte>      ... ヘキサダンプ\n");
    ut_printf("edit <txt file>            ... エディター\n");
    ut_printf("asm32 <src file>           ... アッセンブル\n");
    ut_printf("banana-c32 <src file>      ... コンパイル\n");
//	ut_printf("run (<hex address>)        ... program execute\n");
	ut_printf("run <hex file>             ... プログラム実行\n");
	ut_printf("gxh                        ... ゼネラルエクセプションハンドラー\n");
}
/********************************************************************************/
/*		f_mgr_edit																*/
/********************************************************************************/
void f_edit_help()
{
	ut_printf("\n");
	ut_printf("F1                       ... ヘルプメッセージ表\示\n");
	ut_printf("c<line>                  ... １行修正\n");
	ut_printf("                  DEL        １文字スキップ\n");
	ut_printf("                  RIGHT      １文字コピー\n");
	ut_printf("                  END        １行コピー\n");
	ut_printf("i<line>                  ... 挿入\n");
	ut_printf("d<start> (<end>)         ... 行削除\n");
	ut_printf("l(<start> (<end>))       ... リスト表\示\n");
	ut_printf("s<string>                ... 文字列検索\n");
	ut_printf("<line numb>              ... 現在行セット\n");
	ut_printf("e(<file name>)           ... 書き込んで終了\n");
	ut_printf("w(<file name>)           ... 書き込み\n");
	ut_printf("q                        ... 中断\n");
}

int f_edit_search(int search_numb, int *ptr)
{
	int result_numb;
	
	*ptr = 0;
	result_numb = 1;
	for(;;){
		if(result_numb==search_numb)
			return result_numb;
		for(;;){
			if(buffer[*ptr]=='\0')
				return result_numb;
			if(buffer[*ptr]=='\n'){
				(*ptr)++;
				break;
			}
			(*ptr)++;
		}
		result_numb++;
	}
}

void f_edit_l()
{
	int start_line, end_line, line_numb, ptr;
	char *pt;
	
	getsym();
	start_line = atoi(symbuf);
	getsym();
	end_line = atoi(symbuf);
	if(start_line==0){
		start_line = end_line = edit_cur_line;
	}
	else if(end_line==0){
		end_line = edit_cur_line + start_line;
		start_line = edit_cur_line;
		edit_cur_line = end_line;
	}
	else{
		edit_cur_line = end_line;
	}
	ptr = 0;
	line_numb = 1;
	for(;;){
		if(buffer[ptr]=='\0')
			break;
		if(line_numb>=start_line && line_numb<=end_line){
			ut_printf("%6d: ", line_numb);
			pt = strchr(&buffer[ptr], '\n');
			if(pt==0){
				ut_printf("%s\n", &buffer[ptr]);
			}
			else{
				*pt = 0;
				ut_printf("%s\n", &buffer[ptr]);
				*pt = '\n';
			}
//			for(i=0; ; i++){
//				if(buffer[ptr+i]=='\0')
//					break;
//				ut_putc(buffer[ptr+i]);
//				if(buffer[ptr+i]=='\n')
//					break;
//			}
		}
		for(;;){
			if(buffer[ptr]=='\0')
				break;
			if(buffer[ptr]=='\n'){
				ptr++;
				break;
			}
			ptr++;
		}
		line_numb++;
	}
}

void f_edit_e(const char *filename)
{
	int len;
	SYS_FS_HANDLE fd;
	
	fd = SYS_FS_FileOpen(filename, SYS_FS_FILE_OPEN_WRITE);
	if(fd == SYS_FS_HANDLE_INVALID){
		ut_error_msg("open");
		return;
	}
	len = strlen(buffer);
	if(SYS_FS_FileWrite(fd, buffer, len) != len){
		ut_printf("*** file write error\n");
	}
	else{
		ut_printf("=== write %dbyte\n", len);
	}
	
	SYS_FS_FileClose(fd);
}

void f_edit_i()
{
	int insert_numb, ptr, ch;
	
	getsym();
	insert_numb = atoi(symbuf);
	if(insert_numb==0)
		insert_numb = edit_cur_line;
	ptr = 0;
	insert_numb = f_edit_search(insert_numb, &ptr);

	for(;;){
		ut_printf("%6d: ", insert_numb);
		ch = f_mgr_gets(linebuf, sizeof(linebuf));
		if(ch){								// function key
			if(ch==ESC){
				ut_putc('\n');
				return;
			}
			continue;
		}
		if(strlen(linebuf)+strlen(buffer)+1 > 0x10000){
			ut_printf("*** out of memory, 0x10000 byte over\n");
			return;
		}
		memmove(&buffer[ptr + strlen(linebuf)], &buffer[ptr], strlen(&buffer[ptr])+1);
		memmove(&buffer[ptr], linebuf, strlen(linebuf));
		ptr += strlen(linebuf);
		insert_numb++;
	}
}

void f_edit_d()
{
	int start_line, end_line, start_ptr, end_ptr;
	
	getsym();
	start_line = atoi(symbuf);
	getsym();
	end_line = atoi(symbuf);
	if(end_line==0)
		end_line = start_line;
	if(start_line==0){
		start_line = end_line = edit_cur_line;
	}
	if(start_line > end_line){
		ut_printf("*** error\n");
		return;
	}
	end_line++;
	edit_cur_line = end_line;
	f_edit_search(start_line, &start_ptr);
	f_edit_search(end_line, &end_ptr);
	memmove(&buffer[start_ptr], &buffer[end_ptr], strlen(&buffer[end_ptr])+1);
	ut_printf("=== success\n");
}

void f_edit_c()
{
	int i, line_numb, ptr, c, ch, p, lp1, lp2;
	
	getsym();
	line_numb = atoi(symbuf);
	if(line_numb==0){
		line_numb = edit_cur_line;
	}
	line_numb = f_edit_search(line_numb, &ptr);
	edit_cur_line = line_numb;
	f_edit_search(line_numb, &lp1);
	f_edit_search(line_numb+1, &lp2);
	p = 0;
	linebuf[0] = '\0';
	ut_printf("%6d: ", line_numb);
	for(i=0; ; i++){
		if(buffer[ptr+i]=='\n' || buffer[ptr+i]=='\0'){
			ut_putc('\n');
			break;
		}
		ut_putc(buffer[ptr+i]);
	}
	ut_printf("%6d: ", line_numb);
	for(;;){
		ch = ut_getc();
		if(ch==DEL){							// skip charactor
			if(buffer[ptr] && buffer[ptr]!='\n'){
				ptr++;
			}
		}
		else if(ch==RIGHT){					// copy charactor
			if(buffer[ptr] && buffer[ptr]!='\n'){
				if(p <= sizeof(linebuf)-2){
					c = buffer[ptr++];
					linebuf[p++] = c;
					linebuf[p] = '\0';
					ut_putc(c);
				}
			}
		}
		else if(ch==END){					// copy to EOL
			while(buffer[ptr] && buffer[ptr]!='\n'){
				if(p <= sizeof(linebuf)-2){
					c = buffer[ptr++];
					linebuf[p++] = c;
					linebuf[p] = '\0';
					ut_putc(c);
				}
				else{
					break;
				}
			}
		}
		else if(ch=='\n'){
			ut_putc(ch);
			if(p <= sizeof(linebuf)-2){
				linebuf[p++] = ch;
				linebuf[p] = '\0';
				memmove(&buffer[lp1] , &buffer[lp2], strlen(&buffer[lp2])+1);
				if(strlen(buffer)+strlen(linebuf) >= 0x10000){
					ut_printf("*** out of memory, over 0x10000byte\n");
					return;
				}
				memmove(&buffer[lp1+strlen(linebuf)], &buffer[lp1], strlen(&buffer[lp1])+1);
				memmove(&buffer[lp1], linebuf, strlen(linebuf));
				return;
			}
			else{
				ut_printf("*** too long line\n");
				return;
			}
		}
		else if((ch>=0x20 && ch<0x80) || ch=='\t'){	// 一般の文字
			if(p <= sizeof(linebuf)-2){
				linebuf[p++] = ch;
				linebuf[p] = '\0';
				ut_putc(ch);
			}
		}
	}
}

void f_edit_s()
{
	int line_numb, ptr;
	char *p, *pp;
	
	getsym();								// symbuf: search string
	f_edit_search(edit_cur_line+1, &ptr);
	p = strstr(&buffer[ptr], symbuf);
	if(p==0){
		ut_printf("*** not found\n");
		return;
	}
	pp = buffer;
	for(line_numb=1; ; line_numb++){
		pp = strchr(pp, '\n');
		if(pp==0){
			break;
		}
		if(pp>p){
			break;
		}
		++pp;
	}
	edit_cur_line = line_numb;
	ut_printf("=== found\n");
}

void f_mgr_edit()
{
    int len, ch;
    char filename[MAX_SYMBUF_LEN];
	SYS_FS_HANDLE fd;
	
	ut_printf("\nF1 キーを押すとヘルプメッセージが出ます\n");
	edit_cur_line = 1;
	getsym();								// ファイル名取り込み
    strcpy(filename, symbuf);
	fd = SYS_FS_FileOpen(filename, SYS_FS_FILE_OPEN_READ);
	if(fd == SYS_FS_HANDLE_INVALID){
		buffer[0] = '\0';
		ut_printf("新規ファイル\n");
	}
	else{									// open success
		len = SYS_FS_FileRead(fd, buffer, 0x10000);
		SYS_FS_FileClose(fd);
		
		if(len<0){
			ut_error_msg("Read");
			return;
		}
		if(len >= 0x10000){
			ut_printf("*** too large file\n");
			return;
		}
		buffer[len] = '\0';
	}

	for(;;){
		ut_printf("> ");
		ch = f_mgr_gets(linebuf, sizeof(linebuf));
		if(ch){
			if(ch==F1)
				f_edit_help();
			ut_putc('\n');
			continue;
		}
		linebufp = linebuf;
		if(isdigit(*linebufp)){		// 行番号指定
			edit_cur_line = atoi(linebufp);
			ut_printf("=== 現在行 = %d\n", edit_cur_line);
			continue;
		}
		ch = *linebufp++;
//		getsym();
		
		if(ch=='\0'){
			continue;
		}
		else if(ch=='s'){	// search
			f_edit_s();
		}
		else if(ch=='c'){	// 1 line edit
			f_edit_c();
		}
		else if(ch=='i'){	// insert
			f_edit_i();
		}
		else if(ch=='d'){	// delete
			f_edit_d();
		}
		else if(ch=='l'){	// list
			f_edit_l();
		}
		else if(ch=='w'){	// write to file
			getsym();
			if(symbuf[0])
				strcpy(filename, symbuf);
			f_edit_e(filename);
		}
		else if(ch=='e'){	// end of editor
			getsym();
			if(symbuf[0])
				strcpy(filename, symbuf);
			f_edit_e(filename);
			return;
		}
		else if(ch=='q'){	// quit
			ut_printf("=== 保存せずに中断しました\n");
			return;
		}
		else{
			ut_printf("%s\n*** error\n", linebuf);
		}
	}
}
/********************************************************************************/
/*		f_mgr_ls																*/
/********************************************************************************/
void f_mgr_ls_sub(SYS_FS_FSTAT *stat)
{
	uint8_t fattrib = stat->fattrib;
	char *name;
	
	if(*stat->lfname==0)
		name = stat->fname;
	else
		name = stat->lfname;
	
	if(fattrib & SYS_FS_ATTR_DIR){
		ut_printf("<DIR>      %s\n", name);
	}
	else{
		ut_printf("%10d %s\n", stat->fsize, name);
	}
}

void f_mgr_ls()
{
//	char buf[256];
	SYS_FS_HANDLE fd_dir;
	SYS_FS_FSTAT stat;
	char lfname[300];

	getsym();							// ?f?B???N?g????
	stat.lfname = lfname;				// long file name
	stat.lfsize = 300;
	if((fd_dir = SYS_FS_DirOpen(symbuf))==SYS_FS_HANDLE_INVALID){
		ut_error_msg("DirOpen");
		return;							// error
	}
//	buf[256-1] = 0;
//	if(SYS_FS_CurrentWorkingDirectoryGet(buf, 256-1)==SYS_FS_RES_SUCCESS){
//		ut_printf("Directory: %s\n", buf);
//	}
//	else{
//		ut_error_msg("DirectoryGet");
//		return;
//	}
	for(;;){
		if(SYS_FS_DirSearch(fd_dir, "*", SYS_FS_ATTR_MASK, &stat)== SYS_FS_RES_FAILURE){
			SYS_FS_DirClose(fd_dir);
			return;					// ?I??
		}
		f_mgr_ls_sub(&stat);			// 1 ???C???\??
	}
}
/********************************************************************************/
/*		f_mgr_cd																*/
/********************************************************************************/
void f_mgr_cd()
{
    getsym();
	if(SYS_FS_DirectoryChange(symbuf)==SYS_FS_RES_SUCCESS){
		ut_printf("=== success\n");
		return;
	}
	ut_error_msg("DirectoryChange");
}
/********************************************************************************/
/*		f_mgr_rm																*/
/********************************************************************************/
void f_mgr_rm()
{
	SYS_FS_RESULT result;
	
	getsym();
	result = SYS_FS_FileDirectoryRemove(symbuf);
	if(result== SYS_FS_RES_FAILURE){
		ut_error_msg("Remove");
	}
	else{
		ut_printf("=== success\n");
	}
}
/********************************************************************************/
/*		f_mgr_asm32																*/
/********************************************************************************/
void f_mgr_asm32()
{
	getsym();
	if(asm32(symbuf)){
		ut_printf("*** error\n");
	}
	else{
		ut_printf("=== success\n");
	}
}
/********************************************************************************/
/*		f_mgr_prep																*/
/********************************************************************************/
void f_mgr_prep()
{
	getsym();
	if(prep(symbuf)){
		ut_printf("*** error\n");
	}
	else{
		ut_printf("=== success\n");
	}
}
/********************************************************************************/
/*		f_mgr_erase																*/
/********************************************************************************/
//
//	512バイト書き込み
//	return 0:suceess
//
int f_mgr_prg_sub(int rom_addr, int ram_addr)
{
	if(on_ram_flag==0){
	    ut_printf("flush writer addr = %08X\n", rom_addr);
		dma_stop();
		
	    __builtin_disable_interrupts();
	
		// 書き込みモードの指定
		// 0001 - 4バイト単位で書き込む
		// 0011 - 512バイト単位で書き込む（指定アドレスは512の倍数）
		// 0100 - 4KB単位で消去
		// 0101 - プログラム領域をすべて消去（ブート領域でのみ実行可）
	    NVMCONbits.NVMOP = 0b0011; // 0011 - 512バイト単位で書き込む（指定アドレスは512の倍数）
	
	    NVMCONbits.WREN = 1;					// ??????????????
	
	    NVMADDR = rom_addr & 0x1fffffff;		// ?????????????¨???A?h???X
	    NVMSRCADDR = ram_addr & 0x1fffffff;		// ???f?[?^?A?h???X
	//	NVMDATA = 0x12345678;					// ???????????￠?l
	
	    NVMKEY = 0xAA996655;					// ???b?N?????????????????L?[???o?^
	    NVMKEY = 0x556699AA;
	
		// ?????????????s?B?uNVMCONbits.WR = 1?v?????????????????s?????????????B
	    NVMCONSET = 0x8000;
	    while (NVMCONbits.WR);					// ?????????????t???O?????Z?b?g??????????????
	    
	    NVMCONbits.WREN = 0;					// ?????????????~????
	
	    __builtin_enable_interrupts();			// ????????????
	
		dma_start();
	    return (NVMCON & 0x3000);				// NVMERR?ALVDERR???G???[?t???O????????0???O??????
	}
	else{
		memcpy((char*)rom_addr, (char*)ram_addr, 512);
		return 0;
	}
}

//
//	4KB 消去
//	return 0:suceess
//
int f_mgr_erase_sub(int addr)
{

	dma_stop();
	
    __builtin_disable_interrupts();			// ???????????~

	// 書き込みモードの指定
	// 0001 - 4バイト単位で書き込む
	// 0011 - 512バイト単位で書き込む（指定アドレスは512の倍数）
	// 0100 - 4KB単位で消去
	// 0101 - プログラム領域をすべて消去（ブート領域でのみ実行可）
    NVMCONbits.NVMOP = 0b0100; // 0100 - 4KB単位で消去

    NVMCONbits.WREN = 1;					// ??????????????

    NVMADDR = addr & 0x1fffffff;			// ?????????????¨???A?h???X
//	NVMDATA = 0x12345678;					// ???????????￠?l

    NVMKEY = 0xAA996655;					// ???b?N?????????????????L?[???o?^
    NVMKEY = 0x556699AA;

	// ?????????????s?B?uNVMCONbits.WR = 1?v?????????????????s?????????????B
    NVMCONSET = 0x8000;
    while (NVMCONbits.WR);					// ?????????????t???O?????Z?b?g??????????????
    
    NVMCONbits.WREN = 0;					// ?????????????~????

    __builtin_enable_interrupts();			// ????????????

	dma_start();
    return (NVMCON & 0x3000);				// NVMERR?ALVDERR???G???[?t???O????????0???O??????
}

void f_mgr_erase()
{
	int i, err;
	int addr = 0x9d000000;
	
//	for(i=0; i<0x40; i++){
//		ut_printf("%02X%c", ptr[i], i%16==15?'\n':' ');
//	}
	ut_printf("=== eraseing addr=%08X - %08X\n", addr, addr+0x40000);
	for(i=0; i<0x40000; i+=4096){
		if((err=f_mgr_erase_sub(addr+i))){
			ut_printf("*** erase error addr=%08X err=%08X\n", addr+i, err);
			return;
		}
//        wait_ms(1);
	}
	ut_printf("=== erase success\n");
}
/********************************************************************************/
/*		f_mgr_prg																*/
/********************************************************************************/
unsigned int f_mgr_prg_s_addr;
int f_mgr_prg_buf[512/sizeof(int)];
int f_mgr_prg_buf_addr;
int f_mgr_prg_buf_addr_valid;

int f_mgr_prg_write(unsigned int addr, char data)
{
	unsigned int start, offset;
	
//    if(addr<0x9d000000 || addr>=0x9d040000)
//        return 0;                           // success アドレス外
	start = addr/512*512;
	offset = addr - start;
	if(f_mgr_prg_buf_addr_valid==0){		// ?V???￠????
		f_mgr_prg_buf_addr_valid = 1;
		memset(f_mgr_prg_buf, 0xff, sizeof(f_mgr_prg_buf));
		f_mgr_prg_buf_addr = start;
		((char*)f_mgr_prg_buf)[offset] = data;
	}
	else{
		if(f_mgr_prg_buf_addr != start){	// ?o?b?t?@?t???b?V??
			if(f_mgr_prg_sub(f_mgr_prg_buf_addr, (int)f_mgr_prg_buf)){	// error
				ut_printf("*** flush memory write error\n");
				return 1;					// error
			}
			memset(f_mgr_prg_buf, 0xff, sizeof(f_mgr_prg_buf));
			f_mgr_prg_buf_addr = start;
		}
		((char*)f_mgr_prg_buf)[offset] = data;
	}
	return 0;								// success
}

int hex2dec(char h)
{
	if(h>='0' && h<='9')
		return h-'0';
	else if(h>='A' && h<='F')
		return h-'A'+10;
	else if(h>='a' && h<='f')
		return h-'a'+10;
	else{
		ut_printf("*** hex2dec illegal charactor %02x\n", h);
		return 0;
	}
}

int s_chksum(unsigned char *buf, int byte)
{
	int i, sum;
	
	sum = 0;
	for(i=1; i<=byte+1; i++){
		sum += hex2dec(buf[i*2])*16 + hex2dec(buf[i*2+1]);
	}
	if((sum & 0xff) != 0xff){
		ut_printf("*** error chksum, %s %02x\n", buf, sum);
		return 0;							// error
	}
	return 1;								// success
}

int s_file_loader(char *filename)
{
	SYS_FS_HANDLE fd_file_in;
	unsigned char buf[256];
	int i, byte, err, data;
	unsigned int addr;
		
	err = 1;										// preset success
	fd_file_in = SYS_FS_FileOpen(filename, SYS_FS_FILE_OPEN_READ);
	if(fd_file_in == SYS_FS_HANDLE_INVALID){
        ut_error_msg("open");
//		ut_printf("*** %s open error, %s\n", filename, ut_error_msg());
		return 0;					// error
	}
	for(;;){
		if(SYS_FS_FileStringGet(fd_file_in, (char*)buf, 256) != SYS_FS_RES_SUCCESS)
			break;
//		ut_fprintf(1000, "%s", buf);
		if(buf[0] != 'S'){
			ut_printf("*** top charactor not \'S\'\n");
			ut_printf("%s\n", buf);
			continue;
		}
		switch(buf[1]){
		case '0':						// ?R?????g???t?@?C???????L?q?????t?B?[???h
			byte = hex2dec(buf[2])*16 + hex2dec(buf[3]);
			if(s_chksum(buf, byte)==0){	// if error
				err = 0;				// error
				break;
			}
			break;
		case '1':						// ?f?[?^?????????Q?o?C?g?A?h???X
			byte = hex2dec(buf[2])*16 + hex2dec(buf[3]);
			if(s_chksum(buf, byte)==0){	// if error
				err = 0;				// error
				break;
			}
			addr = 0;
			for(i=0; i<4; i++){
				addr = addr*16 + hex2dec(buf[i+4]);
			}
			for(i=4; i<4+byte-3; i++){
				data = hex2dec(buf[i*2])*16 + hex2dec(buf[i*2+1]);
//                *(unsigned char *)addr++ = data;								//####################
				if(f_mgr_prg_write(addr++, data)){
					err = 0;			// error
					break;
				}
			}
			break;
		case '3':						// ?f?[?^?????????S?o?C?g?A?h???X
			byte = hex2dec(buf[2])*16 + hex2dec(buf[3]);
			if(s_chksum(buf, byte)==0){	// if error
				err = 0;				// error
				break;
			}
			addr = 0;
			for(i=0; i<8; i++){
				addr = addr*16 + hex2dec(buf[i+4]);
			}
			for(i=6; i<6+byte-5; i++){
				data = hex2dec(buf[i*2])*16 + hex2dec(buf[i*2+1]);
//                *(unsigned char *)addr++ = data;								//#####################
				if(f_mgr_prg_write(addr++, data)){
					err = 0;			// error
					break;
				}
			}
			break;
		case '7':
			byte = hex2dec(buf[2])*16 + hex2dec(buf[3]);
			if(s_chksum(buf, byte)==0){	// if error
				err = 0;				// error
				break;
			}
			addr = 0;
			for(i=0; i<8; i++){
				addr = addr*16 + hex2dec(buf[i+4]);
			}
			f_mgr_prg_s_addr = addr;
			ut_printf("=== start addr=%08x\n", addr);
            break;
		case '9':
			byte = hex2dec(buf[2])*16 + hex2dec(buf[3]);
			if(s_chksum(buf, byte)==0){	// if error
				err = 0;				// error
				break;
			}
			addr = 0;
			for(i=0; i<4; i++){
				addr = addr*16 + hex2dec(buf[i+4]);
			}
			f_mgr_prg_s_addr = addr;
			ut_printf("=== start addr=%04x\n", addr);
            break;
		}
	}
//	for(i=0; i<0x100; i+=0x10){
//		ut_printf("%04x ", i);
//		for(byte=0; byte<0x10; byte++){
//			ut_printf(" %02x", dos[i+byte]);
//		}
//		ut_printf("\n");
//	}
	
	SYS_FS_FileClose(fd_file_in);
	if(f_mgr_prg_sub(f_mgr_prg_buf_addr, (int)f_mgr_prg_buf)){
		ut_printf("*** flush memory write error\n");
		return 0;						// error
	}

	return err;							// 1:success 0:error
}

void f_mgr_prg()
{
	on_ram_flag = 0;
	getsym();								// symbuf = hex file name
	f_mgr_prg_buf_addr_valid = 0;
	if(s_file_loader(symbuf)){
		ut_printf("=== program write success\n");
		ut_printf("start addr = 0x%08X\n", f_mgr_prg_s_addr);
	}
	else{
		ut_printf("*** program write error\n");
	}

    int i;
    
    for(i=0; i<256; i++){
        ut_printf("%02X%c", ((unsigned char *)0x9d000000)[i], i%16==15?'\n':' ');
    }
}
/********************************************************************************/
/*		f_mgr_load																*/
/********************************************************************************/
typedef int (*f_mgr_func1)();

void f_mgr_load()
{
	on_ram_flag = 1;
	getsym();								// symbuf = hex file name
	f_mgr_prg_buf_addr_valid = 0;
	if(s_file_loader(symbuf)){
		ut_printf("=== program write success\n");
		ut_printf("start addr = 0x%08X\n", f_mgr_prg_s_addr);
	}
	else{
		ut_printf("*** program write error\n");
		return;
	}
    BMXDKPBA = 0x14000;
    BMXDUDBA = 0x20000;
    BMXDUPBA = 0x20000;
	
    int ret;
    f_mgr_func1 s_func;
    
    s_func = (f_mgr_func1)f_mgr_prg_s_addr;
    ret = s_func();
    ut_printf("ret = 0x%08X : %d\n", ret, ret);
}
/********************************************************************************/
/*		f_mgr_run																*/
/********************************************************************************/
void f_mgr_run()
{
    int i, ret;
    f_mgr_func1 s_func;
    
    getsym();
    if(symbuf[0]){
		f_mgr_prg_s_addr = 0;
		for(i=0; ; i++){
			if(symbuf[i]==0)
				break;
			f_mgr_prg_s_addr *= 16;
			f_mgr_prg_s_addr += hex2dec(symbuf[i]);
		}
	}
    s_func = (f_mgr_func1)f_mgr_prg_s_addr;
    ret = s_func();
    ut_printf("ret = 0x%08X : %d\n", ret, ret);
}
/********************************************************************************/
/*		f_mgr_gethex															*/
/*		str を hex data に変換する												*/
/********************************************************************************/
unsigned int f_mgr_gethex(const char *str)
{
	unsigned int i, sum;
	
	for(i=sum=0; ; i++){
		if(str[i]==0)
			return sum;
		sum *= 16;
		sum += hex2dec(str[i]);
	}
}
/********************************************************************************/
/*		f_mgr_dump																*/
/********************************************************************************/
void f_mgr_dump()
{
	unsigned int addr, byte;
	
	getsym();
	addr = f_mgr_gethex(symbuf);
	getsym();
	byte = f_mgr_gethex(symbuf);
	for(;;){
		ut_printf("%08X ", addr);
		for(; byte; ){
			byte--;
			ut_printf(" %02X", *(unsigned char *)addr);
			if((++addr & 0x0f)==0)
				break;
		}
        ut_printf("\n");
        if(byte==0)
            break;
	}
}
/********************************************************************************/
/*		f_mgr_c32																*/
/********************************************************************************/
void f_mgr_c32()
{
	char *ptr;
	
	getsym();
	if(prep(symbuf)){
		ut_printf("*** error\n");
		return;
	}
	ptr = strrchr(symbuf, '.');
	if(ptr)
		*ptr = 0;
	strcat(symbuf, ".prep");
	
	if(c32(symbuf)){
		ut_printf("*** error\n");
		return;
	}
	ptr = strrchr(symbuf, '.');
	if(ptr)
		*ptr = 0;
	strcat(symbuf, ".s");
	
	if(asm32(symbuf)){
		ut_printf("*** error\n");
		return;
	}
	ut_printf("=== success\n");
}
/********************************************************************************/
/*		f_mgr_debug																*/
/********************************************************************************/
void f_mgr_gxh()
{
	char ptr[256];
	
	ee_read(ptr, 256, 0);
	ut_printf(ptr);
}
/********************************************************************************/
/*		f_mgr_debug																*/
/********************************************************************************/
void f_mgr_debug()
{
    for(;;){
        ut_printf("GetTickCount()=%d\n", GetTickCount()/1000);
        wait_ms(100);
    }
}
/********************************************************************************/
/*		f_mgr																	*/
/********************************************************************************/
void f_mgr()
{
	int ch;
	SYS_FS_RESULT ret;
    
    ut_printf("\nF1 キーを押すとヘルプメッセージが出ます\n\n");
	for(;;){
		ret = SYS_FS_CurrentWorkingDirectoryGet(linebuf, sizeof(linebuf));
		if(ret==SYS_FS_RES_SUCCESS)
			ut_printf("%s $ ", linebuf);
		else
			ut_printf("$ ");
		ch = f_mgr_gets(linebuf, sizeof(linebuf));
		if(ch){
			if(ch==F1)
				f_mgr_help();
			ut_putc('\n');
			continue;
		}
		linebufp = linebuf;
		getsym();
		
		if(symbuf[0]=='\0'){
			continue;
		}
		else if(strcmp(symbuf, "ls")==0){
			f_mgr_ls();
		}
		else if(strcmp(symbuf, "cd")==0){
			f_mgr_cd();
		}
		else if(strcmp(symbuf, "rm")==0){
			f_mgr_rm();
		}
		else if(strcmp(symbuf, "edit")==0){
			f_mgr_edit();
		}
		else if(strcmp(symbuf, "dump")==0){
			f_mgr_dump();
		}
		else if(strcmp(symbuf, "asm32")==0){
			f_mgr_asm32();
		}
		else if(strcmp(symbuf, "prep")==0){
			f_mgr_prep();
		}
		else if(strcmp(symbuf, "banana-c32")==0){
			f_mgr_c32();
		}
//		else if(strcmp(symbuf, "erase")==0){
//			f_mgr_erase();
//		}
//		else if(strcmp(symbuf, "prg")==0){
//			f_mgr_prg();
//		}
//		else if(strcmp(symbuf, "run")==0){
//			f_mgr_run();
//		}
		else if(strcmp(symbuf, "run")==0){
			f_mgr_load();
		}
		else if(strcmp(symbuf, "gxh")==0){
			f_mgr_gxh();
		}
		else if(strcmp(symbuf, "debug")==0){
			f_mgr_debug();
		}
        else{
            ut_printf("%s\n*** error\n", linebuf);
        }
	}
}
