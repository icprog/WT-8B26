/********************************************************************************
	Copyright (C), 2016-2026, Suzhou LZY Tech. Co., Ltd.
  File name:      wt_windows_selftest.c
  Author: wujun      Version:  2.0      Date: 2016.7.28
  Description:    WT-8A46 main function
                  
  Others:          

	Function List:  
    1. ....
  History:        
    1. Date:
       Author:
       Modification:
    2. ...

******************************************************************************/

#include "DIALOG.h"
#include "wt_bsp.h" 
#include "wt_task_gui.h"
#include "wt_task_wireselfcheck.h"
#include "wt_task_wirestudy.h"
#include "tools.h"
#include "main.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define ID_WINDOW_0     		(GUI_ID_USER + 0x00)
#define ID_BUTTON_OK     		(GUI_ID_USER + 0x01)
#define ID_BUTTON_CANCEL        (GUI_ID_USER + 0x02)
#define ID_LISTVIEW_0     	    (GUI_ID_USER + 0x03)
#define ID_LISTVIEW_1     	    (GUI_ID_USER + 0x04)
#define ID_PROGBAR0				(GUI_ID_USER + 0x05)



extern GUI_CONST_STORAGE GUI_BITMAP bm_ICO_smail;  //wujun added
extern GUI_CONST_STORAGE GUI_BITMAP bmwt_bm_selftest;

extern void update_table_study(void);

//extern uint16_t * get_items_studyindex();
extern void get_items_studyindex(uint16_t items_studyindex[256]);
extern uint32_t get_items_studyshow(void);

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static void Startup(WM_HWIN hWin, uint16_t xpos, uint16_t ypos);
static WM_HWIN hWindows;
static void Listview_Result(void);
static void update_item_study(int16_t *index);
static uint16_t items_study_index[256];

K_ModuleItem_Typedef  wt_self =
{	
	3,
	"�Լ�",
	&bm_ICO_smail,	
	//&CreateSystemInfo,
	Startup,
	0,
};


/*********************************************************************
*
*       _aDialogCreate
*/
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
    { WINDOW_CreateIndirect, "Window", ID_WINDOW_0, 0, 0, 480, 222, 0, 0x0, 0 },
    { LISTVIEW_CreateIndirect, "Listview", ID_LISTVIEW_0, 0, 0, 480, 100, 0, 0x0, 0 },
    { LISTVIEW_CreateIndirect, "Listview2", ID_LISTVIEW_1, 0, 100, 480, 100, 0, 0x0, 0 },
    { BUTTON_CreateIndirect, "��ʼ�Լ�", ID_BUTTON_OK, 320, 200, 80, 20, 0, 0x0, 0 },  
    { BUTTON_CreateIndirect, "�ر�", ID_BUTTON_CANCEL,  400, 200, 80, 20, 0, 0x0, 0 },
    { PROGBAR_CreateIndirect,  0, ID_PROGBAR0, 2,  200, 250, 20, WM_CF_HIDE, 0 },
};


/*********************************************************************
*
*       _cbDialog
*/
static void _cbDialog(WM_MESSAGE * pMsg) {
    WM_HWIN      hItem;
    HEADER_Handle hHeader;
    PROGBAR_Handle hProg;

  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
        hItem = pMsg->hWin;
        WINDOW_SetBkColor(hItem, 0x00FFFFFF);

        hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_OK);
        BUTTON_SetFont(hItem,&GUI_FontHZ12);
        BUTTON_SetSkinClassic(hItem);
        BUTTON_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
        BUTTON_SetBkColor(hItem,BUTTON_CI_UNPRESSED, GUI_GREEN);

        hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_CANCEL);
        BUTTON_SetFont(hItem,&GUI_FontHZ12);
        BUTTON_SetSkinClassic(hItem);
        BUTTON_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
        BUTTON_SetBkColor(hItem,BUTTON_CI_UNPRESSED, GUI_RED);
	
        hItem = WM_GetDialogItem(pMsg->hWin, ID_LISTVIEW_0);
        //WM_SetFocus(hItem);
        //LISTVIEW_SetFont(hItem, &GUI_Fonthzk12);
        hHeader = LISTVIEW_GetHeader(hItem);
        //HEADER_SetTextColor(hHeader, GUI_GREEN);
        HEADER_SetFont(hHeader,&GUI_FontHZ12);
        HEADER_SetHeight(hHeader,20);
        LISTVIEW_AddColumn(hItem, 80, "�Լ���Ŀ", GUI_TA_HCENTER | GUI_TA_VCENTER);
        LISTVIEW_AddColumn(hItem, 80, "���״̬", GUI_TA_HCENTER | GUI_TA_VCENTER);
        LISTVIEW_AddColumn(hItem, 80, "�����", GUI_TA_HCENTER | GUI_TA_VCENTER);
        LISTVIEW_AddColumn(hItem, 80, "�Լ���Ŀ", GUI_TA_HCENTER | GUI_TA_VCENTER);
        LISTVIEW_AddColumn(hItem, 80, "���״̬", GUI_TA_HCENTER | GUI_TA_VCENTER);
        LISTVIEW_AddColumn(hItem, 80, "�����", GUI_TA_HCENTER | GUI_TA_VCENTER);
        //LISTVIEW_SetAutoScrollV(hItem, 1);
        LISTVIEW_SetGridVis(hItem,1);
        LISTVIEW_SetDefaultGridColor(GUI_GRAY);
        LISTVIEW_SetRowHeight(hItem, 20);
        LISTVIEW_EnableSort(hItem);
        LISTVIEW_SetFont(hItem, &GUI_FontHZ16);

        hItem = WM_GetDialogItem(pMsg->hWin, ID_LISTVIEW_1);
        hHeader = LISTVIEW_GetHeader(hItem);
        HEADER_SetFont(hHeader,&GUI_FontHZ12);
        HEADER_SetHeight(hHeader,20);
        LISTVIEW_AddColumn(hItem, 60, "���", GUI_TA_HCENTER | GUI_TA_VCENTER);
        LISTVIEW_AddColumn(hItem, 80, "��������", GUI_TA_HCENTER | GUI_TA_VCENTER);
        LISTVIEW_AddColumn(hItem, 80, "�ڵ�", GUI_TA_HCENTER | GUI_TA_VCENTER);
        LISTVIEW_AddColumn(hItem, 80, "�ڵ�", GUI_TA_HCENTER | GUI_TA_VCENTER);
        LISTVIEW_AddColumn(hItem, 180, "��������", GUI_TA_HCENTER | GUI_TA_VCENTER);
        //LISTVIEW_SetAutoScrollV(hItem, 1);
        LISTVIEW_SetGridVis(hItem,1);
        LISTVIEW_SetDefaultGridColor(GUI_GRAY);
        LISTVIEW_SetRowHeight(hItem, 20);
        LISTVIEW_EnableSort(hItem);
        LISTVIEW_SetFont(hItem, &GUI_FontHZ16);
        WM_HideWindow(hItem);

        hProg = WM_GetDialogItem(pMsg->hWin, ID_PROGBAR0);
        WM_HideWindow(hProg);
    break;
  default:
    WM_DefaultProc(pMsg);
    break;
  }
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

static void Startup(WM_HWIN hWin, uint16_t xpos, uint16_t ypos)
{
    //WM_HWIN hWindow;
    WM_HWIN hItem;
    PROGBAR_Handle hProg;
    uint32_t i;
    int32_t sum=0;
    //unsigned long ulDelay = 0;
    const char * pText[4][6]={
        "SD��","δ���","-","U��","δ���","-",
        "�忨1","δ���","-","�忨2","δ���","-",
        "�忨3","δ���","-","�忨4","δ���","-",
        "����ģ��","δ���","-","ĸ����","δ���","-",
    };
    wt_SetText_Title("");
    wt_SetText_Menu(wt_self.name);
    wt_SetText_Status("�豸�Լ�");
    //hWindow = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
    hWindows = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, hWin, xpos, ypos);
    hItem = WM_GetDialogItem(hWindows, ID_LISTVIEW_0);
    LISTVIEW_SetFont(hItem, &GUI_FontHZ16);
    LISTVIEW_SetGridVis(hItem,1);
    LISTVIEW_SetDefaultGridColor(GUI_GRAY);

	for(i=0;i<4;i++)
	{
		LISTVIEW_AddRow(hItem,pText[i]);
	}
	
	loop1:
	while(1)
  {
		//key detect
		if(BSP_GetKEY_State(KeyCancle) == 1)
		{
				GUI_EndDialog(hWindows,0);
				//GUI_Exec();
				KeyLed_State.wheel=0;
				//wheel_value=0;
				Number_Windos = 0;
				osMessagePut(UartAudioEvent, UartAudioSTOP_TX_Event, 0);	//�ر�����
				LED3_OFF();
				return;
		}
		
		if(BSP_GetKEY_State(KeyOK) == 1 )//ִ���Լ����
		{
			wt_SetText_Status("�Լ���");
			osMessagePut(WireSelfCheckEvent, WIRESELFCHECK_START_EVENT, 0);//��ʼ�Լ�
			i=100;
			while(i--)
			{
				osDelay(10);
				if(SelfCheckItem.state == 0) break;
			}
			
			//��ʾ������
			hProg = WM_GetDialogItem(hWindows, ID_PROGBAR0);
			WM_ShowWindow(hProg);
			PROGBAR_SetValue(hProg, 0);
			sum=add(SelfCheckItem.port_board_number*64);
			if(sum==0) 
			{	
				wt_SetText_Status("�Լ����");
				Listview_Result();
				goto loop1;
			}
			PROGBAR_SetMinMax(hProg, 0, sum);
			//��ʼѧϰ
			osMessagePut(WireStudyEvent, WIRESTUDY_START_EVENT, 0);
			while(1)
			{
				GUI_Delay(1000);
				if(StudyFile->item_index>=sum) break;
				PROGBAR_SetValue(hProg, StudyFile->item_index);
			}

			if(StudyFile->command!=2)
			{
				//��ʼ��ѧϰ�Ż�
				osMessagePut(WireStudyEvent, WIRESTUDY_OPTIMIZE_EVENT, 0);
				osDelay(500);
				for(;;)
				{
					if(StudyFile->task_status == 0)
						break;
				}
			}

			if(StudyFile->number_point > 0) SelfCheckItem.board_state=2;
			else SelfCheckItem.board_state=1;
			WM_HideWindow(hProg);
			GUI_Exec();
			wt_SetText_Status("�Լ����");
			Listview_Result();

		}

		//delay
		osDelay(30); 
		GUI_Exec();
}
}


//��ʾ���Խ��
static void Listview_Result(void)
{
	uint32_t i;
	uint32_t num_rows;
	WM_HWIN hItem;
	const char * pText[2][6];

	hItem = WM_GetDialogItem(hWindows, ID_LISTVIEW_0);
	LISTVIEW_SetFont(hItem, &GUI_FontHZ16);
	LISTVIEW_SetGridVis(hItem,1);
	LISTVIEW_SetDefaultGridColor(GUI_GRAY);
	//���������
	num_rows=LISTVIEW_GetNumRows(hItem);
	for(i=0;i<num_rows;i++)
	{
		LISTVIEW_DeleteRow(hItem,0);
	}
	//��ʾ������Ŀ
	// sd card state
	if(SelfCheckItem.sd_card==0)//sd��״̬��0:not init, 1:ok, 2:error, 3:not connect
	{
		pText[0][0]=(char *)"SD��";
		pText[0][1]=(char *)"�Ѽ��";
		pText[0][2]=(char *)"δ��ʼ��";
		//LISTVIEW_AddRow(hItem,pText[0]);
	}
	else if(SelfCheckItem.sd_card==1)
	{
		pText[0][0]=(char *)"SD��";
		pText[0][1]=(char *)"�Ѽ��";
		pText[0][2]=(char *)"����";
		//LISTVIEW_AddRow(hItem,pText[0]);
	}
	else if(SelfCheckItem.sd_card==2)
	{
		pText[0][0]=(char *)"SD��";
		pText[0][1]=(char *)"�Ѽ��";
		pText[0][2]=(char *)"������";
		//LISTVIEW_AddRow(hItem,pText[0]);
	}
	else //if(SelfCheckItem.sd_card==3)
	{
		pText[0][0]=(char *)"SD��";
		pText[0][1]=(char *)"�Ѽ��";
		pText[0][2]=(char *)"δ����";
		//LISTVIEW_AddRow(hItem,pText[0]);
	}
	
	// usb state
	if(SelfCheckItem.usb_disc==0)	//u ��״̬,0:not init, 1:ok, 2:error, 3:not connect
	{
		pText[0][3]=(char *)"U��";
		pText[0][4]=(char *)"�Ѽ��";
		pText[0][5]=(char *)"δ��ʼ��";
		LISTVIEW_AddRow(hItem,pText[0]);
	}
	else if(SelfCheckItem.usb_disc==1)
	{
		pText[0][3]=(char *)"U��";
		pText[0][4]=(char *)"�Ѽ��";
		pText[0][5]=(char *)"����";
		LISTVIEW_AddRow(hItem,pText[0]);
	}
	else	if(SelfCheckItem.usb_disc==2)
	{
		pText[0][3]= (char *)"U��";
		pText[0][4]= (char *)"�Ѽ��";
		pText[0][5]= (char *)"����";
		LISTVIEW_AddRow(hItem,pText[0]);
	}
	else //if(SelfCheckItem.usb_disc==3)
	{
		pText[0][3]=(char *)"U��";
		pText[0][4]=(char *)"�Ѽ��";
		pText[0][5]=(char *)"δ����";
		LISTVIEW_AddRow(hItem,pText[0]);
	}
	
	//��չ��״̬(4��忨����3���ֽ�)
	//byte0, 	0:not init, 1:ok, 2:error, 3:not connect
	//byte1, 	FW Vision;
	//byte2,  Voltage 15V * 10
	//board - 1
	if(SelfCheckItem.port_board[0][0]==0)
	{
		pText[0][0]=(char *)"�忨1";
		pText[0][1]=(char *)"�Ѽ��";
		pText[0][2]=(char *)"δ��ʼ��";
		//LISTVIEW_AddRow(hItem,pText[0]);
	}
	else if(SelfCheckItem.port_board[0][0]==1)
	{
		pText[0][0]=(char *)"�忨1";
		pText[0][1]=(char *)"�Ѽ��";
		pText[0][2]=(char *)"����";
		//LISTVIEW_AddRow(hItem,pText[0]);
	}
	else if(SelfCheckItem.port_board[0][0]==2)
	{
		pText[0][0]=(char *)"�忨1";
		pText[0][1]=(char *)"�Ѽ��";
		pText[0][2]=(char *)"����";
		//LISTVIEW_AddRow(hItem,pText[0]);
	}
	else //if(SelfCheckItem.port_board[0][0]==3)
	{
		pText[0][0]=(char *)"�忨1";
		pText[0][1]=(char *)"�Ѽ��";
		pText[0][2]=(char *)"δ����";
		//LISTVIEW_AddRow(hItem,pText[0]);
	}
	//board - 2
	if(SelfCheckItem.port_board[1][0]==0)
	{
		pText[0][3]=(char *)"�忨2";
		pText[0][4]=(char *)"�Ѽ��";
		pText[0][5]=(char *)"δ��ʼ��";
		LISTVIEW_AddRow(hItem,pText[0]);
	}
	else if(SelfCheckItem.port_board[1][0]==1)
	{
		pText[0][3]=(char *)"�忨2";
		pText[0][4]=(char *)"�Ѽ��";
		pText[0][5]=(char *)"����";
		LISTVIEW_AddRow(hItem,pText[0]);
	}
	else if(SelfCheckItem.port_board[1][0]==2)
	{
		pText[0][3]=(char *)"�忨2";
		pText[0][4]=(char *)"�Ѽ��";
		pText[0][5]=(char *)"����";
		LISTVIEW_AddRow(hItem,pText[0]);
	}
	else //if(SelfCheckItem.port_board[1][0]==3)
	{
		pText[0][3]=(char *)"�忨2";
		pText[0][4]=(char *)"�Ѽ��";
		pText[0][5]=(char *)"δ����";
		LISTVIEW_AddRow(hItem,pText[0]);
	}
	//board - 3
	if(SelfCheckItem.port_board[2][0]==0)
	{
		pText[0][0]=(char *)"�忨3";
		pText[0][1]=(char *)"�Ѽ��";
		pText[0][2]=(char *)"δ��ʼ��";
		//LISTVIEW_AddRow(hItem,pText[0]);
	}
	else if(SelfCheckItem.port_board[2][0]==1)
	{
		pText[0][0]=(char *)"�忨3";
		pText[0][1]=(char *)"�Ѽ��";
		pText[0][2]=(char *)"����";
		//LISTVIEW_AddRow(hItem,pText[0]);
	}
	else if(SelfCheckItem.port_board[2][0]==2)
	{
		pText[0][0]=(char *)"�忨3";
		pText[0][1]=(char *)"�Ѽ��";
		pText[0][2]=(char *)"����";
		//LISTVIEW_AddRow(hItem,pText[0]);
	}
	else //if(SelfCheckItem.port_board[2][0]==3)
	{
		pText[0][0]=(char *)"�忨3";
		pText[0][1]=(char *)"�Ѽ��";
		pText[0][2]=(char *)"δ����";
		//LISTVIEW_AddRow(hItem,pText[0]);
	}
	//board - 4
	if(SelfCheckItem.port_board[3][0]==0)
	{
		pText[0][3]=(char *)"�忨4";
		pText[0][4]=(char *)"�Ѽ��";
		pText[0][5]=(char *)"δ��ʼ��";
		LISTVIEW_AddRow(hItem,pText[0]);
	}
	else if(SelfCheckItem.port_board[3][0]==1)
	{
		pText[0][3]=(char *)"�忨4";
		pText[0][4]=(char *)"�Ѽ��";
		pText[0][5]=(char *)"����";
		LISTVIEW_AddRow(hItem,pText[0]);
	}
	else if(SelfCheckItem.port_board[3][0]==2)
	{
		pText[0][3]=(char *)"�忨4";
		pText[0][4]=(char *)"�Ѽ��";
		pText[0][5]=(char *)"����";
		LISTVIEW_AddRow(hItem,pText[0]);
	}
	else //if(SelfCheckItem.port_board[3][0]==3)
	{
		pText[0][3]=(char *)"�忨4";
		pText[0][4]=(char *)"�Ѽ��";
		pText[0][5]=(char *)"δ����";
		LISTVIEW_AddRow(hItem,pText[0]);
	}

	// ����ģ��	
	if(SelfCheckItem.wireless==0)//����ģ��״̬��0:not init, 1:ok, 2:error,
	{
		pText[0][0]=(char *)"����ģ��";
		pText[0][1]=(char *)"�Ѽ��";
		pText[0][2]=(char *)"δ��ʼ��";
		//LISTVIEW_AddRow(hItem,pText[0]);
	}
	else if(SelfCheckItem.wireless==1)//����ģ��״̬��0:not init, 1:ok, 2:error,
	{
		pText[0][0]=(char *)"����ģ��";
		pText[0][1]=(char *)"�Ѽ��";
		pText[0][2]=(char *)"����";
		//LISTVIEW_AddRow(hItem,pText[0]);
	}
	else if(SelfCheckItem.wireless==2)//����ģ��״̬��0:not init, 1:ok, 2:error,
	{
		pText[0][0]=(char *)"����ģ��";
		pText[0][1]=(char *)"�Ѽ��";
		pText[0][2]=(char *)"����";
		//LISTVIEW_AddRow(hItem,pText[0]);
	}
	
	// ĸ����	
	if(SelfCheckItem.board_state==0)//ĸ��״̬��0:not init, 1:ok, 2:error,
	{
		pText[0][3]=(char *)"ĸ����";
		pText[0][4]=(char *)"�Ѽ��";
		pText[0][5]=(char *)"δ��ʼ��";
		LISTVIEW_AddRow(hItem,pText[0]);
		LED3_OFF();
	}
	else if(SelfCheckItem.board_state==1)//ĸ��״̬��0:not init, 1:ok, 2:error,
	{
		pText[0][3]=(char *)"ĸ����";
		pText[0][4]=(char *)"�Ѽ��";
		pText[0][5]=(char *)"����";
		LISTVIEW_AddRow(hItem,pText[0]);
		osMessagePut(UartAudioEvent, UartAudioOK_TX_Event, 0);	//����OK����
		LED3_OFF();
		hItem = WM_GetDialogItem(hWindows, ID_LISTVIEW_1);
		WM_HideWindow(hItem);
	}
	else if(SelfCheckItem.board_state==2)//ĸ��״̬��0:not init, 1:ok, 2:error,
	{
		pText[0][3]=(char *)"ĸ����";
		pText[0][4]=(char *)"�Ѽ��";
		pText[0][5]=(char *)"����";
		int16_t  index_item = 0;
		LISTVIEW_AddRow(hItem,pText[0]);
		LISTVIEW_SetItemTextColor(hItem,5,3,LISTVIEW_CI_UNSEL,GUI_RED);
		//BSP_UartAudioNG_BeepOn(1);
		osMessagePut(UartAudioEvent, UartAudioNG_TX_Event, 0);//����NG����
		LED3_ON_R();
		hItem = WM_GetDialogItem(hWindows, ID_LISTVIEW_1);
		
		WM_ShowWindow(hItem);
		WM_SetFocus(hItem);
		update_table_study();
		update_item_study(&index_item);
	}
}

//����WRDC��ʾ
static void update_item_study(int16_t *index)
{
	WM_HWIN  hItem;
	uint8_t  buf8;
	uint16_t buf16;
	uint16_t i;
	uint32_t items_study_show;
	static uint32_t items_study_show_last=0;

	const char * pText[2][5];
	char id[5];
	char p1[5];
	char p2[5];
	char param[20];
	items_study_show = get_items_studyshow();
	get_items_studyindex(items_study_index);
	
	hItem = WM_GetDialogItem(hWindows, ID_LISTVIEW_1);
	if(items_study_show <= 0) 
	{
		*index=0;
		//delete items
		while(1)
		{
			buf8 = LISTVIEW_GetNumRows(hItem);
			if(buf8 != 0) LISTVIEW_DeleteRow(hItem,0);
			else break;
		}
		return;
	}
	// check index
	if(*index < 0) *index=0;
	
	else if(*index >= items_study_show) *index = items_study_show-1;
	
	
	//���������ؼ�
	if( items_study_show_last != items_study_show)
	{
		
		items_study_show_last = items_study_show;
		while(1)
		{
			buf8 = LISTVIEW_GetNumRows(hItem);
			if(buf8 != 0) LISTVIEW_DeleteRow(hItem,0);
			else break;
		}
		
		if(items_study_show > 9) 	buf8 = 9;
		else								buf8 = items_study_show;
		for(i=0;i<buf8;i++)
		{
			buf16 = i;
			itoa(buf16+1,id,10);
			pText[0][0]=id;

			//itoa(StudyFile->study_item[items_study_index[buf16]].p1,p1,10);
			num_converter(StudyFile->study_item[items_study_index[buf16]].p1,p1);
			pText[0][2]=p1;
			//itoa(StudyFile->study_item[items_study_index[buf16]].p2,p2,10);
			num_converter(StudyFile->study_item[items_study_index[buf16]].p2,p2);
			pText[0][3]=p2;

			if(StudyFile->study_item[items_study_index[buf16]].type=='W')
			{
				pText[0][1]="��ͨ";
				sprintf(param,"��·");
			}
			else if(StudyFile->study_item[items_study_index[buf16]].type=='D')
			{
				pText[0][1]="������";
				sprintf(param,"V=%d",StudyFile->study_item[items_study_index[buf16]].param2);
			}
			else if(StudyFile->study_item[items_study_index[buf16]].type=='C')
			{
				pText[0][1]="����";
				sprintf(param,"C=%d",StudyFile->study_item[items_study_index[buf16]].param2);
			}
			else if(StudyFile->study_item[items_study_index[buf16]].type=='R')
			{
				pText[0][1]="����";
				sprintf(param,"R=%d",StudyFile->study_item[items_study_index[buf16]].param2);
			}
			else if(StudyFile->study_item[items_study_index[buf16]].type=='Z')
			{
				pText[0][1]="��·";
				sprintf(param,"Z=%d",StudyFile->study_item[items_study_index[buf16]].param2);
			}
			else
			{
				pText[0][1]="����";
				sprintf(param,"K=%d",StudyFile->study_item[items_study_index[buf16]].param2);
			}
			pText[0][4]=param;
			
			LISTVIEW_AddRow(hItem,pText[0]);
		}
		LISTVIEW_SetSel(hItem,0);
	}
	items_study_show_last=0;
}
