#if (1 == ROIDMI)
#include "pwm.h"
#include "uart.h"
#include "spi_flash.h"
#include "app_console.h"
#include "streamdatad.h"
#include "roidmi_includes.h"

uint8_t BLEAddress[6];
/*******Button\LED\Buzzer related*******/
bool BeepOneFlag;
bool BeepTwoFlag;
bool BeepLongFlag;
bool BeepThreeFlag;
bool BeepSet;
bool KeyLongFlag;
bool KeyShortFlag;
bool b_Hand_Flag;
bool CarbonLedSet;
bool CommonLedSet;
uint8_t KeyValue;

/*******voltage related********/
bool SampleFinishFlag;
bool PowerOverLoadFlag;
uint16_t VoltageValue;
uint16_t VoltageLowTime;
uint16_t VolatgeOnTime;
uint16_t VoltageOverTime;

/*******PM2.5sensor related********/
float PMSensorLPOT = 0.0;
uint16_t PMSensorLow;
uint16_t PMSampleTime;
uint16_t PMSensorLowTime;
uint16_t PMSensorHigh;
uint16_t PMSensorHighTime;
uint16_t PMSampleValue;
uint16_t PMSampleValueNew;
uint16_t PMSampleValueOld;
uint8_t  AirQuality;

/*******Wind Turbine related*******/
uint8_t FanCurrentSpeed;
uint8_t FanCurrentState;
uint8_t SystemStatus = 1;
uint16_t Fan1Error;
uint16_t Fan2Error;

/*******Filter life related*******/
bool ChangeFilterFlag = false;
bool ChangeFilterFlagOld;
uint8_t FilterType;
uint8_t FilterTypeOld;

/*******Device Work related*******/
uint8_t SystemStatus;
uint8_t WorkMode;
uint8_t WarmUp;
bool b_Open_Flag;
bool b_Auto_Open_Flag;
bool b_Stay_Flag;
bool b_Close_Flag;
bool b_Force_Flag;
bool APPisConnected;

/*******Time related*******/
bool SecondFlag;
uint16_t WarmUpTime;
uint32_t DeviceWorkTime;
uint16_t ui_ADC_Time;
uint16_t ui_Air_Cnt1;
uint16_t AirQualityContinueTime;
uint16_t EEPROMWriteTime;

/*******Factory related********/
bool b_Factory_Key_Long_Flag;
bool b_Factory_Key_Short_Flag;
bool b_BD_Flag;
bool b_Power_Low_Flag;
bool b_Working_Flag;
bool b_Factory_Key_Down;
bool b_Factory_Flag;
uint16_t ui_Factory_Time;
uint8_t  uc_Factory_Step;
uint8_t  uc_Factory_Mode;

bool b_ADC_OK_Flag;
bool b_Time100ms_Flag;
bool OtaisRunning = false;

uint8_t  uc_Flash_Flag;
uint16_t ui_Flash_Time;

uint8_t  uc_FB1_Over_L = 0;  
uint8_t  uc_FB1_Data_L = 0;
uint8_t	 uc_FB1_CNT_L = 0;
uint8_t  uc_FB2_Over_L = 0;
uint8_t  uc_FB2_Data_L = 0;
uint8_t	 uc_FB2_CNT_L = 0;

uint8_t ErrorCode;

uint16_t ui_adc_data2[10];
uint16_t ui_AD_Value = 0;


struct ROIDMI_FLASH_SAVE roidmi_flash_save_data;
struct AD_FLASH_SAVE AD_SAVE;
	
uint8_t ul_RestData;
uint16_t ui_Close_Delay_Time;
uint32_t RoidmiUserID = 0x20180502;
uint8_t SystemStatusOld;
uint8_t WorkModeOld;
uint8_t AirQualityLevelOld;

uint16 ui_Standby_LED_Time;//2016.9.18

uint8_t uc_Work_Mode_Set = 0;//2016.9.18
uint32_t ul_First_Time;  //2016.9.18

uint8_t uc_Car_Style = 0;

struct AD_FLASH_SAVE  AD_SAVE_TEMP;//2016.9.23
uint16_t ui_Power_Value_New;//2016.9.23

uint16_t SendErrorCode;
uint16_t SendErrorCodeOld;
uint8_t  uc_PWM_Bar_Data;

void SR_Air_Get(void);
void Motor_FeedBack(void);

void PWM_Config(void){
	set_tmr_enable(CLK_PER_REG_TMR_ENABLED);
	timer2_init(HW_CAN_NOT_PAUSE_PWM_2_3_4, PWM_2_3_4_SW_PAUSE_ENABLED, PWM_Time);
	timer2_set_pwm3_duty_cycle(0);
	timer2_set_pwm4_duty_cycle(0);
	timer2_set_sw_pause(PWM_2_3_4_SW_PAUSE_DISABLED); // release sw pause to let pwm2,pwm3,pwm4 run
}

void timer_init(void){ //0.5ms interrupt
	set_tmr_enable(CLK_PER_REG_TMR_ENABLED);
	set_tmr_div(CLK_PER_REG_TMR_DIV_8);
	timer0_init(TIM0_CLK_FAST, PWM_MODE_ONE, TIM0_CLK_DIV_BY_10);
	timer0_set(100, 0, 0);
	timer0_enable_irq();
	timer0_start();
}

 /******fan speed control*****/
void Fan_Speed(uint16_t Out){
	timer2_set_pwm3_duty_cycle(Out);
	timer2_set_pwm4_duty_cycle(Out);
}

void Delay_us(uint16_t nof_us){
	while( nof_us-- ){
		__nop();
		__nop();
		__nop();
		__nop();
		__nop();
		__nop();
		__nop();
		__nop();
		__nop();
		__nop();
		__nop();
	}
}

//************************************2016.8.22
uint16_t ui_FA_CNT_L;
uint8_t  uc_FA_Over_L;
uint16_t ui_FA_Data_L;

void Factory_Air_Get(void){
	if(GPIO_GetPinStatus(GPIO_PORT_2,GPIO_PIN_3)){
		ui_FA_CNT_L++;
		uc_FA_Over_L=1;
	}
	else{
		if(uc_FA_Over_L==1){
			uc_FA_Over_L = 0;
			ui_FA_Data_L = ui_FA_CNT_L/2;
		}
		ui_FA_CNT_L=0;
	}
}
//************************************************

void timer_callback(void)
{
	static unsigned char HalfMicroseconds;
	static unsigned int  Microseconds;
	ui_Factory_Time++;
	if(!b_Factory_Flag){
		Motor_FeedBack();
		SR_Air_Get();
	}
	else{
		PowerOverLoadFlag = 0;
		FanCurrentState = OPEN;
		Motor_FeedBack();
		Factory_Air_Get();
	}
	
	if(++HalfMicroseconds >= 2){
		HalfMicroseconds = 0;
		Microseconds++;
		if(Microseconds >=1000){
			Microseconds = 0;
			SecondFlag = 1;
			WarmUpTime++;
			if(FanCurrentState == OPEN){
				EEPROMWriteTime++;
			}
		}
	}
}

void Beep_Ring(uint16_t ContinueTime){
	if(BeepSet == 0){
		while(ContinueTime--){
			BEEP_ON;
			adc_usDelay(100);
			BEEP_OFF;
			adc_usDelay(200);
		}
	}
}

void Beep_Control (void){
	if(BeepSet==0){
		if(BeepOneFlag == 1){ //beep one
			if(!BeepLongFlag){
				Beep_Ring(300);
				BeepOneFlag = 0;
			}
			else{
				Beep_Ring(1500);
				BeepOneFlag = 0;
				BeepLongFlag = 0;
			}
		}
		if(BeepTwoFlag == 1){
			Beep_Ring(300);
			adc_usDelay(50000);
			BEEP_OFF;
			adc_usDelay(50000);
			Beep_Ring(300);
			adc_usDelay(50000);
			BEEP_OFF;
			BeepTwoFlag = 0;
		}
		if(BeepThreeFlag == 1){
			Beep_Ring(250);
			adc_usDelay(25000);
			BEEP_OFF;
			adc_usDelay(25000);
			Beep_Ring(250);
			adc_usDelay(25000);
			BEEP_OFF;
			adc_usDelay(25000);
			Beep_Ring(250);
			adc_usDelay(25000);
			BEEP_OFF;
			BeepThreeFlag = 0;
		}
	}
}

void Beep_Two(void){
	Beep_Ring(300);
	adc_usDelay(50000);
	BEEP_OFF;
	adc_usDelay(50000);
	Beep_Ring(300);
	adc_usDelay(50000);
	BEEP_OFF;
}


void EEPROM_Read(void)
{
	struct ROIDMI_FLASH_SAVE roidmi_flash_save_data0;
	struct ROIDMI_FLASH_SAVE roidmi_flash_save_data1;
	spi_flash_read_data((uint8_t *)&roidmi_flash_save_data0.FlashErasetimesEE, ROIDMI_FLASH_OFFSET_0, sizeof(struct ROIDMI_FLASH_SAVE));
	spi_flash_read_data((uint8_t *)&roidmi_flash_save_data1.FlashErasetimesEE, ROIDMI_FLASH_OFFSET_1, sizeof(struct ROIDMI_FLASH_SAVE));
	if ((roidmi_flash_save_data0.FlashErasetimesEE == -1) && (roidmi_flash_save_data1.FlashErasetimesEE == -1)){
		memset((uint8_t *)&roidmi_flash_save_data.FlashErasetimesEE, 0,  sizeof(struct ROIDMI_FLASH_SAVE));
	}
	else{
		if (roidmi_flash_save_data0.FlashErasetimesEE >= roidmi_flash_save_data1.FlashErasetimesEE)
			memcpy((uint8_t *)&roidmi_flash_save_data.FlashErasetimesEE, (uint8_t *)&roidmi_flash_save_data0.FlashErasetimesEE, sizeof(struct ROIDMI_FLASH_SAVE));
		else
			memcpy((uint8_t *)&roidmi_flash_save_data.FlashErasetimesEE, (uint8_t *)&roidmi_flash_save_data1.FlashErasetimesEE, sizeof(struct ROIDMI_FLASH_SAVE));
	}
}

void EEPROM_Write(void){
	struct ROIDMI_FLASH_SAVE roidmi_flash_save_data0;
	struct ROIDMI_FLASH_SAVE roidmi_flash_save_data1;
	SysTick->CTRL &= ~(1 << 1);
	spi_flash_read_data((uint8_t *)&roidmi_flash_save_data0.FlashErasetimesEE, ROIDMI_FLASH_OFFSET_0, sizeof(struct ROIDMI_FLASH_SAVE));
	spi_flash_read_data((uint8_t *)&roidmi_flash_save_data1.FlashErasetimesEE, ROIDMI_FLASH_OFFSET_1, sizeof(struct ROIDMI_FLASH_SAVE));
	roidmi_flash_save_data.FlashErasetimesEE++;
	if ((roidmi_flash_save_data0.FlashErasetimesEE <= roidmi_flash_save_data1.FlashErasetimesEE) || (roidmi_flash_save_data0.FlashErasetimesEE == -1)){
		spi_flash_block_erase(ROIDMI_FLASH_OFFSET_0, SECTOR_ERASE);
		__nop();
		spi_flash_write_data((uint8_t *)&roidmi_flash_save_data.FlashErasetimesEE, ROIDMI_FLASH_OFFSET_0, sizeof(struct ROIDMI_FLASH_SAVE));
		__nop();
	}
	else{
		spi_flash_block_erase(ROIDMI_FLASH_OFFSET_1, SECTOR_ERASE);
		__nop();
		spi_flash_write_data((uint8_t *)&roidmi_flash_save_data.FlashErasetimesEE, ROIDMI_FLASH_OFFSET_1, sizeof(struct ROIDMI_FLASH_SAVE));
		__nop();
	}
	SysTick->CTRL |= (1 << 1);
}

/*******EEPROM数据保存*******/
void EEPROM_Save(void){
	if(EEPROMWriteTime >= 300){
		EEPROM_Write();
		EEPROMWriteTime = 0;
	}
}

/**
 ****************************************************************************************
 * @brief SPI and SPI flash Initialization function
  * 
 ****************************************************************************************
 */
static void spi_flash_peripheral_init()
{
	SPI_Pad_t spi_FLASH_CS_Pad;
	int8_t detected_spi_flash_device_index;
	spi_FLASH_CS_Pad.pin  = SPI_CS_PIN;
	spi_FLASH_CS_Pad.port = SPI_GPIO_PORT;
	spi_init(&spi_FLASH_CS_Pad, SPI_MODE_8BIT, SPI_ROLE_MASTER, SPI_CLK_IDLE_POL_LOW, SPI_PHA_MODE_0, SPI_MINT_DISABLE, SPI_XTAL_DIV_8);  
	detected_spi_flash_device_index = spi_flash_auto_detect();  
	if (detected_spi_flash_device_index == SPI_FLASH_AUTO_DETECT_NOT_DETECTED){
		spi_flash_init(SPI_FLASH_SIZE, SPI_FLASH_PAGE);
	}
}

void Data_Init(void)
{
	ChangeFilterFlagOld = 0;	
	b_Open_Flag = 0;
	b_Auto_Open_Flag = 0;
	b_Stay_Flag = 0;
	b_Close_Flag = 0;
	b_Force_Flag = 0;
	APPisConnected = 0;
	PowerOverLoadFlag = 0;
	SampleFinishFlag = 0;
	b_ADC_OK_Flag = 0;
	SecondFlag = 0;
	ChangeFilterFlag = 0;
	BeepOneFlag = 0;
	BeepTwoFlag = 0;
	BeepLongFlag = 0;
	BeepThreeFlag = 0;
	KeyValue = 0;
	FanCurrentSpeed = 0;
	FanCurrentState = 0;
	SystemStatus = 1;
	WorkMode = roidmi_flash_save_data.WorkModeEE; //remember state
	WarmUp = 0;
	AirQuality = 0;
	uc_Flash_Flag = 0;
	ui_Flash_Time = 0;
	uc_FB1_Over_L = 0;
	uc_FB1_Data_L = 0;
	uc_FB1_CNT_L = 0;
	uc_FB2_Over_L = 0;
	uc_FB2_Data_L = 0;
	uc_FB2_CNT_L = 0;
	Fan1Error = 0;
	Fan2Error = 0;
	ErrorCode = 0;
	ui_ADC_Time = 0;
	AirQualityContinueTime = 0;
	ui_Air_Cnt1 = 0;
	WarmUpTime = 0;
	EEPROMWriteTime = 0;
	PMSensorLow = 0;
	PMSampleTime = 0;
	PMSensorLowTime = 0;
	PMSensorHigh = 0;
	PMSensorHighTime = 0;
	PMSensorLPOT = 0.0;
	ui_AD_Value = 0;
	VoltageValue = 0;
	VoltageLowTime = 0;
	WorkMode = 0;
	ui_Close_Delay_Time = 0;
	ui_Standby_LED_Time=0; //2016.9.18
	uc_Work_Mode_Set=0;
	BeepSet= roidmi_flash_save_data.BeepModeEE; //2016.9.18
	CarbonLedSet = roidmi_flash_save_data.LedModeEE; //2017.3.17
	CommonLedSet = 0;
	b_Power_Low_Flag = 0;
	b_BD_Flag = 0;
//	b_Beep_Two_Power_On = 0;
//	b_Beep_Two_Auto_Flag = 0;
//	b_Beep_Long_Auto_Flag = 0;
//	b_Beep_Long_Power_On = 0;
	FilterType  = 0;
}

void AD_Flash_Read(void)
{
	spi_flash_read_data((uint8_t *)&AD_SAVE_TEMP.uc_AD_Flash_Flag, ROIDMI_FLASH_OFFSET_3, sizeof(struct AD_FLASH_SAVE));
	if(AD_SAVE_TEMP.uc_AD_Flash_Flag==-1){
		AD_SAVE.uc_AD_Flash_Flag = 0;
		AD_SAVE.f_AD_Ratio = 1.0;
	}
	else{
		AD_SAVE.f_AD_Ratio = AD_SAVE_TEMP.f_AD_Ratio;
		memcpy((uint8_t *)&AD_SAVE.uc_AD_Flash_Flag, (uint8_t *)&AD_SAVE_TEMP.uc_AD_Flash_Flag, sizeof(struct AD_FLASH_SAVE));
	}
}

/*****/
void System_Init(void)
{
	//struct AD_FLASH_SAVE  AD_SAVE_TEMP;
	if (ul_RestData != RESET_FLAG){
		Data_Init();
		ul_RestData = RESET_FLAG;
	}
	spi_flash_peripheral_init();
	EEPROM_Read();
	WorkMode = roidmi_flash_save_data.WorkModeEE;  
	BeepSet = roidmi_flash_save_data.BeepModeEE;
	CarbonLedSet = roidmi_flash_save_data.LedModeEE;
	FilterType = roidmi_flash_save_data.FilterTypeEE;
	DeviceWorkTime = roidmi_flash_save_data.DeviceWorkTimeEE;
	
	uc_Car_Style = roidmi_flash_save_data.uc_Car_EE;
	uc_PWM_Bar_Data = roidmi_flash_save_data.uc_PWM_Data_EE;
	ul_First_Time = roidmi_flash_save_data.ul_Work_First_Time;
	
	EEPROM_Write();
	spi_flash_read_data((uint8_t *)&RoidmiUserID, ROIDMI_FLASH_OFFSET_2, 4);
	if (RoidmiUserID == 0xffffffff){
		RoidmiUserID = 0x20180502;
		spi_flash_block_erase(ROIDMI_FLASH_OFFSET_2, SECTOR_ERASE);
		__nop();
		spi_flash_write_data((uint8_t *)&RoidmiUserID, ROIDMI_FLASH_OFFSET_2, 4);
		__nop();
	}
	AD_Flash_Read();
}


void Motor_FeedBack(void){
	if((!PowerOverLoadFlag) && (FanCurrentState == OPEN)){
		if (++Fan1Error >= 6000){
			Fan1Error = 6000;
			ErrorCode |= 0x01;
			SendErrorCode |= 0x0100;
		}
		if(!GPIO_GetPinStatus(GPIO_PORT_1, GPIO_PIN_1)){
			uc_FB1_CNT_L++;
			uc_FB1_Over_L = 1;
			Fan1Error = 0;
			ErrorCode &= 0xfe;
			SendErrorCode &=~0x0100;
		}
		else{
			if(uc_FB1_Over_L == 1){
				uc_FB1_Over_L = 0;
				uc_FB1_Data_L = uc_FB1_CNT_L;
			}
			uc_FB1_CNT_L = 0;
		}
		if(++Fan2Error >= 6000){
			Fan2Error = 6000;
			ErrorCode |= 0x02;
			SendErrorCode |= 0x0100;
			}
		if(!GPIO_GetPinStatus(GPIO_PORT_1, GPIO_PIN_0)){     
			uc_FB2_CNT_L++;
			uc_FB2_Over_L = 1;
			Fan2Error = 0;
			ErrorCode &= ~0x02;
			if((ErrorCode&0x01) == 0)
				SendErrorCode &=~0x0100; //2016.10.26
		}
		else{
			if(uc_FB2_Over_L == 1){
				uc_FB2_Over_L = 0;
				uc_FB2_Data_L = uc_FB2_CNT_L;
			}
			uc_FB2_CNT_L = 0;
		}
	}
}

#define N 32
uint16_t LPOTValueBuffer[N+1];
uint32_t SUM;
uint16_t LPOTValue;

/*******PM sensor Low vlotage time count*******/
unsigned int PMLpotCount(void){
	uint8_t i;
	uint16_t Value;
	SUM = 0;
	LPOTValueBuffer[N] = PMSensorLowTime;
	for(i = 0;i < N;i++){
		LPOTValueBuffer[i] = LPOTValueBuffer[i+1];
		SUM += LPOTValueBuffer[i];
	}
	Value = SUM/N;
	return Value;
}
//*******************************************
void SR_Air_Get(void){
	if(++PMSampleTime <= 2000){	//sample air quality 1s/time
		if(!GPIO_GetPinStatus(GPIO_PORT_2, GPIO_PIN_3)) PMSensorLow++;
//		else PMSensorHigh++;
	}
	else{
		PMSensorLowTime  = PMSensorLow/2;
//		PMSensorHighTime = PMSensorHigh/2;
		PMSensorLow = 0;
//		PMSensorHigh = 0;
		PMSampleTime = 0;
		LPOTValue = PMLpotCount();
		PMSensorLPOT = LPOTValue/(1*10.0);
		if(WarmUpTime > 35){//2016.9.18
			PMSampleValue = 20 * PMSensorLPOT + 7;							
			PMSampleValueNew = PMSampleValue*8;
			print_word(PMSensorLPOT);
			print_word(PMSampleValueNew);
			if(PMSampleValueNew >= 1000){
				PMSampleValueNew = 999;
			}
			if(AirQuality == 0){
				if(PMSampleValueNew < 160)	AirQuality = AirGood;
				else if(PMSampleValueNew <= 320)	AirQuality = AirNormal;
				else	AirQuality = AirBad;
			}
			SampleFinishFlag = 1;
		}
	}
}

void Air_Class(void){
	if(SampleFinishFlag){
		if(WarmUpTime > 35){
			SampleFinishFlag = 0;
			switch(AirQuality){	
				case AirGood:
					if(PMSampleValueNew >= 165){
						if(++AirQualityContinueTime >= 3){
							AirQuality = AirNormal;
							AirQualityContinueTime = 0;
						}
					}
					else{
						AirQualityContinueTime=0;
					}
				break;
				
				case AirNormal:
					if(PMSampleValueNew >= 325){
						if(++AirQualityContinueTime >= 3){
							AirQuality = AirBad;
							AirQualityContinueTime = 0;
						}
					}
					else if(PMSampleValueNew <= 155){
						if(++AirQualityContinueTime >= 10){
							AirQuality = AirGood;
							AirQualityContinueTime = 0;
						}
					}
					else{
						AirQualityContinueTime = 0;
					}
				break;
				
				case AirBad:
					if(PMSampleValueNew <= 315){
						if(++AirQualityContinueTime >= 10){
							AirQuality = AirNormal;
							AirQualityContinueTime = 0;
						}
					}
					else{
						AirQualityContinueTime = 0;														
					}
				break;
				default:
					AirQuality = AirNormal;
					AirQualityContinueTime = 0;	
				break;
			}
		}
	}
}

void LED_Control(void){
	static uint16_t WarnningCount;
	if(!b_Time100ms_Flag){
		ui_Flash_Time++;
		if(SystemStatus == WORK){
			ui_Standby_LED_Time = 0;
			if (ErrorCode == 0){				
				if(WarmUpTime <= 35){ //orange
					LED_R_ON;
					LED_G_ON;
					LED_B_OFF;
				}
				else{
					if(AirQuality == AirBad){ //red
						LED_R_ON;
						LED_G_OFF;
						LED_B_OFF;
					}
					else if(AirQuality == AirNormal){ //orange
						LED_R_ON;
						LED_G_ON;
						LED_B_OFF;
					}
					else if(AirQuality == AirGood){ //green
						LED_R_OFF;
						LED_G_ON;
						LED_B_OFF;
					}
					else{
						LED_R_OFF;
						LED_G_ON;
						LED_B_OFF;
					}
				}
				WarnningCount = 0;
			}
			else if(ErrorCode & 0x04){ //voltage is low
				if(++WarnningCount > 50){
					WarnningCount = 0;
					if(SystemStatus == WORK){
						LED_R_ON;
						LED_G_OFF;
						LED_B_OFF;
						SystemStatus = STANDBY;
						BeepLongFlag = 1;
						BeepOneFlag = 1;
					}
				}
			}
			else if((ErrorCode & 0x01)){ //Fan1 error
				if(!PowerOverLoadFlag){
					if(++WarnningCount > 50){
						WarnningCount = 0;
						if(SystemStatus == WORK){
							LED_R_ON;
							LED_G_OFF;
							LED_B_OFF;
							SystemStatus = STANDBY;
							BeepLongFlag = 1;
							BeepOneFlag = 1;
						}
					}
				}
			}
			else if((ErrorCode & 0x02)){ //Fan2 error
				if(!PowerOverLoadFlag){
					if(++WarnningCount > 50){
						WarnningCount = 0;
						if(SystemStatus == WORK){
							LED_R_ON;
							LED_G_OFF;
							LED_B_OFF;
							SystemStatus = STANDBY;
							BeepLongFlag = 1;
							BeepOneFlag = 1;
						}
					}
				}
			}
			else if (ErrorCode & 0x08){ //need change filter
				if(ui_Flash_Time >= 20){
					LED_R_ON;
					LED_G_ON;
					LED_B_OFF;
					ui_Flash_Time = 0;
					uc_Flash_Flag = !uc_Flash_Flag;
				}
			}
		}
		else{
			if(++ui_Standby_LED_Time <= 1480){
				LED_R_OFF;
				LED_G_OFF;
				LED_B_OFF;
			}
			else if(ui_Standby_LED_Time <= 1500){
				LED_R_ON;
				LED_G_OFF;
				LED_B_OFF;
			}
			else{
				ui_Standby_LED_Time = 0;
			}
		}
    }
}

void Work_Process(void){
	if (SystemStatus == WORK){
		if ((ErrorCode == 0x00) || (ErrorCode == 0x08)){
			if(PowerOverLoadFlag){ //power is over
				Fan_Speed(0);
				FanCurrentState = CLOSE;
			}
			else{
				if(FanCurrentState == CLOSE){
					FanCurrentState = OPEN;
				}
			}
			PM25_POWER_ON;
			if(WarmUpTime < HERT_TIME){
				WarmUp = 0; //warm up haven't finished
			}
			else{
				WarmUpTime = HERT_TIME;
				WarmUp = 1; //warm up is finished
				Air_Class(); //get the air quality
			}
		}
		else if(ErrorCode & 0x04){ //power low
			Fan_Speed(0);
			PM25_POWER_OFF;
			FanCurrentState = CLOSE; //close the fan moto
		}
	}
	else{
		Fan_Speed(0);
		PM25_POWER_OFF;
		WarmUpTime = 0;
		WarmUp = 0;
		WorkMode = roidmi_flash_save_data.WorkModeEE;
		FilterType = roidmi_flash_save_data.FilterTypeEE;
		AirQuality = 0;
		FanCurrentSpeed = 0;
		FanCurrentState = CLOSE;
		ErrorCode = 0;	
		Fan1Error = 0;
		Fan2Error = 0;
		AirQualityContinueTime = 0;
		PMSensorLow = 0;
		PMSensorHigh = 0;
		PMSampleTime = 0;
		SampleFinishFlag = 0;
		VoltageLowTime = 0;
		b_Force_Flag = 0;
//		b_Beep_Long_Auto_Flag = 0;
//		b_Beep_Two_Auto_Flag = 0;
		}
}

void Work_Time_Judge(void){
	if(SecondFlag == 1){
		SecondFlag = 0;
		if(FanCurrentState == OPEN){
			switch(FanCurrentSpeed){
				case HighSpeed://high
					roidmi_flash_save_data.PurifyTotalTimeEE+=3; //净化总时间
					roidmi_flash_save_data.FilterWorkTimeEE+=3; //当前滤芯工作时间
					roidmi_flash_save_data.DeviceWorkTimeEE+=2; //设备工作时间
				    DeviceWorkTime = roidmi_flash_save_data.DeviceWorkTimeEE;
				break;
				case MidSpeed://mid
					roidmi_flash_save_data.PurifyTotalTimeEE+=2;
					roidmi_flash_save_data.FilterWorkTimeEE+=2;		
				    roidmi_flash_save_data.DeviceWorkTimeEE+=1.5;
					DeviceWorkTime = roidmi_flash_save_data.DeviceWorkTimeEE;
				break;
				case LowSpeed://low
					roidmi_flash_save_data.PurifyTotalTimeEE+=1;
					roidmi_flash_save_data.FilterWorkTimeEE+=1;
					roidmi_flash_save_data.DeviceWorkTimeEE+=1;
					DeviceWorkTime = roidmi_flash_save_data.DeviceWorkTimeEE;
					break;
				default:
					break;
			}
		}
	}
	if(roidmi_flash_save_data.FilterWorkTimeEE >= FILTER_TIME){ //滤芯寿命到期
		roidmi_flash_save_data.FilterWorkTimeEE = FILTER_TIME; //900 hours
		ChangeFilterFlag = 1;
		ErrorCode |= 0x08;
	}
	else{
		ChangeFilterFlag = 0;		
		ErrorCode &= ~0x08;
	}
}

uint16_t HighSpeedValue = 8000; //80%
uint16_t MidSpeedValue = 6000; //60%
uint16_t LowSpeedValue = 3000; //30%
/******fan control function******/
void Fan_Control(void){	 
	if (FanCurrentState == OPEN){
		switch (WorkMode){
			case Auto_State:
				if(WarmUp == 0){ //warmup havn't finished
					Fan_Speed(LowSpeedValue);
					FanCurrentSpeed = LowSpeed;
				}
				else{
					if(AirQuality == AirBad){
						Fan_Speed(HighSpeedValue);
						FanCurrentSpeed = HighSpeed;
					}
					
					else if(AirQuality == AirNormal){
						Fan_Speed(MidSpeedValue);
						FanCurrentSpeed = MidSpeed;
					}
					
					else if(AirQuality == AirGood){
						Fan_Speed(LowSpeedValue);
						FanCurrentSpeed = LowSpeed;
					}
				}
				break;
				
			case Low_State:
				Fan_Speed(LowSpeedValue);
				FanCurrentSpeed = LowSpeed;
				break;
				
			case Mid_State:
				Fan_Speed(MidSpeedValue);
				FanCurrentSpeed = MidSpeed;
				break;
				
			case High_State:
				Fan_Speed(HighSpeedValue);                     
				FanCurrentSpeed = HighSpeed;
				break;
			default:
			break;
		}
	}
	else{
		Fan_Speed(0);
	}
}
 
/********************************************************
***		//ffd1:
***		//ffd2:
***		//ffd3:
***		//ffd4:
***		//ffd5:
***		//ffd6:
***		//ffd7:
***		//ffd8:
***		//ffd9:
***		//ffda:MAC地址
***		//
**********************************************************/
void Send_To_APP(void)
{
	static uint16_t DASendTime = 100;
	uint8_t PMSensorData[3];
	uint8_t SwitchData[4];
	uint8_t VoltageData[3];
	if(b_Working_Flag==0){
		b_Working_Flag = 1;
		PMSensorData[0] = AirQuality;
		PMSensorData[1] = 0;
		PMSensorData[2] = 0;
		attmdb_att_set_value(STREAMDATAD_DIR_VAL_HANDLE(8), 3, (uint8_t *)PMSensorData);
		prf_server_send_event((prf_env_struct *) & (streamdatad_env.con_info), false, STREAMDATAD_DIR_VAL_HANDLE(8)); //Air quality send
		
		SwitchData[0] = WorkMode;
		SwitchData[1] = uc_PWM_Bar_Data; //stepless speed regulating
		SwitchData[2] = (roidmi_flash_save_data.BeepModeEE + 14); //beep switch
		if(FilterType == 0x00){ //filter type common filter
			SwitchData[3] = (CommonLedSet + 24);
		}
		else if(FilterType == 0x01){
			SwitchData[3] = (roidmi_flash_save_data.LedModeEE + 24);
		}								
		attmdb_att_set_value(STREAMDATAD_DIR_VAL_HANDLE(3), 4, (uint8_t *)SwitchData);
		prf_server_send_event((prf_env_struct *) & (streamdatad_env.con_info), false, STREAMDATAD_DIR_VAL_HANDLE(3));	
	}
	if (++DASendTime >= 100){ 
		DASendTime = 0;
		if(SystemStatusOld != SystemStatus){
			SystemStatusOld = SystemStatus;
			attmdb_att_set_value(STREAMDATAD_DIR_VAL_HANDLE(1), sizeof(SystemStatus), (uint8_t *)&SystemStatus);
			prf_server_send_event((prf_env_struct *) & (streamdatad_env.con_info), false, STREAMDATAD_DIR_VAL_HANDLE(1));
		}
		if((WorkModeOld != WorkMode)||(FilterTypeOld != FilterType)){ //工作模式或者滤芯类型发生变化
			WorkModeOld = WorkMode;
			FilterTypeOld = FilterType;
			SwitchData[0] = WorkMode; //工作状态
			SwitchData[1] = uc_PWM_Bar_Data;
			SwitchData[2] = (roidmi_flash_save_data.BeepModeEE + 14);
			if(FilterType == 0x00){
				SwitchData[3] = (CommonLedSet + 24); //LED
			}
			else if(FilterType == 0x01){
				SwitchData[3] = (roidmi_flash_save_data.LedModeEE + 24);
			}								
			attmdb_att_set_value(STREAMDATAD_DIR_VAL_HANDLE(3), 4, (uint8_t *)SwitchData);
			prf_server_send_event((prf_env_struct *) & (streamdatad_env.con_info), false, STREAMDATAD_DIR_VAL_HANDLE(3));
		}	
		attmdb_att_set_value(STREAMDATAD_DIR_VAL_HANDLE(4), 4, (uint8_t *)&DeviceWorkTime); //设备工作时间
		prf_server_send_event((prf_env_struct *) & (streamdatad_env.con_info), false, STREAMDATAD_DIR_VAL_HANDLE(4));
		attmdb_att_set_value(STREAMDATAD_DIR_VAL_HANDLE(5), 4, (uint8_t *)&roidmi_flash_save_data.PurifyTotalTimeEE); //设备净化时间
		prf_server_send_event((prf_env_struct *) & (streamdatad_env.con_info), false, STREAMDATAD_DIR_VAL_HANDLE(5));
		attmdb_att_set_value(STREAMDATAD_DIR_VAL_HANDLE(6), 4, (uint8_t *)&roidmi_flash_save_data.FilterWorkTimeEE); //当前滤芯工作时间
		prf_server_send_event((prf_env_struct *) & (streamdatad_env.con_info), false, STREAMDATAD_DIR_VAL_HANDLE(6));
		if (ChangeFilterFlagOld != ChangeFilterFlag){ //滤芯到期
			ChangeFilterFlagOld = ChangeFilterFlag;
			attmdb_att_set_value(STREAMDATAD_DIR_VAL_HANDLE(7), sizeof(ChangeFilterFlag), (uint8_t *)&ChangeFilterFlag); //更换滤芯提醒
			prf_server_send_event((prf_env_struct *) & (streamdatad_env.con_info), false, STREAMDATAD_DIR_VAL_HANDLE(7));
		}
		if((PMSampleValueOld != PMSampleValueNew) || (AirQualityLevelOld != AirQuality)){ //PM2.5发生变化
			PMSampleValueOld = PMSampleValueNew;
			AirQualityLevelOld = AirQuality;
			PMSensorData[0] = AirQuality;
			memcpy((uint8_t *)(PMSensorData + 1), (uint8_t *)&PMSampleValueNew, 2);
			attmdb_att_set_value(STREAMDATAD_DIR_VAL_HANDLE(8), 3, (uint8_t *)PMSensorData); //空气质量等级
			prf_server_send_event((prf_env_struct *) & (streamdatad_env.con_info), false, STREAMDATAD_DIR_VAL_HANDLE(8));
		}
		if(SendErrorCodeOld != SendErrorCode){
			SendErrorCodeOld = SendErrorCode;
			attmdb_att_set_value(STREAMDATAD_DIR_VAL_HANDLE(10),  sizeof(SendErrorCode), (uint8_t *)&SendErrorCode); //错误代码
			prf_server_send_event((prf_env_struct *)&(streamdatad_env.con_info), false, STREAMDATAD_DIR_VAL_HANDLE(10));
		}
		VoltageData[0] = uc_Car_Style;
		memcpy((uint8_t *)(VoltageData + 1), (uint8_t *)&ui_Power_Value_New, 2); //电瓶电压值
		attmdb_att_set_value(STREAMDATAD_DIR_VAL_HANDLE(11), 3, (uint8_t *)VoltageData);
	}
}

void Factory_Test(void)
{
	static uint16_t cnt;
	static uint8_t temp_time,time_100ms,temp_time1;
	if(b_Factory_Flag){
		//LED_R_ON;
		PM25_POWER_ON;
		if(true == GPIO_GetPinStatus(KEY_PORT, KEY_PIN)){
			b_Factory_Key_Down = 1;
			if(++cnt>=300){
				cnt = 300;
				if(b_Factory_Key_Long_Flag==0){
					b_Factory_Key_Long_Flag=1;
				}
			}
		}
		else{
			if(b_Factory_Key_Down){
				b_Factory_Key_Down = 0;
				if((cnt>=10) && (cnt < 300)){
					if(b_Factory_Key_Short_Flag == 0){
						if(AD_SAVE.uc_AD_Flash_Flag != 0){
							b_Factory_Key_Short_Flag = 1;
							PM25_POWER_ON;
							uc_FB1_Data_L = 0;
							uc_FB2_Data_L = 0;
							ui_FA_Data_L = 0;
							timer2_set_pwm3_duty_cycle(0); //fan speed control
							timer2_set_pwm4_duty_cycle(0);
							temp_time = 0;											
							BeepOneFlag = 1;								
							uc_Factory_Step = 1;
						}												
					}
				}
			}
			cnt = 0;
			b_Factory_Key_Long_Flag = 0;
			b_Factory_Key_Short_Flag = 0;
			b_Factory_Key_Down = 0;
		}
		if(++time_100ms>=50){
			time_100ms=0;				
			switch(uc_Factory_Step){
				case 0:
					if(AD_SAVE.uc_AD_Flash_Flag==0){
						BeepOneFlag = 1;
//						LED_O_ON;
					}
				break;
				case 1:
					if(++temp_time>=2){
						temp_time=0;
//                      LED_G_ON;
						uc_Factory_Step++;
					}
				break;
				case 2:
					if(++temp_time>=2){
						temp_time=0;
						//GCM_POWER_ON;
						uc_Factory_Step++;
					}
				break;
				case 3:
					if(++temp_time>=2){
						temp_time=0;
						//GCM_POWER_ON;
						uc_Factory_Step++;
					}
				break;
				case 4:
					if(++temp_time>=2){
						temp_time=0;
						//GCM_POWER_ON;
						uc_Factory_Step++;
					}
				break;
				case 5:
//					FAN_POWER_ON;
					timer2_set_pwm3_duty_cycle(3333);
					if((uc_FB1_Data_L>=25)&&(uc_FB1_Data_L<=40)){
						uc_Factory_Step++;
						temp_time=0;
						temp_time1=0;
					}
					else{
						if(++temp_time1>=5){
							temp_time1=5;
//							Beep_One();
							BeepOneFlag  = 1;
						}
						temp_time=0;
					}
				break;	
				case 6:
					//WS_Set(0xffffff);
//					FAN_POWER_ON;
					timer2_set_pwm3_duty_cycle(0);
					timer2_set_pwm4_duty_cycle(3333);
					if((uc_FB2_Data_L>=25)&&(uc_FB2_Data_L<=40)){
						uc_Factory_Step++;
						temp_time=0;
						temp_time1=0;
					}
					else{
						if(++temp_time1>=5){
							temp_time1=5;
	//						Beep_One();
							BeepOneFlag = 1;
						}
						temp_time=0;
					}
				break;
				case 7:
//					FAN_POWER_OFF;
					timer2_set_pwm3_duty_cycle(0);
					timer2_set_pwm4_duty_cycle(0);
					if(ui_FA_Data_L>=100){
						uc_Factory_Step++;
						temp_time=0;
						temp_time1=0;
					}
					else{
						if(++temp_time1>=10){
							temp_time1=10;												
//							Beep_One();
							BeepOneFlag  = 1;
						}
						temp_time=0;
					}
				break;
				case 8:
//					FAN_POWER_OFF;
					timer2_set_pwm3_duty_cycle(0);
					timer2_set_pwm4_duty_cycle(0);
					if((ui_Power_Value_New>=244) && (ui_Power_Value_New<=260)){//12V
						if(++temp_time>=2){
							temp_time=0;
							uc_Factory_Step++;
							BeepThreeFlag = 1;
//							Beep_Three();
						}
						temp_time1=0;
					}
					else{
						if(++temp_time1>=5){
							temp_time1=5;
//							Beep_One();
							BeepOneFlag  = 1;
						}
						temp_time=0;
					}
				break;
				case 9:
					uc_Factory_Step++;
					break;
				case 10:
					uc_Factory_Step++;
				break;
				case 11:
					uc_Factory_Step=9;
				break;
				default:
					break;

			}
			}
		}	
		if(ui_Factory_Time<800){
			if (true == GPIO_GetPinStatus( KEY_PORT, KEY_PIN )){
				uc_Factory_Mode=1;
			}
			else{
				if(uc_Factory_Mode == 1){
					ui_Factory_Time = 800;
					uc_Factory_Mode = 2;
				}
			}
		}
		else if((ui_Factory_Time<2200) &&(uc_Factory_Mode==2)){
			if(true == GPIO_GetPinStatus( KEY_PORT, KEY_PIN )){
				if(++cnt>10){
					BeepTwoFlag = 1;
					b_Factory_Flag = 1;
					uc_Factory_Step = 0;
//					LED_B_ON;
					cnt=0;
				}
			}
		}
		else{
			ui_Factory_Time=2200;
		}
}

uint16_t KeyDownTime10ms;
bool KeyDownFlag=0;
void Key_Scan(void){
	if(GPIO_GetPinStatus( KEY_PORT, KEY_PIN )==1){ //key press down
		KeyDownFlag	=1;			
		KeyDownTime10ms++;	
		if(KeyDownTime10ms>=300){ //Key press down 3 seconds			
			KeyDownTime10ms = 300;
			if(KeyLongFlag == 0){
				KeyLongFlag = 1;					
				KeyValue = 1;			
			}					
		}	
	}
	else{ 			
		if(KeyDownFlag){
			KeyDownFlag	=0;  
			if((KeyDownTime10ms>=10)&&(KeyDownTime10ms<200)){				
				if(KeyShortFlag == 0){
					KeyShortFlag = 1;							
					KeyValue = 2;
				}
			}
		}
		KeyDownTime10ms = 0;
		KeyLongFlag  = 0;
		KeyShortFlag = 0;
		KeyDownFlag	 = 0;
	}
}

void Key_Control(void){
	switch(KeyValue){
		case 1:
			b_Hand_Flag = 1;
			if(SystemStatus == WORK){
				SystemStatus = STANDBY;				
				WarmUp = 0; //warm up flag clear
				BeepLongFlag = 1;
				BeepOneFlag = 1;
				EEPROM_Write();
			}
			else if(SystemStatus == STANDBY){
				SystemStatus = WORK;
				EEPROM_Read();
				BeepTwoFlag = 1;
				PMSampleValue = 0;
				b_Working_Flag = 0;
			}
			attmdb_att_set_value(STREAMDATAD_DIR_VAL_HANDLE(1), sizeof(SystemStatus), (uint8_t *)&SystemStatus);
			prf_server_send_event((prf_env_struct *) & (streamdatad_env.con_info), false, STREAMDATAD_DIR_VAL_HANDLE(1));
		break;
    case 2:
		WorkMode++;
		if(WorkMode >= 4){
			WorkMode = 0;
		}
		BeepOneFlag = 1;
		roidmi_flash_save_data.WorkModeEE = WorkMode;
		roidmi_flash_save_data.FilterTypeEE = FilterType; //滤芯类型
		EEPROM_Write();
        attmdb_att_set_value(STREAMDATAD_DIR_VAL_HANDLE(3), sizeof(WorkMode), (uint8_t *)&WorkMode);
        prf_server_send_event((prf_env_struct *) & (streamdatad_env.con_info), false, STREAMDATAD_DIR_VAL_HANDLE(3));
		break;    
		default:
		break;
    }
    KeyValue = 0;
}

/*****12V电压校准*******/
void BD_Pro(void){		
	static uint16_t ui_BD_Time;
	if (false == GPIO_GetPinStatus( GPIO_PORT_2, GPIO_PIN_9 )){
		if(++ui_BD_Time>=200){	
			ui_BD_Time=200;
			if(b_BD_Flag==0){
				b_BD_Flag = 1;
				if((VoltageValue>=200) && (VoltageValue<=300)){
					AD_SAVE.f_AD_Ratio = (1.0*VoltageValue)/252.0;
					AD_SAVE.uc_AD_Flash_Flag = 0x5a;									
					spi_flash_block_erase(ROIDMI_FLASH_OFFSET_3, SECTOR_ERASE);      
					spi_flash_write_data((uint8_t *)&AD_SAVE.uc_AD_Flash_Flag, ROIDMI_FLASH_OFFSET_3, sizeof(struct AD_FLASH_SAVE));
				}
				else{
				}
			}	
		}
	}
	else{
		ui_BD_Time =0;
	}
}

int ROIDMI_AdcHandle(ke_msg_id_t const msgid,
                     void const *param,
                     ke_task_id_t const dest_id,
                     ke_task_id_t const src_id)
{
	uint8_t i;
	uint16_t temp = 0;
	for(i = 0; i < 10; i++){
		ui_adc_data2[i] = adc_get_sample();
		temp += ui_adc_data2[i];
	}
	VoltageValue = temp/10;
	ui_Power_Value_New = VoltageValue/AD_SAVE.f_AD_Ratio;
//	uart2_send_byte(ui_Power_Value_New);
//	print_string("\r\n");
	if(!b_Factory_Flag){
		if((!b_Force_Flag) &&(!b_BD_Flag)){
			if(++VolatgeOnTime >= 200){
				VolatgeOnTime = 200;
				if((b_Open_Flag == 0)){
					b_Open_Flag = 1;    
					SystemStatus = WORK;				
					BeepTwoFlag = 1;         
					EEPROM_Read();
				}
				else if(b_Stay_Flag == 1){
					SystemStatus = WORK;
					BeepTwoFlag = 1;
					EEPROM_Read();
					b_Stay_Flag = 0;
				}
			}
			ErrorCode &= ~0x04;								
			b_Close_Flag = 0;
			SendErrorCode &=~0x0001; //feedback normal 0
		}
		PowerOverLoadFlag = 0;
		b_Power_Low_Flag = 0;
		VoltageLowTime = 0;      
		VoltageOverTime = 0;
/*		
		if(ui_Power_Value_New <= POWER_LOW){ //voltage < 11.5
			if(++VoltageLowTime >= 300*20){ //300*20=60s
				VoltageLowTime = 0;     
				b_Power_Low_Flag = 1;
				b_Stay_Flag = 1;
//				b_Beep_Long_Auto_Flag = 0;
				b_Beep_Two_Auto_Flag = 0; 
				ErrorCode|=0x04;
				SendErrorCode |=0x0001;
			}
			VolatgeOnTime  = 0;
			VoltageOverTime= 0;
			b_Auto_Open_Flag  = 1;
//		    b_Open_Flag = 0; //2016.9.18
			b_Beep_Long_Power_On=0;
			b_Beep_Two_Power_On =0;
		}
		else if((ui_Power_Value_New > POWER_LOW) && (ui_Power_Value_New <= POWER_HIGH)){ //11.5 < voltage <= 16V
			if((!b_Force_Flag) &&(!b_BD_Flag)&&(ui_Power_Value_New >= POWER_ON)){ //voltage >= 12V, 2016/11/15
				if(++VolatgeOnTime >= 200){
					VolatgeOnTime = 200;
					if((b_Open_Flag == 0)){ //First Run
						b_Open_Flag = 1;    
						SystemStatus = WORK; 
						b_Beep_Two_Auto_Flag = 1;
						BeepTwoFlag  = 1;						
//						Beep_Two();         
						EEPROM_Read();
					}
					else if(b_Stay_Flag == 1){ //into low power and warnning
						SystemStatus = WORK;
						b_Beep_Two_Auto_Flag = 1; //2016.11.19
						BeepTwoFlag  = 1;
//						Beep_Two();
						EEPROM_Read();
						b_Stay_Flag = 0;
					}
				}
				ErrorCode &= ~0x04;								
				b_Close_Flag = 0;
				SendErrorCode &=~0x0001; //feedback normal 0
			}
			PowerOverLoadFlag = 0;
			b_Power_Low_Flag = 0; //2016.9.21
			VoltageLowTime = 0;      
			VoltageOverTime = 0;
		}		
		else{
			if(++VoltageOverTime > 50){ 
				VoltageOverTime = 50;
				PowerOverLoadFlag = 1;     //Over Power
			}
			VoltageLowTime = 0;
			VolatgeOnTime  = 0;
			b_Power_Low_Flag  = 0;
		}*/		
	}
	app_timer_set(ROIDMI_ADC, TASK_APP, 1);
	return 0;
}

int ROIDMI_FunctionHandle(ke_msg_id_t const msgid,
						  void const *param,
                          ke_task_id_t const dest_id,
                          ke_task_id_t const src_id)
{
	Key_Scan();
	Beep_Control();
	if(!b_Factory_Flag){
		Fan_Control();
		Key_Control();
		LED_Control();
		Work_Process();
		Air_Class();
		Work_Time_Judge();
		if(!OtaisRunning && APPisConnected)
			Send_To_APP();
		EEPROM_Save();
	}
	app_timer_set(ROIDMI_FUNCTION, TASK_APP, 1);
	return 0; 
}
#endif
