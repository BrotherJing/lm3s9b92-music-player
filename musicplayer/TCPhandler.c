#include "HardwareLibrary.h"
#include "LuminaryDriverLibrary.h"

#include<string.h>
										
#include "utils/locator.h"
#include "utils/lwiplib.h"			   
#include "utils/uartstdio.h"

#include "TCPhandler.h"

extern int Cmd_ls(int argc, char *argv[]);
extern void switchMusic(const char* name);

const static char testdata[] = {"hello world tcp"};
char buffer[1024];

const static char REQUEST_HEAD[] = {"CMD "};
const static char REQUEST_END[] = {" END"};

void buf_receive(struct pbuf *p){
	struct pbuf *q;
	int i;
	unsigned long len;
	//unsigned long *buf_ptr;
	unsigned char *buf_ptr_char;

	len = 0;
	q = p;
	while(q!=NULL){
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
	
	buffer[p->tot_len-1] = '\0';

	UARTprintf("the content is %s: \n",buffer);
	/*for(i=0;i<p->tot_len;++i){
		UARTprintf("%c",buffer[i]);
	}*/	
	UARTprintf("\n");
}

err_t my_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err){
	
	//char *data;
	char *cmd;
	int i;
	if(err==ERR_OK){
		buf_receive(p);
		tcp_recved(pcb,p->tot_len);
		//data = p->payload;
		cmd = &buffer[4];
		if(strncmp(buffer,REQUEST_HEAD,4)==0){
			UARTprintf("req head detected\n");
			for(i=4;i<p->len-4;i++){//check if the cmd is complete.
				if(strncmp(buffer+i,REQUEST_END,4)==0)break;
			}
			if(i>p->len-4){//not complete
				UARTprintf("the command is not complete\n");
				pbuf_free(p);
				return ERR_OK;
			}		
			//strncpy(buffer,cmd,i-4);
			buffer[i]='\0';
			UARTprintf("in the buffer:%s\n",cmd);
			parseTCPCmd(pcb,cmd);
		}
	}	
	return ERR_OK;
}

err_t my_accept(void *arg,struct tcp_pcb *pcb,err_t err){
	if(err==ERR_OK){
		UARTprintf("connect succeed!!\n");
  		tcp_recv(pcb, my_recv);
	}
	else
		UARTprintf("fuck fail.\n");
	return ERR_OK;
}

void TCPInitial(void){
	struct tcp_pcb *pcb;
	pcb = tcp_new();
	tcp_bind(pcb, IP_ADDR_ANY, 80);
	pcb = tcp_listen(pcb);
	tcp_accept(pcb, my_accept);
}


err_t TcpCli_Connected(void *arg,struct tcp_pcb *pcb,err_t err)
{
   tcp_write(pcb,buffer,sizeof(buffer),0);      //发送数据
   
   tcp_close(pcb);
   
   return ERR_OK;
}

void send_buf(){
	struct tcp_pcb *Clipcb;
	struct ip_addr ipaddr;
  
	IP4_ADDR(&ipaddr,192,168,1,105);
  
	Clipcb = tcp_new();                       // 建立通信的TCP控制块(Clipcb) 
  
	tcp_bind(Clipcb,IP_ADDR_ANY,1000);       // 绑定本地IP地址和端口号 
  
	tcp_connect(Clipcb,&ipaddr,8888,TcpCli_Connected);
}

void parseTCPCmd(struct tcp_pcb *pcb,const char* cmd){
	char* argv[1];
	if(strncmp(cmd,"play",4)==0){
		UARTprintf("play what? %s\n",cmd+5);
		switchMusic(cmd+5);	
	}
	else if(strncmp(cmd,"cd",2)==0){
		UARTprintf("cd to %s\n",cmd+3);
	}		
	else if(strncmp(cmd,"ls",2)==0){
		argv[0] = buffer;
		if(Cmd_ls(1,argv)==0){
			//UARTprintf("%s\n",buffer);
			//pcb->remote_port = 8888;
			send_buf();
			//tcp_write(pcb,buffer,strlen(buffer),0);
		}
		else{
			UARTprintf("ls fail\n");
		}
	}	
}
