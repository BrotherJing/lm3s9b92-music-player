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
							  
#include "drivers/extram.h"
#include "drivers/bget.h"

#include "grlib/grlib.h"
#include "grlib/widget.h"
#include "grlib/canvas.h"
#include "grlib/slider.h"										
#include "utils/locator.h"
#include "utils/lwiplib.h"			   
#include "utils/uartstdio.h"   
#include "third_party/fatfs/src/ff.h"
#include "third_party/fatfs/src/diskio.h"

#include "TCPhandler.h"
#include "main.h"

extern tCanvasWidget g_sRecFileInfo;
//extern tSliderWidget g_sDldProgressBar;
								
extern unsigned long TheSysClock;
extern int current_page;
extern FATFS* g_sFatFs;
unsigned char buffer[1536];
unsigned long buffer_len;
unsigned long file_size,received_size;
unsigned char isFileReceiving;
char recFileInfo[64];
char recFileName[16];
unsigned char *tempFile, *ptFile;

FIL g_sOutputFile;

const static char REQUEST_HEAD[] = {"CMD "};
const static char REQUEST_END[] = {" END"};

void buf_receive(struct pbuf *p){
	struct pbuf *q;
	int i;
	unsigned long tot_len;
	unsigned long len;
	unsigned char *buf_ptr_char;

	tot_len = p->tot_len;
	len = 0;
	q = p;
	while(q!=NULL&&len<tot_len){
		buf_ptr_char = q->payload;
		for(i=0;i<q->len;++i,++len){
			*(buffer+len)=*buf_ptr_char++;
		}
		q=q->next;				
	}
	
	buffer[tot_len] = '\0';
	buffer_len = tot_len;

	//UARTprintf("the content is %s: \n",buffer);
	//UARTprintf("\n");
}

err_t my_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err){
	
	unsigned char *cmd;
	int i;
	if(err==ERR_OK){
		buf_receive(p);//read data in pcb to buffer
		tcp_recved(pcb,p->tot_len);
		cmd = buffer+REQ_HEAD_LEN;
		if(strncmp(buffer,REQUEST_HEAD,REQ_HEAD_LEN)==0){
			//UARTprintf("req head detected\n");
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
			//UARTprintf("in the buffer:%s\n",cmd);
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
}

void parseTCPCmd(struct tcp_pcb *pcb,char* cmd){
	char* argv[1];
	char* divider;
	if(strncmp(cmd,"play",4)==0){
		//UARTprintf("play what? %s\n",cmd+5);
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
	else if(strncmp(cmd,"res",3)==0){
		resumeMusic();
	}
	else if(strncmp(cmd,"pau",3)==0){
		pauseMusic();
	}
	else if(strncmp(cmd,"file",4)==0){
		divider = splitFileInfo(cmd+5);//the divider of file size and file name
		received_size = 0;
		UARTprintf("receiving file: %s\n",divider+1);
		if(current_page == PAGE_DOWNLOAD){
			usprintf(recFileInfo,"%s %d Bytes",divider+1,file_size);
			CanvasTextSet(&g_sRecFileInfo,recFileInfo); 
			WidgetPaint((tWidget*)&g_sRecFileInfo);	
		}
		if((tempFile=ExtRAMAlloc(file_size))!=0){
			strcpy(recFileName,divider+1);
			ptFile = tempFile;
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
	//FRESULT result = FR_OK;
	unsigned short usCount;
	unsigned long ulCount = 0;

	UARTprintf("%d \n",buffer_len);
	for(usCount=0;usCount<buffer_len;++usCount){
		*ptFile++ = buffer[usCount];
	}
	*(unsigned short*)(&(ulCount))= usCount;
	received_size += ulCount;
	UARTprintf("%d/%d\n",received_size,file_size);
	if(current_page == PAGE_DOWNLOAD){
		//SliderValueSet(&g_sDldProgressBar,received_size*100/file_size);
		//WidgetPaint((tWidget*)&g_sDldProgressBar);
	}

	if(received_size>=file_size){
		UARTprintf("start writing file\n");

		SysCtlDelay(TheSysClock);
		finishReceiving();

	    UARTprintf("finish receiving\n"); 
		//CanvasTextSet(&g_sRecFileInfo,"Finish!!"); 
		//WidgetPaint((tWidget*)&g_sRecFileInfo);
		//SliderValueSet(&g_sDldProgressBar,0);
		//WidgetPaint((tWidget*)&g_sDldProgressBar);	
	}
}

void finishReceiving(void){
	FRESULT result;	  
	unsigned long ulCount=0,write_size;
	unsigned short usCount;
	ptFile = tempFile;
	//SliderValueSet(&g_sDldProgressBar,0);	 
	//WidgetPaint((tWidget*)&g_sDldProgressBar);
	if(openFileWrite(recFileName)!=FR_OK){
		return;	
	}
	if(file_size>2048)
	for(write_size=0;write_size<file_size-2048;){
		result = f_write(&g_sOutputFile,ptFile,2048,&usCount);
		if(result!=FR_OK){
			UARTprintf("write fail\n");
			break;
		}		  		 
		//SliderValueSet(&g_sDldProgressBar,write_size*100/file_size);
		//WidgetPaint((tWidget*)&g_sDldProgressBar);
		ptFile+=usCount;
		*(unsigned short*)(&(ulCount))= usCount;
		write_size+=ulCount;	   
		UARTprintf("%d/%d\n",write_size,file_size);
	}
	result = f_write(&g_sOutputFile,ptFile,file_size-write_size,&usCount);
	if(result!=FR_OK){
		UARTprintf("write fail\n");
	}

	ExtRAMFree(tempFile);
	isFileReceiving = 0;
	f_close(&g_sOutputFile);
}
