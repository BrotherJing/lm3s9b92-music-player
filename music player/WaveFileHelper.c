#include "HardwareLibrary.h"
#include "LuminaryDriverLibrary.h"
   	
#include "utils/ustdlib.h"			   
#include "utils/uartstdio.h"
#include "third_party/fatfs/src/ff.h"
#include "third_party/fatfs/src/diskio.h"
#include "drivers/sound.h"
#include "drivers/touch.h"
#include "drivers/set_pinout.h"

#include "WaveFileHelper.h"
#include "FileHelper.h"

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
		UARTprintf("%d played\n",g_ulBytesPlayed);
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
		//UARTStringPut(UART0_BASE,"fail to open file\n");
        return(Result);
    }
    Result = f_read(pFile, g_pucBuffer, 12, &usCount);
    if(Result != FR_OK)
    {	
		//UARTStringPut(UART0_BASE,"fail to read RIFF chunk\n");
        f_close(pFile);
        return(Result);
    }
    if((pulBuffer[0] != RIFF_CHUNK_ID_RIFF) || (pulBuffer[2] != RIFF_TAG_WAVE))
    {							  
		//UARTStringPut(UART0_BASE,"no RIFF chunk found\n");
        f_close(pFile);
        return(FR_INVALID_NAME);
    }
    Result = f_read(pFile, g_pucBuffer, 8, &usCount);
    if(Result != FR_OK)
    {	   
		//UARTStringPut(UART0_BASE,"fail to read FMT chunk\n");
        f_close(pFile);
        return(Result);
    }

    if(pulBuffer[0] != RIFF_CHUNK_ID_FMT)
    {	
		//UARTStringPut(UART0_BASE,"no FMT chunk found\n");
        f_close(pFile);
        return(FR_INVALID_NAME);
    }
    ulChunkSize = pulBuffer[1];

    Result = f_read(pFile, g_pucBuffer, ulChunkSize, &usCount);
    if(Result != FR_OK)
    {	
		//UARTStringPut(UART0_BASE,"fail to read FMT chunk\n");
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
		//UARTStringPut(UART0_BASE,"channels number error\n");
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
		//UARTprintf("fail to read DATA chunk\n");
        f_close(pFile);
        return(Result);
    }

    if(pulBuffer[0] != RIFF_CHUNK_ID_DATA)
    {	
		//UARTprintf("no DATA chunk found\n");
        f_close(pFile);
        return(Result);
    }
    pWaveHeader->ulDataSize = pulBuffer[1];

    g_usSeconds = pWaveHeader->ulDataSize/pWaveHeader->ulAvgByteRate;		 //the length of the song?
    g_usMinutes = g_usSeconds/60;
    g_usSeconds -= g_usMinutes*60;
    g_ulBytesRemaining = pWaveHeader->ulDataSize;
    if((pWaveHeader->usNumChannels == 1) && (pWaveHeader->usBitsPerSample == 8))
    {
        pWaveHeader->ulAvgByteRate <<=1;
    }
    SoundSetFormat(pWaveHeader->ulSampleRate, pWaveHeader->usBitsPerSample,
                   pWaveHeader->usNumChannels);
	UARTprintf("open .wav file succeed\n");
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
        //WidgetMessageQueueProcess();
    }
    WaveClose(pFile);
    return(0);
}
