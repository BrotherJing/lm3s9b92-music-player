#include <stdio.h>
#include "HardwareLibrary.h"
#include "LuminaryDriverLibrary.h"

//#include "SysCtlConfigure.h"
#include "UARTConfigure.h"
#include "main.h"

unsigned int buffer_ptr;
unsigned char uart_buffer[UART_BUFFER_SIZE];

void UART0Initial(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);	//使能UART0模块  
	UARTConfigSet(UART0_BASE,						//配置为UART0端口 
    			  115200,       					//波特率：115200 
    			  UART_CONFIG_WLEN_8 |				//数据位：8 
    			  UART_CONFIG_STOP_ONE |			//停止位：1 
    			  UART_CONFIG_PAR_NONE);   			//校验位：无 

	UARTFIFOLevelSet(UART0_BASE, 
					 UART_FIFO_TX4_8,				//设置FIFO发射深度为4/8
					 UART_FIFO_RX1_8);				//设置FIFO接收深度为1/8
 	UARTIntEnable(UART0_BASE,UART_INT_RX |
				  			 UART_INT_RT);			//使能接收中断和接收超时中断
	IntEnable(INT_UART0);
	UARTEnable(UART0_BASE);							//使能UART0   

	buffer_ptr = 0;
}

void UARTStringPut(unsigned long ulBase,const char *message)
{
	while(*message!='\0')
	{
		UARTCharPut(ulBase,*(message++));
	}
}

void UARTStringPutDefault(void){
	char *message = uart_buf;
	while(*message!='\0')
		UARTCharPut(UART0_BASE,*(message++));
}

void UART1Initial(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);	//使能UART1模块  
	UARTConfigSet(UART1_BASE,						//配置为UART1端口 
    			  9600,       						//波特率：9600 
    			  UART_CONFIG_WLEN_8 |				//数据位：8 
    			  UART_CONFIG_STOP_ONE |			//停止位：1 
    			  UART_CONFIG_PAR_NONE);   			//校验位：无 
	UARTEnable(UART1_BASE);							//使能UART1   
}
