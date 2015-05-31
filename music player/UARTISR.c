#include "HardwareLibrary.h"
#include "LuminaryDriverLibrary.h"

#include <string.h>
#include "utils/uartstdio.h"
#include "UARTConfigure.h"
#include "UARTISR.h"			  
#include "third_party/fatfs/src/ff.h"
#include "WaveFileHelper.h"	 
#include "FileHelper.h"

extern FIL g_sFileObject; 
extern volatile unsigned long g_ulFlags; 
extern tWaveHeader g_sWaveHeader;

void UART0_ISR(void)
{
	unsigned long ulStatus;
	unsigned char receiveChar;

	ulStatus=UARTIntStatus(UART0_BASE,true);
	UARTIntClear(UART0_BASE,ulStatus);
	
	if ((ulStatus &	UART_INT_RX)||(ulStatus & UART_INT_RT))
	{
		receiveChar=UARTCharGetNonBlocking(UART0_BASE);
		if (receiveChar!='\n'){
			uart_buffer[buffer_ptr++]=receiveChar;	
		}
		else{
			//uart_buffer[buffer_ptr++]=receiveChar;
			uart_buffer[buffer_ptr]='\0';
			//UARTprintf(uart_buffer);
			buffer_ptr=0;	
			parseCmd(uart_buffer);
		}
	}	 
}

void parseCmd(const char* cmd){
	if(strncmp(cmd,"play",4)==0){
		UARTprintf("play what? %s\n",cmd+5);
		WaveClose(&g_sFileObject);
		g_ulFlags &= ~BUFFER_PLAYING;
		if(WaveOpen(&g_sFileObject, cmd+5, &g_sWaveHeader)== FR_OK){
    		g_ulFlags |= BUFFER_PLAYING;
		}	
	}		
}
