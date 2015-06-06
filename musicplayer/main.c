#include <string.h>

#include "HardwareLibrary.h"
#include "LuminaryDriverLibrary.h"

#include "grlib/grlib.h"
#include "grlib/widget.h"
#include "grlib/canvas.h"
#include "grlib/slider.h"
#include "grlib/listbox.h"
#include "grlib/pushbutton.h"

#include "utils/ustdlib.h"			   
#include "utils/uartstdio.h"		 
#include "utils/locator.h"
#include "utils/lwiplib.h"

#include "third_party/fatfs/src/ff.h"
#include "third_party/fatfs/src/diskio.h"

#include "drivers/kitronix320x240x16_ssd2119_idm_sbc.h"
#include "drivers/sound.h"
#include "drivers/touch.h"
#include "drivers/set_pinout.h"

#include "UARTConfigure.h"
#include "GPIODriverConfigure.h"
#include "TimerConfigure.h"
#include "SysCtlConfigure.h"
#include "EthernetConfigure.h"
#include "TCPhandler.h"
#include "WaveFileHelper.h"
#include "FileHelper.h"
#include "main.h" 

#ifdef ewarm
#pragma data_alignment=1024
tDMAControlTable sDMAControlTable[64];
#elif defined(ccs)
#pragma DATA_ALIGN(sDMAControlTable, 1024)
tDMAControlTable sDMAControlTable[64];
#else
tDMAControlTable sDMAControlTable[64] __attribute__ ((aligned(1024)));
#endif

//file processing
static FATFS g_sFatFs;
static DIR g_sDirObject;
static FILINFO g_sFileInfo;
FIL g_sFileObject;  

static const char *g_ppcDirListStrings[NUM_LIST_STRINGS];
static char g_pcFilenames[NUM_LIST_STRINGS][MAX_FILENAME_STRING_LEN];
static char g_cCwdBuf[PATH_BUF_SIZE] = "/";

static char music_progress[MUSIC_PROGRESS_TEXT_LEN];
static unsigned long g_ulBytesPlayed;
static unsigned char g_pucBuffer[AUDIO_BUFFER_SIZE];
static unsigned long g_ulMaxBufferSize;
volatile unsigned long g_ulFlags;
static unsigned long g_ulBytesRemaining;
static unsigned short g_usMinutes;
static unsigned short g_usSeconds;
static unsigned short g_usMinutesPlayed;
static unsigned short g_usSecondsPlayed;

tWaveHeader g_sWaveHeader;
unsigned int current_page;		//current page on screen

extern tCanvasWidget g_sListBackground;
extern tCanvasWidget g_sDetailBackground;
extern tCanvasWidget g_sHeading;
extern tPushButtonWidget g_sBackBtn;
extern tCanvasWidget g_sProgressText;
extern tCanvasWidget g_sTextFrame;
extern tCanvasWidget g_sListHeading;

void OnListBoxChange(tWidget *pWidget, short usSelected);
void OnBackBtnPress(tWidget *pWidget);
static int PopulateFileListBox(tBoolean bRedraw);

//file list
ListBox(g_sDirList, &g_sListBackground, 0, 0,
        &g_sKitronix320x240x16_SSD2119,
        0, 30, 320, 210, 0, ClrWhite, 0x00bdbdbd,
        ClrBlack, ClrBlack, ClrSilver, &g_sFontCmss12, g_ppcDirListStrings,
        NUM_LIST_STRINGS, 0, OnListBoxChange);

Canvas(g_sListBackground, WIDGET_ROOT, 0, &g_sListHeading, 
      &g_sKitronix320x240x16_SSD2119, 0, 0, 320, 240,
      CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);

Canvas(g_sDetailBackground,WIDGET_ROOT,0,&g_sTextFrame,
	&g_sKitronix320x240x16_SSD2119,0,30,320,210,
	CANVAS_STYLE_FILL,ClrWhite,0,0,0,0,0,0);

Canvas(g_sListHeading, &g_sListBackground, &g_sDirList, 0,
       &g_sKitronix320x240x16_SSD2119, 0, 0, 320, 30,
       (CANVAS_STYLE_FILL | CANVAS_STYLE_TEXT),
       0x0058A7F8, 0, ClrWhite, &g_sFontCm20, "List", 0, 0);

Canvas(g_sHeading, WIDGET_ROOT, &g_sDetailBackground, &g_sBackBtn,
       &g_sKitronix320x240x16_SSD2119, 0, 0, 320, 30,
       (CANVAS_STYLE_FILL | CANVAS_STYLE_TEXT),
       0x0058A7F8, 0, ClrWhite, &g_sFontCm20, "Music", 0, 0);

RectangularButton(g_sBackBtn,&g_sHeading,0,0,
	&g_sKitronix320x240x16_SSD2119,0,0,30,30,
	(PB_STYLE_TEXT|PB_STYLE_FILL),
	0x0058A7F8,0x0058A7F8,0,ClrWhite,
	&g_sFontCmss22b,"..",0,0,0,0,OnBackBtnPress);

Canvas(g_sTextFrame,&g_sDetailBackground,0,&g_sProgressText,
	&g_sKitronix320x240x16_SSD2119,0,80,320,40,
	CANVAS_STYLE_FILL,
	ClrWhite,0,0,0,"",0,0);

Canvas(g_sProgressText,&g_sTextFrame,0,0,
	&g_sKitronix320x240x16_SSD2119,0,80,320,40,
	CANVAS_STYLE_TEXT_HCENTER | CANVAS_STYLE_TEXT,
	ClrWhite,0,ClrBlack,&g_sFontCm20,"",0,0);
			

// ∆¡ƒª”“≤‡“Ù¡ø±‰ªØ°£
void
OnSliderChange(tWidget *pWidget, long lValue)
{
    SoundVolumeSet(lValue);
}

static const char *
StringFromFresult(FRESULT fresult)
{
    unsigned int uIdx;

    //
    // Enter a loop to search the error code table for a matching
    // error code.
    //
    for(uIdx = 0; uIdx < NUM_FRESULT_CODES; uIdx++)
    {
        //
        // If a match is found, then return the string name of the
        // error code.
        //
        if(g_sFresultStrings[uIdx].fresult == fresult)
        {
            return(g_sFresultStrings[uIdx].pcResultStr);
        }
    }
    return("UNKNOWN ERROR CODE");
}

//after playing a buffer of data, this function will be called. This will free the buffer and refresh bytes played.
void
BufferCallback(void *pvBuffer, unsigned long ulEvent)
{
    if(ulEvent & BUFFER_EVENT_FREE)
    {
        if(pvBuffer == g_pucBuffer)
        {
            g_ulFlags |= BUFFER_BOTTOM_EMPTY;
        }
        else
        {
            g_ulFlags |= BUFFER_TOP_EMPTY;
        }
        g_ulBytesPlayed += AUDIO_BUFFER_SIZE >> 1;
    }
}

//open the .wav file in sd card.
FRESULT
WaveOpen(FIL *pFile, const char *pcFileName, tWaveHeader *pWaveHeader)
{
    unsigned long *pulBuffer;
    unsigned short *pusBuffer;
    unsigned long ulChunkSize;
    unsigned short usCount;
    unsigned long ulBytesPerSample;
    FRESULT Result;

    pulBuffer = (unsigned long *)g_pucBuffer;
    pusBuffer = (unsigned short *)g_pucBuffer;

    Result = f_open(pFile, pcFileName, FA_READ);
    if(Result != FR_OK)
    {
		UARTprintf("fail to open file:%s\n",(char*)StringFromFresult(Result));
		while(Result!=FR_OK){
			f_mount(0, &g_sFatFs);
			SysCtlDelay(TheSysClock/100);
			Result = f_open(pFile, pcFileName, FA_READ);
			UARTprintf("fail to open file:%s\n",(char*)StringFromFresult(Result));
		}
		//return(Result);
    }
    Result = f_read(pFile, g_pucBuffer, 12, &usCount);
    if(Result != FR_OK)
    {	
		UARTprintf("fail to read RIFF chunk\n");
        f_close(pFile);
        return(Result);
    }
    if((pulBuffer[0] != RIFF_CHUNK_ID_RIFF) || (pulBuffer[2] != RIFF_TAG_WAVE))
    {							  
		UARTprintf("no RIFF chunk found\n");
        f_close(pFile);
        return(FR_INVALID_NAME);
    }
    Result = f_read(pFile, g_pucBuffer, 8, &usCount);
    if(Result != FR_OK)
    {	   
		UARTprintf("fail to read FMT chunk\n");
        f_close(pFile);
        return(Result);
    }

    if(pulBuffer[0] != RIFF_CHUNK_ID_FMT)
    {	
		UARTprintf("no FMT chunk found\n");
        f_close(pFile);
        return(FR_INVALID_NAME);
    }
    ulChunkSize = pulBuffer[1];

    Result = f_read(pFile, g_pucBuffer, ulChunkSize, &usCount);
    if(Result != FR_OK)
    {	
		UARTprintf("fail to read FMT chunk\n");
        f_close(pFile);
        return(Result);
    }

    pWaveHeader->usFormat = pusBuffer[0];
    pWaveHeader->usNumChannels =  pusBuffer[1];
    pWaveHeader->ulSampleRate = pulBuffer[1];
    pWaveHeader->ulAvgByteRate = pulBuffer[2];
    pWaveHeader->usBitsPerSample = pusBuffer[7];
    g_ulBytesPlayed = 0;
    ulBytesPerSample = (pWaveHeader->usBitsPerSample *
                        pWaveHeader->usNumChannels) >> 3;					   //bit*channel/8

    if(((AUDIO_BUFFER_SIZE >> 1) / ulBytesPerSample) > 1024)
    {
        g_ulMaxBufferSize = 1024 * ulBytesPerSample;
    }
    else
    {
        g_ulMaxBufferSize = AUDIO_BUFFER_SIZE >> 1;
    }
    if(pWaveHeader->usNumChannels > 2)
    {  
		UARTprintf("channels number error\n");
        f_close(pFile);
        return(FR_INVALID_NAME);
    }
	
    if(ulChunkSize > 16)
    {
		Result = f_read(pFile,g_pucBuffer,8,&usCount);		//fact chunk
		Result = f_read(pFile,g_pucBuffer,pulBuffer[1],&usCount);
    }

    Result = f_read(pFile, g_pucBuffer, 8, &usCount);
    if(Result != FR_OK)
    {
		UARTprintf("fail to read DATA chunk\n");
        f_close(pFile);
        return(Result);
    }

    if(pulBuffer[0] != RIFF_CHUNK_ID_DATA)
    {	
		UARTprintf("no DATA chunk found\n");
        f_close(pFile);
        return(Result);
    }
    pWaveHeader->ulDataSize = pulBuffer[1];

    g_usSeconds = pWaveHeader->ulDataSize/pWaveHeader->ulAvgByteRate;		 //the length of the song?
    g_usMinutes = g_usSeconds/60;
    g_usSeconds -= g_usMinutes*60;
	g_usMinutesPlayed = 0;
	g_usSecondsPlayed = 0;
    g_ulBytesRemaining = pWaveHeader->ulDataSize;
    if((pWaveHeader->usNumChannels == 1) && (pWaveHeader->usBitsPerSample == 8))
    {
        pWaveHeader->ulAvgByteRate <<=1;
    }
    SoundSetFormat(pWaveHeader->ulSampleRate, pWaveHeader->usBitsPerSample,
                   pWaveHeader->usNumChannels);	
	UARTprintf("open .wav file succeed\n");

	usprintf(music_progress, "00:00/%02d:%02d",g_usMinutes,g_usSeconds);
	CanvasTextSet(&g_sProgressText,music_progress);		
	WidgetPaint((tWidget*)&g_sProgressText);

    return(FR_OK);
}

//close .wav file
void
WaveClose(FIL *pFile)
{
	f_close(pFile);
}

void
Convert8Bit(unsigned char *pucBuffer, unsigned long ulSize)
{
    unsigned long ulIdx;

    for(ulIdx = 0; ulIdx < ulSize; ulIdx++)
    {
        *pucBuffer = ((short)(*pucBuffer)) - 128;
        pucBuffer++;
    }
}

unsigned short
WaveRead(FIL *pFile, tWaveHeader *pWaveHeader, unsigned char *pucBuffer)
{
    unsigned long ulBytesToRead;
    unsigned short usCount;
    if(g_ulBytesRemaining < g_ulMaxBufferSize)
    {
        ulBytesToRead = g_ulBytesRemaining;
    }
    else
    {
        ulBytesToRead = g_ulMaxBufferSize;
    }
    if(f_read(&g_sFileObject, pucBuffer, ulBytesToRead, &usCount) != FR_OK)
    {
        return(0);
    }
    g_ulBytesRemaining -= usCount;
    if(pWaveHeader->usBitsPerSample == 8)
    {
        Convert8Bit(pucBuffer, usCount);
    }

    return(usCount);
}

//play music
unsigned long
WavePlay(FIL *pFile, tWaveHeader *pWaveHeader)
{
    static unsigned short usCount;
    g_ulFlags = BUFFER_BOTTOM_EMPTY | BUFFER_TOP_EMPTY;
    g_ulFlags |= BUFFER_PLAYING;

    while(1)
    {
        IntDisable(INT_I2S0);
        if(g_ulFlags & BUFFER_BOTTOM_EMPTY)
        {
            usCount = WaveRead(pFile, pWaveHeader, g_pucBuffer);
            SoundBufferPlay(g_pucBuffer, usCount, BufferCallback);
            g_ulFlags &= ~BUFFER_BOTTOM_EMPTY;
        }
        if(g_ulFlags & BUFFER_TOP_EMPTY)
        {
            usCount = WaveRead(pFile, pWaveHeader,
                               &g_pucBuffer[AUDIO_BUFFER_SIZE >> 1]);
            SoundBufferPlay(&g_pucBuffer[AUDIO_BUFFER_SIZE >> 1],
                            usCount, BufferCallback);
            g_ulFlags &= ~BUFFER_TOP_EMPTY;
        }
        if((usCount < g_ulMaxBufferSize) || (g_ulBytesRemaining == 0))
        {
            g_ulFlags &= ~BUFFER_PLAYING;
            break;
        }
        IntEnable(INT_I2S0);
        WidgetMessageQueueProcess();
    }
    WaveClose(pFile);
    return(0);
}

void
SysTickHandler(void)
{
    disk_timerproc();
	lwIPTimer(SYSTICKMS);
}

void Timer_ISR(void){
	unsigned long ulStatus;		// define a variable to hold the interrupt status
	ulStatus = TimerIntStatus(TIMER0_BASE,true);				//read the interrup status
	TimerIntClear(TIMER0_BASE,ulStatus);	//clear the interrupt
		
	if(ulStatus&TIMER_TIMA_TIMEOUT&&current_page==PAGE_DETAIL) 			//identify the interrupt source
	{ 
		g_usSecondsPlayed = g_ulBytesPlayed/g_sWaveHeader.ulAvgByteRate;
		g_usMinutesPlayed = g_usSecondsPlayed/60;
		g_usSecondsPlayed %= 60;
		usprintf(music_progress, "%02d:%02d/%02d:%02d",
		g_usMinutesPlayed,
		g_usSecondsPlayed,
		g_usMinutes,g_usSeconds);
		CanvasTextSet(&g_sProgressText,music_progress);
		WidgetPaint((tWidget*)&g_sTextFrame);
	}		
}

static int
PopulateFileListBox(tBoolean bRedraw)
{
    unsigned long ulItemCount;
    FRESULT fresult;
    ListBoxClear(&g_sDirList);
    if(bRedraw)
    {
        WidgetPaint((tWidget *)&g_sDirList);
    }
    fresult = f_opendir(&g_sDirObject, g_cCwdBuf);
    if(fresult != FR_OK)
    {
        return(fresult);
    }

    ulItemCount = 0;
    while(1)
    {
        fresult = f_readdir(&g_sDirObject, &g_sFileInfo);
        if(fresult != FR_OK)
        {
            return(fresult);
        }
        if(!g_sFileInfo.fname[0])
        {
            break;
        }
        if(ulItemCount < NUM_LIST_STRINGS)
        {
            if((g_sFileInfo.fattrib & AM_DIR) == 0)
            {
                usprintf(g_pcFilenames[ulItemCount], "[%c] %s",
                     (g_sFileInfo.fattrib & AM_DIR) ? 'D' : 'F',
                      g_sFileInfo.fname);
                ListBoxTextAdd(&g_sDirList, g_pcFilenames[ulItemCount]);
							  
            }
        }
        if((g_sFileInfo.fattrib & AM_DIR) == 0)
        {
            ulItemCount++;
        }
    }
    return(0);
}

int
Cmd_ls(int argc, char *argv[])
{
    unsigned long ulTotalSize, ulItemCount, ulFileCount, ulDirCount;
	char* output_buffer;
	char inner_buffer[128];
	char useEthernet;//output the result to UART or Ethernet?
    FRESULT fresult;
    fresult = f_opendir(&g_sDirObject, g_cCwdBuf);
	
	if(argc==0)useEthernet = 0;
	else{
		useEthernet = 1;
		output_buffer = argv[0];
	}

    if(fresult != FR_OK)
    {
		if(useEthernet)
			usprintf(output_buffer,
			"Error from SD Card: %s\n",(char*)StringFromFresult(fresult));	
		else
			UARTprintf("Error from SD Card: %s\n",(char*)StringFromFresult(fresult));
		return(fresult);
    }

    ulTotalSize = 0;
    ulFileCount = 0;
    ulDirCount = 0;
    ulItemCount = 0;

    for(;;)
    {
        //
        // Read an entry from the directory.
        //
        fresult = f_readdir(&g_sDirObject, &g_sFileInfo);

        //
        // Check for error and return if there is a problem.
        //
        if(fresult != FR_OK)
        {
			if(useEthernet)
				usprintf(output_buffer,"error reading dir\n");
			else
				UARTprintf("error reading dir\n");		
            return(fresult);
        }

        //
        // If the file name is blank, then this is the end of the
        // listing.
        //
        if(!g_sFileInfo.fname[0])
        {
            break;
        }

        //
        // Print the entry information on a single line with formatting
        // to show the attributes, date, time, size, and name.
        //
		if(useEthernet){
			usprintf(inner_buffer,"[%c] %s\n",
                     (g_sFileInfo.fattrib & AM_DIR) ? 'D' : 'F',
                      g_sFileInfo.fname);
			strcpy(output_buffer,inner_buffer);
			output_buffer = output_buffer + strlen(inner_buffer);
		}
		else
			UARTprintf("[%c] %s\n",
                     (g_sFileInfo.fattrib & AM_DIR) ? 'D' : 'F',
                      g_sFileInfo.fname);

        //
        // If the attribute is directory, then increment the directory count.
        //
        if(g_sFileInfo.fattrib & AM_DIR)
        {
            ulDirCount++;
        }

        //
        // Otherwise, it is a file.  Increment the file count, and
        // add in the file size to the total.
        //
        else
        {
            ulFileCount++;
            ulTotalSize += g_sFileInfo.fsize;
        }

        //
        // Move to the next entry in the item array we use to populate the
        // list box.
        //
        ulItemCount++;

    }   // endfor

    return(0);
}

int
main(void)
{
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_16MHZ);	
	GPIOInitial();
	UART0Initial();
	TimerInitial();
    UARTStdioInit(0);
    PinoutSet();
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    SysCtlDelay(10);
    uDMAControlBaseSet(&sDMAControlTable[0]);
    uDMAEnable();
    ROM_SysTickPeriodSet(SysCtlClockGet() / TICKS_PER_SECOND);
    ROM_SysTickEnable();
    ROM_SysTickIntEnable();
    ROM_IntMasterEnable();
    Kitronix320x240x16_SSD2119Init();
    TouchScreenInit();
    TouchScreenCallbackSet(WidgetPointerMessage);
	EthernetInitial();

	WidgetAdd(WIDGET_ROOT,(tWidget*)&g_sListBackground);
	WidgetAdd((tWidget*)&g_sListBackground,(tWidget*)&g_sDirList);
    WidgetPaint(WIDGET_ROOT);
    WidgetMessageQueueProcess();

    if(f_mount(0, &g_sFatFs) != FR_OK)return(1);

    PopulateFileListBox(true);

	current_page = PAGE_LIST;
    g_ulFlags = 0;
    SoundInit(0);
	TCPInitial();

    while(1)
    {
        if(g_ulFlags & BUFFER_PLAYING)
        {
            WavePlay(&g_sFileObject, &g_sWaveHeader);
        }
        WidgetMessageQueueProcess();
    }
}

void OnListBoxChange(tWidget *pWidget, short usSelected){
	short selected;
	selected = ListBoxSelectionGet(&g_sDirList);		
	if(selected==-1)return;
	else{
		if(g_pcFilenames[selected][1]=='D'){
			//cd to the dir..
		}	
		else{
			current_page = PAGE_DETAIL;
			WidgetRemove((tWidget*)&g_sListBackground);
			//WidgetRemove((tWidget*)&g_sListHeading);
			WidgetAdd(WIDGET_ROOT,(tWidget*)&g_sHeading);
			WidgetAdd(WIDGET_ROOT,(tWidget*)&g_sDetailBackground);
			WidgetPaint(WIDGET_ROOT);
			switchMusic(&g_pcFilenames[selected][4]);
			UARTprintf("select %s\n",&g_pcFilenames[selected][4]);
		}
	}
}

//go back to list page
void OnBackBtnPress(tWidget *pWidget){
	current_page = PAGE_LIST;
	WidgetRemove((tWidget*)&g_sHeading);
	WidgetRemove((tWidget*)&g_sDetailBackground);
	WidgetAdd(WIDGET_ROOT,(tWidget*)&g_sListBackground);
	//WidgetAdd(WIDGET_ROOT,(tWidget*)&g_sListHeading);
	WidgetPaint(WIDGET_ROOT);
}

void switchMusic(const char* name){
	UARTprintf("play %s,len=%d\n",name,strlen(name));
	WaveClose(&g_sFileObject);
	g_ulFlags &= ~BUFFER_PLAYING;
	if(WaveOpen(&g_sFileObject, name, &g_sWaveHeader)== FR_OK){
		g_ulFlags |= BUFFER_PLAYING;
	}
}
