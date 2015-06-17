#ifndef MUSIC_MAIN_H
#define MUSIC_MAIN_H

#define PAGE_LIST 0
#define PAGE_DETAIL 1
#define PAGE_DOWNLOAD 2

#define MUSIC_PROGRESS_TEXT_LEN 12
					  
#define FRESULT_ENTRY(f)        { (f), (#f) }
#define NUM_FRESULT_CODES  14	 
#define TICKS_PER_SECOND 100
#define SYSTICKMS 10

#define MAIN_COLOR	0x0058A7F8
#define LIGHT_GREY	0x00bdbdbd
#define PB_COLOR	0x0057C43C
#define GREY		0x004a4a4a

typedef struct
{
    FRESULT fresult;
    char *pcResultStr;
}
tFresultString;

static tFresultString g_sFresultStrings[] =
{
    FRESULT_ENTRY(FR_OK),
    FRESULT_ENTRY(FR_NOT_READY),
    FRESULT_ENTRY(FR_NO_FILE),
    FRESULT_ENTRY(FR_NO_PATH),
    FRESULT_ENTRY(FR_INVALID_NAME),
    FRESULT_ENTRY(FR_INVALID_DRIVE),
    FRESULT_ENTRY(FR_DENIED),
    FRESULT_ENTRY(FR_EXIST),
    FRESULT_ENTRY(FR_RW_ERROR),
    FRESULT_ENTRY(FR_WRITE_PROTECTED),
    FRESULT_ENTRY(FR_NOT_ENABLED),
    FRESULT_ENTRY(FR_NO_FILESYSTEM),
    FRESULT_ENTRY(FR_INVALID_OBJECT),
    FRESULT_ENTRY(FR_MKFS_ABORTED)
};

void switchMusic(const char* name);			   
void pauseMusic(void);
void resumeMusic(void);
void switchPage(int page);
int Cmd_ls(int argc, char *argv[]);		  
const char *StringFromFresult(FRESULT fresult);

void allocMem(void);

#endif
