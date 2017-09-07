/**
  ******************************************************************************
  * @progect LZY Wire Cube Tester
	* @file    wt_bsp_spi_flash.c
  * @author  LZY Zhang Manxin
  * @version V1.0.0
  * @date    2016-08-09
  * @brief   support FM25VF10 & W25Q128FV.
  ******************************************************************************
  */

#define WT_BSP_SPI_FLASH_GLOBALS

/* Includes ------------------------------------------------------------------*/
#include "wt_bsp_spi_flash.h"
#include <string.h>


static SPI_HandleTypeDef SpiHandle;
uint32_t SpixTimeout = sFLASH_SPI_TIMEOUT_MAX; /*<! Value of Timeout when SPI communication fails */  
SFLASH_T sflash;



static void sFLASH_Wait_Busy(void);


static void SPI6_MspInit(SPI_HandleTypeDef *hspi);
static void SPI6_Init(void);
static void SPI6_Error(void);
static uint8_t SPI6_WriteRead(uint8_t Byte);
static uint8_t sFLASH_ReadSR(void);
//static void sFLASH_Write_SR(uint8_t sr);
static uint16_t sFLASH_ReadID(void);
static void sFLASH_ReadInfo(void);
static void sFLASH_Write_Page(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite);
static void sFLASH_Write_NoCheck(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite);


/**
  * @brief  SPI6 MSP Init.
  * @param  hspi: SPI handle
  * @retval None
  */
static void SPI6_MspInit(SPI_HandleTypeDef *hspi)
{
  GPIO_InitTypeDef   GPIO_InitStructure;

  /* Enable SPIx clock  */
  sFLASH_SPI_CLK_ENABLE();
	
  /* Enable DISCOVERY_SPI GPIO clock */
  sFLASH_SPI_GPIO_CLK_ENABLE();
	sFLASH_SPI_NSS_GPIO_CLK_ENABLE();
	
  /* configure SPI NSS Pin */
  sFLASH_SPI_NSS_H();
  GPIO_InitStructure.Pin    = sFLASH_SPI_NSS_PIN;
  GPIO_InitStructure.Mode   = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Pull   = GPIO_PULLUP;
  GPIO_InitStructure.Speed  = GPIO_SPEED_MEDIUM;
  HAL_GPIO_Init(sFLASH_SPI_NSS_PORT, &GPIO_InitStructure);   
  
  /* configure SPI SCK, MOSI and MISO */    
  GPIO_InitStructure.Pin    = (sFLASH_SPI_MISO_PIN | sFLASH_SPI_SCK_PIN | sFLASH_SPI_MOSI_PIN);
  GPIO_InitStructure.Mode   = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull   = GPIO_PULLDOWN;
  GPIO_InitStructure.Speed  = GPIO_SPEED_MEDIUM;
  GPIO_InitStructure.Alternate = sFLASH_SPI_AF;
  HAL_GPIO_Init(sFLASH_SPI_PORT, &GPIO_InitStructure);      
}

/**
  * @brief  SPI6 Bus initialization
  * @param  None
  * @retval None
  */
static void SPI6_Init(void)
{
  if(HAL_SPI_GetState(&SpiHandle) == HAL_SPI_STATE_RESET)
  {
    /* SPI Config */
    SpiHandle.Instance = sFLASH_SPI;
    /* SPI6 baudrate is set to 42 MHz (PCLK2/SPI_BaudRatePrescaler = 84/2 = 42 MHz) 
       to verify these constraints:
       - sFLASH SPI interface max baudrate is 104MHz
       - PCLK2 frequency is set to 84 MHz 
       - SPI6 can communicate at up to 45 Mbits/s
    */  
    SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    SpiHandle.Init.Direction      = SPI_DIRECTION_2LINES;
    SpiHandle.Init.CLKPhase       = SPI_PHASE_1EDGE;
    SpiHandle.Init.CLKPolarity    = SPI_POLARITY_LOW;
    SpiHandle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
    SpiHandle.Init.CRCPolynomial  = 7;
    SpiHandle.Init.DataSize       = SPI_DATASIZE_8BIT;
    SpiHandle.Init.FirstBit       = SPI_FIRSTBIT_MSB;
    SpiHandle.Init.NSS            = SPI_NSS_SOFT;
    SpiHandle.Init.TIMode         = SPI_TIMODE_DISABLED;
    SpiHandle.Init.Mode           = SPI_MODE_MASTER;
  
    SPI6_MspInit(&SpiHandle);
    HAL_SPI_Init(&SpiHandle);
		
		
		/* Chip Select */
		sFLASH_SPI_NSS_H();
		CLK_Delay(10);
  }
}

/**
  * @brief  SPIx error treatment function.
  * @param  None
  * @retval None
  */
static void SPI6_Error(void)
{
  /* De-initialize the SPI communication BUS */
  HAL_SPI_DeInit(&SpiHandle);
  
  /* Re- Initialize the SPI communication BUS */
  SPI6_Init();
}

/**
  * @brief  Sends a Byte through the SPI interface and return the Byte received 
  *         from the SPI bus.
  * @param  Byte: Byte send.
  * @retval The received byte value
  */
static uint8_t SPI6_WriteRead(uint8_t Byte)
{
  uint8_t receivedbyte = 0;
  
  /* Send a Byte through the SPI peripheral */
  /* Read byte from the SPI bus */
  if(HAL_SPI_TransmitReceive(&SpiHandle, (uint8_t*) &Byte, (uint8_t*) &receivedbyte, 1, SpixTimeout) != HAL_OK)
  {
    SPI6_Error();
  }
  
  return receivedbyte;
}

//---------------------------------------------------------------------------
//��ȡsFLASH��״̬�Ĵ���
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:Ĭ��0,״̬�Ĵ�������λ,���WPʹ��
//TB,BP2,BP1,BP0:FLASH����д��������
//WEL:дʹ������
//BUSY:æ���λ(1,æ;0,����)
//Ĭ��:0x00
static uint8_t sFLASH_ReadSR(void)
{  
  uint8_t byte=0;   
  sFLASH_SPI_NSS_L();                  //ʹ������   
  SPI6_WriteRead(sFLASH_CMD_ReadStatusReg); //���Ͷ�ȡ״̬�Ĵ�������    
  byte=SPI6_WriteRead(0X00);          //��ȡһ���ֽ�  
  sFLASH_SPI_NSS_H();                  //ȡ��Ƭѡ     
  return byte;   
} 
//дsFLASH״̬�Ĵ���
//ֻ��SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)����д!!!
//static void sFLASH_Write_SR(uint8_t sr)
//{   
//  sFLASH_SPI_NSS_L();                    //ʹ������   
//  SPI6_WriteRead(sFLASH_CMD_WriteStatusReg);  //����дȡ״̬�Ĵ�������    
//  SPI6_WriteRead(sr);              	    //д��һ���ֽ�  
//  sFLASH_SPI_NSS_H();                    //ȡ��Ƭѡ     	      
//}   
//sFLASHдʹ��	
//��WEL��λ   
void sFLASH_Write_Enable(void)   
{
  sFLASH_SPI_NSS_L();                  //ʹ������   
  SPI6_WriteRead(sFLASH_CMD_WriteEnable);   //����дʹ��  
  sFLASH_SPI_NSS_H();                  //ȡ��Ƭѡ     	      
} 
//sFLASHд��ֹ	
//��WEL����  
void sFLASH_Write_Disable(void)   
{  
  sFLASH_SPI_NSS_L();                    //ʹ������   
  SPI6_WriteRead(sFLASH_CMD_WriteDisable);  //����д��ָֹ��    
  sFLASH_SPI_NSS_H();                    //ȡ��Ƭѡ     	      
} 
//��ȡоƬID
//����ֵ����:				   
//0XEF13,��ʾоƬ�ͺ�ΪW25Q80  
//0XEF14,��ʾоƬ�ͺ�ΪW25Q16    
//0XEF15,��ʾоƬ�ͺ�ΪW25Q32  
//0XEF16,��ʾоƬ�ͺ�ΪW25Q64 
//0XEF17,��ʾоƬ�ͺ�ΪW25Q128 	  
static uint16_t sFLASH_ReadID(void)
{
	uint16_t Temp = 0;

  /* Chip Select */
	sFLASH_SPI_NSS_L();
  
  /* Send Command & Address of the indexed register */
	SPI6_WriteRead(sFLASH_CMD_ManufactDeviceID);
  
  /* Receive the data that will be read from the device (MSB First) */
  /* Send dummy byte (0x00) to generate the SPI clock to GYRO (Slave device) */
	Temp|=SPI6_WriteRead(0x00)<<8; 
	Temp|=SPI6_WriteRead(0x00);
  
  /* Chip Disable */
	sFLASH_SPI_NSS_H();
  
	return Temp;
}   	
//	�� �� ��: sFLASH_ReadInfo
//	����˵��: ��ȡ����ID,�������������
//  ��    ��:  ��
//	�� �� ֵ: ��
static void sFLASH_ReadInfo(void)
{
	/* �Զ�ʶ����Flash�ͺ� */
	{
		sflash.ChipID = sFLASH_ReadID();	/* оƬID */

		switch (sflash.ChipID)
		{
			case sFLASH_W25Q80_ID:
				strcpy(sflash.ChipName, "W25Q80");
				sflash.TotalSize = 1 * 1024 * 1024;	/* ������ = 1M */
				sflash.PageSize = 4 * 1024;			    /* ҳ���С = 4K */
				break;

			case sFLASH_W25Q32_ID:
				strcpy(sflash.ChipName, "W25Q32");
				sflash.TotalSize = 4 * 1024 * 1024;	/* ������ = 4M */
				sflash.PageSize = 4 * 1024;			    /* ҳ���С = 4K */
				break;

			case sFLASH_W25Q64_ID:
				strcpy(sflash.ChipName, "W25Q64");
				sflash.TotalSize = 8 * 1024 * 1024;	/* ������ = 8M */
				sflash.PageSize = 4 * 1024;			    /* ҳ���С = 4K */
				break;
      
      case sFLASH_W25Q128_ID:
				strcpy(sflash.ChipName, "W25Q128");
				sflash.TotalSize = 16 * 1024 * 1024;	/* ������ = 16M */
				sflash.PageSize = 4 * 1024;			    /* ҳ���С = 4K */
				break;

			default:
				strcpy(sflash.ChipName, "Unknow Flash");
				sflash.TotalSize = 2 * 1024 * 1024;
				sflash.PageSize = 4 * 1024;
				break;
		}
	}
}

//�ȴ�����
static void sFLASH_Wait_Busy(void)   
{   
	while((sFLASH_ReadSR()&0x01)==0x01);   // �ȴ�BUSYλ���
}
//�������ģʽ
void sFLASH_PowerDown(void)   
{ 
  sFLASH_SPI_NSS_L();                            //ʹ������   
  SPI6_WriteRead(sFLASH_CMD_PowerDown);     //���͵�������  
  sFLASH_SPI_NSS_H();                            //ȡ��Ƭѡ     	      
  CLK_Delay(10);                            //�ȴ�TPD  
}
//����
void sFLASH_WAKEUP(void)   
{  
  sFLASH_SPI_NSS_L();                            	//ʹ������   
  SPI6_WriteRead(sFLASH_CMD_ReleasePowerDown);	//  send sFLASH_CMD_PowerDown command 0xAB    
  sFLASH_SPI_NSS_H();                            	//ȡ��Ƭѡ     	      
  CLK_Delay(10);                               	//�ȴ�TRES1
}
//��ȡSPI FLASH  
//��ָ����ַ��ʼ��ȡָ�����ȵ�����
//pBuffer:���ݴ洢��
//ReadAddr:��ʼ��ȡ�ĵ�ַ(24bit)
//NumByteToRead:Ҫ��ȡ���ֽ���(���65535)
void sFLASH_Read(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead)   
{ 
  /* Chip Select */
	sFLASH_SPI_NSS_L();
	
  /* Send Command & Address of the indexed register */
  SPI6_WriteRead(sFLASH_CMD_ReadData);
  SPI6_WriteRead((uint8_t)((ReadAddr)>>16)); //����24bit��ַ    
  SPI6_WriteRead((uint8_t)((ReadAddr)>>8));   
  SPI6_WriteRead((uint8_t)ReadAddr);
  
  /* Receive the data that will be read from the device (MSB First) */
  while(NumByteToRead > 0x00)
  {
    /* Send dummy byte (0x00) to generate the SPI clock to GYRO (Slave device) */
    *pBuffer = SPI6_WriteRead(0x00);
    NumByteToRead--;
    pBuffer++;
  }
	
	/* Chip Disable */
	sFLASH_SPI_NSS_H();
}  
//SPI��һҳ(0~65535)��д������256���ֽڵ�����
//��ָ����ַ��ʼд�����256�ֽڵ�����
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���256),������Ӧ�ó�����ҳ��ʣ���ֽ���!!!	 
static void sFLASH_Write_Page(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{
  /* Write Enable */
  sFLASH_Write_Enable();
  
  /* Chip Select */
  sFLASH_SPI_NSS_L();                            //ʹ������   
  
  /* Send Command & Address of the indexed register */
  SPI6_WriteRead(sFLASH_CMD_PageProgram);      //����дҳ����   
  SPI6_WriteRead((uint8_t)((WriteAddr)>>16)); //����24bit��ַ    
  SPI6_WriteRead((uint8_t)((WriteAddr)>>8));   
  SPI6_WriteRead((uint8_t)WriteAddr);
  
  /* Send the data that will be written into the device (MSB First) */
  while(NumByteToWrite > 0x00)
  {
    SPI6_WriteRead(*pBuffer);
    NumByteToWrite--;
    pBuffer++;
  }
  /* Chip Select */
  sFLASH_SPI_NSS_H();                 			//ȡ��Ƭѡ 
  
  sFLASH_Wait_Busy();						//�ȴ�д�����
} 
//�޼���дSPI FLASH 
//����ȷ����д�ĵ�ַ��Χ�ڵ�����ȫ��Ϊ0XFF,�����ڷ�0XFF��д������ݽ�ʧ��!
//�����Զ���ҳ���� 
//��ָ����ַ��ʼд��ָ�����ȵ�����,����Ҫȷ����ַ��Խ��!
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���65535)
//CHECK OK
static void sFLASH_Write_NoCheck(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{ 			 		 
  uint16_t pageremain;	   
  pageremain=256-WriteAddr%256; //��ҳʣ����ֽ���		 	    
  if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;//������256���ֽ�
  while(1)
  {	   
    sFLASH_Write_Page(pBuffer,WriteAddr,pageremain);
    if(NumByteToWrite==pageremain) break;//д�������
    else //NumByteToWrite>pageremain
    {
      pBuffer+=pageremain;
      WriteAddr+=pageremain;	

      NumByteToWrite-=pageremain;			  //��ȥ�Ѿ�д���˵��ֽ���
      if(NumByteToWrite>256)pageremain=256; //һ�ο���д��256���ֽ�
      else pageremain=NumByteToWrite; 	  //����256���ֽ���
    }
  }
} 
//дSPI FLASH  
//��ָ����ַ��ʼд��ָ�����ȵ�����
//�ú�������������!
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)						
//NumByteToWrite:Ҫд����ֽ���(���65535) 
void sFLASH_Write(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)   
{ 
	uint32_t secpos;
	uint16_t secoff;
	uint16_t secremain;	   
 	uint16_t i;    
	uint8_t * sFLASH_BUF;	
#if sFLASH_USE_MALLOC==0 
  uint8_t sFLASH_BUFFER[4096];		 
#endif
  
#if	sFLASH_USE_MALLOC==1	//��̬�ڴ����
	sFLASH_BUF=mymalloc(SRAMIN,4096);//�����ڴ�
#else
  sFLASH_BUF=sFLASH_BUFFER; 
#endif     
 	secpos=WriteAddr/4096;//������ַ  
	secoff=WriteAddr%4096;//�������ڵ�ƫ��
	secremain=4096-secoff;//����ʣ��ռ��С   
 	//printf("ad:%X,nb:%X\r\n",WriteAddr,NumByteToWrite);//������
 	if(NumByteToWrite<=secremain)secremain=NumByteToWrite;//������4096���ֽ�
	while(1) 
	{	
		sFLASH_Read(sFLASH_BUF,secpos*4096,4096);//������������������
		for(i=0;i<secremain;i++)//У������
		{
			if(sFLASH_BUF[secoff+i]!=0XFF)break;//��Ҫ����  	  
		}
		if(i<secremain)//��Ҫ����
		{
			sFLASH_Erase_Sector(secpos);	//�����������
			for(i=0;i<secremain;i++)	  	//����
			{
				sFLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			sFLASH_Write_NoCheck(sFLASH_BUF,secpos*4096,4096);//д����������  

		}else sFLASH_Write_NoCheck(pBuffer,WriteAddr,secremain);//д�Ѿ������˵�,ֱ��д������ʣ������. 				   
		if(NumByteToWrite==secremain)break;//д�������
		else//д��δ����
		{
			secpos++;//������ַ��1
			secoff=0;//ƫ��λ��Ϊ0 	 

		   	pBuffer+=secremain;  //ָ��ƫ��
			WriteAddr+=secremain;//д��ַƫ��	   
		   	NumByteToWrite-=secremain;				//�ֽ����ݼ�
			if(NumByteToWrite>4096)secremain=4096;	//��һ����������д����
			else secremain=NumByteToWrite;			//��һ����������д����
		}	 
	};	
#if	sFLASH_USE_MALLOC==1		 
	myfree(SRAMIN,sFLASH_BUF);	//�ͷ��ڴ�
#endif	
}
//��������оƬ		  
//�ȴ�ʱ�䳬��...
void sFLASH_Erase_Chip(void)   
{                                   
  sFLASH_Write_Enable();                  //SET WEL 
  sFLASH_Wait_Busy();   
  sFLASH_SPI_NSS_L();                            //ʹ������   
  SPI6_WriteRead(sFLASH_CMD_ChipErase);		//����Ƭ��������  
  sFLASH_SPI_NSS_H();                            //ȡ��Ƭѡ     	      
  sFLASH_Wait_Busy();   				   	//�ȴ�оƬ��������
}   
//����һ������
//Dst_Addr:������ַ ����ʵ����������
//����һ��ɽ��������ʱ��:150ms
void sFLASH_Erase_Sector(uint32_t Dst_Addr)   
{  
	//����falsh�������,������   
  //printf("fe:%x\r\n",Dst_Addr);	  
  Dst_Addr*=4096;
  sFLASH_Write_Enable();                  //SET WEL 	 
  sFLASH_Wait_Busy();   
  sFLASH_SPI_NSS_L();                            //ʹ������   
  SPI6_WriteRead(sFLASH_CMD_SectorErase);  	//������������ָ�� 
  SPI6_WriteRead((uint8_t)((Dst_Addr)>>16));  //����24bit��ַ    
  SPI6_WriteRead((uint8_t)((Dst_Addr)>>8));   
  SPI6_WriteRead((uint8_t)Dst_Addr);  
  sFLASH_SPI_NSS_H();                       		//ȡ��Ƭѡ     	      
  sFLASH_Wait_Busy();   				   	//�ȴ��������
}  
//��ʼ��SPI FLASH��IO��
void sFLASH_Init(void)
{
  SPI6_Init();  //SPI Init
	sFLASH_ReadInfo();  /* �Զ�ʶ��оƬ�ͺ� */
}





/************************ (C) COPYRIGHT CNDZ *****END OF FILE****/

