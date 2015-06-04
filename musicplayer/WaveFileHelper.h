#ifndef WAVEFILEHELPER_H
#define WAVEFILEHELPER_H

#define AUDIO_BUFFER_SIZE       4096
#define BUFFER_BOTTOM_EMPTY     0x00000001
#define BUFFER_TOP_EMPTY        0x00000002
#define BUFFER_PLAYING          0x00000004

#define RIFF_CHUNK_ID_RIFF      0x46464952	//FFIR
#define RIFF_CHUNK_ID_FMT       0x20746d66	// TMF
#define RIFF_CHUNK_ID_DATA      0x61746164	//ATAD

#define RIFF_TAG_WAVE           0x45564157

#define RIFF_FORMAT_UNKNOWN     0x0000
#define RIFF_FORMAT_PCM         0x0001
#define RIFF_FORMAT_MSADPCM     0x0002
#define RIFF_FORMAT_IMAADPCM    0x0011

extern unsigned long g_ulBytesPlayed;
extern unsigned char g_pucBuffer[AUDIO_BUFFER_SIZE];
extern unsigned long g_ulMaxBufferSize;
extern volatile unsigned long g_ulFlags;
extern unsigned long g_ulBytesRemaining;
extern unsigned short g_usMinutes;
extern unsigned short g_usSeconds;

typedef struct
{
    unsigned long ulSampleRate;
    unsigned long ulAvgByteRate;
    unsigned long ulDataSize;
    unsigned short usBitsPerSample;
    unsigned short usFormat;
    unsigned short usNumChannels;
}
tWaveHeader;
extern tWaveHeader g_sWaveHeader;

extern void BufferCallback(void *pvBuffer, unsigned long ulEvent);
extern FRESULT WaveOpen(FIL *pFile, const char *pcFileName, tWaveHeader *pWaveHeader);
extern void WaveClose(FIL *pFile);
extern void Convert8Bit(unsigned char *pucBuffer, unsigned long ulSize);
extern unsigned short WaveRead(FIL *pFile, tWaveHeader *pWaveHeader, unsigned char *pucBuffer);
extern unsigned long WavePlay(FIL *pFile, tWaveHeader *pWaveHeader);

#endif
