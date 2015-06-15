#include "HardwareLibrary.h"
#include "LuminaryDriverLibrary.h"
							
#include "grlib/grlib.h"
#include "grlib/widget.h"
#include "grlib/canvas.h"			
#include "utils/locator.h"
#include "utils/lwiplib.h"			   
#include "utils/uartstdio.h"

#include "EthernetConfigure.h"

static char g_pcTwirl[4] = { '\\', '|', '/', '-' };
static unsigned long g_ulTwirlPos = 0;
static unsigned long g_ulLastIPAddr = 0;

void EthernetInitial(void){

	unsigned long ulUser0,ulUser1;
	unsigned char pucMACArray[6];

    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_ETH);
    ROM_SysCtlPeripheralReset(SYSCTL_PERIPH_ETH);		

	/*ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinConfigure(GPIO_PF2_LED1);
    GPIOPinConfigure(GPIO_PF3_LED0);
    GPIOPinTypeEthernetLED(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3);
	*/
	ROM_FlashUserGet(&ulUser0, &ulUser1);
    if((ulUser0 == 0xffffffff) || (ulUser1 == 0xffffffff))
    {
        UARTprintf("MAC Address Not Programmed!\n");
    }

    pucMACArray[0] = ((ulUser0 >>  0) & 0xff);
    pucMACArray[1] = ((ulUser0 >>  8) & 0xff);
    pucMACArray[2] = ((ulUser0 >> 16) & 0xff);
    pucMACArray[3] = ((ulUser1 >>  0) & 0xff);
    pucMACArray[4] = ((ulUser1 >>  8) & 0xff);
    pucMACArray[5] = ((ulUser1 >> 16) & 0xff);

    //
    // Initialze the lwIP library, using DHCP.
    //
    lwIPInit(pucMACArray, 0, 0, 0, IPADDR_USE_DHCP);

    //
    // Setup the device locator service.
    //
    LocatorInit();
    LocatorMACAddrSet(pucMACArray);
    LocatorAppTitleSet("EK-LM3S9B92 enet_lwip");				
    UARTprintf("Waiting for IP... ");

}

//*****************************************************************************
//
// Required by lwIP library to support any host-related timer functions.
//
//*****************************************************************************
char g_cIPAddress[20];																		    
extern tCanvasWidget g_sListHeading;

void lwIPHostTimerHandler(void)
{
    unsigned long ulIPAddress;

    //
    // Get the local IP address.
    //
    ulIPAddress = lwIPLocalIPAddrGet();

    //
    // See if an IP address has been assigned.
    //
    if(ulIPAddress == 0)
    {
        //
        // Draw a spinning line to indicate that the IP address is being
        // discoverd.
        //
        UARTprintf("\b%c", g_pcTwirl[g_ulTwirlPos]);

        //
        // Update the index into the twirl.
        //
        g_ulTwirlPos = (g_ulTwirlPos + 1) & 3;
    }

    //
    // Check if IP address has changed, and display if it has.
    //
    else if(ulIPAddress != g_ulLastIPAddr)
    {
        //
        // Display the new IP address.
        //
		usprintf(g_cIPAddress,"%d.%d.%d.%d", ulIPAddress & 0xff,
                   (ulIPAddress >> 8) & 0xff, (ulIPAddress >> 16) & 0xff,
                   (ulIPAddress >> 24) & 0xff);
        UARTprintf("\rIP: %d.%d.%d.%d       \n",ulIPAddress & 0xff,
                   (ulIPAddress >> 8) & 0xff, (ulIPAddress >> 16) & 0xff,
                   (ulIPAddress >> 24) & 0xff);
		UARTprintf("%s\n",g_cIPAddress);
		CanvasTextSet(&g_sListHeading,g_cIPAddress); 
		WidgetPaint((tWidget*)&g_sListHeading);

        //
        // Save the new IP address.
        //
        g_ulLastIPAddr = ulIPAddress;

        //
        // Display the new network mask.
        //
        ulIPAddress = lwIPLocalNetMaskGet();
        UARTprintf("Netmask: %d.%d.%d.%d\n", ulIPAddress & 0xff,
                   (ulIPAddress >> 8) & 0xff, (ulIPAddress >> 16) & 0xff,
                   (ulIPAddress >> 24) & 0xff);

        //
        // Display the new gateway address.
        //
        ulIPAddress = lwIPLocalGWAddrGet();
        UARTprintf("Gateway: %d.%d.%d.%d\n", ulIPAddress & 0xff,
                   (ulIPAddress >> 8) & 0xff, (ulIPAddress >> 16) & 0xff,
                   (ulIPAddress >> 24) & 0xff);

    }
}
