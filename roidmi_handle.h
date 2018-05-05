#ifndef __ROIDMI_HANDLE_H
#define __ROIDMI_HANDLE_H

#if (1 == ROIDMI)


#include "roidmi_cfg.h"

struct ROIDMI_FLASH_SAVE
{
	int32_t   FlashErasetimesEE;
	uint32_t  PurifyTotalTimeEE;
	uint32_t  FilterWorkTimeEE; //Ëо٤طʱݤ
	float  	  DeviceWorkTimeEE; //2016.8.22
	uint32_t  ul_Work_First_Time;
	uint8_t   WorkModeEE; 
	uint8_t   FilterTypeEE; //Ëо`э
	uint8_t   uc_Car_EE;
	uint8_t   uc_PWM_Data_EE; //PWMϞַ̙ܶ
	bool      BeepModeEE;
	bool      LedModeEE;
};


struct AD_FLASH_SAVE
{
		int8_t  uc_AD_Flash_Flag;
		float   f_AD_Ratio;
};

#define ROIDMI_FLASH_OFFSET_0		0x3000
#define ROIDMI_FLASH_OFFSET_1		0x4000
#define ROIDMI_FLASH_OFFSET_2		0x5000
#define ROIDMI_FLASH_OFFSET_3		0x6000

extern struct ROIDMI_FLASH_SAVE roidmi_flash_save_data;

extern bool   OtaisRunning;

extern struct AD_FLASH_SAVE AD_SAVE;		//2016.9.23


//int ROIDMI_LedHandle(ke_msg_id_t const msgid,
//                     void const *param,
//                     ke_task_id_t const dest_id,
//                     ke_task_id_t const src_id);

int ROIDMI_AdcHandle(ke_msg_id_t const msgid,
                     void const *param,
                     ke_task_id_t const dest_id,
                     ke_task_id_t const src_id);

//void ROIDMI_KeyCallback(void);

 //int ROIDMI_KeyScanHandle(ke_msg_id_t const msgid,
 //                        void const *param,
 //                        ke_task_id_t const dest_id,
 //                        ke_task_id_t const src_id);

int ROIDMI_FunctionHandle(ke_msg_id_t const msgid,
                          void const *param,
                          ke_task_id_t const dest_id,
                          ke_task_id_t const src_id);



void PWM_Config(void);
void timer_init(void);
void timer_callback(void);
void System_Init(void);

//void Beep_One(void);
void Beep_Two(void);
void Beep_Three(void);
void Beep_Long(void);
void EEPROM_Read(void);
void EEPROM_Write(void);
void EEPROM_Erase(void);
													
#endif
													
#endif // __ROIDMI_HANDLE_H

