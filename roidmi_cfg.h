#ifndef __ROIDMI_CFG_H
#define __ROIDMI_CFG_H

#if (1 == ROIDMI)

#include <stdint.h>
#include "gpio.h"
#include "adc.h"
#include "pwm.h"

#include "core_cminstr.h"
#include "ke_msg.h"
#include "app.h"
#include "app_api.h"
#include "prf_utils.h"
#include "attm_db.h"
#include "app_log.h"

//#define log_printf(...)      	//arch_printf(__VA_ARGS__)
//SPI GPIO
#define SPI_GPIO_PORT  GPIO_PORT_0
#define SPI_CLK_PIN    GPIO_PIN_0
#define SPI_CS_PIN     GPIO_PIN_3
#define SPI_DI_PIN     GPIO_PIN_5
#define SPI_DO_PIN     GPIO_PIN_6    
// SPI Flash options
#define SPI_FLASH_SIZE 131072			   // SPI Flash memory size in bytes
#define SPI_FLASH_PAGE 256                 // SPI Flash memory page size in bytes
//SPI initialization parameters
#define SPI_WORD_MODE  SPI_8BIT_MODE       // Select SPI bit mode
#define SPI_SMN_MODE   SPI_MASTER_MODE     // {SPI_MASTER_MODE, SPI_SLAVE_MODE}
#define SPI_POL_MODE   SPI_CLK_INIT_HIGH   // {SPI_CLK_INIT_LOW, SPI_CLK_INIT_HIGH}
#define SPI_PHA_MODE   SPI_PHASE_1         // {SPI_PHA_MODE_0, SPI_PHA_MODE_1}
#define SPI_MINT_EN    SPI_NO_MINT         // {SPI_MINT_DISABLE, SPI_MINT_ENABLE}
#define SPI_CLK_DIV    SPI_XTAL_DIV_2      // Select SPI clock divider between 8, 4, 2 and 14

/*******UART GPIO*******/
#define GPIO_PORT_UART_TX  GPIO_PORT_0
#define GPIO_PIN_UART_TX   GPIO_PIN_4

/******ADC Sample GPIO*******/
#define ADC_PORT	GPIO_PORT_0
#define ADC_PIN		GPIO_PIN_1
#define GPIO_ADC_CHANNEL	  ADC_CHANNEL_P02
/*******按键以及门开关 GPIO*******/
#define KEY_PORT GPIO_PORT_2
#define KEY_PIN  GPIO_PIN_0
#define GPIO_IRQn_KEY  GPIO2_IRQn
#define DOOR_PORT  GPIO_PORT_2
#define DOOR_PIN   GPIO_PIN_5
#define GPIO_IRQn_DKEY GPIO2_IRQn
/*******风机 GPIO*******/
#define FAN1_PORT	GPIO_PORT_2
#define FAN1_PIN	GPIO_PIN_1				
#define FAN2_PORT	GPIO_PORT_0
#define FAN2_PIN	GPIO_PIN_7

#define FANFB_PORT	GPIO_PORT_1
#define FANFB1_PIN	GPIO_PIN_1
#define FANFB2_PIN	GPIO_PIN_0
/*******Beep、LED GPIO*******/
#define BEEP_PORT	GPIO_PORT_2
#define BEEP_PIN	GPIO_PIN_6

#define RGB_PORT	GPIO_PORT_2
#define RED_PIN		GPIO_PIN_7
#define GREEN_PIN 	GPIO_PIN_8
#define BLUE_PIN 	GPIO_PIN_9
/*******PM2.5接口*******/
#define PM25_PORT  GPIO_PORT_2
#define PM25_PIN   GPIO_PIN_3
#define PM25POWER_PORT  GPIO_PORT_2
#define PM25POWER_PIN   GPIO_PIN_4

/*******IO口高低电平*******/
#define BEEP_ON		GPIO_SetActive(BEEP_PORT, BEEP_PIN)
#define BEEP_OFF	GPIO_SetInactive(BEEP_PORT, BEEP_PIN)

#define LED_R_ON	GPIO_SetActive(RGB_PORT, RED_PIN)
#define LED_R_OFF	GPIO_SetInactive(RGB_PORT, RED_PIN)

#define LED_G_ON	GPIO_SetActive(RGB_PORT, GREEN_PIN)
#define LED_G_OFF	GPIO_SetInactive(RGB_PORT, GREEN_PIN)

#define LED_B_ON	GPIO_SetActive(RGB_PORT, BLUE_PIN)
#define LED_B_OFF	GPIO_SetInactive(RGB_PORT, BLUE_PIN)

#define PM25_POWER_ON	GPIO_SetActive(PM25POWER_PORT, PM25POWER_PIN)
#define PM25_POWER_OFF	GPIO_SetInactive(PM25POWER_PORT, PM25POWER_PIN)

#define RESET_FLAG		0X55AA5A5A
#define FILTER_TIME		3240000 //3240000*120 设备寿命
#define FILTER_TIME1	3240000
#define HERT_TIME		36

#define WORK	1
#define STANDBY	0

#define OPEN	1
#define CLOSE	0

#define POWER_LOW   242//365//319//300   11.5V
#define POWER_ON    252//548//430//360   12.0V
#define POWER_HIGH  339//650//370		 16.0V

/*******空气质量状态*******/
#define AirGood   1
#define AirNormal 2
#define AirBad    3

#define PWM_Time	(10 * 1000)   	  //5ms
//FAN MODEL
#define Auto_State   0
#define Low_State    1
#define Mid_State    2
#define High_State   3
enum
{
	HighSpeed = 1,
	MidSpeed,
	LowSpeed,
};


#endif
#endif
