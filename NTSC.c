#include "app.h"

uint32_t v_sync1[10];
uint32_t v_sync21[10];
uint32_t v_sync23[10];
uint32_t h_sync[10];

//uint32_t video[320][20];				// video RAM
uint32_t __attribute__((section(".mySection1"))) video[320][20];	// video RAM
const uint32_t video_0[20];


void dma_start();
void dma_stop();

/********************************************************************************/
/*		Pset																	*/
/*																				*/
/*    line 0      video[0]														*/
/*    line 2      video[2]														*/
/*    line 4      video[3]														*/
/*      .																		*/
/*      .																		*/
/*    line 316    video[316]													*/
/*    line 318    video[318]													*/
/*																				*/
/*    line 320    [ 0V 出力 102+1 line ]										*/
/*    line 322    [ 0V 出力 102+1 line ]										*/
/*      .																		*/
/*      .																		*/
/*    line 522    [ 0V 出力 102+1 line ]										*/
/*    line 524    [ 0V 出力 102+1 line ]										*/
/*																				*/
/*    line 1      video[1]														*/
/*    line 3      video[3]														*/
/*      .																		*/
/*      .																		*/
/*    line 317    video[317]													*/
/*    line 319    video[319]													*/
/*																				*/
/*    line 321    [ 0V 出力 102 line ]											*/
/*    line 323    [ 0V 出力 102 line ]											*/
/*      .																		*/
/*      .																		*/
/*    line 521    [ 0V 出力 102 line ]											*/
/*    line 523    [ 0V 出力 102 line ]											*/
/*																				*/
/*	解像度 448x320                                                              */
/*	uint32_t video[320][20];   Video RAM										*/
/*	video[y][x]: y=0～319, x=4～17                                              */
/********************************************************************************/
void Pset(int x, int y, int c)
{
	uint32_t bit;
	int xx;
	
	if(x<0 || x>=448 || y<0 || y>=320)
		return;							// out of range
	xx = x/32+4;
	bit = 0x80000000 >> x%32;
	if(c)
		video[y][xx] |= bit;
	else
		video[y][xx] &= ~bit;
}

int Pget(int x, int y)
{
	uint32_t bit;
	int xx;
	
	if(x<0 || x>=448 || y<0 || y>=320)
		return 0;						// out of range
	xx = x/32+4;
	bit = 0x80000000 >> x%32;
	return video[y][xx] & bit;
}
/********************************************************************************/
/*		sync_data_set															*/
//
//      IO clock = 20MHz
//      shift clock = 20MHz/2/(1+1) = 5MHz  1bit:0.2μs
//      同期信号 10転送 = 32x0.2x10 = 64μs
//      h_sync: 4.8μs(24bit) low, 59.2μs(296bit) high
//      v_sync: 59.2μs(296bit) high, 4.8μs(24bit) low
//      同期信号は v_sync:3cycle + h_sync:259cycle
//
//		v_sync1  x3
//		h_sync   x259
//		v_sync21 x1
//		v_sync1  x2
//		v_sync23 x1
//		h_sync   x259
/********************************************************************************/
void sync_data_set()
{
    int i, byte;
    uint32_t bit;

	bit = 0x80000000;
	byte = 0;
	for(i=0; i<10*32; i++){
		if(i<296)
			v_sync1[byte] &= ~bit;
		else
			v_sync1[byte] |= bit;
		
		if(i<24)
			v_sync21[byte] &= ~bit;
		else if(i<160)
			v_sync21[byte] |= bit;
		else if(i<296)
			v_sync21[byte] &= ~bit;
		else
			v_sync21[byte] |= bit;
		
		if(i<160)
			v_sync23[byte] &= ~bit;
		else
			v_sync23[byte] |= bit;
		
		if(i<24)
			h_sync[byte] &= ~bit;
		else
			h_sync[byte] |= bit;
		if((bit>>=1)==0){
			bit = 0x80000000;
			++byte;
		}
	}

#if 0
    bit = 0x80000000;
    byte = 0;
    for(i=0; i<10*32; i++){
		if(i<296)
			v_sync[byte] &= ~bit;
		else
			v_sync[byte] |= bit;
		
		if(i<24)
			h_sync[byte] &= ~bit;
		else
			h_sync[byte] |= bit;
		if((bit>>=1)==0){
			bit = 0x80000000;
			++byte;
		}
	}
#endif
}

/********************************************************************************/
/*		spi_init																*/
/********************************************************************************/
void spi_init()
{
//	int i;

	sync_data_set();

	/*** SPI setting	***/
	RPD11R = 8;								// SDO1
	SPI1CON = 0x1083C;						// 32bit
	SPI1CONSET = 0x8000;					// Enable
	SPI1BRG = 4-1;							// 40MHz/2/(3+1)=5MHz
//	IPC7bits.SPI1IP = 1;
//	IFS1bits.SPI1TXIF = 0;
//	IEC1bits.SPI1TXIE = 1;
	
	RPD10R = 6;								// SDO2
	SPI2CON = 0x1083C;						// 32bit
	SPI2CONSET = 0x8000;					// Enable
	SPI2BRG = 2-1;							// 40MHz/2/(1+1)=10MHz
	
//    for(;;){
//        SPI1BUF = 0x55555555;//AAAAA
//        SPI2BUF = 0x55555555;
//
//        volatile int cnt;
//       	for(cnt=0; cnt<100; cnt++)
//       		;
//    }
        
    
	/*** DMA setting	***/
	DMACON = 0x8000;						// DMA Enable
	
	//channel 2
	DCH2CON = 0;
	DCH2CONbits.CHAEN = 0;					// channel automatic off
	DCH2ECONbits.CHSIRQ = _SPI1_TX_IRQ;
	DCH2ECONbits.SIRQEN = 1;				// channel start IRQ enable bit
	DCH2SSA = ((uint32_t) v_sync1)&0x1FFFFFFF;
	DCH2DSA = ((uint32_t) (&SPI1BUF))&0x1FFFFFFF;
	DCH2SSIZ = sizeof(v_sync1);
	DCH2DSIZ = 4;
	DCH2CSIZ = 4;							// 1?????]???o?C?g??
	DCH2INTbits.CHBCIE = 1;					// Block transfer complete interrupt
	DCH2INTbits.CHBCIF = 0;					// Block transfer complete interrupt flag clear
	IPC11bits.DMA2IP = 5;
	IFS2bits.DMA2IF = 0;
	IEC2bits.DMA2IE = 1;
	
	//channel 3
	DCH3CON = 0;
	DCH3CONbits.CHAEN = 0;					// channel automatic off
	DCH3ECONbits.CHSIRQ = _SPI2_TX_IRQ;
	DCH3ECONbits.SIRQEN = 1;				// channel start IRQ enable bit
	DCH3SSA = ((uint32_t) video_0)&0x1FFFFFFF;
	DCH3DSA = ((uint32_t) (&SPI2BUF))&0x1FFFFFFF;
	DCH3SSIZ = sizeof(video_0);
	DCH3DSIZ = 4;
	DCH3CSIZ = 4;							// 1?????]???o?C?g??
	DCH3INTbits.CHBCIE = 1;					// Block transfer complete interrupt
	DCH3INTbits.CHBCIF = 0;					// Block transfer complete interrupt flag clear
	IPC11bits.DMA3IP = 5;
	IFS2bits.DMA3IF = 0;
	IEC2bits.DMA3IE = 1;
	
	__builtin_disable_interrupts();
	DCH2CONbits.CHEN = 1;					// DMA ch2 enable
	DCH3CONbits.CHEN = 1;					// DMA ch3 enable
	__builtin_enable_interrupts();
	
	/*** timer2 setting for voice ***/
//	T2CON = 0;
//	PR2 = 255;
//	TMR2 = 0;
//    T2CONbits.ON = 1;
////	IPC2bits.T2IP = 4;
////	IFS0bits.T2IF = 0;
////	IEC0bits.T2IE = 1;
}
/********************************************************************************/
/*		interrupt																*/
/********************************************************************************/

int DMA2_cnt = 0;
//	NTSC Sync
void __ISR(_DMA2_VECTOR, IPL5AUTO) _DMA2Interrupt ()
{
//	static int cnt;
	
	IFS2bits.DMA2IF = 0;					// Clear DMA ch1 IF
	DCH2INTbits.CHBCIF = 0;
	if(DMA2_cnt < 263){
		if(DMA2_cnt < 3+7)
			DCH2SSA = ((uint32_t) v_sync1)&0x1FFFFFFF;
		else if(DMA2_cnt < 262)
			DCH2SSA = ((uint32_t) h_sync)&0x1FFFFFFF;
		else
			DCH2SSA = ((uint32_t) v_sync21)&0x1FFFFFFF;
	}
	else{
		if(DMA2_cnt < 265+7)
			DCH2SSA = ((uint32_t) v_sync1)&0x1FFFFFFF;
		else if(DMA2_cnt < 266+7)
			DCH2SSA = ((uint32_t) v_sync23)&0x1FFFFFFF;
		else
			DCH2SSA = ((uint32_t) h_sync)&0x1FFFFFFF;
	}
	DCH2CONbits.CHEN = 1;
    if(++DMA2_cnt >= (262*2+1))
        DMA2_cnt = 0;
}


int DMA3_cnt = 413;
//	NTSC Video  y=0～319
void __ISR(_DMA3_VECTOR, IPL5AUTO) _DMA3Interrupt ()
{
//	static int cnt= 413;
	
	IFS2bits.DMA3IF = 0;					// Clear DMA ch1 IF
	DCH3INTbits.CHBCIF = 0;

	// cnt >= 321～524: 0V output
	if(DMA3_cnt >= 320){
		DCH3SSA = ((uint32_t) video_0)&0x1FFFFFFF;
	}
	else{
		if(DMA3_cnt & 1){
			// cnt = 1, 3 ... 317, 319
			DCH3SSA = ((uint32_t) video[DMA3_cnt])&0x1FFFFFFF;
		}
		else{
			// cnt = 0, 2 ... 320, 322
			DCH3SSA = ((uint32_t) video[DMA3_cnt])&0x1FFFFFFF;
		}
	}
	
	DCH3CONbits.CHEN = 1;
	DMA3_cnt += 2;
	if(DMA3_cnt >= 525)
		DMA3_cnt -= 525;
}

void dma_start()
{
	DMA2_cnt = 0;
	DMA3_cnt = 413;

	spi_init();
}

void dma_stop()
{
	DCH2CONbits.CHEN = 0;					// DMA ch2 disable
	DCH3CONbits.CHEN = 0;					// DMA ch3 disable
}
