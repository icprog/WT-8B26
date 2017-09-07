/********************************************************************************
	Copyright (C), 2016-2026, Suzhou LZY Tech. Co., Ltd.
  File name:      wt_windows_edit.c
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
#include "wt_task_gui.h"
#include "wt_bsp.h" 
#include "tools.h"
#include "k_storage.h"



/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define ID_WINDOW_0     	(GUI_ID_USER + 0x00)
#define ID_TEXT_TITLE     	(GUI_ID_USER + 0x01)
#define ID_TEXT_USB         (GUI_ID_USER + 0x02)
#define ID_TEXT_SD          (GUI_ID_USER + 0x03)
#define ID_LISTBOX_USB      (GUI_ID_USER + 0x04)
#define ID_LISTBOX_SD    	(GUI_ID_USER + 0x05)
#define ID_BUTTON_USB2SD    (GUI_ID_USER + 0x06)
#define ID_BUTTON_SD2USB    (GUI_ID_USER + 0x07)
#define ID_BUTTON_CANCEL    (GUI_ID_USER + 0x08)
#define ID_MULTIPAGE        (GUI_ID_USER + 0x09)
#define ID_LISTBOX_DEL      (GUI_ID_USER + 0x0A)
#define ID_TEXT_FILENUM     (GUI_ID_USER + 0x0B)
#define ID_TEXT_COPYINFO    (GUI_ID_USER + 0x0C)


extern GUI_CONST_STORAGE GUI_BITMAP bm_ICO_edit;  //wujun added
extern WM_HWIN Create_EditDlgWindow(WM_HWIN hWin_para);
extern WM_HWIN CreateMessageBox(WM_HWIN hWin_para);
extern WM_HWIN Create_EditSaveDlgWindow( WM_HWIN hWin_para);
extern WM_HWIN CreateWarningBox(WM_HWIN hWin_para);

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static void Startup(WM_HWIN hWin, uint16_t xpos, uint16_t ypos);

static WM_HWIN hWindow;
static uint8_t win_stat=1;//0--close window  1--not close window
//static uint8_t edit_stat=0;//0-not sutdy  1- study ok
//static uint8_t edit_stat_last=0;//0-not sutdy  1- study ok

static char usbfolder[10][30];
static char sdfolder[10][30];
static uint8_t num_usbdir;
static uint8_t num_sddir;

static uint8_t WT_SDFolder_Init(void);
static uint8_t WT_USBFolder_Init(void);
static uint8_t WT_CopyFile2USB(char *filedir);
static uint8_t WT_CopyFile2SD(char *filedir);

uint8_t mf_dcopy(uint8_t *psrc,uint8_t *pdst,uint8_t fwmode);

K_ModuleItem_Typedef  wt_edit =
{	
	2,
	"�༭",
	&bm_ICO_edit,	
	//&CreateSystemInfo,
	Startup,
	0,
};

static const GUI_WIDGET_CREATE_INFO _aDialog[] = 
{
  
  { WINDOW_CreateIndirect, "Edit", ID_WINDOW_0, 0, 0, 480, 222, 0, 0x0, 0 },
  { MULTIPAGE_CreateIndirect, "Multipage", ID_MULTIPAGE, 0, 0, 480, 222, 0, 0x0, 0 },  
};

/*********************************************************************
*
*       _aDialogFileCopy
*/
static const GUI_WIDGET_CREATE_INFO _aDialogFileCopy[] = {
    { WINDOW_CreateIndirect, "File Copy", 0, 0,   0, 480, 222, FRAMEWIN_CF_MOVEABLE },  
    { TEXT_CreateIndirect, "usb", ID_TEXT_USB, 100, 5, 120, 20, 0, 0x64, 0 },
    { TEXT_CreateIndirect, "sd", ID_TEXT_SD, 340, 5, 120, 20, 0, 0x64, 0 },
    { LISTBOX_CreateIndirect, "Listboxusb", ID_LISTBOX_USB, 10, 30, 200, 145, 0, 0x64, 0 },
    { LISTBOX_CreateIndirect, "Listboxsd", ID_LISTBOX_SD, 260, 30, 200, 145, 0, 0x64, 0 },
    { BUTTON_CreateIndirect, "==>", ID_BUTTON_USB2SD, 220, 80, 30, 20, 0, 0x0, 0 },  
    { BUTTON_CreateIndirect, "<==", ID_BUTTON_SD2USB, 220, 110, 30, 20, 0, 0x0, 0 },  
    { TEXT_CreateIndirect, "copyinfo", ID_TEXT_COPYINFO, 150, 175, 200, 20, 0, 0x64, 0 },  
};

/*********************************************************************
*
*       _aDialogFileDelete
*/
static const GUI_WIDGET_CREATE_INFO _aDialogFileDel[] = {
    { WINDOW_CreateIndirect, "File Delete", 0, 0, 0, 480, 222, FRAMEWIN_CF_MOVEABLE }, 
    { TEXT_CreateIndirect, "File Num", ID_TEXT_FILENUM, 90, 0, 120, 20, 0, 0x64, 0 },
    { LISTBOX_CreateIndirect, "ListboxDel", ID_LISTBOX_DEL, 90, 20, 300, 160, 0, 0x64, 0 },
};




/*********************************************************************
*
*       _cbDialog
*/
static void _cbFileCopyDialog(WM_MESSAGE * pMsg) {
    WM_HWIN      hItem;
		uint8_t      i;
    uint8_t  result = 0;
    char dirname[30];

  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
        hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_TITLE);
        TEXT_SetFont(hItem,&GUI_FontHZ16);
        TEXT_SetText(hItem, "�ļ�����");  
  
        hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_USB);
        TEXT_SetFont(hItem,&GUI_FontHZ16);
        TEXT_SetText(hItem, "U��"); 
				
  
        hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_SD);
        TEXT_SetFont(hItem,&GUI_FontHZ16);
        TEXT_SetText(hItem, "�ڲ��洢"); 
	
        hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_COPYINFO);
        TEXT_SetFont(hItem,&GUI_FontHZ16);
        TEXT_SetText(hItem, " ");  
        WM_HideWindow(hItem);
				
        hItem = WM_GetDialogItem(pMsg->hWin, ID_LISTBOX_USB);
        if(0 == WT_USBFolder_Init())
        {
            for(i=0;i<num_usbdir;i++)
            {
                LISTBOX_AddString(hItem, usbfolder[i]);
            }
            LISTBOX_SetFont(hItem,GUI_FONT_20_1);
            LISTBOX_SetSel(hItem,TestFolder->number_CurrentFile);
        }
        WM_SetFocus(hItem);
	
        hItem = WM_GetDialogItem(pMsg->hWin, ID_LISTBOX_SD);
        if(0 == WT_SDFolder_Init())
        {
            for(i=0;i<num_sddir;i++)
            {
                LISTBOX_AddString(hItem, sdfolder[i]);
            }
            LISTBOX_SetFont(hItem,GUI_FONT_20_1);
            LISTBOX_SetSel(hItem,TestFolder->number_CurrentFile);
        }
  
        hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_USB2SD);
        BUTTON_SetFont(hItem,&GUI_FontHZ12);
        BUTTON_SetSkinClassic(hItem);
        BUTTON_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
        BUTTON_SetBkColor(hItem,BUTTON_CI_UNPRESSED, GUI_GREEN);
				
  
        hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_SD2USB);
        BUTTON_SetFont(hItem,&GUI_FontHZ12);
        BUTTON_SetSkinClassic(hItem);
        BUTTON_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
        BUTTON_SetBkColor(hItem,BUTTON_CI_UNPRESSED, GUI_GREEN);

         
        break;
    case MY_MESSAGE_WHEEL://��������¼�
        hItem = WM_GetFocussedWindow();
 
        if(1 == KeyLed_State.rotate)
        {
            if(hItem == WM_GetDialogItem(pMsg->hWin, ID_LISTBOX_SD))
            {
                LISTBOX_IncSel(WM_GetDialogItem(pMsg->hWin, ID_LISTBOX_SD));
            }
            if(hItem == WM_GetDialogItem(pMsg->hWin, ID_LISTBOX_USB))
            {
                LISTBOX_IncSel(WM_GetDialogItem(pMsg->hWin, ID_LISTBOX_USB));
            }

        }
        else if(2 == KeyLed_State.rotate)
        {
            if(hItem == WM_GetDialogItem(pMsg->hWin, ID_LISTBOX_SD))
            {
                LISTBOX_DecSel(WM_GetDialogItem(pMsg->hWin, ID_LISTBOX_SD));
            }
            if(hItem == WM_GetDialogItem(pMsg->hWin, ID_LISTBOX_USB))
            {
                LISTBOX_DecSel(WM_GetDialogItem(pMsg->hWin, ID_LISTBOX_USB));
            }
        }

        break;
    case MY_MESSAGE_OK:
        if(WM_HasFocus(WM_GetDialogItem(pMsg->hWin, ID_BUTTON_SD2USB)))//���ļ���SD�����Ƶ�U��
        {
            hItem = WM_GetDialogItem(pMsg->hWin, ID_LISTBOX_SD);
            LISTBOX_GetItemText(hItem,LISTBOX_GetSel(hItem),dirname,20);
            WT_CopyFile2USB(dirname);
        }
        if(WM_HasFocus(WM_GetDialogItem(pMsg->hWin, ID_BUTTON_USB2SD)))//���ļ���U�̸��Ƶ�SD��
        {
        
            hItem = WM_GetDialogItem(pMsg->hWin, ID_LISTBOX_USB);
            if(LISTBOX_GetNumItems(hItem) > 0)
            {
                LISTBOX_GetItemText(hItem,LISTBOX_GetSel(hItem),dirname,20);
                hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_COPYINFO);
                WM_ShowWindow(hItem);
                TEXT_SetText(hItem, "���ڸ����ļ�"); 
                TEXT_SetTextColor(hItem, GUI_RED);                
               
                result = WT_CopyFile2SD(dirname);
                if( 0== result)
                {
                    TEXT_SetText(hItem, "�ļ��������");  
                    TEXT_SetTextColor(hItem, GUI_BLUE);
                }
                else
                {
                    TEXT_SetText(hItem, "�ļ����ƴ���");  
                }
            }
        }     
    break;
	case WM_PAINT://���Ʊ߿�

    break;
  default:
    WM_DefaultProc(pMsg);
    break;
  }
}


/*********************************************************************
*
*       _cbDialog
*/
static void _cbFileDelDialog(WM_MESSAGE * pMsg) {
    WM_HWIN      hItem;
	uint8_t      i;
    char fileinfo[30];
    char filename[50];  
    int 	 sel;
    uint8_t  items;
    
    FRESULT res;

  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
        
        hItem = WM_GetDialogItem(pMsg->hWin, ID_LISTBOX_DEL);
        WT_TestFolder_Init();

        for(i=0;i<TestFolder->number_TotalFile;i++)
        {
            LISTBOX_AddString(hItem, (char *)TestFolder->FilesName[i]);
        }
        LISTBOX_SetFont(hItem,GUI_FONT_20_1);
        LISTBOX_SetSel(hItem,0);
        
        hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_FILENUM);
        TEXT_SetFont(hItem,&GUI_FontHZ16);
        sprintf(fileinfo,"�ļ�������%d",TestFolder->number_TotalFile);
        TEXT_SetText(hItem, fileinfo);  
      
    break;
	case MY_MESSAGE_OK:
		GUI_EndDialog(pMsg->hWin,0);
		win_stat=0;//0--close window  1--not close window
		break;
    case MY_MESSAGE_BUTTONDELETE://ɾ���ļ�
        hItem = WM_GetDialogItem(pMsg->hWin, ID_LISTBOX_DEL);
        memset(filename,0,80);
        strcpy(filename,path_testfile_sd);
        strcat(filename,"/");
        sel = LISTBOX_GetSel(hItem);
        if(sel < 0) break;
        strcat(filename,(char *)TestFolder->FilesName[LISTBOX_GetSel(hItem)]);
        res=f_unlink((const TCHAR*)filename);
        if(res == FR_OK)
        {
            WT_TestFolder_Init();
            hItem = WM_GetDialogItem(pMsg->hWin, ID_LISTBOX_DEL);
            items = LISTBOX_GetNumItems(hItem);
            for(i=0;i<items;i++)
            {
                LISTBOX_DeleteItem(hItem,0);
            }
            for(i=0;i<TestFolder->number_TotalFile;i++)
            {
                LISTBOX_AddString(hItem, (char *)TestFolder->FilesName[i]);
            }
            WM_SetFocus(hItem);
            if(sel > 0) LISTBOX_SetSel(hItem,sel - 1);
            else LISTBOX_SetSel(hItem,0);
            
            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_FILENUM);
            TEXT_SetFont(hItem,&GUI_FontHZ16);
            sprintf(fileinfo,"�ļ�������%d",TestFolder->number_TotalFile);
            TEXT_SetText(hItem, fileinfo); 

        }    
    
    
 
        break;
    case MY_MESSAGE_WHEEL://��������¼�
        hItem = WM_GetFocussedWindow();
        if(hItem == WM_GetDialogItem(pMsg->hWin, ID_LISTBOX_DEL))
        {
            if(1 == KeyLed_State.rotate)
            {
				LISTBOX_IncSel(WM_GetDialogItem(pMsg->hWin, ID_LISTBOX_DEL));
            }
            else if(2 == KeyLed_State.rotate)
            {
                LISTBOX_DecSel(WM_GetDialogItem(pMsg->hWin, ID_LISTBOX_DEL));
            }
        }
        break;
	case WM_PAINT://���Ʊ߿�
    break;
  default:
    WM_DefaultProc(pMsg);
    break;
  }
}

/**
  * @brief  Callback routine of the dialog
  * @param  pMsg: pointer to data structure of type WM_MESSAGE
  * @retval None
  */

static void _cbDialog(WM_MESSAGE * pMsg) {
  WM_HWIN hItem, hDialog;
  
  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
    hItem = pMsg->hWin;
    
    hItem = WM_GetDialogItem(pMsg->hWin, ID_MULTIPAGE);
    
	MULTIPAGE_SetFont(hItem,&GUI_FontHZ16);
   /* Create and attache the MULTIPAGE dialog windows */
    hDialog = GUI_CreateDialogBox(_aDialogFileDel, 
                                  GUI_COUNTOF(_aDialogFileDel), 
                                  &_cbFileDelDialog, WM_UNATTACHED, 0, 0);
    MULTIPAGE_AddPage(hItem, hDialog, "�ļ�ɾ��");
  
    hDialog = GUI_CreateDialogBox(_aDialogFileCopy, 
                                  GUI_COUNTOF(_aDialogFileCopy), 
                                  &_cbFileCopyDialog, WM_UNATTACHED, 0, 0);
    MULTIPAGE_AddPage(hItem, hDialog, "�ļ�����");
    
    MULTIPAGE_SelectPage(hItem, 0);  
    
    break;
    case MY_MESSAGE_OK:
		GUI_EndDialog(pMsg->hWin,0);
		win_stat=0;//0--close window  1--not close window
		break;
   
  default:
    WM_DefaultProc(pMsg);
    break;
  }
}


/*********************************************************************
*
*       CreateWindow
*/
static void Startup(WM_HWIN hWin, uint16_t xpos, uint16_t ypos)
{
    WM_HWIN hItem;
	WM_HWIN hWarningbox;
	WM_MESSAGE Msg;
	WM_HWIN hParent;
	uint32_t i;
    uint8_t index=0;
	
	wt_SetText_Title("");
	wt_SetText_Menu(wt_edit.name);
	hWindow = GUI_CreateDialogBox(_aDialog, GUI_COUNTOF(_aDialog), _cbDialog, hWin, xpos, ypos);
    
	while(1)
    {
		BSP_Update_Wheel();	
        i = 0;
		//�ļ�����
		if(BSP_GetKEY_State(KeyOK) == 1 )
		{
			while(BSP_GetKEY_State(KeyOK) == 1)
			{
				osDelay(10);
			}
            
            Msg.MsgId = MY_MESSAGE_OK;
            hItem = WM_GetFocussedWindow();
            hParent = WM_GetParent(hItem);
            WM_SendMessage(hParent,&Msg);
			
			
		}	
		if(BSP_GetKEY_State(KeyUp) == 1 )//�ı�ؼ�����
		{
			while(BSP_GetKEY_State(KeyUp) == 1)
			{
				osDelay(10);
			}
            GUI_SendKeyMsg(GUI_KEY_TAB, 1);
            
            hItem = WM_GetDialogItem(hWin, ID_TEXT_COPYINFO);
            TEXT_SetTextColor(hItem, GUI_BLUE);
            if(WM_HasFocus(WM_GetDialogItem(hWin, ID_BUTTON_USB2SD)))
            {                        
                WM_ShowWindow(hItem);
                TEXT_SetText(hItem, "���ļ���U�̸��Ƶ��ڲ��洢");                                     
            }
            else if(WM_HasFocus(WM_GetDialogItem(hWin, ID_BUTTON_SD2USB)))
            {
                WM_ShowWindow(hItem);
                TEXT_SetText(hItem, "���ļ����ڲ��洢���Ƶ�U��");   
            }
            else
            {
               WM_HideWindow(hItem); 
            }
  
		}
		if(1 == KeyLed_State.rotate || 2 == KeyLed_State.rotate)//��ť��ת��������Ϣ�����洦��
		{

            Msg.MsgId = MY_MESSAGE_WHEEL;
            hItem = WM_GetFocussedWindow();
            hParent = WM_GetParent(hItem);
            WM_SendMessage(hParent,&Msg);

		}

        //����Wheel��ɾ���ļ�
		if(BSP_GetKEY_State(KeyWheel) == 1 )
		{
			//WM_SendMessage(hParent,&Msg);
			while(BSP_GetKEY_State(KeyWheel) == 1)
			{
				i++;
				osDelay(100);
			}
			if(i > 20)
			{
				if(WM_HasFocus(WM_GetDialogItem(hWin, ID_LISTBOX_DEL)))
                {
                    //ɾ���ļ�����������ļ�����
                    Msg.MsgId = MY_MESSAGE_BUTTONDELETE;
                    Msg.hWinSrc = hWindow;
                    hItem=WM_GetFocussedWindow();
                    hParent=WM_GetParent(hItem);
                    WM_SendMessage(hParent,&Msg);
                    
                }
			}
            else//�л���ǩ
            {
                hItem = WM_GetDialogItem(hWin, ID_MULTIPAGE);
                index++;			
                if(index==2)
                {
                    index=0;
                }
                MULTIPAGE_SelectPage(hItem,index);
            }
		}	
		//�رղ��Գ���
		if(BSP_GetKEY_State(KeyCancle) == 1  )
		{
			//�����澯�Ի���
			while(BSP_GetKEY_State(KeyCancle) == 1)
			{
				osDelay(10);
			}
			hWarningbox = CreateWarningBox(hWindow);			
			//WM_BringToTop(hWarningbox);
			//WM_SetFocus(hWarningbox);
			GUI_Exec();
			win_stat=1;
			while(WM_IsWindow(hWarningbox))
			{
				if(BSP_GetKEY_State(KeyOK) == 1 )//ȷ���ر�
				{
					while(BSP_GetKEY_State(KeyOK) == 1)
					{
						osDelay(10);
					}
					
					Msg.MsgId = MY_MESSAGE_OK;
					Msg.hWinSrc = hWindow;
					hItem=WM_GetFocussedWindow();
					//hParent=WM_GetParent(hItem);
					hParent=hWarningbox;

					TestFile->item_total=0;
					WM_SendMessage(hParent,&Msg);
					if(win_stat != 1  )//0--close window  1--not close window
					{						
						Number_Windos = 0;
						return;
					}
					
				}
				if(BSP_GetKEY_State(KeyCancle) == 1 )//ȡ���ر�
				{
					while(BSP_GetKEY_State(KeyCancle) == 1)
					{
						osDelay(10);
					}
					GUI_EndDialog(hWarningbox,0);
					GUI_Exec();
				}
			
				osDelay(10);
			}		
			
			osDelay(30);
		}
		//delay
		osDelay(30); 
		GUI_Exec();
	}
}

/**
  * @brief  ��SD��Ŀ¼
  * @param  None
  * @retval None
	* //0-ok, 1-no files
  */
static uint8_t WT_SDFolder_Init(void)
{
    FRESULT res;
    FILINFO fno;
    DIR dir;
    char *fn;
    uint8_t i=0;
    //	uint8_t buf8;
    num_usbdir = 0;
	
	#if _USE_LFN
    static char lfn[_MAX_LFN];   /* Buffer to store the LFN */
    fno.lfname = lfn;
    fno.lfsize = sizeof lfn;
	#endif
	res = f_opendir(&dir, "1:/LZY_WireTester");  /* Open the directory */
    if (res == FR_OK) 
	{
		while (i < 10)
		{
			res = f_readdir(&dir, &fno);
			
			if (res != FR_OK || fno.fname[0] == 0)
			{
				break;
			}
			#if _USE_LFN
            fn = *fno.lfname ? fno.lfname : fno.fname;
			#else
						fn = fno.fname;
			#endif
			if (fno.fattrib & AM_DIR) /* It is a directory */
			{    
					num_sddir++;
					strcpy(sdfolder[num_sddir - 1],fn);
				  
			}
		}
		return 0;
	}
	else
	{
		return 1;
	}
}


/**
  * @brief  ��U��Ŀ¼
  * @param  None
  * @retval None
	* //0-ok, 1-no files
  */
static uint8_t WT_USBFolder_Init(void)
{
    FRESULT res;
    FILINFO fno;
    DIR dir;
    char *fn;
    uint8_t i=0;
//	uint8_t buf8;
	num_sddir = 0;
	#if _USE_LFN
    static char lfn[_MAX_LFN + 1];   /* Buffer to store the LFN */
    fno.lfname = lfn;
    fno.lfsize = sizeof lfn;
	#endif
	res = f_opendir(&dir, "0:/LZY_WireTester");  /* Open the directory */
    if (res == FR_OK) 
    {
        while (i < 10)
        {
            res = f_readdir(&dir, &fno);
            
            if (res != FR_OK || fno.fname[0] == 0)
            {
                break;
            }
            #if _USE_LFN
            fn = *fno.lfname ? fno.lfname : fno.fname;
            #else
                        fn = fno.fname;
            #endif
            if (fno.fattrib & AM_DIR) /* It is a directory */
            {                
                num_usbdir ++;	
                strcpy(usbfolder[num_usbdir - 1],fn);
                  
            }
        }
        return 0;
    }
    else
    {
        return 1;
    }
}

/**
  * @brief  ��SD���е��ļ����Ƶ�U��
  * @param  None
  * @retval None
	* //0-ok, 1-no files
  */
static uint8_t WT_CopyFile2USB(char *filedir)
{
//	FRESULT res;
//  FILINFO fno;
//  DIR dir;
	uint8_t res;
	char sd_dir[30];
	strcpy(sd_dir, "1:/LZY_WireTester/");
	strcat(sd_dir,filedir);
	res = mf_dcopy((uint8_t *)sd_dir,"0:/",0);
    return res;
}

/**
  * @brief  ��U���е��ļ����Ƶ�SD��
  * @param  None
  * @retval None
	* //0-ok, 1-no files
  */
static uint8_t WT_CopyFile2SD(char *filedir)
{
	uint8_t res;
	char src_dir[30];
    char dst_dir[30];
    char delete_dir[40];
	strcpy(dst_dir, "1:/LZY_WireTester");
    strcpy(src_dir, "0:/LZY_WireTester/");
	strcat(src_dir,filedir);
    strcpy(delete_dir, "1:/LZY_WireTester/");
    strcat(delete_dir,filedir);

    //��ɾ���ļ���
    res = WT_DeleteDir(delete_dir);
    if(res != 0) return 1;
//    else 
//    {
//        TEXT_SetText(hItem, "�ļ���ɾ��OK");  
//        TEXT_SetTextColor(hItem, GUI_BLUE);
//    }
    osDelay(500);
    //�����ļ���
	res = mf_dcopy((uint8_t *)src_dir,(uint8_t *)dst_dir,0);
    if(res == 0 )return 0;
    else return 1;
}

//�ļ�����
//��psrc�ļ�,copy��pdst.
//psrc,pdst:Դ�ļ���Ŀ���ļ�
//fwmode:�ļ�д��ģʽ
//0:������ԭ�е��ļ�
//1:����ԭ�е��ļ�
uint8_t mf_copy(uint8_t *psrc,uint8_t *pdst,uint8_t fwmode)
{
	uint8_t res;
	uint16_t br=0;
	uint16_t bw=0;
	FIL *fsrc=0;
	FIL *fdst=0;
	uint8_t *fbuf=0;
	fsrc=(FIL*)mymalloc(SRAMIN,sizeof(FIL));//�����ڴ�
	fdst=(FIL*)mymalloc(SRAMIN,sizeof(FIL));
	fbuf=(uint8_t*)mymalloc(SRAMIN,512);
	if(fsrc==NULL||fdst==NULL||fbuf==NULL)res=100;//ǰ���ֵ����fatfs
	else
	{   
		if(fwmode==0)fwmode=FA_CREATE_NEW;//������
		else fwmode=FA_CREATE_ALWAYS;   //���Ǵ��ڵ��ļ�

		res=f_open(fsrc,(const TCHAR*)psrc,FA_READ|FA_OPEN_EXISTING); //��ֻ���ļ�
		if(res==0)res=f_open(fdst,(const TCHAR*)pdst,FA_WRITE|fwmode);  //��һ���򿪳ɹ�,�ſ�ʼ�򿪵ڶ���
		if(res==0)//�������򿪳ɹ���
		{
			while(res==0)//��ʼ����
			{
				res=f_read(fsrc,fbuf,512,(UINT*)&br); //Դͷ����512�ֽ�
				if(res||br==0)
                {
                    break;
                }
                osDelay(20);
				res=f_write(fdst,fbuf,(UINT)br,(UINT*)&bw); //д��Ŀ���ļ�
				if(res||bw<br)
                {
                    break;  
                }                    
                      
			}
			f_close(fsrc);
			f_close(fdst);
		}
	}
	myfree(SRAMIN,fsrc);//�ͷ��ڴ�
	myfree(SRAMIN,fdst);
	myfree(SRAMIN,fbuf);
	return res;
}
//�õ�·���µ��ļ���
//����ֵ:0,·�����Ǹ�����.
//    ����,�ļ��������׵�ַ
uint8_t* get_src_dname(uint8_t* dpfn)
{
	uint16_t temp=0;
	while(*dpfn!=0)
	{
		dpfn++;
		temp++; 
	}
	if(temp<4)return 0; 
	while((*dpfn!=0x5c)&&(*dpfn!=0x2f))dpfn--; //׷����������һ��"\"����"/"�� 
	return ++dpfn;
}


//�ļ��и���
//��psrc�ļ���,copy��pdst�ļ���.
//pdst:��������"X:"/"X:XX"/"X:XX/XX"֮���.����Ҫʵ��ȷ����һ���ļ��д���
//psrc,pdst:Դ�ļ��к�Ŀ���ļ���
//fwmode:�ļ�д��ģʽ
//0:������ԭ�е��ļ�
//1:����ԭ�е��ļ�
#define MAX_PATHNAME_DEPTH 512+1 //���Ŀ���ļ�·��+�ļ������
uint8_t mf_dcopy(uint8_t *psrc,uint8_t *pdst,uint8_t fwmode)
{

	uint8_t res=0;   
	DIR *srcdir=0;  //ԴĿ¼
	DIR *dstdir=0;  //ԴĿ¼
	FILINFO *finfo=0; //�ļ���Ϣ
	uint8_t *fn=0;     //���ļ���
	uint8_t * dstpathname=0; //Ŀ���ļ���·��+�ļ���
	uint8_t * srcpathname=0; //Դ�ļ���·��+�ļ���

	uint16_t dstpathlen=0; //Ŀ��·������
	uint16_t srcpathlen=0; //Դ·������
    
    uint8_t filecnt = 0;

	srcdir=(DIR*)mymalloc(SRAMIN,sizeof(DIR));//�����ڴ�
	dstdir=(DIR*)mymalloc(SRAMIN,sizeof(DIR));
	finfo=(FILINFO*)mymalloc(SRAMIN,sizeof(FILINFO));
	if(srcdir==NULL||dstdir==NULL||finfo==NULL)res=100;
	if(res==0)
	{
		finfo->lfsize=_MAX_LFN*2+1;
		finfo->lfname=mymalloc(SRAMIN,finfo->lfsize);//�����ڴ�
		dstpathname=mymalloc(SRAMIN,MAX_PATHNAME_DEPTH);
		srcpathname=mymalloc(SRAMIN,MAX_PATHNAME_DEPTH);
		if(finfo->lfname==NULL||dstpathname==NULL||srcpathname==NULL)res=101;    
		if(res==0)
		{
			dstpathname[0]=0;
			srcpathname[0]=0;
            filecnt = 0;
			strcat((char*)srcpathname,(const char*)psrc);  //����ԭʼԴ�ļ�·�� 
			strcat((char*)dstpathname,(const char*)pdst);  //����ԭʼĿ���ļ�·�� 
			res=f_opendir(srcdir,(const TCHAR*)psrc);   //��ԴĿ¼
			if(res==0)//��Ŀ¼�ɹ� 
			{
				strcat((char*)dstpathname,(const char*)"/");//����б��
				fn=get_src_dname(psrc);
				if(fn==0)//��꿽��
				{
				dstpathlen=strlen((const char*)dstpathname);
				dstpathname[dstpathlen]=psrc[0]; //��¼���
				dstpathname[dstpathlen+1]=0;  //������ 
				}else strcat((char*)dstpathname,(const char*)fn);//���ļ���  
				res=f_mkdir((const TCHAR*)dstpathname);//����ļ����Ѿ�����,�Ͳ�����.��������ھʹ����µ��ļ���.
				if(res==FR_EXIST)res=0;
				while(res==0)//��ʼ�����ļ�������Ķ���
				{
					res=f_readdir(srcdir,finfo);     //��ȡĿ¼�µ�һ���ļ�
					if(res!=FR_OK||finfo->fname[0]==0)break;  //������/��ĩβ��,�˳�
					if(finfo->fname[0]=='.')continue;        //�����ϼ�Ŀ¼
					fn=(uint8_t*)(*finfo->lfname?finfo->lfname:finfo->fname);  //�õ��ļ���
					dstpathlen=strlen((const char*)dstpathname); //�õ���ǰĿ��·���ĳ���
					srcpathlen=strlen((const char*)srcpathname); //�õ�Դ·������
					strcat((char*)srcpathname,(const char*)"/");//Դ·����б��
					if(finfo->fattrib&0X10)//����Ŀ¼   �ļ�����,0X20,�鵵�ļ�;0X10,��Ŀ¼;
					{
//						strcat((char*)srcpathname,(const char*)fn);  //Դ·��������Ŀ¼����
//						printf("\r\ncopy folder %s to %s\r\n",srcpathname,dstpathname);//�����ļ�
//						res=mf_dcopy(srcpathname,dstpathname,fwmode); //�����ļ���
                        continue;//��Ŀ¼������
					}else //��Ŀ¼
					{
						strcat((char*)dstpathname,(const char*)"/");//Ŀ��·����б��
						strcat((char*)dstpathname,(const char*)fn); //Ŀ��·�����ļ���
						strcat((char*)srcpathname,(const char*)fn); //Դ·�����ļ���
						//printf("\r\ncopy file %s to %s\r\n",srcpathname,dstpathname);//�����ļ�
						res = mf_copy(srcpathname,dstpathname,fwmode); //�����ļ�
                        if(res) break;
                        filecnt++;
                    }
                    osDelay(20);
				srcpathname[srcpathlen]=0;//���������
				dstpathname[dstpathlen]=0;//���������     
			} 
		}   
		myfree(SRAMIN,dstpathname);
		myfree(SRAMIN,srcpathname);
		myfree(SRAMIN,finfo->lfname);
		}
	}
	myfree(SRAMIN,srcdir);
	myfree(SRAMIN,dstdir);
	myfree(SRAMIN,finfo);
	return res;   
}
