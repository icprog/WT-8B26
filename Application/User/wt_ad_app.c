/**
  ******************************************************************************
  * @progect LZY Wire Cube Tester
	* @file    wt_bsp_ad.c
  * @author  LZY wujun
  * @version V2.0.0
  * @date    2016-9-28
  * @brief   This file provides the AD functions
  ******************************************************************************
  */

#define WT_AD_APP_GLOBALS

/* Includes ------------------------------------------------------------------*/
#include "wt_ad_app.h"
#include "wt_task_wirefindpoint.h"
#include "wt_bsp.h"

extern uint8_t ack1_flag;
extern uint8_t ack2_flag;
extern uint8_t ack3_flag;
extern uint8_t ack4_flag;

static uint8_t Update_Ack(void);
static void Clear_Ack(void);
static uint8_t Ack_Wait(void);
static uint8_t TestBoard_Reset(void);

//������ʱ�ȴ�ʱ��
//static uint16_t __WireTest_Wait_Time[] = {60,120,200,300,600,30000};
//ACK�ȴ���ʱ
static uint16_t Ack_Wait_Time[] = {15,20,30,40};  //0.1uF 0.47uF 1.0uF 2.0uF
//������ֵ
//static uint32_t Res_Threshold[] = {500,2000,10000};  //400 2K  10K

#define  TESTWAIT_DELAY  250
#define  TESTNUM         30
#define  TESTOKNUM       25
#define  RETESTNUM       3

/**
  * @brief  ����ack�˿�
  * @param  None
* @retval 0:ok, 1:error
  */
static uint8_t Update_Ack(void)
{
    uint8_t buf8 = 0;
    if(ack1_flag == 1)
    {
        buf8 |= 1<<0;
        //ack1_flag = 0;
    }
    if(ack2_flag == 1)
    {
        buf8 |= 1<<1;
        //ack2_flag = 0;
    }
    if(ack3_flag == 1)
    {
        buf8 |= 1<<2;
        //ack3_flag = 0;
    }
    if(ack4_flag == 1)
    {
        buf8 |= 1<<3;
        //ack4_flag = 0;
    }
    return buf8;
}

/**
  * @brief ���ack�˿�
  * @param  None
* @retval 0:ok, 1:error
  */
static void Clear_Ack(void)
{
    ack1_flag = 0;
    ack2_flag = 0;
    ack3_flag = 0;
    ack4_flag = 0;  
}

/**
  * @brief �ȴ�ack��Ӧ
  * @param  None
* @retval 0:ok, 1:error
  */
static uint8_t Ack_Wait(void)
{
    uint8_t i;
    uint8_t  ack_stat;
    uint8_t  cnt_error = 0;
    
    for(i=0;i<100;i++)
    {
        ack_stat = Update_Ack();
        if(ack_stat != 0) break;
        else {cnt_error++;}
    }
    if(cnt_error == 100) 
    {
        return 1;
    }
    else 
    {
        return 0;
    }
    
}

/**
  * @brief ���԰忨��λ
  * @param  None
* @retval 0:ok, 1:error
  */
static uint8_t TestBoard_Reset(void)
{
    uint8_t ack_stat = 0;
    //���Ͱ忨��λ�ź�
    PS_MOSI_RST_L;
    CLK_Delay(56);	//1us == 168/3=56
    Clear_Ack();	
    PS_MOSI_RST_H;
    ack_stat = Ack_Wait();
    if( ack_stat == 0) return 0; 
    else return 1;
}

/**
  * @brief ����PS_MOSI_INC_OUT�ź�
  * @param  None
  * @retval 0:ok, 1:error
  */
static uint8_t Mosi_Inc_Out(void)
{
    uint8_t ack_stat = 0;
    //����PS_MOSI_INC_OUT�ۼ��ź�
    Clear_Ack();
    PS_MOSI_INC_OUT_L;
    CLK_Delay(56);	//1us == 168/3		
    PS_MOSI_INC_OUT_H;
    ack_stat = Ack_Wait();
    if( ack_stat == 0) return 0;
    else return 1;
}

/**
  * @brief ����PS_MOSI_INC_IN�ź�
  * @param  None
  * @retval 0:ok, 1:error
  */
static uint8_t Mosi_Inc_In(void)
{
    uint8_t ack_stat = 0;
    //����PS_MOSI_INC_IN�ۼ�
    Clear_Ack();
    PS_MOSI_INC_IN_L;
    CLK_Delay(56);	//1us == 168/3
    PS_MOSI_INC_IN_H;
    ack_stat = Ack_Wait();
    if( ack_stat == 0) return 0;
    else return 1;
}

/**
  * @brief  �������� - ��ͨ
  * @param  item, result value
  * @retval 0:ok, 1:value error, 2:test error,
  */
uint8_t AD_TestItem_Wire(void)
{
    uint32_t tmp=0;
    uint8_t res = 0;
    uint16_t cnt_retest = 0;
    uint8_t i;
    uint8_t cnt_w = 0;
	uint16_t buf16;
    uint32_t resval;
	 
	//PS_MOSI_INC_IN�ۼ�	
    if(Mosi_Inc_In() == 0)
    {
        //��ȡ�����ź�
        while(cnt_retest < RETESTNUM)
        {
            CLK_Delay(56*Ack_Wait_Time[WT_Config.Mode_TestCap]);//3us
            for( i=0; i<TESTNUM ; i++ )
            {
                //�ж�����PC1 PC2, 00��ʾZ   01��ʾR  11��ʾW */
                tmp = AD_InputState & 0x0006;
                tmp = tmp >> 1;
                if(tmp == 3)
                {
                    cnt_w++;
                } 
            }
            if(cnt_w >= TESTOKNUM)
            {
                return 0;
            }
            cnt_retest++;
        }
        if(tmp == 1)
        {
            //���ڵ�����������Ҫ����ʱ250us,���ж�����
            CLK_Delay(56 * TESTWAIT_DELAY);//250us
            tmp = AD_InputState & 0x0006;
            tmp = tmp >> 1;
            if(tmp == 0)
            {
               return 0;
            }
            else if(tmp == 1)
            {             
                res = GetADValue(&buf16);
                if(res != 0) 
                {
                    return 2;
                }
                
                GetResValue( buf16 , &resval );
                if(resval <= 800 )//����ֵС��800�ж�Ϊ��ͨ
                {
                    return 0; //����												
                }
                else return 1;
            }
            else return 1;
        }
        else
        {
            return 1;
        }
    }
    else 
    {
        return 2;
    }
	
}

/**
  * @brief  �������� - ����
  * @param  item, result value
  * @retval 0:ok, 1:value error, 2:test error,
  */
uint8_t AD_TestItem_Z(void)
{
    uint32_t tmp=0;
    uint8_t i;
    uint16_t cnt_retest = 0;
    uint8_t res = 0;
    uint8_t cnt_z = 0;
    uint16_t buf16;
    uint32_t resval;
	
	//PS_MOSI_INC_IN�ۼ�
    if(Mosi_Inc_In() == 0)
    {
        //��ȡ�����ź�
        while(cnt_retest < RETESTNUM)
        {
            CLK_Delay(56*Ack_Wait_Time[WT_Config.Mode_TestCap]);//3us
            
            for( i=0; i<TESTNUM ; i++ )
            {
                //�ж�����PC1 PC2, 00��ʾZ   01��ʾR  11��ʾW */
                tmp = AD_InputState & 0x0006;
                tmp = tmp >> 1;
                if(tmp == 0)
                {
                    cnt_z++;
                } 
            }
            if(cnt_z >= TESTOKNUM)
            {
                return 0;
            }
            cnt_retest++;
        }
        if(tmp == 1)//���ڵ�����������Ҫ����ʱ250us,���ж�����
        {
            CLK_Delay(56 * TESTWAIT_DELAY);//250us
            tmp = AD_InputState & 0x0006;
            tmp = tmp >> 1;
            if(tmp == 0)
            {
                return 0;
            }
            else if(tmp == 1) //����
            {
                res = GetADValue(&buf16);
                if(res != 0) 
                {
                        return 2;
                }
                
                GetResValue( buf16 , &resval );
                if(resval > 10000)
                {
                        return 0; //����												
                }
                else return 1;

            }
            else 
            {
                return 1;
            }

        }
        else
        {
		    return 1;
        }

    }
    else 
    {
        return 2;
    }
}



/**
  * @brief  �������� - ����
  * @param  item, result value
  * @retval 0:ok, 1:value error, 2:test error,
  */
uint8_t AD_TestItem_Res(uint64_t index,uint32_t* result)
{
    uint32_t tmp=0;
    uint8_t res = 0;
    uint8_t i;
    uint8_t cnt_r = 0;
	uint16_t buf16;
    uint32_t resval;
    
	//PS_MOSI_INC_IN�ۼ�
    if(Mosi_Inc_In() == 0)
    {
        //��ȡ�����ź�
        CLK_Delay(56*Ack_Wait_Time[WT_Config.Mode_TestCap]);//3us
								
        for( i=0; i<TESTNUM ; i++ )
        {
            //�ж�����PC1 PC2, 00��ʾZ   01��ʾR  11��ʾW */
            tmp = AD_InputState & 0x0006;
            tmp = tmp >> 1;
            if(tmp == 1)
            {
                cnt_r++;
            } 
        }
        if(cnt_r >= TESTOKNUM)
        {
            //���ڵ�����������Ҫ����ʱ250us,���ж�����
            CLK_Delay(56 * TESTWAIT_DELAY);//250us
            tmp = AD_InputState & 0x0006;
            tmp = tmp >> 1;
            if(tmp == 1) //����
            {

                res = GetADValue(&buf16);
                if(res != 0) 
                {
                        return 2;
                }
                
                GetResValue( buf16 , &resval );
                if(resval > TestFile->test_item[index].param2 && resval < TestFile->test_item[index].param3)
                {
                        return 0; //����												
                }
                else 
                {
                        return 1;
                }
            }
            else if(tmp == 3) //������Խ��Ϊ��ͨ,����
            {
                return 1;
            }
            else  //���� == ����
            {
                return 1;
            }
                
        }
        else if(tmp == 3) //������Խ��Ϊ��ͨ����
        {
                return 1;
        }
        else
        {
                return 1;
        }
         
    }
    else 
    {
        return 2;
    }
}


/**
  * @brief  �������� - �ҵ�
  * @param  item, result value
  * @retval 0:connet test point, 1:not detect, 2:test error,
  */
uint8_t AD_TestItem_FindPoint(uint16_t PortNumber)
{
    uint32_t tmp=0;
	 
	//PS_MOSI_INC_IN�ۼ�
    if(Mosi_Inc_In() == 0)
    {
        //��ȡ�����ź�
        CLK_Delay(56*Ack_Wait_Time[WT_Config.Mode_TestCap]);//3us
     
        /*�ж�����PC1 PC2, 00��ʾZ   01��ʾR  11��ʾW */
        tmp = AD_InputState & 0x0006;
        tmp = tmp >> 1;
		if(tmp == 3 || tmp == 1 )
        {
            return 0; //0����̽��
        }    
        else if(tmp == 0)
        {
            return 1; //1����
        }
        else
        {
            return 2; //test error
        }
    }
    else 
    {
        return 2;
    }

}


/**
  * @brief  ִ��һ����������
    * @param  mode, 0:test, 1:wait for wire remove
	* @retval 0:ok, 1:file errror, 2:test error, 3-user cancel; 
   * ��Ҫ˵���V2.0 ȡ�����μ�⹦��
  */
static uint8_t WireTest_OneTime(uint32_t* cnt_error_total, uint32_t* cnt_error_z)
{
	uint8_t  res = 0;
	uint32_t buf32;
	uint32_t cnt_item_error = 0;
	uint32_t cnt_item_Z_error = 0;
	uint16_t i,j;
    uint64_t index = 0;
    //uint8_t ack_stat;
	
	//���ȡ����ť
	if(TestFile->command == 2) return 3;	//0-no operate, 1-start, 2-cancel
	if(TestFile->item_total == 0) return 1;
    
    //���Ͱ忨��λ�ź�
    if(TestBoard_Reset()) 
    {
        return 1; 
    }
    
    for( i=0; i< (64*SelfCheckItem.port_board_number) -1 ; i++)//PS_MOSI_INC_OUT�ۼ�
	{
        if(Mosi_Inc_Out() == 0)
        {
            for(j=i+1;j<(64*SelfCheckItem.port_board_number);j++)//PS_MOSI_INC_IN�ۼ�
            {
                //���ȡ����ť
                if(TestFile->command == 2) return 3;	//0-no operate, 1-start, 2-cancel               
                
                //���ID
                  TestFile->item_Index = index+1;
                  
                if(TestFile->test_item[index].id != TestFile->item_Index) return 1;
                if(TestFile->test_ErrFlag == 0)	//for max error ID��0-no error, 1-error detect and retest
                {
                    TestFile->item_current = TestFile->test_item[index].id;
                }
                else
                {
                    if(TestFile->test_item[index].id > TestFile->item_current)	//����ͨ������
                    {
                        TestFile->item_current = TestFile->test_item[index].id;
                        TestFile->test_ErrFlag = 0; //for max error ID��0-no error, 1-error detect and retest
                    }
                }
                
                //�л�AD_input
                //Switch_AD_Input(j);

                //��������
                switch(TestFile->test_item[index].type)
                {
                    case 'W':	//��ͨ  
                        //�����ۼ�
                        res = AD_TestItem_Wire();	//0:ok, 1:value error, 2:test error,  
                        
                        // ������Խ��
                        if(res == 0)
                        {
                            TestFile->test_item[index].result = 1; //0:not test, 1:ok, 2:open circuit error 3:dislocation error 4:Z error
                        }
                        else if(res == 1)
                        {
                            TestFile->test_item[index].result = 2;
                            cnt_item_error++; //return 2;
                        }
                        else return 2;                        
                        break;
                    case 'Z':	//����
                        res = AD_TestItem_Z();	//0:ok, 1:value error, 2:test error,3:signal not stable
                        
                        // ������Խ��
                        if(res == 0)
                        {
                            TestFile->test_item[index].result = 1;//0:not test, 1:ok, 2:open circuit error 3:dislocation error 4:Z error
                        }
                        else if(res == 1)
                        {
                            TestFile->test_item[index].result = 4;
                            cnt_item_error++; //return 2;
                            cnt_item_Z_error++;
                        }
                        else 
                        {
                            return 2;
                        }
                        break;
			
                    case 'R':	//����
                        res = AD_TestItem_Res(index,&buf32);	//0:ok, 1:value error, 2:test error,
                        
                        // ������Խ��
                        if(res == 0)
                        {
                            TestFile->test_item[index].result = 1;
                        }
                        else if(res == 1)
                        {
                            TestFile->test_item[index].result = 2;
                            cnt_item_error++; //return 2;
                        }
                        else return 2;
                        break;
                    default:
                        return 1;
                }
                index++;
            }           
        }
        else 
        {
            return 1;
        }
    }
    
	
	
	Update_Loopinfo();
	
	*cnt_error_total = cnt_item_error;
	*cnt_error_z = cnt_item_Z_error;
	
	if(cnt_item_Z_error > 0) // z error
	{
		TestFile->task_status = 5;	//0-waiting, 1-testing, 2-test error, 3-test ok, 4-waiting remove wire, 5-testing & Z error
		//BSP_UartAudioNG_BeepOn(255); 	//0-off, 1- 1 beep, 2-2beep,3-3beep,255-on
		//osMessagePut(UartAudioEvent, UartAudioNG_TX_Event, 0);	//��������
		LED4_ON_R();
	}
	else // no z error
	{
		TestFile->task_status = 1;	//0-waiting, 1-testing, 2-test error, 3-test ok, 4-waiting remove wire, 5-testing & Z error
		//BSP_UartAudioOK_BeepOn(0); 	//0-off, 1- 1 beep, 2-2beep,3-3beep,255-on
		LED4_OFF();
		//Infra_PWM_OFF();//�رպ����ź�
		UART_Infr_Switch = 1;
	}
	
	
	TestFile->item_error_count = cnt_item_error;
	//return ok
	if(cnt_item_error == 0) 
	{
		LED4_OFF();
		//Infra_PWM_OFF();//�رպ����ź�
		UART_Infr_Switch = 1;
		//osMessagePut(UartAudioEvent, UartAudioSTOP_TX_Event, 0);	//ֹͣ��������
		return 0;
	}
	else										return 2;
}


/**
  * @brief  �������Գ���
  * @param  None
	* @retval 0:ok, 1:file errror, 2:test error, 3-user cancel; 
  */
uint8_t WireTest_Process(void)
{
	uint32_t i;
	uint8_t res;
	uint32_t cnt_error_total = 0;
	uint32_t cnt_error_z = 0;
	static uint8_t state_audio = 0;	//0-stop, 1-repeat play, 2- single play
	uint32_t buf32[2]={0}; //for test only
	//uint8_t cnt_loop=0;
//    uint16_t wait_cnt = 0;
	
	// Init restult
	for(i=0; i< TestFile->item_total; i++)
		TestFile->test_item[i].result = 0;
	TestFile->test_ErrFlag = 0;	//for max error ID��0-no error, 1-error detect and retest

	while(1)
	{
		//���������ͨ
		buf32[0] = HAL_GetTick();
		
		res = WireTest_OneTime(&cnt_error_total, &cnt_error_z);	//0:ok, 1:file errror, 2:test error, 3-user cancel; 
		
		buf32[1] = HAL_GetTick() - buf32[0];
        
//        if(res == 0 || res == 2)
//        {
//            res = WireTest_OneTime(&cnt_error_total, &cnt_error_z);//�����һ�Σ�ȷ�����Խ��׼ȷ
//        }
        
		if(state_audio == 0)	//0-stop, 1-repeat play, 2- single play
		{
			if(cnt_error_z > 0)
			{
				osMessagePut(UartAudioEvent, UartAudioNG_TX_Event, 0);	//��������
				//osDelay(50); 
				state_audio = 1;	//0-stop, 1-repeat play, 2- single play
			}
		}
		else if(state_audio == 1)	//0-stop, 1-repeat play, 2- single play
		{
			if(cnt_error_z == 0)
			{
				osMessagePut(UartAudioEvent, UartAudioSTOP_TX_Event, 0);	//ֹͣ����
				//osDelay(50); 
				state_audio = 0;	//0-stop, 1-repeat play, 2- single play
			}
		}
				
		if(res == 2)	//
		{
			TestFile->test_ErrFlag = 1;	//for max error ID��0-no error, 1-error detect and retest
			if(TestFile->item_Index < TestFile->item_current)
			{
				for(i=TestFile->item_Index;i<TestFile->item_current;i++)
				{
					TestFile->test_item[i].result = 0;	//0:not test, 1:ok, 2:error
				}
			}
		}
		else
		{
			//BSP_UartAudioOK_BeepOn(0); 	//0-off, 1- 1 beep, 2-2beep,3-3beep,255-on
			LED4_OFF();
			//Infra_PWM_OFF();//�رպ����ź�
			UART_Infr_Switch = 1;
			return res;
		}
        
//		if(TestFile->cnt_okloop > 0 && TestFile->test_ErrFlag == 1 && cnt_error_z > 0) wait_cnt++;//����������ʱ�ſ�ʼ����
//        else wait_cnt = 0;
//		if(wait_cnt > __WireTest_Wait_Time[WT_Config.Test_Wait] )//�ȴ�n�Σ�����δͨ�������Ͳ��Խ��2
//		{
//			return 2;
//		}
		// short dealy
		osDelay(100); 
	}
}

/**
  * @brief  ִ��һ���������ԣ��ȴ�����ȫ������
	* @param  None
	* @retval 0:all remove, 1:file errror, 2:not all remove, 3-user cancel; 
  */
static uint8_t WireTest_WaitRemove_OneTime(void)
{
	uint8_t  res = 0;
	uint32_t buf32;
	uint32_t temp = 0;
	uint16_t i,j;
	uint32_t index;
	//uint8_t ack_stat;

	//���ȡ����ť
	if(TestFile->command == 2) return 3;	//0-no operate, 1-start, 2-cancel
	if(TestFile->item_total == 0) return 1;
    
    index = 0;
    
    //���԰忨��λ
    if(TestBoard_Reset())
    {
        return 1; 
    }
    
    for( i=0; i< (64*SelfCheckItem.port_board_number) -1 ; i++)//PS_MOSI_INC_OUT�ۼ�
	{
        if(Mosi_Inc_Out() == 0)
        {
            for(j=i+1;j<(64*SelfCheckItem.port_board_number);j++)//PS_MOSI_INC_IN�ۼ�
            {
                //���ȡ����ť
                if(TestFile->command == 2) return 3;	//0-no operate, 1-start, 2-cancel               
                
                //���ID
                TestFile->item_Index = index+1;
                  
                if(TestFile->test_item[index].id != TestFile->item_Index) 
                {
                    return 1;
                }
                if(TestFile->test_ErrFlag == 0)	//for max error ID��0-no error, 1-error detect and retest
                {
                    TestFile->item_current = TestFile->test_item[index].id;
                }
                else
                {
                    if(TestFile->test_item[index].id > TestFile->item_current)	//����ͨ������
                    {
                        TestFile->item_current = TestFile->test_item[index].id;
                        TestFile->test_ErrFlag = 0; //for max error ID��0-no error, 1-error detect and retest
                    }
                }
                
                 //�л�AD_input
                //Switch_AD_Input(j);

                //��������
                switch(TestFile->test_item[index].type)
                {
                    case 'W':	//��ͨ 
                        if(TestFile->test_item[index].result == 0) 
                        {
                            break;	//0:not test, 1:ok, 2:error
                        }
                        temp++;
                    
                        //�����ۼ�
                        res = AD_TestItem_Wire();	//0:ok, 1:value error, 2:test error,
                        // ������Խ��
                        if(res == 0)	//W - ok
                        {
                        }
                        else if(res == 1) //Z
                        {
                            TestFile->test_item[index].result = 0;	//0:not test, 1:ok, 2:error
                        }
                        else
                        {
                            return 2;
                        }  
                        break;
                    case 'R':	//����
                        if(TestFile->test_item[index].result == 0) break;	//0:not test, 1:ok, 2:error
                        temp++;
                    
                        res = AD_TestItem_Res(index,&buf32);		//0:ok, 1:value error, 2:test error,
                        // ������Խ��
                        if(res == 0)	//R - ok
                        {
                        }
                        else if(res == 1) //Z
                        {
                            TestFile->test_item[index].result = 0;	//0:not test, 1:ok, 2:error
                        }
                        else
                        {
                            return 2;
                        }
                        break;
                    case 'Z':	//����
//                        if(TestFile->test_item[index].result == 0) break;	//0:not test, 1:ok, 2:error
//                        temp++;
                        res = AD_TestItem_Z();	//0:ok, 1:value error, 2:test error,3:signal not stable
                        if(res == 2)
                        {
                            return 2;
                        }
//                        if(res == 0)	//R - ok
//                        {
//                        }
//                        else if(res == 1) //Z
//                        {
//                            TestFile->test_item[index].result = 0;	//0:not test, 1:ok, 2:error
//                        }
//                        else
//                        {
//                            return 2;
//                        }
                        break;
                    default:	                       
                        break;                     
                }
                index++;
            }           
        }
        else
        {
            return 1;
        }
    }
	//return ok
	if(temp == 0)	return 0;
	else 			return 2;
}

/**
  * @brief  �Զ�����ģʽ���ȴ���������
  * @param  None
	* @retval 0:all remove, 1:file errror, 3-user cancel; 
  */
uint8_t WireTest_WaitForNext(void)				
{
	uint8_t res;
	while(1)
	{
		//���������ͨ
		res = WireTest_WaitRemove_OneTime();	//0:all remove, 1:file errror, 2:not all remove, 3-user cancel; 
		if(res != 2) 
		{
			return res;
		}
		osDelay(100);
	}
}

/**
  * @brief  ������ѧϰ
  * @param  j1, j2, result(����ֵ)
  * @retval 0:W, 1:Z, 2-study error,3:R
  */
uint8_t AD_StudyItem(uint16_t i, uint16_t j, uint32_t* result)
{
	uint8_t res;
//	float votage;
	uint16_t buf16;
	uint32_t tmp=0;	
//    uint16_t cmp1 = 0;
//    uint16_t cmp2 = 0;
    uint8_t cnt_w = 0;
    uint8_t cnt_z = 0;
	
    *result = 0;
	
	//PS_MOSI_INC_IN�ۼ�	
    if( Mosi_Inc_In() == 0)
    {
//        //���òο���ѹ  DA = (2.363/3.3)*4095 = 2932  == 0xB74 
//        cmp1 = (2.55/3.3)*4095;
//        cmp2 = (2.59/3.3)*4095;
//        SetCmpVoltage(cmp1,cmp2);
        
        //��ȡ�����ź�
        CLK_Delay(56*Ack_Wait_Time[WT_Config.Mode_TestCap]);//3us
        
//        if(i==8 && j==122)
//        {
//             CLK_Delay(56);//3us
//        }

        /* �ж�����PC1 PC2, 00��ʾZ   01��ʾR  11��ʾW */
        for( i=0; i<TESTNUM ; i++ )
        {
            //�ж�����PC1 PC2, 00��ʾZ   01��ʾR  11��ʾW */
            tmp = AD_InputState & 0x0006;
            tmp = tmp >> 1;
            if(tmp == 0)
            {
                cnt_z++;
            }
            else if(tmp == 3)
            {
                cnt_w++;
            }
        }
        if(cnt_w >= TESTOKNUM)// ��ͨ
        {
            return 0;
        }
        else if(cnt_z >= TESTOKNUM)//����
        {
            return 1;        
        }
        else if(tmp == 1)//����/����
        {
            //���ڵ�����������Ҫ����ʱ250us,���ж�����
            CLK_Delay(56 * TESTWAIT_DELAY);//250us
            tmp = AD_InputState & 0x0006;
            tmp = tmp >> 1;
            if(tmp == 1) //����
            {
                res = GetADValue(&buf16);
                if(res != 0) return 2;
                GetResValue( buf16 , result );
                //if(*result <= Res_Threshold[WT_Config.Mode_TestW])//����ֵ�ڵ�����ֵ��Χ��
                if(*result <= 800)//����ֵС��800�ж�Ϊ��ͨ
                {
                    return 0; 
                }
                else if(*result <= 10000)//����ֵ�ڵ�����ֵ��Χ��(11K)
                {
                    return 3; 
                }
                else return 1; //����ֵ����������ֵ��ΧΪ����
            }
            else if(tmp == 0)
            {
                return 0;
            }
            else  //���� == ����
            {
                return 1;
            }
           
        }
        else //����
        {
            return 1;
        }

    }
    else 
    {
        return 2;
    }
	
}

uint16_t get_index_total_bill(uint8_t m, uint8_t n)
{
	uint16_t NumOfPort = SelfCheckItem.port_board_number * 64;
 
	//return ((256 + (256 - m)) * (m+1) / 2 - 256 + (n - m) - 1);
	return ((NumOfPort + (NumOfPort - m)) * (m+1) / 2 - NumOfPort + (n - m) - 1);

}

/**
  * @brief  �Ż�ѧϰ�ļ�
  * @param  None
  * @retval result, 0:ok,
  */
uint8_t StudyFile_Optimize(void)
{
	uint16_t i, j;
	uint16_t PortState[256];
	uint8_t  cnt_line;
	uint8_t  PortBoardInfo[4][3];	//byte0:Point count, byte1:min Point, byte2:max Point,
	uint8_t  min_point, max_point;
//	uint16_t index, index_Total;
//	uint16_t port_pL0, port_pH0;
//	uint16_t port_pL1, port_pH1;
//	uint32_t total;
	
	//init
//	total = StudyFile->item_total;
	
	//��Ƕ˿�״̬R+W
	for(i=0;i<256;i++) PortState[i] = 0;
	for(i=0;i<StudyFile->item_total;i++)
	{
		if(StudyFile->study_item[i].type == 'W' || StudyFile->study_item[i].type == 'R')
		{
			PortState[StudyFile->study_item[i].p1 - 1] += 1;	// cnt
			PortState[StudyFile->study_item[i].p2 - 1] += 1;	// cnt
		}
	}
	
	//�����λ����
	//��ǰ忨״̬
	for(i=0;i<4;i++) {for(j=0;j<3;j--){PortBoardInfo[i][j] = 0;}}//byte0:Point count, byte1:min Point, byte2:max Point,
	for(i=0;i<4;i++)	//board 1-4
	{
		min_point = 63;
		max_point = 0;
		for(j=0;j<64;j++)	//port 1-64
		{
			if(PortState[i*64 + j] != 0)
			{
				PortBoardInfo[i][0]++;
				if(j < min_point) min_point = j;
				if(j > max_point) max_point = j;
			}
		}
		PortBoardInfo[i][1] = min_point;
		PortBoardInfo[i][2] = max_point;
	}
	StudyFile->number_point = PortBoardInfo[0][0]+PortBoardInfo[1][0]+PortBoardInfo[2][0]+PortBoardInfo[3][0];
	
	//���±��W�˿�״̬
	for(i=0;i<256;i++) PortState[i] = 0;
	for(i=0;i<StudyFile->item_total;i++)
	{
		if(StudyFile->study_item[i].type == 'W' )
		{
			PortState[StudyFile->study_item[i].p1 - 1] += 1;	// cnt
			PortState[StudyFile->study_item[i].p2 - 1] += 1;	// cnt
		}
	}
	
	//����W��·����
	cnt_line  = 0;
	for(i=0;i<StudyFile->item_total;i++)
	{
		if(StudyFile->study_item[i].type == 'W' )
		{
			if((PortState[StudyFile->study_item[i].p1 - 1] == 1) && (PortState[StudyFile->study_item[i].p2 - 1] == 1) )
			{
				cnt_line++;
			}
			else if((PortState[StudyFile->study_item[i].p1 - 1] > 1) && (PortState[StudyFile->study_item[i].p2 - 1] == 1))
			{
				PortState[StudyFile->study_item[i].p1 - 1]--;
			}
			else if((PortState[StudyFile->study_item[i].p1 - 1] == 1) && (PortState[StudyFile->study_item[i].p2 - 1] > 1))
			{
				PortState[StudyFile->study_item[i].p2 - 1]--;
			}
			else if((PortState[StudyFile->study_item[i].p1 - 1] > 1) && (PortState[StudyFile->study_item[i].p2 - 1] > 1))
			{
				PortState[StudyFile->study_item[i].p1 - 1]--;
				PortState[StudyFile->study_item[i].p2 - 1]--;
			}
			else continue;
		}
	}
	StudyFile->number_line = cnt_line;
	
	//���±��R�˿�״̬
	for(i=0;i<256;i++) PortState[i] = 0;
	for(i=0;i<StudyFile->item_total;i++)
	{
		if(StudyFile->study_item[i].type == 'R')
		{
			PortState[StudyFile->study_item[i].p1 - 1] += 1;	// cnt
			PortState[StudyFile->study_item[i].p2 - 1] += 1;	// cnt
		}
	}
	//����R��·����
	cnt_line  = 0;
	for(i=0;i<StudyFile->item_total;i++)
	{
		if(StudyFile->study_item[i].type == 'R')
		{
			if((PortState[StudyFile->study_item[i].p1 - 1] == 1) && (PortState[StudyFile->study_item[i].p2 - 1] == 1) )
			{
				cnt_line++;
			}
			else if((PortState[StudyFile->study_item[i].p1 - 1] > 1) && (PortState[StudyFile->study_item[i].p2 - 1] == 1))
			{
				PortState[StudyFile->study_item[i].p1 - 1]--;
			}
			else if((PortState[StudyFile->study_item[i].p1 - 1] == 1) && (PortState[StudyFile->study_item[i].p2 - 1] > 1))
			{
				PortState[StudyFile->study_item[i].p2 - 1]--;
			}
			else if((PortState[StudyFile->study_item[i].p1 - 1] > 1) && (PortState[StudyFile->study_item[i].p2 - 1] > 1))
			{
				PortState[StudyFile->study_item[i].p1 - 1]--;
				PortState[StudyFile->study_item[i].p2 - 1]--;
			}
			else continue;
		}
	}
	StudyFile->number_line += cnt_line;
	
//	//--------------------------------------------------------------------------------------------------------------------
//	//�޸������С��λ���Ż�ģʽ������ģʽ��
//	//--------------------------------------------------------------------------------------------------------------------
////	if(WT_Config.StudyOptimizingMode == 0) //0:full test, 1:only test Z bettwn max & min W point,
////	{
//		//����ģʽ
//		for(i=0;i<4;i++)
//		{
//			if(PortBoardInfo[i][0] > 0) //find point
//			{
//				PortBoardInfo[i][1] = 0;	//min
//				PortBoardInfo[i][2] = 63;	//max
//			}
//		}
////	}
//		
//	//=========================================================
//	/* step 3: ����ѧϰ�ļ� */
//	//=========================================================
//	index = 0;
//	for(i=0;i<4;i++) //first board
//	{
//		if(PortBoardInfo[i][0] != 0)	//first board - point
//		{
//			port_pL0 = i*64 + PortBoardInfo[i][1];
//			port_pH0 = i*64 + PortBoardInfo[i][2];
//			for(m=port_pL0;m<=port_pH0;m++) //First point
//			{
//				for(j=i;j<4;j++) // second board
//				{
//					if(PortBoardInfo[j][0] != 0)	//second board - point
//					{
//						port_pL1 = j*64 + PortBoardInfo[j][1];
//						port_pH1 = j*64 + PortBoardInfo[j][2];
//						for(n=port_pL1;n<=port_pH1;n++) //second point
//						{
//							if(n > m) //����ͬһ��
//							{
//								index_Total = get_index_total_bill(m,n);
//								//copy to new pointer
//								StudyFile->study_item[index] = StudyFile->study_item[index_Total];
//								StudyFile->study_item[index].id = index + 1;
//								index++;	
//							} //end if(m != n) //����ͬһ��
//						} //end for(n=port_pL1;n<=port_pH1;n++) //second point
//					} //end if(PortBoardInfo[j][0] != 0)	//second board - point
//				} //for(j=i;j<4;j++) // second board
//			} //end for(m=port_pL0;m<=port_pH0;m++) //First point
//		} //end if(PortBoardInfo[i][0] != 0)	//first board - point
//	} //end for(i=0;i<4;i++) //first board
//	StudyFile->item_total = index;
//	
//	//=========================================================
//	/* step 4: ��������� */
//	//=========================================================
//	for(;index<total;index++)
//	{
//		StudyFile->study_item[index].id = 0;
//		StudyFile->study_item[index].p1 = 0;
//		StudyFile->study_item[index].p2 = 0;
//		StudyFile->study_item[index].param1 = 0;
//		StudyFile->study_item[index].param2 = 0;
//		StudyFile->study_item[index].param3 = 0;
//		StudyFile->study_item[index].type = 0;
//	}

	StudyFile->item_index = 0;
	//return
	return 0;
}


/**
  * @brief  ������ѧϰ����
  * @param  None
* @retval 0:ok, 1:study error, 2:user cancel; 3��R
  */
uint8_t WireStudy_Process(uint16_t 	NumStudyPort)	//0:ok, 1:file errror, 2:test error, 3-user cancel; 
{
	uint8_t  res;
	uint16_t i, j;
	uint32_t buf32 = 0;
//	uint8_t  ack_stat;
    
	//init
	StudyFile->item_total = 0;
	StudyFile->item_index = 0;
	StudyFile->max_point = 0;
	StudyFile->min_point = 0;
    
    //���Ͱ忨��λ�ź�
    if(TestBoard_Reset()) return 1;
    
    start_study:	
	for(i=0;i<NumStudyPort-1;i++)//PS_MOSI_INC_OUT�ۼ�
	{
        if( Mosi_Inc_Out() == 0)
        {
            for(j=i+1;j<NumStudyPort;j++)//PS_MOSI_INC_IN�ۼ�
            {
                // user cancle
                if(StudyFile->command == 2) return 2; //0-no operate, 1-start, 2-cancel
                
                 //�л�AD_input
                //Switch_AD_Input(j);
                //test
                res = AD_StudyItem(i, j, &buf32);	//0:W, 1:Z, 2-study error,3��R
                if(res == 0)	//W
                {
                    StudyFile->study_item[StudyFile->item_index].type = 'W';
                }
                else if(res == 1)	//Z
                {
                    StudyFile->study_item[StudyFile->item_index].type = 'Z';
                }
                else if(res == 3)	//R
                {
                    StudyFile->study_item[StudyFile->item_index].type = 'R';
                }
                else if(res == 4)	//signal not stable
                {
                    return 1; //study error
                }
                else	//error
                {
                    return 2;
                }
                
                //update result
                StudyFile->study_item[StudyFile->item_index].p1 = i+1;
                StudyFile->study_item[StudyFile->item_index].p2 = j+1;
                StudyFile->study_item[StudyFile->item_index].id = StudyFile->item_index+1;
                StudyFile->study_item[StudyFile->item_index].param1 = buf32;
                if(res == 3)//������������Χ
                {
                    if(buf32 < 1000)
                    {
                        StudyFile->study_item[StudyFile->item_index].param2 = (uint32_t)(buf32 - 100);
                        StudyFile->study_item[StudyFile->item_index].param3 = (uint32_t)(buf32 + 100);
                    }
                    else
                    {
                        
                        StudyFile->study_item[StudyFile->item_index].param2 = (uint32_t)(buf32*0.5);
                        StudyFile->study_item[StudyFile->item_index].param3 = (uint32_t)(buf32*1.5);
                    }
                }
                else
                {
                    StudyFile->study_item[StudyFile->item_index].param2 = 0;
                    StudyFile->study_item[StudyFile->item_index].param3 = 0;
                }
                StudyFile->item_index++;
            }		
            
        }
        else  //û���յ���Ӧ����λ���԰忨
        {
            //���Ͱ忨��λ�ź�
            if(TestBoard_Reset()) return 1; 
            else {goto start_study;}
        }
       
	}

	StudyFile->item_total = StudyFile->item_index;
	return 0;
}

/**
  * @brief  ִ��һ���ҵ�ɨ��
	* @param  None
  * @retval 0:ok, 1:error, 2-user cancel
  */
static uint8_t FindPoint_OneTime(uint16_t PortNumber)
{
	uint8_t  res = 0;
	uint16_t i;
	uint8_t  buf8[256] = {0};
	uint16_t cnt = 0;
	static uint16_t cnt_last = 0;
	static uint8_t audio_stat = 0; //0-δ����  1-�Ѳ���
	
    //���Ͱ忨��λ�ź�
    if(TestBoard_Reset()) return 1; 
	
	//���ȡ����ť
	if(FindPoint.command == 2) return 2;	//0-no operate, 1-start, 2-cancel
	if((PortNumber <= 0) || (PortNumber > 256*4)) return 1;
	for(i=0; i<PortNumber; i++)
	{
		//���ȡ����ť
		if(FindPoint.command == 2) return 2;	//0-no operate, 1-start, 2-cancel
		
		res = AD_TestItem_FindPoint(i);	//0:connet test point, 1:not detect, 2:test error,
		// ������Խ��
		if(res == 0)	//0:connet test point
		{
			buf8[cnt] = i;
			cnt++;
		}
		else if(res == 1) //1:not detect
		{
		}
		else
		{
			return 1;
		}
	}

	if(cnt == cnt_last)
	{
		//update data
		FindPoint.task_status = 2; //0-waiting, 1-finding, 2-finding & update data	
		FindPoint.Items_count = cnt;
		for(i=0;i<cnt;i++) 	FindPoint.Items_Point[i]= buf8[i];
		for(;i<256;i++)			FindPoint.Items_Point[i]= 0;
		FindPoint.task_status = 1; //0-waiting, 1-finding, 2-finding & update data	
	}
	cnt_last = cnt;
	if(cnt > 0) 
	{
		if(audio_stat == 0)
		{
			osMessagePut(UartAudioEvent, UartAudioOK_REPEAT_TX_Event, 0);
			audio_stat = 1;
		}
	}
	else 
	{
		if(audio_stat == 1)
		{
			osMessagePut(UartAudioEvent, UartAudioSTOP_TX_Event, 0);
			audio_stat = 0;
		}
	}
	return 0;
}


/**
  * @brief  WireFindPoint_Process
  * @param  None
  * @retval 0-ok, 1-error, 2-user cancel
  */
uint8_t WireFindPoint_Process(uint16_t PortNumber)
{
	uint8_t res;
	uint32_t i;

	FindPoint.task_status = 1;	//0-waiting, 1-finding
	FindPoint.Items_count = 0;
	for(i=0;i<sizeof(FindPoint.Items_Point);i++) FindPoint.Items_Point[i] = 0;

	while(1)
	{
		res = FindPoint_OneTime(PortNumber);	//0:ok, 1:error, 2-user cancel
		if(res != 0)	//
		{
			//BSP_UartAudioFind_BeepOn(0); //0-off, 1- 1 beep, 2-2beep,3-3beep,255-on
			//BSP_AD_ChangeMode(0);	//0-Normal Mode, 1-FindPoint Mode;
			return res;
		}
		osDelay(100);
	}

}

/**
  * @brief  ���»�·��Ϣ
  * @param  None
  * @retval 0:ok,
  */
uint8_t Update_Loopinfo(void)
{
	uint16_t i;
	uint16_t cnt_line;
	uint32_t total;
	uint32_t index;
	uint16_t PortState[256];

	total = TestFile->item_total;
	index = total;
	//���W�˿�״̬
	for(i=0;i<256;i++)
	{
		PortState[i] = 0;
	}
	while(index)
	{
		index--;
		if(TestFile->test_item[index].type == 'W')
		{
			PortState[TestFile->test_item[index].p1 - 1] += 1;	// cnt
			PortState[TestFile->test_item[index].p2 - 1] += 1;	// cnt
		}
	}
	
	//����W��·����
	cnt_line  = 0;
	
	for(i=0;i<total;i++)
	{
		if(TestFile->test_item[i].type == 'W' )
		{
			if((PortState[TestFile->test_item[i].p1 - 1] == 1) && (PortState[TestFile->test_item[i].p2 - 1] == 1) )
			{
				if(TestFile->test_item[i].result == 1) //0:not test, 1:ok, 2:open circuit error 3:dislocation error 4:Z error
				{
					cnt_line++;
				}
			}
			else if((PortState[TestFile->test_item[i].p1 - 1] > 1) && (PortState[TestFile->test_item[i].p2 - 1] == 1))
			{
				if(TestFile->test_item[i].result == 1) //0:not test, 1:ok, 2:open circuit error 3:dislocation error 4:Z error
				{
					PortState[TestFile->test_item[i].p1 - 1] = PortState[TestFile->test_item[i].p1 - 1] -1;
				}
			}
			else if((PortState[TestFile->test_item[i].p1 - 1] == 1) && (PortState[TestFile->test_item[i].p2 - 1] > 1))
			{
				if(TestFile->test_item[i].result == 1) //0:not test, 1:ok, 2:open circuit error 3:dislocation error 4:Z error
				{
					PortState[TestFile->test_item[i].p2 - 1] = PortState[TestFile->test_item[i].p2 - 1] -1;
				}
			}
			else if((PortState[TestFile->test_item[i].p1 - 1] > 1) && (PortState[TestFile->test_item[i].p2 - 1] > 1))
			{
				if(TestFile->test_item[i].result == 1) //0:not test, 1:ok, 2:open circuit error 3:dislocation error 4:Z error
				{
					PortState[TestFile->test_item[i].p1 - 1] = PortState[TestFile->test_item[i].p1 - 1] -1;
					PortState[TestFile->test_item[i].p2 - 1] = PortState[TestFile->test_item[i].p2 - 1] -1;
				}
			}
			else continue;
		}
	}
	
	TestFile->cnt_okloop = cnt_line;
	
	//���R�˿�״̬
	index = total;
	for(i=0;i<256;i++)
	{
		PortState[i] = 0;
	}
	while(index)
	{
		index--;
		if(TestFile->test_item[index].type == 'R')
		{
			PortState[TestFile->test_item[index].p1 - 1] += 1;	// cnt
			PortState[TestFile->test_item[index].p2 - 1] += 1;	// cnt
		}
	}
	
	//����W��·����
	cnt_line  = 0;
	
	for(i=0;i<total;i++)
	{
		if(TestFile->test_item[i].type == 'R')
		{
			if((PortState[TestFile->test_item[i].p1 - 1] == 1) && (PortState[TestFile->test_item[i].p2 - 1] == 1) )
			{
				if(TestFile->test_item[i].result == 1) //0:not test, 1:ok, 2:open circuit error 3:dislocation error 4:Z error
				{
					cnt_line++;
				}
			}
			else if((PortState[TestFile->test_item[i].p1 - 1] > 1) && (PortState[TestFile->test_item[i].p2 - 1] == 1))
			{
				if(TestFile->test_item[i].result == 1) //0:not test, 1:ok, 2:open circuit error 3:dislocation error 4:Z error
				{
					PortState[TestFile->test_item[i].p1 - 1] = PortState[TestFile->test_item[i].p1 - 1] -1;
				}
			}
			else if((PortState[TestFile->test_item[i].p1 - 1] == 1) && (PortState[TestFile->test_item[i].p2 - 1] > 1))
			{
				if(TestFile->test_item[i].result == 1) //0:not test, 1:ok, 2:open circuit error 3:dislocation error 4:Z error
				{
					PortState[TestFile->test_item[i].p2 - 1] = PortState[TestFile->test_item[i].p2 - 1] -1;
				}
			}
			else if((PortState[TestFile->test_item[i].p1 - 1] > 1) && (PortState[TestFile->test_item[i].p2 - 1] > 1))
			{
				if(TestFile->test_item[i].result == 1) //0:not test, 1:ok, 2:open circuit error 3:dislocation error 4:Z error
				{
					PortState[TestFile->test_item[i].p1 - 1] = PortState[TestFile->test_item[i].p1 - 1] -1;
					PortState[TestFile->test_item[i].p2 - 1] = PortState[TestFile->test_item[i].p2 - 1] -1;
				}
			}
			else continue;
		}
	}
	
	TestFile->cnt_okloop += cnt_line;
	return 0;
}

/**
  * @brief  ���ݲ��Ե�λ�л���ͬ�İ忨
  * @param  portid
  * @retval NULL
  */
void Switch_AD_Input(int portid)
{
    if(portid < 64) //board1
    {
        BROAD_A0_CTL_L;
        BROAD_A1_CTL_L;
    }
    else if( portid >=64 && portid < 127)//board2
    {
        BROAD_A0_CTL_L;
        BROAD_A1_CTL_H;
    }
    else if( portid >=127 && portid < 192)//board3
    {
        BROAD_A0_CTL_H;
        BROAD_A1_CTL_L;
    }
    else//board4
    {
        BROAD_A0_CTL_H;
        BROAD_A1_CTL_H;
    }
}
