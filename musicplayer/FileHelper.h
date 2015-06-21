#ifndef FILEHELPER_H
#define FILEHELPER_H

#define MAX_FILENAME_STRING_LEN 17
#define NUM_LIST_STRINGS 48							  
#define PATH_BUF_SIZE   80
#define CMD_BUF_SIZE    64		 
#define FRESULT_ENTRY(f)        { (f), (#f) }
#define NUM_FRESULT_CODES  14	 

									   
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

//file processing
extern FATFS g_sFatFs;
extern DIR g_sDirObject;
extern FILINFO g_sFileInfo;
extern FIL g_sFileObject;  

extern const char *g_ppcDirListStrings[NUM_LIST_STRINGS];
extern char g_pcFilenames[NUM_LIST_STRINGS][MAX_FILENAME_STRING_LEN];
extern char g_cCwdBuf[PATH_BUF_SIZE];  
const char *StringFromFresult(FRESULT fresult);

#endif
