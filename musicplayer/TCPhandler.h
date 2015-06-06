#ifndef TCP_HANDLER_H
#define TCP_HANDLER_H

extern void TCPInitial(void);
extern void parseTCPCmd(struct tcp_pcb *pcb,const char* cmd);

#endif
