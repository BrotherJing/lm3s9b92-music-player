#include "HardwareLibrary.h"
#include "LuminaryDriverLibrary.h"

#include <string.h>
#include "UARTConfigure.h"
#include "UARTISR.h"			  
#include "third_party/fatfs/src/ff.h"
#include "WaveFileHelper.h"	 
#include "FileHelper.h"
#include "main.h"

extern int Cmd_ls(int argc, char *argv[]);
extern int current_page;

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
		//UARTprintf("play what? %s\n",cmd+5);
		if(current_page!=PAGE_DETAIL)
			switchPage(PAGE_DETAIL);
		switchMusic(cmd+5);	
	}
	else if(strncmp(cmd,"cd",2)==0){
		//UARTprintf("cd to %s\n",cmd+3);
	}		
	else if(strncmp(cmd,"ls",2)==0){
		Cmd_ls(0,NULL);
	}
	else if(strncmp(cmd,"res",3)==0){
		resumeMusic();
	}
	else if(strncmp(cmd,"pau",3)==0){
		pauseMusic();
	}
}
