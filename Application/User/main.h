/**
  ******************************************************************************
  * @file    main.h 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    26-February-2014
  * @brief   Header for main.c file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
 extern "C" {
#endif
   
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx_it.h"

/* EVAL includes component */
#include "wt_bsp.h"

/* FatFs includes component */
#include "ff_gen_drv.h"
#include "usbh_diskio.h"
#include "sd_diskio.h"
#include "gui.h"

/* function include */
#include "wt_ad_app.h"
#include "wt_uart_24GHz.h"
#include "wt_uart_audio.h"
#include "wt_uart_COM1.h"
#include "wt_uart_COM2.h"
#include "wt_task_gui.h"

/* module include */
#include "k_bsp.h"
#include "k_module.h"
#include "k_rtc.h"
#include "k_storage.h"
#include "k_log.h"
#include "k_mem.h"

//others
#include "cmsis_os.h"
#include "malloc.h"



/* Exported types ------------------------------------------------------------*/
typedef union
{
  uint32_t d32;
  struct
  {
    uint32_t enable_sprite     : 1;
    uint32_t enable_background : 1;
    uint32_t use_180Mhz        : 1;
    uint32_t disable_flex_skin : 1;  
  }b;
}
SystemSettingsTypeDef;


//�����ļ�
typedef struct{
	
	uint8_t 	BackLight;		//�������ȣ�0-10
	uint8_t 	AudioVolume;	//����������0-10
	uint8_t  	Print;			  //���ش�ӡ 0:no, 1:yes
	uint8_t 	InfoRTC[7];		//ʱ�䡢���ڡ�����
	uint8_t 	Mode_TestW;		//��ͨ��ֵ0:365R,1:1K 2:10K
	uint8_t 	Mode_TestZ;		//�������ģʽ, 0:12R, 1:27R, 2:100R, 3:125R; 4:365R, 5:1K, 6:10k
	uint8_t   Mode_TestK;		//���ز���ģʽ, 0:12R, 1:27R, 2:100R, 3:125R; 4:365R, 5:1K, 6:10k,
	uint8_t 	NetWork[15];	//����SSID
	uint8_t 	SecKey[15];	  //��Կ
	uint8_t 	Server_IP[4];	//��������ַ
	uint8_t 	TestMode;   	//0-�Զ�ģʽ  1-�ֶ�ģʽ  2-��ʱģʽ
	uint8_t 	AudioOK;		//�����ļ���0-3
	uint8_t 	AudioNG;		//�����ļ���0-3
	uint8_t     Lockstat;	    //0:unlock  1:lock
	uint8_t     TestRule;       //���Թ���--52
	uint8_t     Passwd[6];      //����
	uint8_t     IP_Addr[4];     //IP��ַ
	uint8_t     MAC_Addr[6];    //MAC��ַ
	uint8_t 	Mode_TestCap;	//������ֵ, 0:0.1uF, 1:0.47uF, 2:1.0uF, 3:4.7uF; 
	uint8_t 	PrintServ;		//�Ƿ�Ϊ��ӡ������ 0:��, 1:��--70
	uint8_t 	Print_Mode;		//��ӡ��ʽ��0�����ڴ�ӡ 1�����ڴ�ӡ
	uint8_t 	Test_Wait;		//���Եȴ�ʱ��
	uint8_t 	IMS_Mode;		//IMS���ӿ��� 0����IMS���� 1��IMS����--73
	uint16_t 	TCP_Port;	    //�˿ں�
	uint16_t 	Dev_ID;	        //�豸���
	uint16_t 	Print_SN;	    //��ӡ��ˮ��
    uint8_t 	Loop_Num;		//��ˮ�߱�ţ���λ�ţ�--80
}
WT_ConfigTypedef;  


//��ͨ��״̬
typedef struct{
	
	uint8_t 	mode;					//0������״̬��1���������в��Գ���2������������ѧϰ����
													//3�����������ҵ����4�����������Լ����5���������е��Գ���
	uint8_t 	error;				//0��no error, 1:error
	
}
WT_TesterStatusTypedef;  

/* Exported variables --------------------------------------------------------*/
extern WT_TesterStatusTypedef TesterStatus;


/* Exported constants --------------------------------------------------------*/
#define NUM_DISK_UNITS       2
#define USB_DISK_UNIT        0
#define MSD_DISK_UNIT        1

#define CALIBRATION_BKP0                    RTC_BKP_DR0
#define CALIBRATION_BKP1                    RTC_BKP_DR1
#define CALIBRATION_GENERAL_SETTINGS_BKP    RTC_BKP_DR2
#define CALIBRATION_IMAGE_SETTINGS_BKP      RTC_BKP_DR3
#define CALIBRATION_BENCH_SETTING_BKP       RTC_BKP_DR4
#define CALIBRATION_VIDEOPLAYER_SETTING_BKP RTC_BKP_DR5




#define DEF_WT_NAME			"���������"
#define DEF_WT_MODEL		"WT-8B26"

/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 

extern SystemSettingsTypeDef 	settings;
extern osMessageQId 					WireTestEvent;
extern osMessageQId 					WireStudyEvent;
extern osMessageQId 					WireSelfCheckEvent;
extern osMessageQId 					WireFindPointEvent;
extern osMessageQId 					UartCOM1Event;
extern osMessageQId 					UartCOM2Event;
extern osMessageQId 					UartAudioEvent;
extern osMessageQId 					Uart24GHzEvent;
extern osMessageQId 					UartInfrEvent;
extern WT_ConfigTypedef				WT_Config;

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
