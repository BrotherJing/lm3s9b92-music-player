#ifndef TCP_HANDLER_H
#define TCP_HANDLER_H
									
#define REQ_HEAD_LEN 4
#define REQ_END_LEN 4

extern void TCPInitial(void);
extern void parseTCPCmd(struct tcp_pcb *pcb,char* cmd);
extern FRESULT openFileWrite(char* filename);
extern void writeFile(void);	
extern void finishReceiving(void);					
extern char* splitFileInfo(char* info);
extern int str2int(char* str);

#endif
