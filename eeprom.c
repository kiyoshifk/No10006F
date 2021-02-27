#include "app.h"


#define EEPROM_SDA	0x0008				// RD3
#define EEPROM_SCL	0x0002				// RD1
#define EE_PORT		PORTD
#define EE_SDA_H	LATDSET=EEPROM_SDA
#define EE_SDA_L	LATDCLR=EEPROM_SDA
#define EE_SCL_H	LATDSET=EEPROM_SCL
#define EE_SCL_L	LATDCLR=EEPROM_SCL


void ee_write_sub(char *data, int len, int startaddr);

/********************************************************************************/
/*		wait_1us																*/
/*		at 80MHz clock															*/
/********************************************************************************/
void wait_1us()
{
	volatile int i;
	
    i = 1;
	for(i=0; i<6; i++)
		;
}
/********************************************************************************/
/*		ee_send																	*/
/*		send 1 byte																*/
/*		return Ack																*/
/********************************************************************************/
int ee_send(int data)
{
	int i, ack;
	
	for(i=0; i<8; i++){						// 8bit 送出
		EE_SCL_L;
		wait_1us();
		if(data & 0x80)						// 送信
			EE_SDA_H;
		else
			EE_SDA_L;
		data <<= 1;
		wait_1us();
		
		EE_SCL_H;
		wait_1us();
		wait_1us();
	}
	/*** Ack サイクル	***/
	EE_SCL_L;
	wait_1us();
	EE_SDA_H;
	wait_1us();
	
	EE_SCL_H;
	wait_1us();
	ack = EE_PORT & EEPROM_SDA;				// 受信
	wait_1us();
	return ack;
}
/********************************************************************************/
/*		ee_recv																	*/
/*		return: received data, Ack: 1 or 0 send									*/
/********************************************************************************/
int ee_recv(int ack)
{
	int i, data;
	
	data = 0;
	for(i=0; i<8; i++){						// 8bit 受信
		EE_SCL_L;
		wait_1us();
		EE_SDA_H;
		wait_1us();
		
		EE_SCL_H;
		wait_1us();
		data <<= 1;
		if(EE_PORT & EEPROM_SDA)			// 受信
			data |= 1;
		wait_1us();
	}
	/*** Ack サイクル	***/
	EE_SCL_L;
	wait_1us();
	if(ack)									// 送信
		EE_SDA_H;
	else
		EE_SDA_L;
	wait_1us();
	
	EE_SCL_H;
	wait_1us();
	wait_1us();
	return data;
}
/********************************************************************************/
/*		ee_start																*/
/********************************************************************************/
void ee_start()
{
	EE_SCL_L;
	wait_1us();
	EE_SDA_H;
	wait_1us();
	
	EE_SCL_H;
	wait_1us();
	EE_SDA_L;
	wait_1us();
}
/********************************************************************************/
/*		ee_stop																	*/
/********************************************************************************/
void ee_stop()
{
	EE_SCL_L;
	wait_1us();
	EE_SDA_L;
	wait_1us();
	EE_SCL_H;
	wait_1us();
	EE_SDA_H;
	wait_1us();
}
/********************************************************************************/
/*		ee_addr_set																*/
/********************************************************************************/
void ee_addr_set(int startaddr)
{
    int i;
    
	for(i=0; i<20000; i++){
		ee_start();
		if(ee_send(0xa0)==0)				// send write command
			break;							// Ack OK
	}
    if(i==20000)
        ut_printf("eeprom busy10\n");
	if(ee_send(startaddr >> 8))				// high addr send
		ut_printf("eeprom no Ack11\n");
	if(ee_send(startaddr))					// low addr send
		ut_printf("eeprom no Ack12\n");
}
/********************************************************************************/
/*		ee_write																*/
/*      1 バイト当たり 40μs                                                     */
/********************************************************************************/
void ee_write(void *datax, int len, int startaddr)
{
	int end, nextpagetop, byte;
    char *data = (char*)datax;
	
	end = startaddr + len;
	for(;;){
		nextpagetop = (startaddr & ~(64-1)) + 64;
		if(end > nextpagetop){
			byte = nextpagetop - startaddr;
			ee_write_sub(data, byte, startaddr);
			data += byte;
			startaddr += byte;
		}
		else{
			byte = end - startaddr;
			ee_write_sub(data, byte, startaddr);
			break;
		}
	}
}
//
//		len は 64 以下であること
void ee_write_sub(char *data, int len, int startaddr)
{
	int i;
	
	ee_addr_set(startaddr);					// START + addr
	for(i=0; i<len; i++){
		if(ee_send(data[i]))				// send data
			ut_printf("eeprom no Ack13\n");
	}
	ee_stop();								// STOP
}
/********************************************************************************/
/*		ee_read																	*/
/*      1 バイト当たり 40μs                                                     */
/********************************************************************************/
void ee_read(void *datax, int len, int startaddr)
{
	int i;
	char *data = (char*)datax;
	
	ee_addr_set(startaddr);					// アドレスセット
	ee_start();								// START
	if(ee_send(0xa1))						// send read command
		ut_printf("eeprom no Ack14\n");
	for(i=0; i< len-1; i++){
		data[i] = ee_recv(0);				// 1 バイト受信 Ack 付
	}
	data[i] = ee_recv(1);					// 1 バイト受信 Ack 無し
	ee_stop();								// STOP
}
