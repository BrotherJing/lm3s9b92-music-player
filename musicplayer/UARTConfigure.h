#ifndef __UARTCONFIGURE_H_		//UART Module
#define __UARTCONFIGURE_H_


#define UART_BUFFER_SIZE 1024
extern unsigned char uart_buffer[UART_BUFFER_SIZE];
extern unsigned int buffer_ptr;

//Uart0ͨ��FT2232Dͨ��оƬ����λ�����ӣ�������ñ
//ͨ�Ų�������Ϊ115200
extern void UART0Initial(void);
extern void UARTStringPut(unsigned long ulBase,const char *);
void UARTStringPutDefault(void);

extern void UART1Initial(void);

extern void UARTSystemCheckInformationTransmit(unsigned long ulBase,unsigned char tSystemState);

#endif
