#include "HardwareLibrary.h"
#include "LuminaryDriverLibrary.h"

#include <string.h>
#include "utils/uartstdio.h"
#include "UARTConfigure.h"
#include "UARTISR.h"			  
#include "third_party/fatfs/src/ff.h"
#include "WaveFileHelper.h"	 
#include "FileHelper.h"
#include "main.h"

extern int Cmd_ls(int argc, char *argv[]);

void UART0_ISR(void)
{
	unsigned long ulStatus;
	unsigned char receiveChar;

	ulStatus=UARTIntStatus(UART0_BASE,true);
	UARTIntClear(UART0_BASE,ulStatus);
	
	if ((ulStatus &	UART_INT_RX)||(ulStatus & UART_INT_RT))
	{
		if(!UARTCharsAvail(UART0_BASE))return;
		receiveChar=UARTCharGetNonBlocking(UART0_BASE);
		if (receiveChar!='\n'){
			uart_buffer[buffer_ptr++]=receiveChar;	
		}
		else{
			uart_buffer[buffer_ptr]='\0';
			buffer_ptr=0;	
			parseCmd(uart_buffer);
		}
	}	 
}

//parse the received command
void parseCmd(const char* cmd){
	if(strncmp(cmd,"play",4)==0){
		UARTprintf("play what? %s\n",cmd+5);
		switchMusic(cmd+5);	
	}
	if(strncmp(cmd,"cd",2)==0){
		UARTprintf("cd to %s\n",cmd+3);

	}		
	if(strncmp(cmd,"ls",2)==0){
		Cmd_ls(0,NULL);
	}
}
