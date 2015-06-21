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
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);	//ʹ��UART0ģ��  
	UARTConfigSet(UART0_BASE,						//����ΪUART0�˿� 
    			  115200,       					//�����ʣ�115200 
    			  UART_CONFIG_WLEN_8 |				//����λ��8 
    			  UART_CONFIG_STOP_ONE |			//ֹͣλ��1 
    			  UART_CONFIG_PAR_NONE);   			//У��λ���� 

	UARTFIFOLevelSet(UART0_BASE, 
					 UART_FIFO_TX4_8,				//����FIFO�������Ϊ4/8
					 UART_FIFO_RX1_8);				//����FIFO�������Ϊ1/8
 	UARTIntEnable(UART0_BASE,UART_INT_RX |
				  			 UART_INT_RT);			//ʹ�ܽ����жϺͽ��ճ�ʱ�ж�
	IntEnable(INT_UART0);
	UARTEnable(UART0_BASE);							//ʹ��UART0   

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
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);	//ʹ��UART1ģ��  
	UARTConfigSet(UART1_BASE,						//����ΪUART1�˿� 
    			  9600,       						//�����ʣ�9600 
    			  UART_CONFIG_WLEN_8 |				//����λ��8 
    			  UART_CONFIG_STOP_ONE |			//ֹͣλ��1 
    			  UART_CONFIG_PAR_NONE);   			//У��λ���� 
	UARTEnable(UART1_BASE);							//ʹ��UART1   
}
