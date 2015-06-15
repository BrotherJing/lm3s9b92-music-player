#include "HardwareLibrary.h"
#include "LuminaryDriverLibrary.h"

#include "SysCtlConfigure.h"
#include "TimerConfigure.h"

void TimerInitial(void)
{
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);		//enable TIMER0
	TimerConfigure(TIMER0_BASE,TIMER_CFG_32_BIT_PER);		//set up TIMER0 as a 32-bit wrap-on-zero timer
	TimerLoadSet(TIMER0_BASE,TIMER_A,(TheSysClock));	 	//the wrap duration is 1s	
	SysTickEnable();
	TimerEnable(TIMER0_BASE,TIMER_A);		//start TIMER0 to cound down
	TimerIntEnable(TIMER0_BASE,TIMER_TIMA_TIMEOUT);		//enables the indicated timer interrupt sources
	IntEnable(INT_TIMER0A);	
}
