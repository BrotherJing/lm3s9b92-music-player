#ifndef	__GPIODRIVERCONFIGURE_H__
#define __GPIODRIVERCONFIGURE_H__

#define LCD_KITRONIX320X240X16_SSD2119

#define LED_0						0
#define LED_1						1
#define LED_ALL						2

#define KEY_PRESS					1
#define KEY_LEFT					2
#define KEY_RIGHT					3
#define KEY_UP						4
#define KEY_DOWN					5

#define LED0_PIN					GPIO_PIN_3
#define LED0_BASE					GPIO_PORTF_BASE
#define LED1_PIN					GPIO_PIN_2
#define LED1_BASE					GPIO_PORTF_BASE

#define KEY_PRESS_PIN				GPIO_PIN_5
#define KEY_PRESS_BASE				GPIO_PORTE_BASE
#define KEY_LEFT_PIN				GPIO_PIN_4
#define KEY_LEFT_BASE				GPIO_PORTB_BASE
#define KEY_RIGHT_PIN				GPIO_PIN_6
#define KEY_RIGHT_BASE				GPIO_PORTB_BASE
#define KEY_UP_PIN					GPIO_PIN_4
#define KEY_UP_BASE					GPIO_PORTE_BASE
#define KEY_DOWN_PIN				GPIO_PIN_1
#define KEY_DOWN_BASE				GPIO_PORTF_BASE

#define UART0RX_PIN					GPIO_PIN_0
#define UART0TX_PIN					GPIO_PIN_1
#define UART0_PIN_BASE				GPIO_PORTA_BASE

#define UART1RX_PIN					GPIO_PIN_0
#define UART1TX_PIN					GPIO_PIN_1
#define UART1_PIN_BASE				GPIO_PORTD_BASE

#define I2C0SCL_PIN					GPIO_PIN_2
#define I2C0SDA_PIN					GPIO_PIN_3
#define I2C0_PIN_BASE				GPIO_PORTB_BASE

#define BUZZER_PIN					GPIO_PIN_3
#define BUZZER_BASE					GPIO_PORTF_BASE

#ifdef	LCD_KITRONIX320X240X16_SSD2119
	#define LCD_DATA_PINS			0xFF
	#define LCD_DATA_BASE			GPIO_PORTD_BASE
	
	#define LCD_RST_PIN				GPIO_PIN_7
	#define LCD_RST_BASE			GPIO_PORTB_BASE
	#define LCD_RD_PIN				GPIO_PIN_5
	#define LCD_RD_BASE				GPIO_PORTB_BASE
	#define LCD_WR_PIN				GPIO_PIN_6
	#define LCD_WR_BASE				GPIO_PORTH_BASE
	#define LCD_DC_PIN				GPIO_PIN_7
	#define LCD_DC_BASE				GPIO_PORTH_BASE

	#define TOUCHSCREEN_XP_PIN		GPIO_PIN_6
	#define TOUCHSCREEN_YP_PIN		GPIO_PIN_7
	#define TOUCHSCREEN_P_BASE		GPIO_PORTE_BASE

	#define TOUCHSCREEN_XN_PIN		GPIO_PIN_2
	#define TOUCHSCREEN_YN_PIN		GPIO_PIN_3
	#define TOUCHSCREEN_N_BASE		GPIO_PORTE_BASE
#endif

#define THUMBWHEEL_PIN				GPIO_PIN_4
#define THUMBWHEEL_BASE				GPIO_PORTB_BASE
#define ADC_VREF_PIN				GPIO_PIN_6
#define ADC_VREF_BASE				GPIO_PORTB_BASE

#define ACCELEROMETER_INT_PIN		GPIO_PIN_2
#define ACCELEROMETER_INT_BASE		GPIO_PORTF_BASE

extern void GPIOInitial(void);

extern void LEDOn(unsigned char LEDNum); 
extern void LEDOff(unsigned char LEDNum);
extern void LEDOverturn(unsigned char LEDNum);

extern unsigned char GetKeyNumber(void);

extern unsigned char I2C0PullUpTest(void);

#endif
