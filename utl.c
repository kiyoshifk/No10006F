#include "app.h"

/*
 *    wait 200ms at 40MHz clock
 *    volatile int i;
 *    for(i=0; i<1000000; i++) ;
 */
void wait_ms(int ms)
{
	int j;
	volatile int i;
	
	for(j=0; j<ms; j++){
		for(i=0; i<10000; i++)
			;
	}
}
/********************************************************************************/
/*		MCrnd																	*/
/********************************************************************************/
int MCrnd(int i)
{
	static unsigned long rnd_init = 780907;
	
	rnd_init = rnd_init * 1234567 + 123456781;
	return (rnd_init + (rnd_init*rnd_init >> 16)) % i;
}
