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
#include "third_party/fatfs/src/ff.h"
#include "third_party/fatfs/src/diskio.h"
#include "drivers/kitronix320x240x16_ssd2119_idm_sbc.h"
#include "drivers/sound.h"
#include "drivers/touch.h"
#include "drivers/set_pinout.h"

#include "UARTConfigure.h"
#include "GPIODriverConfigure.h"
#include "WaveFileHelper.h"
#include "FileHelper.h"


#ifdef ewarm
#pragma data_alignment=1024
tDMAControlTable sDMAControlTable[64];
#elif defined(ccs)
#pragma DATA_ALIGN(sDMAControlTable, 1024)
tDMAControlTable sDMAControlTable[64];
#else
tDMAControlTable sDMAControlTable[64] __attribute__ ((aligned(1024)));
#endif
					   
#define TICKS_PER_SECOND 100
/*
#define MAX_FILENAME_STRING_LEN (8 + 1 + 3 + 1)
#define NUM_LIST_STRINGS 48							  
#define PATH_BUF_SIZE   80
#define CMD_BUF_SIZE    64	 */
typedef struct
{
    FRESULT fresult;
    char *pcResultStr;
}
tFresultString;

#define FRESULT_ENTRY(f)        { (f), (#f) }
#define NUM_FRESULT_CODES  14 

tFresultString g_sFresultStrings[] =
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
static FATFS g_sFatFs;
static DIR g_sDirObject;
static FILINFO g_sFileInfo;
FIL g_sFileObject;  

static const char *g_ppcDirListStrings[NUM_LIST_STRINGS];
static char g_pcFilenames[NUM_LIST_STRINGS][MAX_FILENAME_STRING_LEN];
static char g_cCwdBuf[PATH_BUF_SIZE] = "/";

static unsigned long g_ulBytesPlayed;
static unsigned char g_pucBuffer[AUDIO_BUFFER_SIZE];
static unsigned long g_ulMaxBufferSize;
volatile unsigned long g_ulFlags;
static unsigned long g_ulBytesRemaining;
static unsigned short g_usMinutes;
static unsigned short g_usSeconds;

tWaveHeader g_sWaveHeader;

void QuaChoose1(tWidget *pWidget);
void QuaChoose2(tWidget *pWidget);
void QuaChoose3(tWidget *pWidget);
void TuneChoose1(tWidget *pWidget);
void TuneChoose2(tWidget *pWidget);
void TuneChoose3(tWidget *pWidget);
void OnSliderChange(tWidget *pWidget, long lValue);
void OnBtnPlay1(tWidget *pWidget);
void OnBtnPlay111(tWidget *pWidget);
void OnBtnPlay2(tWidget *pWidget);
void OnBtnPlay222(tWidget *pWidget);
void OnBtnPlay3(tWidget *pWidget);
void OnBtnPlay4(tWidget *pWidget);
void OnBtnPlay444(tWidget *pWidget);
void OnBtnPlay5(tWidget *pWidget);
void OnBtnPlay555(tWidget *pWidget);
void OnBtnPlay6(tWidget *pWidget);
void OnBtnPlay666(tWidget *pWidget);
void OnBtnPlay7(tWidget *pWidget);
static int PopulateFileListBox(tBoolean bRedraw);

extern tCanvasWidget g_sListBackground;
extern tCanvasWidget g_sChooseBackground;
extern tCanvasWidget g_sPlayBackground1;
extern tCanvasWidget g_sPlayBackground11;
extern tCanvasWidget g_sPlayBackground111;
extern tCanvasWidget g_sPlayBackground2;
extern tCanvasWidget g_sPlayBackground22;
extern tCanvasWidget g_sPlayBackground222;
extern tCanvasWidget g_sPlayBackground3;
extern tCanvasWidget g_sPlayBackground33;
extern tCanvasWidget g_sPlayBackground4;
extern tCanvasWidget g_sPlayBackground44;
extern tCanvasWidget g_sPlayBackground444;
extern tCanvasWidget g_sPlayBackground5;
extern tCanvasWidget g_sPlayBackground55;
extern tCanvasWidget g_sPlayBackground555;
extern tCanvasWidget g_sPlayBackground6;
extern tCanvasWidget g_sPlayBackground66;
extern tCanvasWidget g_sPlayBackground666;
extern tCanvasWidget g_sPlayBackground7;
extern tCanvasWidget g_sPlayBackground77;
static int pPlay;
static int a;
static int b;

ListBox(g_sDirList, &g_sListBackground, 0, 0,
        &g_sKitronix320x240x16_SSD2119,
        0, 180, 1, 1, LISTBOX_STYLE_OUTLINE, ClrBlack, ClrDarkBlue,
        ClrSilver, ClrWhite, ClrWhite, &g_sFontCmss12, g_ppcDirListStrings,
        NUM_LIST_STRINGS, 0, 0);

// 屏幕左下方的音色选择和右下方的音调选择。
RectangularButton(g_sQuaBtn1, &g_sChooseBackground, 0, 0,
                  &g_sKitronix320x240x16_SSD2119, 17, 180, 80, 15,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrBlack, ClrDarkBlue, ClrWhite, ClrWhite,
                   &g_sFontCm12,"Piano Ver", 0, 0, 0, 0, QuaChoose1);

RectangularButton(g_sQuaBtn2, &g_sChooseBackground, 0, 0,
                  &g_sKitronix320x240x16_SSD2119, 113, 180, 80, 15,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrBlack, ClrDarkBlue, ClrWhite, ClrWhite,
                   &g_sFontCm12,"Organ Ver", 0, 0, 0, 0, QuaChoose2);
									 
RectangularButton(g_sQuaBtn3, &g_sChooseBackground, 0, 0,
                  &g_sKitronix320x240x16_SSD2119, 210, 180, 80, 15,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrBlack, ClrDarkBlue, ClrWhite, ClrWhite,
                   &g_sFontCm12,"Guitar Ver", 0, 0, 0, 0, QuaChoose3);//音色
									 
RectangularButton(g_sTuneBtn1, &g_sChooseBackground, 0, 0,
                  &g_sKitronix320x240x16_SSD2119, 17, 200, 80, 15,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrBlack, ClrDarkBlue, ClrWhite, ClrWhite,
                   &g_sFontCm12,"C Major", 0, 0, 0, 0, TuneChoose1);									 

RectangularButton(g_sTuneBtn2, &g_sChooseBackground, 0, 0,
                  &g_sKitronix320x240x16_SSD2119, 113, 200, 80, 15,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrBlack, ClrDarkBlue, ClrWhite, ClrWhite,
                   &g_sFontCm12,"D Major", 0, 0, 0, 0, TuneChoose2);
									 
RectangularButton(g_sTuneBtn3 , &g_sChooseBackground, 0, 0,
                  &g_sKitronix320x240x16_SSD2119, 210, 200, 80, 15,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrBlack, ClrDarkBlue, ClrWhite, ClrWhite,
                   &g_sFontCm12,"E Major", 0, 0, 0, 0, TuneChoose3);
									 
Canvas(g_sChooseBackground, WIDGET_ROOT, 0, 0,
       &g_sKitronix320x240x16_SSD2119,    17, 180, 273, 40,
       CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);
			 

// 屏幕右侧的音量调节
#define INITIAL_VOLUME_PERCENT 50
Slider(g_sSlider, &g_sListBackground, 0, 0,
       &g_sKitronix320x240x16_SSD2119, 300, 30, 20, 180, 0, 100,
       INITIAL_VOLUME_PERCENT, (SL_STYLE_FILL | SL_STYLE_BACKG_FILL |
       SL_STYLE_OUTLINE | SL_STYLE_VERTICAL), ClrBlue, ClrBlack, ClrWhite,
       ClrWhite, ClrWhite, 0, 0, 0, 0, OnSliderChange);

Canvas(g_sListBackground, WIDGET_ROOT, &g_sSlider, &g_sDirList, 
      &g_sKitronix320x240x16_SSD2119, 10, 200, 60, 50,
      CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);
			


// 琴键显示
RectangularButton(g_sPlayBtn1, &g_sPlayBackground1, 0, 0,
                  &g_sKitronix320x240x16_SSD2119, 17, 30, 26, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrWhite, ClrDarkBlue, ClrWhite, ClrWhite,
                   &g_sFontCm20,"", 0, 0, 0, 0, OnBtnPlay1);
									 
Canvas(g_sPlayBackground1, WIDGET_ROOT, 0, 0,
       &g_sKitronix320x240x16_SSD2119,    17, 30, 26, 80,
       CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);//do

RectangularButton(g_sPlayBtn11, &g_sPlayBackground11, 0, 0,
                  &g_sKitronix320x240x16_SSD2119, 17, 110, 38, 55,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrWhite, ClrDarkBlue, ClrWhite, ClrWhite,
                   &g_sFontCm20, "", 0, 0, 0, 0, OnBtnPlay1);
									 
Canvas(g_sPlayBackground11, WIDGET_ROOT, 0, 0,
       &g_sKitronix320x240x16_SSD2119,    17, 110, 39, 55,
       CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);//do	

RectangularButton(g_sPlayBtn111, &g_sPlayBackground111, 0, 0,
                  &g_sKitronix320x240x16_SSD2119, 43, 30, 26, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrBlack, ClrDarkBlue, ClrWhite, ClrBlack,
                   &g_sFontCm20, "", 0, 0, 0, 0, OnBtnPlay111);
									 
Canvas(g_sPlayBackground111, WIDGET_ROOT, 0, 0,
       &g_sKitronix320x240x16_SSD2119,    43, 30, 26, 80,
       CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);//#do
			 
RectangularButton(g_sPlayBtn2, &g_sPlayBackground2, 0, 0,
                  &g_sKitronix320x240x16_SSD2119, 69, 30, 13, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrWhite, ClrDarkBlue, ClrWhite, ClrWhite,
                   &g_sFontCm20, "", 0, 0, 0, 0, OnBtnPlay2);
									 
Canvas(g_sPlayBackground2, WIDGET_ROOT, 0, 0,
       &g_sKitronix320x240x16_SSD2119,    69, 30, 13, 80,
       CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);//re
			 
RectangularButton(g_sPlayBtn22, &g_sPlayBackground22, 0, 0,
                  &g_sKitronix320x240x16_SSD2119, 57, 110, 37, 55,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrWhite, ClrDarkBlue, ClrWhite, ClrWhite,
                   &g_sFontCm20, "", 0, 0, 0, 0, OnBtnPlay2);
									 
Canvas(g_sPlayBackground22, WIDGET_ROOT, 0, 0,
       &g_sKitronix320x240x16_SSD2119,    56, 110, 39, 55,
       CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);//re
			 
RectangularButton(g_sPlayBtn222, &g_sPlayBackground222, 0, 0,
                  &g_sKitronix320x240x16_SSD2119, 82, 30, 26, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrBlack, ClrDarkBlue, ClrWhite, ClrBlack,
                   &g_sFontCm20, "", 0, 0, 0, 0, OnBtnPlay222);
									 
Canvas(g_sPlayBackground222, WIDGET_ROOT, 0, 0,
       &g_sKitronix320x240x16_SSD2119,    82, 30, 26, 80,
       CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);//#re
			 
RectangularButton(g_sPlayBtn3, &g_sPlayBackground3, 0, 0,
                  &g_sKitronix320x240x16_SSD2119, 108, 30, 25, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrWhite, ClrDarkBlue, ClrWhite, ClrWhite,
                   &g_sFontCm20,"", 0, 0, 0, 0, OnBtnPlay3);
									 
Canvas(g_sPlayBackground3, WIDGET_ROOT, 0, 0,
       &g_sKitronix320x240x16_SSD2119,    108, 30, 26, 80,
       CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);//mi

RectangularButton(g_sPlayBtn33, &g_sPlayBackground33, 0, 0,
                  &g_sKitronix320x240x16_SSD2119, 96, 110, 37, 55,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrWhite, ClrDarkBlue, ClrWhite, ClrWhite,
                   &g_sFontCm20, "", 0, 0, 0, 0, OnBtnPlay3);
									 
Canvas(g_sPlayBackground33, WIDGET_ROOT, 0, 0,
       &g_sKitronix320x240x16_SSD2119,    95, 110, 39, 55,
       CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);//mi

RectangularButton(g_sPlayBtn4, &g_sPlayBackground4, 0, 0,
                  &g_sKitronix320x240x16_SSD2119, 135, 30, 26, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrWhite, ClrDarkBlue, ClrWhite, ClrWhite,
                   &g_sFontCm20,"", 0, 0, 0, 0, OnBtnPlay4);
									 
Canvas(g_sPlayBackground4, WIDGET_ROOT, 0, 0,
       &g_sKitronix320x240x16_SSD2119,    134, 30, 26, 80,
       CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);//fa

RectangularButton(g_sPlayBtn44, &g_sPlayBackground44, 0, 0,
                  &g_sKitronix320x240x16_SSD2119, 135, 110, 37, 55,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrWhite, ClrDarkBlue, ClrWhite, ClrWhite,
                   &g_sFontCm20, "", 0, 0, 0, 0, OnBtnPlay4);
									 
Canvas(g_sPlayBackground44, WIDGET_ROOT, 0, 0,
       &g_sKitronix320x240x16_SSD2119,    134, 110, 39, 55,
       CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);//fa	

RectangularButton(g_sPlayBtn444, &g_sPlayBackground444, 0, 0,
                  &g_sKitronix320x240x16_SSD2119, 160, 30, 26, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrBlack, ClrDarkBlue, ClrWhite, ClrBlack,
                   &g_sFontCm20, "", 0, 0, 0, 0, OnBtnPlay444);
									 
Canvas(g_sPlayBackground444, WIDGET_ROOT, 0, 0,
       &g_sKitronix320x240x16_SSD2119,    160, 30, 26, 80,
       CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);//#fa

RectangularButton(g_sPlayBtn5, &g_sPlayBackground5, 0, 0,
                  &g_sKitronix320x240x16_SSD2119, 186, 30, 13, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrWhite, ClrDarkBlue, ClrWhite, ClrWhite,
                   &g_sFontCm20, "", 0, 0, 0, 0, OnBtnPlay5);
									 
Canvas(g_sPlayBackground5, WIDGET_ROOT, 0, 0,
       &g_sKitronix320x240x16_SSD2119,    186, 30, 13, 80,
       CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);//so
			 
RectangularButton(g_sPlayBtn55, &g_sPlayBackground55, 0, 0,
                  &g_sKitronix320x240x16_SSD2119, 174, 110, 37, 55,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrWhite, ClrDarkBlue, ClrWhite, ClrWhite,
                   &g_sFontCm20, "", 0, 0, 0, 0, OnBtnPlay5);
									 
Canvas(g_sPlayBackground55, WIDGET_ROOT, 0, 0,
       &g_sKitronix320x240x16_SSD2119,    173, 110, 39, 55,
       CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);//so
			 
RectangularButton(g_sPlayBtn555, &g_sPlayBackground555, 0, 0,
                  &g_sKitronix320x240x16_SSD2119, 199, 30, 26, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrBlack, ClrDarkBlue, ClrWhite, ClrBlack,
                   &g_sFontCm20, "", 0, 0, 0, 0, OnBtnPlay555);
									 
Canvas(g_sPlayBackground555, WIDGET_ROOT, 0, 0,
       &g_sKitronix320x240x16_SSD2119,    199, 30, 26, 80,
       CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);//#so

RectangularButton(g_sPlayBtn6, &g_sPlayBackground6, 0, 0,
                  &g_sKitronix320x240x16_SSD2119, 225, 30, 13, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrWhite, ClrDarkBlue, ClrWhite, ClrWhite,
                   &g_sFontCm20, "", 0, 0, 0, 0, OnBtnPlay6);
									 
Canvas(g_sPlayBackground6, WIDGET_ROOT, 0, 0,
       &g_sKitronix320x240x16_SSD2119,    225, 30, 13, 80,
       CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);//la
			 
RectangularButton(g_sPlayBtn66, &g_sPlayBackground66, 0, 0,
                  &g_sKitronix320x240x16_SSD2119, 213, 110, 37, 55,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrWhite, ClrDarkBlue, ClrWhite, ClrWhite,
                   &g_sFontCm20, "", 0, 0, 0, 0, OnBtnPlay6);
									 
Canvas(g_sPlayBackground66, WIDGET_ROOT, 0, 0,
       &g_sKitronix320x240x16_SSD2119,    212, 110, 39, 55,
       CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);//la
			 
RectangularButton(g_sPlayBtn666, &g_sPlayBackground666, 0, 0,
                  &g_sKitronix320x240x16_SSD2119, 238, 30, 26, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrBlack, ClrDarkBlue, ClrWhite, ClrBlack,
                   &g_sFontCm20, "", 0, 0, 0, 0, OnBtnPlay666);
									 
Canvas(g_sPlayBackground666, WIDGET_ROOT, 0, 0,
       &g_sKitronix320x240x16_SSD2119,    238, 30, 26, 80,
       CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);//#la

RectangularButton(g_sPlayBtn7, &g_sPlayBackground7, 0, 0,
                  &g_sKitronix320x240x16_SSD2119, 264, 30, 26, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrWhite, ClrDarkBlue, ClrWhite, ClrWhite,
                   &g_sFontCm20,"", 0, 0, 0, 0, OnBtnPlay7);
									 
Canvas(g_sPlayBackground7, WIDGET_ROOT, 0, 0,
       &g_sKitronix320x240x16_SSD2119,    264, 30, 26, 80,
       CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);//si

RectangularButton(g_sPlayBtn77, &g_sPlayBackground77, 0, 0,
                  &g_sKitronix320x240x16_SSD2119, 252, 110, 38, 55,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                   PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                   ClrWhite, ClrDarkBlue, ClrWhite, ClrWhite,
                   &g_sFontCm20, "", 0, 0, 0, 0, OnBtnPlay7);
									 
Canvas(g_sPlayBackground77, WIDGET_ROOT, 0, 0,
       &g_sKitronix320x240x16_SSD2119,    251, 110, 39, 55,
       CANVAS_STYLE_FILL, ClrBlack, 0, 0, 0, 0, 0, 0);//si


// 标题栏
Canvas(g_sHeading, WIDGET_ROOT, &g_sListBackground, 0,
       &g_sKitronix320x240x16_SSD2119, 0, 0, 320, 23,
       (CANVAS_STYLE_FILL | CANVAS_STYLE_OUTLINE | CANVAS_STYLE_TEXT),
       ClrBlue, ClrWhite, ClrWhite, &g_sFontCm20, "Piano", 0, 0);



//音色、音调选择函数。
void QuaChoose1(tWidget *pWidget)
{
	a=0;
}

void QuaChoose2(tWidget *pWidget)
{
	a=16;
}

void QuaChoose3(tWidget *pWidget)
{
	a=32;
}

void TuneChoose1(tWidget *pWidget)
{
	b=0;
}

void TuneChoose2(tWidget *pWidget)
{
	b=2;
}

void TuneChoose3(tWidget *pWidget)
{
	b=4;
}
	

// 屏幕右侧音量变化。
void
OnSliderChange(tWidget *pWidget, long lValue)
{
    SoundVolumeSet(lValue);
}

//每个琴键按钮对应的函数。
void OnBtnPlay1(tWidget *pWidget)
{
        pPlay = 0+a+b;
	      if (pPlay == 49)
				{
					pPlay = 0;
				}
        if(WaveOpen(&g_sFileObject, g_pcFilenames[pPlay], &g_sWaveHeader)== FR_OK)
					{
            g_ulFlags |= BUFFER_PLAYING;
}
}

void OnBtnPlay111(tWidget *pWidget)
{
        pPlay = 1+a+b;
	      if (pPlay == 49)
				{
					pPlay = 0;
				}
        if(WaveOpen(&g_sFileObject, g_pcFilenames[pPlay], &g_sWaveHeader)== FR_OK)
					{
            g_ulFlags |= BUFFER_PLAYING;
}
}

void OnBtnPlay2(tWidget *pWidget)
{
        pPlay = 2+a+b;
	      if (pPlay == 49)
				{
					pPlay = 0;
				}
        if(WaveOpen(&g_sFileObject, g_pcFilenames[pPlay], &g_sWaveHeader)== FR_OK)
					{
            g_ulFlags |= BUFFER_PLAYING;
}
}

void OnBtnPlay222(tWidget *pWidget)
{
        pPlay = 3+a+b;
	      if (pPlay == 49)
				{
					pPlay = 0;
				}
        if(WaveOpen(&g_sFileObject, g_pcFilenames[pPlay], &g_sWaveHeader)== FR_OK)
					{
            g_ulFlags |= BUFFER_PLAYING;
}
}

void OnBtnPlay3(tWidget *pWidget)
{
        pPlay = 4+a+b;
	      if (pPlay == 49)
				{
					pPlay = 0;
				}
        if(WaveOpen(&g_sFileObject, g_pcFilenames[pPlay], &g_sWaveHeader)== FR_OK)
					{
            g_ulFlags |= BUFFER_PLAYING;
}
}

void OnBtnPlay4(tWidget *pWidget)
{
        pPlay = 5+a+b;
	      if (pPlay == 49)
				{
					pPlay = 0;
				}
        if(WaveOpen(&g_sFileObject, g_pcFilenames[pPlay], &g_sWaveHeader)== FR_OK)
					{
            g_ulFlags |= BUFFER_PLAYING;
}
}

void OnBtnPlay444(tWidget *pWidget)
{
        pPlay = 6+a+b;
	      if (pPlay == 49)
				{
					pPlay = 0;
				}
        if(WaveOpen(&g_sFileObject, g_pcFilenames[pPlay], &g_sWaveHeader)== FR_OK)
					{
            g_ulFlags |= BUFFER_PLAYING;
}
}

void OnBtnPlay5(tWidget *pWidget)
{
        pPlay = 7+a+b;
	      if (pPlay == 49)
				{
					pPlay = 0;
				}
        if(WaveOpen(&g_sFileObject, g_pcFilenames[pPlay], &g_sWaveHeader)== FR_OK)
					{
            g_ulFlags |= BUFFER_PLAYING;
}
}

void OnBtnPlay555(tWidget *pWidget)
{
        pPlay = 8+a+b;
	      if (pPlay == 49)
				{
					pPlay = 0;
				}
        if(WaveOpen(&g_sFileObject, g_pcFilenames[pPlay], &g_sWaveHeader)== FR_OK)
					{
            g_ulFlags |= BUFFER_PLAYING;
}
}

void OnBtnPlay6(tWidget *pWidget)
{
        pPlay = 9+a+b;
	      if (pPlay == 49)
				{
					pPlay = 0;
				}
        if(WaveOpen(&g_sFileObject, g_pcFilenames[pPlay], &g_sWaveHeader)== FR_OK)
					{
            g_ulFlags |= BUFFER_PLAYING;
}
}

void OnBtnPlay666(tWidget *pWidget)
{
        pPlay = 10+a+b;
	      if (pPlay == 49)
				{
					pPlay = 0;
				}
        if(WaveOpen(&g_sFileObject, g_pcFilenames[pPlay], &g_sWaveHeader)== FR_OK)
					{
            g_ulFlags |= BUFFER_PLAYING;
}
}

void OnBtnPlay7(tWidget *pWidget)
{
        pPlay = 11+a+b;
	      if (pPlay == 49)
				{
					pPlay = 0;
				}
        if(WaveOpen(&g_sFileObject, g_pcFilenames[pPlay], &g_sWaveHeader)== FR_OK)
					{
            g_ulFlags |= BUFFER_PLAYING;
}
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

    //
    // At this point no matching code was found, so return a
    // string indicating unknown error.
    //
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
		//UARTprintf("%d played\n",g_ulBytesPlayed);
		//UARTStringPut(UART0_BASE,"playing..\n");
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
		f_mount(0, &g_sFatFs);
		return(Result);
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
        WidgetMessageQueueProcess();
    }
    WaveClose(pFile);
    return(0);
}

void
SysTickHandler(void)
{
    disk_timerproc();
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
    fresult = f_opendir(&g_sDirObject, "/");
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
                strncpy(g_pcFilenames[ulItemCount], g_sFileInfo.fname,
                         MAX_FILENAME_STRING_LEN);
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

//*****************************************************************************
//
// This function implements the "ls" command.  It opens the current
// directory and enumerates through the contents, and prints a line for
// each item it finds.  It shows details such as file attributes, time and
// date, and the file size, along with the name.  It shows a summary of
// file sizes at the end along with free space.
//
//*****************************************************************************
int
Cmd_ls(int argc, char *argv[])
{
    unsigned long ulTotalSize, ulItemCount, ulFileCount, ulDirCount;
    FRESULT fresult;
    //FATFS *pFatFs;

    //
    // Empty the list box on the display.
    //
    //ListBoxClear(&g_sDirList);

    //
    // Make sure the list box will be redrawn next time the message queue
    // is processed.
    //
    //WidgetPaint((tWidget *)&g_sDirList);

    //
    // Open the current directory for access.
    //
    fresult = f_opendir(&g_sDirObject, g_cCwdBuf);

    //
    // Check for error and return if there is a problem.
    //
    if(fresult != FR_OK)
    {
        //
        // Ensure that the error is reported.
        //
		//UARTprintf("%s\n",(char*)StringFromFresult(fresult));
        //ListBoxTextAdd(&g_sDirList, "Error from SD Card:");
        //ListBoxTextAdd(&g_sDirList, (char *)StringFromFresult(fresult));
        return(fresult);
    }

    ulTotalSize = 0;
    ulFileCount = 0;
    ulDirCount = 0;
    ulItemCount = 0;

    //
    // Give an extra blank line before the listing.
    //
    UARTprintf("\n");

    //
    // Enter loop to enumerate through all directory entries.
    //
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
        UARTprintf("%c%c%c%c%c %u/%02u/%02u %02u:%02u %9u  %s\n",
                 (g_sFileInfo.fattrib & AM_DIR) ? 'D' : '-',
                 (g_sFileInfo.fattrib & AM_RDO) ? 'R' : '-',
                 (g_sFileInfo.fattrib & AM_HID) ? 'H' : '-',
                 (g_sFileInfo.fattrib & AM_SYS) ? 'S' : '-',
                 (g_sFileInfo.fattrib & AM_ARC) ? 'A' : '-',
                 (g_sFileInfo.fdate >> 9) + 1980,
                 (g_sFileInfo.fdate >> 5) & 15,
                 g_sFileInfo.fdate & 31,
                 (g_sFileInfo.ftime >> 11),
                 (g_sFileInfo.ftime >> 5) & 63,
                 g_sFileInfo.fsize,
                 g_sFileInfo.fname);

        //
        // Add the information as a line in the listbox widget.
        //
        if(ulItemCount < NUM_LIST_STRINGS)
        {
            usprintf(g_pcFilenames[ulItemCount], "(%c) %12s",
                     (g_sFileInfo.fattrib & AM_DIR) ? 'D' : 'F',
                      g_sFileInfo.fname);
            //ListBoxTextAdd(&g_sDirList, g_pcFilenames[ulItemCount]);
        }

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

        //
        // Wait for the UART transmit buffer to empty.
        //
        //UARTFlushTx(false);

    }   // endfor

    //
    // Print summary lines showing the file, dir, and size totals.
    //
    UARTprintf("\n%4u File(s),%10u bytes total\n%4u Dir(s)",
                ulFileCount, ulTotalSize, ulDirCount);

    //
    // Get the free space.
    //
    //fresult = f_getfree("/", &ulTotalSize, &pFatFs);

    //
    // Check for error and return if there is a problem.
    //
    //if(fresult != FR_OK)
    //{
    //    return(fresult);
    //}

    //
    // Display the amount of free space that was calculated.
    //
    //UARTprintf(", %10uK bytes free\n", ulTotalSize * pFatFs->sects_clust / 2);

    //
    // Wait for the UART transmit buffer to empty.
    //
    //UARTFlushTx(false);

    //
    // Made it to here, return with no errors.
    //
    return(0);
}

#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
}
#endif

int
main(void)
{
    FRESULT fresult;
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_16MHZ);
	
	GPIOInitial();
	UART0Initial();
	//
    // Set GPIO A0 and A1 as UART.
    //
    //ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    //
    // Initialize the UART as a console for text I/O.
    //
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

    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sHeading);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sPlayBtn1);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sPlayBtn11);
		WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sPlayBtn111);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sPlayBtn2);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sPlayBtn22);
		WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sPlayBtn222);    
		WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sPlayBtn3);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sPlayBtn33);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sPlayBtn4);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sPlayBtn44);
		WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sPlayBtn444);    
		WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sPlayBtn5);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sPlayBtn55);
		WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sPlayBtn555);    
		WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sPlayBtn6);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sPlayBtn66);
		WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sPlayBtn666);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sPlayBtn7);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sPlayBtn77);	 
		WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sQuaBtn1);
		WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sQuaBtn2);
		WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sQuaBtn3);
		WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sTuneBtn1);
		WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sTuneBtn2);
		WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sTuneBtn3);
    WidgetPaint(WIDGET_ROOT);
    WidgetMessageQueueProcess();

    fresult = f_mount(0, &g_sFatFs);
    if(fresult != FR_OK)
    {
        return(1);
    }
    PopulateFileListBox(true);

	Cmd_ls(0,NULL);

    g_ulFlags = 0;
    SoundInit(0);

	if(WaveOpen(&g_sFileObject, "bara.wav", &g_sWaveHeader)== FR_OK){
    	g_ulFlags |= BUFFER_PLAYING;
	}

    while(1)
    {
        if(g_ulFlags & BUFFER_PLAYING)
        {
            WavePlay(&g_sFileObject, &g_sWaveHeader);
        }
        WidgetMessageQueueProcess();
    }
}
