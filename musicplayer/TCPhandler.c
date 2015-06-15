#include "HardwareLibrary.h"
#include "LuminaryDriverLibrary.h"

#include<string.h>
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include <lwip/stats.h>
#include <lwip/snmp.h>
#include "netif/etharp.h"
#include "netif/ppp_oe.h"
#include "netif/stellarisif.h"
										
#include "utils/locator.h"
#include "utils/lwiplib.h"			   
#include "utils/uartstdio.h"   
#include "third_party/fatfs/src/ff.h"
#include "third_party/fatfs/src/diskio.h"

#include "TCPhandler.h"
#include "main.h"
								
extern TheSysClock;
extern int current_page;
extern FATFS* g_sFatFs;
//const static char testdata[] = {"hello world tcp"};
unsigned char buffer[1536];
unsigned long buffer_len;
unsigned long file_size,received_size;
unsigned char isFileReceiving;
FIL g_sOutputFile;

const static char REQUEST_HEAD[] = {"CMD "};
const static char REQUEST_END[] = {" END"};

void buf_receive(struct pbuf *p){
	struct pbuf *q;
	int i;
	unsigned long tot_len;
	unsigned long len;
	//unsigned long *buf_ptr;
	unsigned char *buf_ptr_char;

	tot_len = p->tot_len;
	len = 0;
	q = p;
	while(q!=NULL&&len<tot_len){
		buf_ptr_char = q->payload;
		for(i=0;i<q->len;++i,++len){
			*(buffer+len)=*buf_ptr_char++;
		}
		/*if(i<q->len){
			buf_ptr_char = (unsigned char *)buf_ptr;
			for(;i<q->len;++i,++len){
				*(buffer+len)=*buf_ptr_char++;	
			}
		}*/
		q=q->next;				
	}
	
	buffer[tot_len] = '\0';
	buffer_len = tot_len;

	UARTprintf("the content is %s: \n",buffer);
	UARTprintf("\n");
}

err_t my_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err){
	
	unsigned char *cmd;
	int i;
	if(err==ERR_OK){
		buf_receive(p);//read data in pcb to buffer
		tcp_recved(pcb,p->tot_len);
		cmd = buffer+REQ_HEAD_LEN;
		if(strncmp(buffer,REQUEST_HEAD,REQ_HEAD_LEN)==0){
			UARTprintf("req head detected\n");
			for(i=REQ_HEAD_LEN;i<p->len-REQ_END_LEN;i++){//check if the cmd is complete.
				if(strncmp(buffer+i,REQUEST_END,REQ_END_LEN)==0)break;
			}
			if(i>p->len-REQ_END_LEN){//not complete
				UARTprintf("the command is not complete\n");
				pbuf_free(p);
				return ERR_OK;
			}		
			//strncpy(buffer,cmd,i-4);
			buffer[i]='\0';
			UARTprintf("in the buffer:%s\n",cmd);
			parseTCPCmd(pcb,cmd);
		}
		else if(isFileReceiving==1){
			//is receiving file
			writeFile();
		}
	}				 
	pbuf_free(p);
	return ERR_OK;
}

err_t my_accept(void *arg,struct tcp_pcb *pcb,err_t err){
	if(err==ERR_OK){
		UARTprintf("connect succeed!!\n");
		//when receive packet, call my_recv
  		tcp_recv(pcb, my_recv);
	}
	else
		UARTprintf("fail to connect.\n");
	return ERR_OK;
}

//build a tcp server, listen to port 80
void TCPInitial(void){
	struct tcp_pcb *pcb;
	pcb = tcp_new();
	tcp_bind(pcb, IP_ADDR_ANY, 80);
	pcb = tcp_listen(pcb);
	//when receive a connect request, call my_accept
	tcp_accept(pcb, my_accept);

	isFileReceiving = 0;
	//g_sOutputFile.fs=&g_sFatFs;
}

void parseTCPCmd(struct tcp_pcb *pcb,char* cmd){
	char* argv[1];
	char* divider;
	if(strncmp(cmd,"play",4)==0){
		UARTprintf("play what? %s\n",cmd+5);
		if(current_page!=PAGE_DETAIL)
			switchPage(PAGE_DETAIL);
		switchMusic(cmd+5);	
	}
	else if(strncmp(cmd,"cd",2)==0){
		UARTprintf("cd to %s\n",cmd+3);
	}		
	else if(strncmp(cmd,"ls",2)==0){
		argv[0] = buffer;
		if(Cmd_ls(1,argv)==0){
			tcp_write(pcb,buffer,strlen(buffer),0);
		}
		else{
			UARTprintf("ls fail\n");
		}
	}
	else if(strncmp(cmd,"file",4)==0){
		/*if(*(cmd+5)=='0'){
			//start receiving file
			UARTprintf("receiving file: %s\n",cmd+7);
			if(openFileWrite(cmd+7)==FR_OK){
				isFileReceiving = 1;
			}
		}else{
			isFileReceiving = 0;
			f_close(&g_sOutputFile);
			UARTprintf("finish receiving file\n");
		}*/
		divider = splitFileInfo(cmd+5);//the divider of file size and file name
		received_size = 0;
		UARTprintf("receiving file: %s\n",divider+1);
		if(openFileWrite(divider+1)==FR_OK){
			isFileReceiving = 1;
		}
	}	
}

char* splitFileInfo(char* info){
	file_size = 0;
	while(*info!='/'){	   
		file_size*=10;
		file_size+=(*info-'0');
		info++;
	}							 
	return info;
}

FRESULT openFileWrite(const char* filename){	
	FRESULT result;
	result = f_open(&g_sOutputFile, filename, FA_CREATE_NEW);
	result = f_open(&g_sOutputFile, filename, FA_WRITE);
	UARTprintf("open file:%s\n",filename);
	if(result!=FR_OK){
		UARTprintf("fail to open output file:%s\n",(char*)StringFromFresult(result));
	}
	return result;
}				  	 

void writeFile(void){
	FRESULT result = FR_OK;
	unsigned short usCount;
	unsigned long ulCount = 0;
	UARTprintf("before write file..\n");
	result = f_write(&g_sOutputFile,buffer,buffer_len,&usCount);
	usCount = buffer_len;
	g_sOutputFile.fs = (FATFS*)&g_sFatFs;
	SysCtlDelay(TheSysClock/10);
	UARTprintf("after write file..\n");
	if(result!=FR_OK){
		finishReceiving();
		UARTprintf("fail to write file:%s\n",(char*)StringFromFresult(result));
	}else{
		*(unsigned short*)(&(ulCount))= usCount;
		received_size += ulCount;
		UARTprintf("current %d bytes,reveive %d bytes,full size %d bytes\n",usCount,received_size,file_size);
	}
	if(received_size>=file_size){
		finishReceiving();
	    UARTprintf("finish receiving\n");
	}
}

void finishReceiving(void){
	isFileReceiving = 0;
	f_close(&g_sOutputFile);
}
