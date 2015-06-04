#ifndef FILEHELPER_H
#define FILEHELPER_H

#define MAX_FILENAME_STRING_LEN 17
#define NUM_LIST_STRINGS 48							  
#define PATH_BUF_SIZE   80
#define CMD_BUF_SIZE    64

//file processing
extern FATFS g_sFatFs;
extern DIR g_sDirObject;
extern FILINFO g_sFileInfo;
extern FIL g_sFileObject;  

extern const char *g_ppcDirListStrings[NUM_LIST_STRINGS];
extern char g_pcFilenames[NUM_LIST_STRINGS][MAX_FILENAME_STRING_LEN];
extern char g_cCwdBuf[PATH_BUF_SIZE];

#endif
