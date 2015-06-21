#ifndef MUSIC_MAIN_H
#define MUSIC_MAIN_H

#define PAGE_LIST 0
#define PAGE_DETAIL 1
#define PAGE_DOWNLOAD 2

#define MUSIC_PROGRESS_TEXT_LEN 12
#define UART_BUF_LEN 64
					  
#define TICKS_PER_SECOND 100
#define SYSTICKMS 10

#define MAIN_COLOR	0x0058A7F8
#define LIGHT_GREY	0x00bdbdbd
#define PB_COLOR	0x0057C43C
#define GREY		0x004a4a4a

void switchMusic(const char* name);			   
void pauseMusic(void);
void resumeMusic(void);
void switchPage(int page);
int Cmd_ls(int argc, char *argv[]);		  

void allocMem(void);

extern char uart_buf[];

#endif
