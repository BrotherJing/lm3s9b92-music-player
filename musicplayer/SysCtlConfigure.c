#include "HardwareLibrary.h"
#include "LuminaryDriverLibrary.h"

#include "SysCtlConfigure.h"	 

unsigned long TheSysClock = 16000000UL;

//  系统时钟初始化
void ClockInitial(void)
{
	
	SysCtlLDOSet(SYSCTL_LDO_2_75V);			//  配置PLL前须将LDO设为2.75V
	SysCtlClockSet(SYSCTL_USE_PLL |			//  系统时钟设置，采用PLL
                   SYSCTL_OSC_MAIN |		//  主振荡器
                   SYSCTL_XTAL_16MHZ |		//  外接16MHz晶振
                   SYSCTL_SYSDIV_4);		//  分频结果为50MHz

	TheSysClock = SysCtlClockGet();			//  获取当前的系统时钟频率
}

