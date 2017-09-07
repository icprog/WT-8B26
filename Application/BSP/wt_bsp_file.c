/**
  ******************************************************************************
  * @progect LZY Wire Cube Tester
	* @file    wt_bsp_file.c
  * @author  LZY Zhang Manxin
  * @version V1.0.0
  * @date    2014-07-18
  * @brief   This file provides the E2PROM functions
  ******************************************************************************
  */

#define WT_BSP_FILE_GLOBALS

/* Includes ------------------------------------------------------------------*/
#include "wt_bsp_file.h"
#include "k_rtc.h"
#include "wt_bsp.h"
#include "k_mem.h"
#include "k_storage.h"
#include "tools.h"

extern osMessageQId StorageEvent;

//FRESULT f_open (FIL* fp, const TCHAR* path, BYTE mode);				/* Open or create a file */
//FRESULT f_close (FIL* fp);											/* Close an open file object */
//FRESULT f_read (FIL* fp, void* buff, UINT btr, UINT* br);			/* Read data from a file */
//FRESULT f_write (FIL* fp, const void* buff, UINT btw, UINT* bw);	/* Write data to a file */
//FRESULT f_forward (FIL* fp, UINT(*func)(const BYTE*,UINT), UINT btf, UINT* bf);	/* Forward data to the stream */
//FRESULT f_lseek (FIL* fp, DWORD ofs);								/* Move file pointer of a file object */
//FRESULT f_truncate (FIL* fp);										/* Truncate file */
//FRESULT f_sync (FIL* fp);											/* Flush cached data of a writing file */
//FRESULT f_opendir (DIR* dp, const TCHAR* path);						/* Open a directory */
//FRESULT f_closedir (DIR* dp);										/* Close an open directory */
//FRESULT f_readdir (DIR* dp, FILINFO* fno);							/* Read a directory item */
//FRESULT f_mkdir (const TCHAR* path);								/* Create a sub directory */
//FRESULT f_unlink (const TCHAR* path);								/* Delete an existing file or directory */
//FRESULT f_rename (const TCHAR* path_old, const TCHAR* path_new);	/* Rename/Move a file or directory */
//FRESULT f_stat (const TCHAR* path, FILINFO* fno);					/* Get file status */
//FRESULT f_chmod (const TCHAR* path, BYTE value, BYTE mask);			/* Change attribute of the file/dir */
//FRESULT f_utime (const TCHAR* path, const FILINFO* fno);			/* Change times-tamp of the file/dir */
//FRESULT f_chdir (const TCHAR* path);								/* Change current directory */
//FRESULT f_chdrive (const TCHAR* path);								/* Change current drive */
//FRESULT f_getcwd (TCHAR* buff, UINT len);							/* Get current directory */
//FRESULT f_getfree (const TCHAR* path, DWORD* nclst, FATFS** fatfs);	/* Get number of free clusters on the drive */
//FRESULT f_getlabel (const TCHAR* path, TCHAR* label, DWORD* sn);	/* Get volume label */
//FRESULT f_setlabel (const TCHAR* label);							/* Set volume label */
//FRESULT f_mount (FATFS* fs, const TCHAR* path, BYTE opt);			/* Mount/Unmount a logical drive */
//FRESULT f_mkfs (const TCHAR* path, BYTE sfd, UINT au);				/* Create a file system on the volume */
//FRESULT f_fdisk (BYTE pdrv, const DWORD szt[], void* work);			/* Divide a physical drive into some partitions */
//int f_putc (TCHAR c, FIL* fp);										/* Put a character to the file */
//int f_puts (const TCHAR* str, FIL* cp);								/* Put a string to the file */
//int f_printf (FIL* fp, const TCHAR* str, ...);						/* Put a formatted string to the file */
//TCHAR* f_gets (TCHAR* buff, int len, FIL* fp);						/* Get a string from the file */


extern char str_mode[20];

uint8_t WT_TestItem_ReadAscii(char * filepath,char * filename);

#define IO_MAXSIZE  200

/**
  * @brief  ��ʼ�������ļ�
  * @param  None
  * @retval None
	* //0-not init, 1-inited, 2-no files, 3-hardware error, 4-no folder
  */
void WT_TestFolder_Init(void)
{
	FRESULT res;
  FILINFO fno;
  DIR dir;
  char *fn,*p;
  uint8_t i=0;
	uint8_t buf8;

	// Init
	TestFolder->number_TotalFile = 0;
	TestFolder->number_CurrentFile = 0;
	TestFolder->status = 0;	
	
	// Check USB
	if(k_StorageGetStatus(USB_DISK_UNIT) == 0 && store_dev == 0 )	//no usb
	//if(k_StorageGetStatus(MSD_DISK_UNIT) == 0)	//no sd card 
	{
		TestFolder->status = 3;	//0-not init, 1-inited, 2-no files, 3-hardware error, 4-no folder
		return;
	}
	// Check SD
	if(k_StorageGetStatus(MSD_DISK_UNIT) == 0 && store_dev == 1 )	//no sd
	//if(k_StorageGetStatus(MSD_DISK_UNIT) == 0)	//no sd card 
	{
		TestFolder->status = 3;	//0-not init, 1-inited, 2-no files, 3-hardware error, 4-no folder
		return;
	}
	
#if _USE_LFN
  static char lfn[_MAX_LFN];
  fno.lfname = lfn;
  fno.lfsize = sizeof(lfn);
#endif
	
	if(store_dev==0) //usb
  res = f_opendir(&dir, path_testfile);
	if(store_dev==1) //sd
	res = f_opendir(&dir, path_testfile_sd);
	//res = f_opendir(&dir, "1:/LZY_WireTester/Test Files");
  if (res == FR_OK)
  {
    
    while (i < WT_Number_TestFiles_MAX)
    {
      res = f_readdir(&dir, &fno);
      
      if (res != FR_OK || fno.fname[0] == 0)
      {
        break;
      }
      if (fno.fname[0] == '.')
      {
        continue;
      }
			
#if _USE_LFN
      fn = *fno.lfname ? fno.lfname : fno.fname;
#else
      fn = fno.fname;
#endif
			p = fn;
			while(*p) p++;
			buf8 = p - fn;
			if(buf8 > WT_FILE_NAME_SIZE+4) continue;
			if((fn[buf8-4] != '.') || (fn[buf8-3] != 'w') || (fn[buf8-2] != 't') || (fn[buf8-1] != 'r'))
			{
				continue;
			}
			strncpy((char *)TestFolder->FilesName[i], (char *)fn, buf8+1);
			i++;
		}
		
		if(i==0)	TestFolder->status = 2;	//0-not init, 1-inited, 2-no files, 3-hardware error, 4-no folder
		else			
		{
			TestFolder->status = 1;	//0-not init, 1-inited, 2-no files, 3-hardware error, 4-no folder
			TestFolder->number_TotalFile = i;
		}
	}
	else
	{
		TestFolder->status = 4;	//0-not init, 1-inited, 2-no files, 3-hardware error, 4-no folder
	}
}

/**
  * @brief  �������ʼ��
  * @param  None
* @retval 0:ok, 1:error
  */
uint8_t WT_StoreFiles_Init(void)
{
	uint8_t res=1;
	
	//read folder
	WT_TestFolder_Init();	
	if(TestFolder->status == 1)	//0-not init, 1-inited, 2-no files, 3-error
	{
//		WT_TestItem_Init();
		if(TestFile->file_status == 1) res=0;//0:not init, 1: init ok, 2: hardware error, 3-no this file
	}
	return res;
}

/**
  * @brief  ��ȡ�����ļ�
  * @param  None
  * @retval None
  */
void WT_StoreFiles_Read(void)
{
	//uint8_t res=1;
	
	FRESULT res;
  FILINFO fno;
  DIR dir;
  char *fn;
  uint8_t i=0;
	
	// Check USB
	if(k_StorageGetStatus(USB_DISK_UNIT) == 0 && store_dev == 0)	//no usb
	//if(k_StorageGetStatus(MSD_DISK_UNIT) == 0)	//no sd card 
	{
		TestFolder->status = 3;	//0-not init, 1-inited, 2-no files, 3-hardware error, 4-no folder
		return;
	}
	
	if(k_StorageGetStatus(MSD_DISK_UNIT) == 0 && store_dev == 1)	//no usb
	//if(k_StorageGetStatus(MSD_DISK_UNIT) == 0)	//no sd card 
	{
		TestFolder->status = 3;	//0-not init, 1-inited, 2-no files, 3-hardware error, 4-no folder
		return;
	}
	
#if _USE_LFN
  static char lfn[_MAX_LFN];
  fno.lfname = lfn;
  fno.lfsize = sizeof(lfn);
#endif
	
  //res = f_opendir(&dir, path_testfile);
	if(store_dev == 0) res = f_opendir(&dir, path_testfile);
	if(store_dev == 1) res = f_opendir(&dir, path_testfile_sd);
	//res = f_opendir(&dir, "1:/LZY_WireTester/Test Files");
  if (res == FR_OK)
  {
    
    while (i < WT_Number_TestFiles_MAX)
    {
      res = f_readdir(&dir, &fno);
      
      if (res != FR_OK || fno.fname[0] == 0)
      {
        break;
      }
      if (fno.fname[0] == '.')
      {
        continue;
      }
			
#if _USE_LFN
      fn = *fno.lfname ? fno.lfname : fno.fname;
#else
      fn = fno.fname;
#endif
			
			strncpy((char *)TestFolder->FilesName[i], (char *)fn, WT_FILE_NAME_SIZE);
			i++;
		}
		
		if(i==0)	TestFolder->status = 2;	//0-not init, 1-inited, 2-no files, 3-hardware error, 4-no folder
		else			
		{
			TestFolder->status = 1;	//0-not init, 1-inited, 2-no files, 3-hardware error, 4-no folder
			TestFolder->number_TotalFile = i;
		}
	}
	else
	{
		TestFolder->status = 4;	//0-not init, 1-inited, 2-no files, 3-hardware error, 4-no folder
	}
}

/**
  * @brief  ��ѧϰ��ʼ��
  * @param  None
* @retval 0:ok, 1:error
  */
uint8_t WT_StudyFiles_Init(void)
{
	uint8_t res=1;
	
	//read folder
	WT_TestFolder_Init();	
	if(TestFolder->status == 1)	//0-not init, 1-inited, 2-no files, 3-error
	{
		if(TestFile->file_status == 1) res=0;//0:not init, 1: init ok, 2: hardware error, 3-no this file
	}
	return res;
}


/**
  * @brief  ���������־�ļ�
  * @param  None
  * @retval 0-ok, 1-hardware error, 2-file operate error
  */
uint8_t WT_LogFiles_Write(uint8_t * path, WT_LogFileTypedef logInfo)
{
	FRESULT  res;
	FIL			 fil;
	uint8_t* p;
	uint8_t  i;
	uint8_t  buf8[100];
	uint32_t buf32;
	char strbuf[6];
	char date[11];
	char time[11];
	uint16_t size=0;
	char short_time[6];
	uint8_t month,day;
	uint16_t year = 0;
	
	// Check disc
	if(k_StorageGetStatus(USB_DISK_UNIT) == 0 && store_dev == 0)	//no usb or SD
	{
		return 1;
	}
	
	if(k_StorageGetStatus(MSD_DISK_UNIT) == 0 && store_dev == 1)	//no usb or SD
	{
		return 1;
	}
	
	// open file
	res = f_open(&fil, (const TCHAR*)path, FA_OPEN_EXISTING | FA_WRITE);
	if((res == FR_NO_FILE)||(res == FR_NO_PATH)) //�ļ�������
	{
		p = path;
		for(;(*p)!=0;p++)
		{
			if(*p == '/')
			{
				for(i=0;i<(p-path);i++) buf8[i]=*(path+i);
				buf8[i] = 0;
				res = f_mkdir((const TCHAR*) buf8);
				if((res != 0)&&(res != FR_EXIST)) {return 2;}
			}
		}
		
		res = f_open(&fil, (const TCHAR*)path, FA_OPEN_ALWAYS | FA_WRITE); //��д��ʽ�� û���ļ��򴴽� 
		if(res)
		{
			f_close(&fil);
		}
	}
	else if(res != FR_OK)	//ok
	{
		return 1;
	}
	
	memset(buf8,0,100);
	memset(short_time,0,6);
	get_system_time(date ,time);
	get_system_date(&day,&month,&year);
	sprintf(date,"%d-%d",month,day);
	strncpy(short_time,time,5);
	//Write Log to file
	strcpy ((char *)buf8, "@1/");
	strncpy ((char *)buf8+3, (char *)logInfo.wireMode,strlen((char *)logInfo.wireMode)-4);
	strcat ((char *)buf8, "/");
	itoa(logInfo.test_total,strbuf,10);
	strcat ((char *)buf8,strbuf );
	strcat ((char *)buf8, "/");
	itoa(logInfo.ok_num,strbuf,10);
	strcat ((char *)buf8,strbuf );
	strcat ((char *)buf8, "/");
	itoa(logInfo.err_num,strbuf,10);
	strcat ((char *)buf8,strbuf );
	strcat ((char *)buf8, "/");
	itoa(logInfo.err_type,strbuf,10);
	strcat ((char *)buf8,strbuf );
	strcat ((char *)buf8, "/");
	strcat ((char *)buf8, date);
	strcat ((char *)buf8, " ");
	strcat ((char *)buf8, short_time);
//	strcat ((char *)buf8,"\r\n");
	
	size = f_size(&fil);
	if(size > 500)
	{
		f_lseek(&fil,0);
		if(res == FR_OK)
		{
			res = f_write(&fil, buf8, strlen((char *)buf8), &buf32); 
		}
	}
	else
	{
		res = f_lseek(&fil, f_size(&fil));
		if(res == FR_OK)
		{
			res = f_write(&fil, buf8, strlen((char *)buf8), &buf32); 
		}
	}
	
	//Save to file
	f_close(&fil);
	return 0;
}

/**
  * @brief  ��ȡAscii��ʽ��־�ļ�
  * @param  
  * @retval 0-ok 1-error
  */
uint8_t WT_LogFiles_Read(WT_LogFileTypedef logInfo[],uint8_t *num)
{
	FRESULT res;
	FIL file;
	char line[100];
	uint32_t i=0,index=0;
	char *ptr;
	char *delim="@/";	
	//WT_LogFileTypedef logInfo[30];

		res = f_open(&file, path_Logfilename_sd, FA_OPEN_EXISTING | FA_READ);
		if (res == FR_OK)
		{		
			while(f_gets(line, sizeof line, &file))
			{
				if(strstr(line, "@"))
				{	
					strtok((line),delim);
					index=0;
					while(1)
					{
						ptr=strtok(NULL,delim);
						if(ptr == 0) break;
						
						switch(index)
						{
							case 0:
								strcpy((char *)logInfo[i].wireMode,ptr);
								index++;
								break;
							case 1:
								logInfo[i].test_total = atoi(ptr);
								index++;
								break;
							case 2:
								logInfo[i].ok_num = atoi(ptr);
								index++;
								break;
							case 3:
								logInfo[i].err_num = atoi(ptr);
								index++;
								break;
							case 4:
								logInfo[i].err_type = atoi(ptr);
								index++;
								break;
							case 5:
								strcpy((char *)logInfo[i].test_time,ptr);
								index++;
								break;
							default:
								break;
						}
					}
					i++;
				}			
			}				
		}
		*num = i;
		f_close(&file);
		return 0;
	}

	
/**
  * @brief  �����־�ļ�
  * @param  
  * @retval 0-ok 1-error
  */
uint8_t WT_LogFiles_Clear(void)
{
	FRESULT res;
	//FIL file;
	res = f_unlink("1:/LZY_WireTester/LogFiles/log.txt");
	if(res == FR_OK)
	{
		return 1;
	}
	else
	{
		return 0;
	}
	
}	

///**
//  * @brief  ɾ���ļ���
//  * @param  
//  * @retval 0-ok 1-error
//  */
//uint8_t WT_DeleteDir(char * dir)
//{
//	FRESULT res;
//	//FIL file;
//	res = f_unlink(dir);
//	if(res == FR_OK)
//	{
//		return 0;
//	}
//	else
//	{
//		return 1;
//	}
//	
//}

//====================================================================================================  
//�� �� �� : f_deldir  
//�������� : �Ƴ�һ���ļ��У������䱾��������ļ��У����ļ�  
//��    �� : const TCHAR *path---ָ��Ҫ�Ƴ��Ŀս�β�ַ��������ָ��  
//��    �� : ��  
//�� �� ֵ : FR_OK(0)��           �����ɹ�   
//           FR_NO_FILE��         �޷��ҵ��ļ���Ŀ¼   
//           FR_NO_PATH��         �޷��ҵ�·��   
//           FR_INVALID_NAME��    �ļ����Ƿ�   
//           FR_INVALID_DRIVE��   �������ŷǷ�   
//           FR_DENIED��          ������������ԭ�򱻾ܾ���   
//               ��������Ϊֻ����  
//               Ŀ¼�·ǿգ�  
//               ��ǰĿ¼��  
//           FR_NOT_READY��       �����������޷�������������������û��ý�������ԭ��   
//           FR_WRITE_PROTECTED�� ý��д����   
//           FR_DISK_ERR��        ����ʧ�����ڴ������е�һ������   
//           FR_INT_ERR��         ����ʧ������һ������� FAT �ṹ���ڲ�����   
//           FR_NOT_ENABLED��     �߼�������û�й�����   
//           FR_NO_FILESYSTEM��   ��������û�кϷ��� FAT ��   
//           FR_LOCKED��          �������������ļ�������ƣ�_FS_SHARE��   
//��    ע : f_deldir ���������Ƴ�һ���ļ��м������ļ��С����ļ����������Ƴ��Ѿ��򿪵Ķ���   
//====================================================================================================  
uint8_t WT_DeleteDir(const char * path)
{  
 FRESULT res;  
    DIR   dir;     /* �ļ��ж��� */ //36  bytes  
    FILINFO fno;   /* �ļ����� */   //32  bytes  
    TCHAR file[_MAX_LFN + 2] = {0};  
#if _USE_LFN  
    TCHAR lname[_MAX_LFN + 2] = {0};  
#endif  
      
#if _USE_LFN  
    fno.lfsize = _MAX_LFN;  
    fno.lfname = lname;    //���븳��ֵ  
#endif  
    //���ļ���  
   // res = f_chdir((const char *)"1:/LZY_WireTester/PrintFiles"); 
  //  res = f_opendir(&dir, (const char *)"1:/LZY_WireTester/PrintFiles"); 
    res = f_opendir(&dir, path);  
      
    //������ȡ�ļ�������  
    while((res == FR_OK) && (FR_OK == f_readdir(&dir, &fno)))  
    {  
        //����"."��".."�ļ��У�����  
        if(0 == strlen(fno.fname))          break;      //���������ļ���Ϊ��  
        if(0 == strcmp(fno.fname, "."))     continue;   //���������ļ���Ϊ��ǰ�ļ���  
        if(0 == strcmp(fno.fname, ".."))    continue;   //���������ļ���Ϊ��һ���ļ���  
          
        memset(file, 0, sizeof(file));  
#if _USE_LFN  
        sprintf((char*)file, "%s/%s", path, (*fno.lfname) ? fno.lfname : fno.fname);  
#else  
        sprintf((char*)file, "%s/%s", path, fno.fname);  
#endif  
        if (fno.fattrib & AM_DIR)  
        {//�����ļ��У��ݹ�ɾ��  
            res = WT_DeleteDir(file);  
        }  
        else  
        {//�����ļ���ֱ��ɾ��  
            res = f_unlink(file);  
            osDelay(10);
        }  
    }  
      
    //ɾ������  
    if(res == FR_OK)    res = f_unlink(path);  
    if(res == FR_NO_PATH)    return FR_OK;     
    return res;  
}  




/**
  * @brief  ��ѧϰ�ļ���ȡ�ַ���
  * @param  None
  * @retval 0-ok, 1-error
  */
static uint8_t GetString_FromStudyItem(uint8_t * str, uint64_t index, uint32_t * len)
{
	uint8_t i = 0;
	uint8_t j;
	char    buf8[50];
	char  para[5];
	
	str[i++] = '@';

	sprintf(buf8,"%d",(uint32_t)StudyFile->study_item[index].id);
	j=0;
	while(buf8[j])
	{
		str[i++] = buf8[j];
		j++;
	}
	str[i++] = ':';
	
	str[i++] = StudyFile->study_item[index].type;
	str[i++] = '/';
	
	str[i++] = 'J';
	str[i++] = ':';
	sprintf(buf8,"%d",(uint32_t)StudyFile->study_item[index].p1);
	j=0;
	while(buf8[j])
	{
		str[i++] = buf8[j];
		j++;
	}
	str[i++] = '/';
	
	str[i++] = 'J';
	str[i++] = ':';
	sprintf(buf8,"%d",(uint32_t)StudyFile->study_item[index].p2);
	j=0;
	while(buf8[j])
	{
		str[i++] = buf8[j];
		j++;
	}
	str[i++] = '/';
	
	str[i++] = 'C';
	str[i++] = 'R';
	str[i++] = ':';
	if(StudyFile->study_item[index].type == 'R')
	{
		itoa(StudyFile->study_item[index].param1,para,10);
		str[i++] = para[0];
		str[i++] = para[1];
		str[i++] = para[2];
		str[i++] = para[3];
	}
	else str[i++] = '0';
	str[i++] = '/';
	
	str[i++] = 'C';
	str[i++] = 'G';
	str[i++] = ':';
	if(StudyFile->study_item[index].type == 'R')
	{
		itoa(StudyFile->study_item[index].param2,para,10);
		str[i++] = para[0];
		str[i++] = para[1];
		str[i++] = para[2];
		str[i++] = para[3];
	}
	else str[i++] = '0';
	str[i++] = '/';
	
	str[i++] = 'C';
	str[i++] = 'B';
	str[i++] = ':';
	if(StudyFile->study_item[index].type == 'R')
	{
		itoa(StudyFile->study_item[index].param3,para,10);
		str[i++] = para[0];
		str[i++] = para[1];
		str[i++] = para[2];
		str[i++] = para[3];
	}
	else str[i++] = '0';
	str[i++] = '/';
	
	str[i++] = 'S';
	str[i++] = ':';
	str[i++] = '0';
	str[i++] = '/';
	
	str[i++] = 'T';
	str[i++] = ':';
	str[i++] = '0';
	str[i++] = '/';
	
	str[i++] = 'T';
	str[i++] = 'I';
	str[i++] = ':';
	str[i++] = '0';
	str[i++] = '/';
	
	str[i++] = 'O';
	str[i++] = ':';
	str[i++] = '0';
	str[i++] = '/';
	
	str[i++] = 'T';
	str[i++] = 'O';
	str[i++] = ':';
	str[i++] = '0';

	str[i++] = '\r';
	str[i++] = '\n';
	
	str[i] = 0;
	*len = i;
	return 0;	
}

/**
  * @brief  �Ӳ����ļ���ȡ�ַ���
  * @param  None
  * @retval 0-ok, 1-error
  */
static uint8_t GetString_FromTestItem(uint8_t * str, uint64_t index, uint32_t * len)
{
	uint8_t i = 0;
	uint8_t j;
	char    buf8[50];
	char  para[5];
	
	str[i++] = '@';

	sprintf(buf8,"%d",(uint32_t)TestFile->test_item[index].id);
	j=0;
	while(buf8[j])
	{
		str[i++] = buf8[j];
		j++;
	}
	str[i++] = ':';
	
	str[i++] = TestFile->test_item[index].type;
	str[i++] = '/';
	
	str[i++] = 'J';
	str[i++] = ':';
	sprintf(buf8,"%d",(uint32_t)TestFile->test_item[index].p1);
	j=0;
	while(buf8[j])
	{
		str[i++] = buf8[j];
		j++;
	}
	str[i++] = '/';
	
	str[i++] = 'J';
	str[i++] = ':';
	sprintf(buf8,"%d",(uint32_t)TestFile->test_item[index].p2);
	j=0;
	while(buf8[j])
	{
		str[i++] = buf8[j];
		j++;
	}
	str[i++] = '/';
	
	str[i++] = 'C';
	str[i++] = 'R';
	str[i++] = ':';
	if(TestFile->test_item[index].type == 'R')
	{
		itoa(TestFile->test_item[index].param1,para,10);
		str[i++] = para[0];
		str[i++] = para[1];
		str[i++] = para[2];
		str[i++] = para[3];
	}
	else str[i++] = '0';
	str[i++] = '/';
	
	str[i++] = 'C';
	str[i++] = 'G';
	str[i++] = ':';
	if(TestFile->test_item[index].type == 'R')
	{
		itoa(TestFile->test_item[index].param2,para,10);
		str[i++] = para[0];
		str[i++] = para[1];
		str[i++] = para[2];
		str[i++] = para[3];
	}
	else str[i++] = '0';
	str[i++] = '/';
	
	str[i++] = 'C';
	str[i++] = 'B';
	str[i++] = ':';
	if(TestFile->test_item[index].type == 'R')
	{
		itoa(TestFile->test_item[index].param3,para,10);
		str[i++] = para[0];
		str[i++] = para[1];
		str[i++] = para[2];
		str[i++] = para[3];
	}
	else str[i++] = '0';
	str[i++] = '/';
	
	str[i++] = 'S';
	str[i++] = ':';
	str[i++] = '0';
	str[i++] = '/';
	
	str[i++] = 'T';
	str[i++] = ':';
	str[i++] = '0';
	str[i++] = '/';
	
	str[i++] = 'T';
	str[i++] = 'I';
	str[i++] = ':';
	str[i++] = '0';
	str[i++] = '/';
	
	str[i++] = 'O';
	str[i++] = ':';
	str[i++] = '0';
	str[i++] = '/';
	
	str[i++] = 'T';
	str[i++] = 'O';
	str[i++] = ':';
	str[i++] = '0';

	str[i++] = '\r';
	str[i++] = '\n';
	
	str[i] = 0;
	*len = i;
	return 0;	
}

#define StudyFile_Title_1		"#1:LZY WIRED TESTER\r\n"
#define StudyFile_Title_2		"#2:VER 1.0\r\n"
#define StudyFile_Title_3		"#3:NAME: WIRED TESTER\r\n"
#define StudyFile_Title_4		"#4:MODEL:TESTER1\r\n"
#define StudyFile_Title_5		"#5:VENDOR:LZY Co,LTD\r\n"
#define StudyFile_Title_6		"#6:DATE:2014-06-09\r\n"
#define StudyFile_Title_7		"#7:CONF:v1.0\r\n"
#define StudyFile_Title_8		"#8:DRAWING NUM00001\r\n"
#define StudyFile_Title_9		"#9:ATTACH:Test\r\n"
#define StudyFile_Title_A		"#A:SERIAL NUM:0001\r\n"
#define StudyFile_Title_B		"#B:P:5 L:1 L:2 L:3 L:7 L:8\r\n"
#define StudyFile_Title_C		"#C:test.wtr"


/**
  * @brief  ����ѧϰ�Ĳ����ļ�
  * @param  None
  * @retval 0-ok, 1-hardware error, 2-file operate error, 
  */
uint8_t WT_StudyFiles_Write(uint8_t * path)
{
	FRESULT  res;
	DIR 		 dir;
	FILINFO  fno;
	FIL			 fil;
	char *	 fn;
	uint8_t  *p, *fn_filename;
	uint8_t  buf8[80], temp8[10];
	uint8_t  temp;
	uint32_t buf32;
	uint32_t i, j,len;
	uint64_t index;
  char  	 systime[30];
	char  	 str_time[40];
	
	// Check disc
	if(k_StorageGetStatus(USB_DISK_UNIT) == 0 && store_dev == 0)	//no usb or SD
	{
		return 1;
	}
	
	if(k_StorageGetStatus(MSD_DISK_UNIT) == 0 && store_dev == 1)	//no usb or SD
	{
		return 1;
	}
	
	// �ж��ļ����Ƿ����ڣ��������򴴽�	
  if(store_dev == 0) res = f_opendir(&dir, path_studyfile);
	if(store_dev == 1) res = f_opendir(&dir, path_studyfile_sd);
  if(res == FR_NO_PATH)	//Ŀ¼�����ڣ�����Ŀ¼
	{
		p = path;
		for(;(*p)!=0;p++)
		{
			if(*p == '/')
			{
				for(i=0;i<(p-path);i++) buf8[i]=*(path+i);
				buf8[i] = 0;
				res = f_mkdir((const TCHAR*) buf8);
				if((res != 0)&&(res != FR_EXIST)) {return 2;}
			}
		}
		//open the new dir
		res = f_opendir(&dir, path_studyfile);
		if(res != FR_OK) return 1;
	}
	else if (res != FR_OK)
  {
		return 1;
	}
	
	// �ж��ļ��Ƿ����� 
	#if _USE_LFN
  static char lfn[_MAX_LFN];
  fno.lfname = lfn;
  fno.lfsize = sizeof(lfn);
	#endif
	
	p = path;
	i = 0;
	for(;(*p)!=0;p++)
	{
		if(*p == '.')
		{
			p--;
			for(;(*p)!='/';p--)
			{
				i++;
				if(i>50) return 2;
			}
			p++;
			break;
		}
	}
	fn_filename = p;
	i = 0;	//0:ok, 1:error, 
	
	for (;;) 
	{
		res = f_readdir(&dir, &fno);                   /* Read a directory item */
		if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
		if (fno.fname[0] == '.') continue;             /* Ignore dot entry */
		#if _USE_LFN
		fn = *fno.lfname ? fno.lfname : fno.fname;
		#else
		fn = fno.fname;
		#endif
		if (fno.fattrib & AM_DIR) continue;  /* It is a directory */
		else /* It is a file. */
		{
			// compare filename & new filename
			p = fn_filename;
			while(*fn != 0)
			{
				if(*fn++ != *p++)
				{
					i = 1;
					break;
				}
				if(i == 1) break;
			}
			
			if((i == 0) && (*p == 0))	//fine the same filename
			{
				i = 1;
				break;
			}
			else	//no filename
			{
				i=0;
			}
		}
	}
	f_closedir(&dir); //�ر��ļ���

	//�����ļ�������
	if(i==1)	//�ļ��Ѵ��ڣ����ء���������ɾ���Ȳ���
	{
		//return 3;
		res = f_unlink((const TCHAR*)path);
		if(res != 0) return 2;
	}
	
	//����ѧϰ�ļ�
	res = f_open(&fil, (const TCHAR*)path, FA_OPEN_ALWAYS | FA_WRITE); //��д��ʽ�� û���ļ��򴴽� 
	if(res)
	{
		f_close(&fil);
		return 2;
	}

	res = f_lseek(&fil, f_size(&fil)); 
	if(res != FR_OK) return 2;
	
	//update title
	get_systemtime(systime);
	strcpy(str_time,"#6:DATE:");
	strcat(str_time,systime);
	strcat(str_time,"\r\n\0");
	res = f_write(&fil, StudyFile_Title_1, sizeof(StudyFile_Title_1)-1, &buf32); 
	res = f_write(&fil, StudyFile_Title_2, sizeof(StudyFile_Title_2)-1, &buf32); 
	res = f_write(&fil, StudyFile_Title_3, sizeof(StudyFile_Title_3)-1, &buf32); 
	res = f_write(&fil, StudyFile_Title_4, sizeof(StudyFile_Title_4)-1, &buf32); 
	res = f_write(&fil, StudyFile_Title_5, sizeof(StudyFile_Title_5)-1, &buf32); 
	//res = f_write(&fil, StudyFile_Title_6, sizeof(StudyFile_Title_6)-1, &buf32); 
	res = f_write(&fil, str_time, strlen(str_time)-1, &buf32); 
	res = f_write(&fil, StudyFile_Title_7, sizeof(StudyFile_Title_7)-1, &buf32); 
	res = f_write(&fil, StudyFile_Title_8, sizeof(StudyFile_Title_8)-1, &buf32); 
	res = f_write(&fil, StudyFile_Title_9, sizeof(StudyFile_Title_9)-1, &buf32); 
	res = f_write(&fil, StudyFile_Title_A, sizeof(StudyFile_Title_A)-1, &buf32); 
	res = f_write(&fil, StudyFile_Title_B, sizeof(StudyFile_Title_B)-1, &buf32); 
	p = fn_filename;
	i=0;
	while(*p) {i++; p++;}
	strcpy ((char *)buf8, "#C:");
	strcat ((char *)buf8,(char *)fn_filename);
	strcat ((char *)buf8,"\r\n");
	
	for(j=0;j<sizeof(temp8);j++) temp8[j]=0;
	sprintf((char *)temp8,"%d/%d\0", StudyFile->number_point, StudyFile->number_line);
	j=strlen((char *)temp8);
	strcat ((char *)buf8, "#D:");
	strcat ((char *)buf8,(char *) temp8);
	strcat ((char *)buf8,"\r\n");
	
	res = f_write(&fil, buf8, 3+i+2+5+j, &buf32); 
	if(res != 0) return 2;
	
	//save data to file
	for(index=0;index<StudyFile->item_total;index++)
	{
		temp = GetString_FromStudyItem(buf8, index, &len);
		if(temp != 0) return 2;
		for(i=0;i<2000;i++){}
			
		res = f_write(&fil, buf8, len, &buf32); 
		for(i=0;i<2000;i++){}
		if(res != FR_OK) return 2;
	}
	
	// save end mark to file
	for(i=0;i<sizeof(temp8);i++) temp8[i]=0;
	sprintf((char *) temp8,"%d", StudyFile->item_total+1);
	strcpy ((char *)buf8, "@");
	i=0;
	while(1)
	{
		buf8[i+1] = temp8[i];
		if(temp8[i] == 0) break;
		i++;
	}
	strcat ((char *)buf8,":E\r\n");
	res = f_write(&fil, buf8, i+4, &buf32); 
	
	//Save to file
	f_close(&fil);
	return 0;
}


/**
  * @brief  ���������ļ�
  * @param  None
  * @retval 0-ok, 1-hardware error, 2-file operate error, 
  */
uint8_t WT_TestFiles_Create(uint8_t * path)
{
	FRESULT  res;
	FIL			 fil;
//	uint8_t* p;
//	uint8_t  i;
//	uint8_t  buf8[100];
	//uint32_t buf32;
	
	// Check disc
	if(k_StorageGetStatus(USB_DISK_UNIT) == 0 && store_dev == 0)	//no usb 
	{
		return 1;
	}
	
	if(k_StorageGetStatus(MSD_DISK_UNIT) == 0 && store_dev == 1)	//no SD
	{
		return 1;
	}
	
	// open file
	//res = f_open(&fil, (const TCHAR*)path, FA_OPEN_EXISTING | FA_WRITE);
	res = f_open(&fil, (const TCHAR*)path, FA_CREATE_NEW | FA_WRITE);
	if(res == FR_EXIST )	//�ļ�����
	{
		res = f_unlink((const TCHAR*)path);
		if(res != 0) return 2;
		
		//����ѧϰ�ļ�
		res = f_open(&fil, (const TCHAR*)path, FA_CREATE_NEW | FA_WRITE); //��д��ʽ�� û���ļ��򴴽� 
		if(res)
		{
			f_close(&fil);
			return 2;
		}
	}
	else if(res != FR_OK)	//ok
	{
		return 1;
	}
	
	//Save to file
	f_close(&fil);
	return 0;
	
}


/**
  * @brief  ������ݵ��ļ�ĩβ
  * @param  None
  * @retval 0-ok, 1-hardware error, 2-file operate error, 
  */
uint8_t WT_TestFiles_Write(uint8_t * path, uint8_t * data)
{
	FRESULT  res;
	FIL			 fil;
	uint8_t* p;
	uint8_t  i;
	uint8_t  buf8[100];
	uint32_t buf32;
	
	// Check disc
	if(k_StorageGetStatus(USB_DISK_UNIT) == 0 && store_dev == 0)	//no usb 
	{
		return 1;
	}
	
	if(k_StorageGetStatus(MSD_DISK_UNIT) == 0 && store_dev == 1)	//no SD
	{
		return 1;
	}
	
	// open file
	res = f_open(&fil, (const TCHAR*)path, FA_OPEN_EXISTING | FA_WRITE);
	if((res == FR_NO_FILE)||(res == FR_NO_PATH)) //�ļ�������
	{
		p = path;
		for(;(*p)!=0;p++)
		{
			if(*p == '/')
			{
				for(i=0;i<(p-path);i++) buf8[i]=*(path+i);
				buf8[i] = 0;
				res = f_mkdir((const TCHAR*) buf8);
				if((res != 0)&&(res != FR_EXIST)) {return 2;}
			}
		}
		
		res = f_open(&fil, (const TCHAR*)path, FA_OPEN_ALWAYS | FA_WRITE); //��д��ʽ�� û���ļ��򴴽� 
		if(res)
		{
			f_close(&fil);
		}
	}
	else if(res != FR_OK)	//ok
	{
		return 1;
	}

	//Write string to file
	res = f_lseek(&fil, f_size(&fil)); 
	if(res != FR_OK) return 2;
	//i = strlen((char *)data);
	res = f_write(&fil, data, strlen((char *)data), &buf32); 
	if(res != 0) return 2;
	
	//Save to file
	f_close(&fil);
	return 0;

}

/**
  * @brief  ��������ļ�
  * @param  None
  * @retval 0-ok, 1-hardware error, 2-file operate error, 
  */
uint8_t WT_TestFiles_Write2card(uint8_t * path)
{
	FRESULT  res;
	DIR 		 dir;
	FILINFO  fno;
	FIL			 fil;
	char *	 fn;
	uint8_t  *p, *fn_filename;
	uint8_t  buf8[80], temp8[10];
	uint8_t  temp;
	uint32_t buf32;
	uint32_t i, j,len;
	uint64_t index;
  char  	 systime[30];
	char  	 str_time[40];
	
	// Check disc
	if(k_StorageGetStatus(USB_DISK_UNIT) == 0 && store_dev == 0)	//no usb or SD
	{
		return 1;
	}
	
	if(k_StorageGetStatus(MSD_DISK_UNIT) == 0 && store_dev == 1)	//no usb or SD
	{
		return 1;
	}
	
	// �ж��ļ����Ƿ����ڣ��������򴴽�	
  if(store_dev == 0) res = f_opendir(&dir, path_studyfile);
	if(store_dev == 1) res = f_opendir(&dir, path_studyfile_sd);
  if(res == FR_NO_PATH)	//Ŀ¼�����ڣ�����Ŀ¼
	{
		p = path;
		for(;(*p)!=0;p++)
		{
			if(*p == '/')
			{
				for(i=0;i<(p-path);i++) buf8[i]=*(path+i);
				buf8[i] = 0;
				res = f_mkdir((const TCHAR*) buf8);
				if((res != 0)&&(res != FR_EXIST)) {return 2;}
			}
		}
		//open the new dir
		res = f_opendir(&dir, path_studyfile);
		if(res != FR_OK) return 1;
	}
	else if (res != FR_OK)
  {
		return 1;
	}
	
	// �ж��ļ��Ƿ����� 
	#if _USE_LFN
  static char lfn[_MAX_LFN];
  fno.lfname = lfn;
  fno.lfsize = sizeof(lfn);
	#endif
	
	p = path;
	i = 0;
	for(;(*p)!=0;p++)
	{
		if(*p == '.')
		{
			p--;
			for(;(*p)!='/';p--)
			{
				i++;
				if(i>50) return 2;
			}
			p++;
			break;
		}
	}
	fn_filename = p;
	i = 0;	//0:ok, 1:error, 
	
	for (;;) 
	{
		res = f_readdir(&dir, &fno);                   /* Read a directory item */
		if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
		if (fno.fname[0] == '.') continue;             /* Ignore dot entry */
		#if _USE_LFN
		fn = *fno.lfname ? fno.lfname : fno.fname;
		#else
		fn = fno.fname;
		#endif
		if (fno.fattrib & AM_DIR) continue;  /* It is a directory */
		else /* It is a file. */
		{
			// compare filename & new filename
			p = fn_filename;
			while(*fn != 0)
			{
				if(*fn++ != *p++)
				{
					i = 1;
					break;
				}
				if(i == 1) break;
			}
			
			if((i == 0) && (*p == 0))	//fine the same filename
			{
				i = 1;
				break;
			}
			else	//no filename
			{
				i=0;
			}
		}
	}
	f_closedir(&dir); //�ر��ļ���

	//�����ļ�������
	if(i==1)	//�ļ��Ѵ��ڣ����ء���������ɾ���Ȳ���
	{
		//return 3;
		res = f_unlink((const TCHAR*)path);
		if(res != 0) return 2;
	}
	
	//��������ļ�
	res = f_open(&fil, (const TCHAR*)path, FA_OPEN_ALWAYS | FA_WRITE); //��д��ʽ�� û���ļ��򴴽� 
	if(res)
	{
		f_close(&fil);
		return 2;
	}

	res = f_lseek(&fil, f_size(&fil)); 
	if(res != FR_OK) return 2;
	
	//update title
	get_systemtime(systime);
	strcpy(str_time,"#6:DATE:");
	strcat(str_time,systime);
	strcat(str_time,"\r\n\0");
//	res = f_write(&fil, TestFile->FileHeader_FormatFlag, sizeof(TestFile->FileHeader_FormatFlag), &buf32); 
//	res = f_write(&fil, TestFile->FileHeader_FormatVersion, sizeof(TestFile->FileHeader_FormatVersion), &buf32); 
//	res = f_write(&fil, TestFile->FileHeader_ProductName, sizeof(TestFile->FileHeader_ProductName), &buf32); 
//	res = f_write(&fil, TestFile->FileHeader_ProductModel, sizeof(TestFile->FileHeader_ProductModel), &buf32); 
//	res = f_write(&fil, TestFile->FileHeader_ProductCompany, sizeof(TestFile->FileHeader_ProductCompany), &buf32); 
//	//res = f_write(&fil, StudyFile_Title_6, sizeof(StudyFile_Title_6)-1, &buf32); 
//	res = f_write(&fil, str_time, strlen(str_time)-1, &buf32); 
//	res = f_write(&fil, TestFile->FileHeader_ProductVersion, sizeof(TestFile->FileHeader_ProductVersion), &buf32); 
//	res = f_write(&fil, TestFile->FileHeader_DrawingNumber, sizeof(TestFile->FileHeader_DrawingNumber), &buf32); 
//	res = f_write(&fil, TestFile->FileHeader_AttachInfo, sizeof(TestFile->FileHeader_AttachInfo), &buf32); 
//	res = f_write(&fil, TestFile->FileHeader_SerialNumber, sizeof(TestFile->FileHeader_SerialNumber), &buf32); 
//	res = f_write(&fil, TestFile->FileHeader_PrintInfo, sizeof(TestFile->FileHeader_PrintInfo), &buf32); 
	
	res = f_write(&fil, StudyFile_Title_1, sizeof(StudyFile_Title_1)-1, &buf32); 
	res = f_write(&fil, StudyFile_Title_2, sizeof(StudyFile_Title_2)-1, &buf32); 
	res = f_write(&fil, StudyFile_Title_3, sizeof(StudyFile_Title_3)-1, &buf32); 
	res = f_write(&fil, StudyFile_Title_4, sizeof(StudyFile_Title_4)-1, &buf32); 
	res = f_write(&fil, StudyFile_Title_5, sizeof(StudyFile_Title_5)-1, &buf32); 
	//res = f_write(&fil, StudyFile_Title_6, sizeof(StudyFile_Title_6)-1, &buf32); 
	res = f_write(&fil, str_time, strlen(str_time)-1, &buf32); 
	res = f_write(&fil, StudyFile_Title_7, sizeof(StudyFile_Title_7)-1, &buf32); 
	res = f_write(&fil, StudyFile_Title_8, sizeof(StudyFile_Title_8)-1, &buf32); 
	res = f_write(&fil, StudyFile_Title_9, sizeof(StudyFile_Title_9)-1, &buf32); 
	res = f_write(&fil, StudyFile_Title_A, sizeof(StudyFile_Title_A)-1, &buf32); 
	res = f_write(&fil, StudyFile_Title_B, sizeof(StudyFile_Title_B)-1, &buf32); 
	p = fn_filename;
	i=0;
	while(*p) {i++; p++;}
	strcpy ((char *)buf8, "#C:");
	strcat ((char *)buf8,(char *)fn_filename);
	strcat ((char *)buf8,"\r\n");
	
	for(j=0;j<sizeof(temp8);j++) temp8[j]=0;
	sprintf((char *)temp8,"%d/%d\0", TestFile->number_point, TestFile->number_line);
	j=strlen((char *)temp8);
	strcat ((char *)buf8, "#D:");
	strcat ((char *)buf8,(char *) temp8);
	strcat ((char *)buf8,"\r\n");
	
	res = f_write(&fil, buf8, 3+i+2+5+j, &buf32); 
	if(res != 0) return 2;
	
	//save data to file
	for(index=0;index<TestFile->item_total;index++)
	{
		temp = GetString_FromTestItem(buf8, index, &len);
		if(temp != 0) return 2;
		for(i=0;i<2000;i++){}
			
		res = f_write(&fil, buf8, len, &buf32); 
		for(i=0;i<2000;i++){}
		if(res != FR_OK) return 2;
	}
	
	// save end mark to file
	for(i=0;i<sizeof(temp8);i++) temp8[i]=0;
	sprintf((char *) temp8,"%d", TestFile->item_total+1);
	strcpy ((char *)buf8, "@");
	i=0;
	while(1)
	{
		buf8[i+1] = temp8[i];
		if(temp8[i] == 0) break;
		i++;
	}
	strcat ((char *)buf8,":E\r\n");
	res = f_write(&fil, buf8, i+4, &buf32); 
	
	//Save to file
	f_close(&fil);
	return 0;
}


/**
  * @brief  ��������Ʋ����ļ�
  * @param  None
  * @retval 0-ok, 1-hardware error, 2-file operate error, 
  */
uint8_t WT_TestFiles_WriteBinary(uint8_t * path)
{
	FRESULT  res;
	DIR 		 dir;
	FILINFO  fno;
	FIL			 fil;
	char *	 fn;
	uint8_t  *p, *fn_filename;
	uint8_t  buf8[80];
	uint32_t buf32;
	uint32_t i;
	uint16_t CRC_Value = 0;
	uint32_t write_bytes = 0;
	
	// Check disc
	if(k_StorageGetStatus(USB_DISK_UNIT) == 0 && store_dev == 0)	//no usb or SD
	{
		return 1;
	}
	
	if(k_StorageGetStatus(MSD_DISK_UNIT) == 0 && store_dev == 1)	//no usb or SD
	{
		return 1;
	}
	
	// �ж��ļ����Ƿ����ڣ��������򴴽�	
  if(store_dev == 0) res = f_opendir(&dir, path_studyfile);
	if(store_dev == 1) res = f_opendir(&dir, path_studyfile_sd);
  if(res == FR_NO_PATH)	//Ŀ¼�����ڣ�����Ŀ¼
	{
		p = path;
		for(;(*p)!=0;p++)
		{
			if(*p == '/')
			{
				for(i=0;i<(p-path);i++) buf8[i]=*(path+i);
				buf8[i] = 0;
				res = f_mkdir((const TCHAR*) buf8);
				if((res != 0)&&(res != FR_EXIST)) {return 2;}
			}
		}
		//open the new dir
		res = f_opendir(&dir, path_studyfile);
		if(res != FR_OK) return 1;
	}
	else if (res != FR_OK)
  {
		return 1;
	}
	
	// �ж��ļ��Ƿ���� 
	#if _USE_LFN
  static char lfn[_MAX_LFN];
  fno.lfname = lfn;
  fno.lfsize = sizeof(lfn);
	#endif
	
	p = path;
	i = 0;
	for(;(*p)!=0;p++)
	{
		if(*p == '.')
		{
			p--;
			for(;(*p)!='/';p--)
			{
				i++;
				if(i>50) return 2;
			}
			p++;
			break;
		}
	}
	fn_filename = p;
	i = 0;	//0:ok, 1:error, 
	
	for (;;) 
	{
		res = f_readdir(&dir, &fno);                   /* Read a directory item */
		if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
		if (fno.fname[0] == '.') continue;             /* Ignore dot entry */
		#if _USE_LFN
		fn = *fno.lfname ? fno.lfname : fno.fname;
		#else
		fn = fno.fname;
		#endif
		if (fno.fattrib & AM_DIR) continue;  /* It is a directory */
		else /* It is a file. */
		{
			// compare filename & new filename
			p = fn_filename;
			while(*fn != 0)
			{
				if(*fn++ != *p++)
				{
					i = 1;
					break;
				}
				if(i == 1) break;
			}
			
			if((i == 0) && (*p == 0))	//fine the same filename
			{
				i = 1;
				break;
			}
			else	//no filename
			{
				i=0;
			}
		}
	}
	f_closedir(&dir); //�ر��ļ���

	//�����ļ�������
	if(i==1)	//�ļ��Ѵ��ڣ����ء���������ɾ���Ȳ���
	{
		//return 3;
		res = f_unlink((const TCHAR*)path);
		if(res != 0) return 2;
	}
	
	//FileStream����
	memset(FileStream,0,sizeof(WT_StudyFileStreamTypedef));
	
	//��������ļ�
	res = f_open(&fil, (const TCHAR*)path, FA_OPEN_ALWAYS | FA_WRITE); //��д��ʽ�� û���ļ��򴴽� 
	if(res)
	{
		f_close(&fil);
		return 2;
	}
	
	strcpy((char *)FileStream->FileHeader_FileName,(char *)fn_filename);
	FileStream->item_total = TestFile->item_total;
	FileStream->number_line = TestFile->number_line;
	FileStream->number_point = TestFile->number_point;
	
	//memcpy(buffer->study_item,StudyFile->study_item,32640*sizeof(StudyFile->study_item));
	for(i=0;i<32640;i++)
	{
		FileStream->study_item[i].id = TestFile->test_item[i].id;
		FileStream->study_item[i].type = TestFile->test_item[i].type;
		FileStream->study_item[i].p1 = TestFile->test_item[i].p1;
		FileStream->study_item[i].p2 = TestFile->test_item[i].p2;
		FileStream->study_item[i].param1 = TestFile->test_item[i].param1;
		FileStream->study_item[i].param2 = TestFile->test_item[i].param2;
		FileStream->study_item[i].param3 = TestFile->test_item[i].param3;
	}
	CRC_Value = Data_CRC16_MOSBUS((uint8_t *)FileStream, (uint16_t)(sizeof(WT_StudyFileStreamTypedef)-2)); //�����CRC-16
	FileStream->CRC_val = CRC_Value;
	write_bytes = sizeof(WT_StudyFileStreamTypedef);
	res = f_write(&fil,FileStream,write_bytes, &buf32); 
	if(res != 0) 
	{
		f_close(&fil);
		return 2;
	}
	
	//Save to file
	f_close(&fil);
	return 0;
}

/**
  * @brief  �����������ѧϰ�ļ�
  * @param  None
  * @retval 0-ok, 1-hardware error, 2-file operate error, 
  */
uint8_t WT_StudyFiles_WriteBinary(uint8_t * path)
{
	FRESULT  res;
	DIR 		 dir;
	FILINFO  fno;
	FIL			 fil;
	char *	 fn;
	uint8_t  *p, *fn_filename;
	uint8_t  buf8[80];
	uint32_t buf32;
	uint32_t i;
	uint32_t write_bytes = 0;
	uint16_t  CRC_Value = 0;//CRCУ��ֵ
	
	// Check disc
	if(k_StorageGetStatus(USB_DISK_UNIT) == 0 && store_dev == 0)	//no usb or SD
	{
		return 1;
	}
	
	if(k_StorageGetStatus(MSD_DISK_UNIT) == 0 && store_dev == 1)	//no usb or SD
	{
		return 1;
	}
	
	// �ж��ļ����Ƿ����ڣ��������򴴽�	
  if(store_dev == 0) res = f_opendir(&dir, path_studyfile);
	if(store_dev == 1) res = f_opendir(&dir, path_studyfile_sd);
  if(res == FR_NO_PATH)	//Ŀ¼�����ڣ�����Ŀ¼
	{
		p = path;
		for(;(*p)!=0;p++)
		{
			if(*p == '/')
			{
				for(i=0;i<(p-path);i++) buf8[i]=*(path+i);
				buf8[i] = 0;
				res = f_mkdir((const TCHAR*) buf8);
				if((res != 0)&&(res != FR_EXIST)) {return 2;}
			}
		}
		//open the new dir
		res = f_opendir(&dir, path_studyfile);
		if(res != FR_OK) return 1;
	}
	else if (res != FR_OK)
  {
		return 1;
	}
	
	// �ж��ļ��Ƿ����� 
	#if _USE_LFN
  static char lfn[_MAX_LFN];
  fno.lfname = lfn;
  fno.lfsize = sizeof(lfn);
	#endif
	
	p = path;
	i = 0;
	for(;(*p)!=0;p++)
	{
		if(*p == '.')
		{
			p--;
			for(;(*p)!='/';p--)
			{
				i++;
				if(i>50) return 2;
			}
			p++;
			break;
		}
	}
	fn_filename = p;
	i = 0;	//0:ok, 1:error, 
	
	for (;;) 
	{
		res = f_readdir(&dir, &fno);                   /* Read a directory item */
		if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
		if (fno.fname[0] == '.') continue;             /* Ignore dot entry */
		#if _USE_LFN
		fn = *fno.lfname ? fno.lfname : fno.fname;
		#else
		fn = fno.fname;
		#endif
		if (fno.fattrib & AM_DIR) continue;  /* It is a directory */
		else /* It is a file. */
		{
			// compare filename & new filename
			p = fn_filename;
			while(*fn != 0)
			{
				if(*fn++ != *p++)
				{
					i = 1;
					break;
				}
				if(i == 1) break;
			}
			
			if((i == 0) && (*p == 0))	//fine the same filename
			{
				i = 1;
				break;
			}
			else	//no filename
			{
				i=0;
			}
		}
	}
	f_closedir(&dir); //�ر��ļ���

	//�����ļ�������
	if(i==1)	//�ļ��Ѵ��ڣ����ء���������ɾ���Ȳ���
	{
		//return 3;
		res = f_unlink((const TCHAR*)path);
		if(res != 0) return 2;
	}
	
	//������ѧϰ�ļ�
	res = f_open(&fil, (const TCHAR*)path, FA_OPEN_ALWAYS | FA_WRITE); //��д��ʽ�� û���ļ��򴴽� 
	if(res)
	{
		f_close(&fil);
		return 2;
	}
	
	strcpy((char *)FileStream->FileHeader_FileName,(char *)fn_filename);
	FileStream->item_total = StudyFile->item_total;
	FileStream->number_line = StudyFile->number_line;
	FileStream->number_point = StudyFile->number_point;
	
	//memcpy(buffer->study_item,StudyFile->study_item,32640*sizeof(StudyFile->study_item));
	for(i=0;i<32640;i++)
	FileStream->study_item[i] =StudyFile->study_item[i];
	CRC_Value = Data_CRC16_MOSBUS((uint8_t *)FileStream, (uint16_t)(sizeof(WT_StudyFileStreamTypedef)-2)); //�����CRC-16
	FileStream->CRC_val = CRC_Value;
	write_bytes = sizeof(WT_StudyFileStreamTypedef);
	res = f_write(&fil,FileStream,write_bytes, &buf32); 
	if(res != 0) return 2;
	
	//Save to file
	f_close(&fil);
	return 0;
}

/**
  * @brief  ��ȡ�����ļ�
  * @param  None
  * @retval 0-ok, 1-hardware error, 2-file operate error, 
  */
uint8_t WT_TestItem_Read(char * filename)
{
	FRESULT res;
	FIL file;
	char str[100];
	char str_head[7];
	char file_head[7] = "#1:LZY";

	uint32_t i=0;
	uint32_t buf32;
	
	uint16_t  CRC_Value = 0;//CRCУ��ֵ
	//uint16_t len = 0;

	// Init
	TestFile->item_current = 0;
	TestFile->item_Index = 0;
	TestFile->item_total = 0;
	TestFile->item_error_count = 0;
	TestFile->test_ErrFlag = 0;	//for max error ID��0-no error, 1-error detect and retest
	TestFile->file_status = 0;	//0:not init, 1: init ok, 2: hardware error, 3-no this file
	TestFile->task_status = 0;	//0-waiting, 1-testing, 2-test error, 3-test ok, 4-waiting remove wire, 5-testing & Z error
	TestFile->command = 0;	//0-no operate, 1-start, 2-cancel
	TestFile->test_num = 0;//��������
	TestFile->number_point = 0;	//�˿�����
	TestFile->number_line = 0;		//ͨ������
	
	memset(FileStream,0,sizeof(WT_StudyFileStreamTypedef));
	memset(str_head,0,7);
	
	if(k_StorageGetStatus(USB_DISK_UNIT) || k_StorageGetStatus(MSD_DISK_UNIT))
	{
		if(store_dev == 0) strcpy (str,(char *)path_testfile);
		if(store_dev == 1) strcpy (str,(char *)path_testfile_sd);

		strcat (str,"/");
		strcat (str,filename);//�����ļ�����·��

		//open file
		res = f_open(&file, str, FA_OPEN_EXISTING | FA_READ);
		if (res == FR_OK)
		{		
			//��ȡǰ6���ַ�
			res = f_read(&file, str_head,6, &buf32); 
			if(res != 0) return 2;
			
			if(!strcmp(str_head,file_head))//�ļ�����"#1:LZY"����ʾ�ļ�Ϊ�ַ��ļ�
			{
				if(WT_TestItem_ReadAscii(str,filename) == 0)//���ַ���ʽ��ȡ�ļ�
				{
					return 0;
				}
				else return 2;
			}
			else //�����Ʒ�ʽ��ȡ�ļ�����
			{
				res = f_lseek(&file, 0); /* Put file header */
				res = f_read(&file, FileStream,sizeof(WT_StudyFileStreamTypedef), &buf32); 
				
				if(res != 0) return 2;
				strcpy ((char*)TestFile->FileName,(char *)filename);
				TestFile->number_point = FileStream->number_point;
				TestFile->number_line = FileStream->number_line;
				TestFile->item_total = FileStream->item_total;
				for(i=0;i<TestFile->item_total;i++)
				{
					TestFile->test_item[i].id = i+1;
					TestFile->test_item[i].type = FileStream->study_item[i].type;
					TestFile->test_item[i].p1 = FileStream->study_item[i].p1;
					TestFile->test_item[i].p2 = FileStream->study_item[i].p2;
					TestFile->test_item[i].param1 = FileStream->study_item[i].param1;
					TestFile->test_item[i].param2 = FileStream->study_item[i].param2;
					TestFile->test_item[i].param3 = FileStream->study_item[i].param3;
					TestFile->test_item[i].result = 0;
				}
				CRC_Value = Data_CRC16_MOSBUS((uint8_t *)FileStream, (uint16_t)(sizeof(WT_StudyFileStreamTypedef)-2)); //�����CRC-16
				if(CRC_Value == FileStream->CRC_val) TestFile->file_status = 1;	//0:not init, 1: init ok, 2: hardware error, 3-no this file
				else TestFile->file_status = 0;
			}
		}
		else	
		{
			TestFile->file_status = 3;	//0:not init, 1: init ok, 2: hardware error, 3-no this file
            osMessagePut ( StorageEvent, MSDDISK_CONNECTION_EVENT, 0);//���³�ʼ��SD�ӿ�
		}

		f_close(&file);
		f_sync(&file);
		return 0;
	}
	else
	{
		TestFile->file_status = 2;	//0:not init, 1: init ok, 2: hardware error, 3-no this file
		return 2;
	}
}

/**
  * @brief  ��ȡAscii��ʽ�����ļ�
  * @param  None
  * @retval 0-ok 1-error
  */
uint8_t WT_TestItem_ReadAscii(char * filepath,char * filename)
{
	FRESULT res;
	FIL file;
	char str[100];
	char line[100];
	uint8_t  j;
	uint32_t i=0,index=0;
	char *ptr;
	char *delim="@:/AZKDVFHLMCRWJCRGBSTIO";
	uint8_t  read_ok = 0;//0-�ļ���ȡ������  1-�ļ���ȡ����
	
		strcat (str,"/");
//    strcat (str,(char *)TestFolder->FilesName[0]);
		strcat (str,filename);
			
		// init head file
		line[0] = '-';
		line[1] = 0;
		strcpy ((char*)TestFile->FileName,(char *)line);
		strcpy ((char*)TestFile->FileHeader_FormatFlag,(char *)line);
		strcpy ((char*)TestFile->FileHeader_FormatVersion,(char *)line);
		strcpy ((char*)TestFile->FileHeader_ProductName,(char *)line);
		strcpy ((char*)TestFile->FileHeader_ProductModel,(char *)line);
		strcpy ((char*)TestFile->FileHeader_ProductCompany,(char *)line);
		strcpy ((char*)TestFile->FileHeader_CreatDate,(char *)line);
		strcpy ((char*)TestFile->FileHeader_ProductVersion,(char *)line);
		strcpy ((char*)TestFile->FileHeader_DrawingNumber,(char *)line);
		strcpy ((char*)TestFile->FileHeader_AttachInfo,(char *)line);
		strcpy ((char*)TestFile->FileHeader_SerialNumber,(char *)line);
		strcpy ((char*)TestFile->FileHeader_PrintInfo,(char *)line);
		strcpy ((char*)TestFile->FileHeader_FileName,(char *)line);
		//open file
		res = f_open(&file, filepath, FA_OPEN_EXISTING | FA_READ);
		if (res == FR_OK)
		{		
			strcpy ((char*)TestFile->FileName,(char *)filename);
			while(1)
			{
				if(f_gets(line, sizeof line, &file) == 0) break;
				if((line[0] == '#') && (line[2] == ':'))	// head mark
				{
					//fine second ':'
					//----------------------------------------
					j=0;
					for(index=3;index<20;index++)
					{
						if(line[index]== ':') //fine title mark
						{
							index++;
							j = index;
							break;
						}
					}
					if(j==0) // no title mark
					{
						index = 3;
						j = index;
					}
					//check if length more than 20 chars
					//----------------------------------------
					for(;index<16+j;index++)
					{
						if(line[index]== '\n') 
						{
							line[index] = 0;
							index = 0;
							break;
						}
						else if(line[index]== 0) 
						{
							index = 0;
							break;
						}
					}
					//length more than 20 chars
					//----------------------------------------
					if(index != 0) //length error
					{
						line[index++] = '.';
						line[index++] = '.';
						line[index++] = '.';
						line[index++] = 0;
					}
					// update to struct
					//----------------------------------------
					switch(line[1])
					{
						case '1':
							strcpy ((char*)TestFile->FileHeader_FormatFlag,(char *)&line[j]);
							break;
						case '2':
							strcpy ((char*)TestFile->FileHeader_FormatVersion,(char *)&line[j]);
							break;
						case '3':
							strcpy ((char*)TestFile->FileHeader_ProductName,(char *)&line[j]);
							break;
						case '4':
							strcpy ((char*)TestFile->FileHeader_ProductModel,(char *)&line[j]);
							break;
						case '5':
							strcpy ((char*)TestFile->FileHeader_ProductCompany,(char *)&line[j]);
							break;
						case '6':
							strcpy ((char*)TestFile->FileHeader_CreatDate,(char *)&line[j]);
							break;
						case '7':
							strcpy ((char*)TestFile->FileHeader_ProductVersion,(char *)&line[j]);
							break;
						case '8':
							strcpy ((char*)TestFile->FileHeader_DrawingNumber,(char *)&line[j]);
							break;
						case '9':
							strcpy ((char*)TestFile->FileHeader_AttachInfo,(char *)&line[j]);
							break;
						case 'A':
							strcpy ((char*)TestFile->FileHeader_SerialNumber,(char *)&line[j]);
							break;
						case 'B':
							strcpy ((char*)TestFile->FileHeader_PrintInfo,(char *)&line[j]);
							break;
						case 'C':
							strcpy ((char*)TestFile->FileHeader_FileName,(char *)&line[j]);
							break;
						default:
							break;
					}
				}
				else //not head lines
				{
					break;
				}
			}				
		}

		f_close(&file);
		
		// init test items
		res = f_open(&file, filepath, FA_OPEN_EXISTING | FA_READ);
		if (res == FR_OK)
		{		
			i=0;
			while(f_gets(line, sizeof line, &file))
			{
				//-----------------------------------------------------------
				// Head line
				//-----------------------------------------------------------
				if(strchr(line, '#'))
				{
					if(strstr(line, "#D:"))
					{
						strtok((line),delim);
						index=0;
						while(1)
						{
							ptr=strtok(NULL,delim);
							if(ptr == 0) break;
							
							switch(index)
							{
								case 0:
									TestFile->number_point=atoi(ptr);
									index++;
									break;
								case 1:
									TestFile->number_line=atoi(ptr);
									index++;
									break;
								default:
									break;
							}
						}
					}
					else continue;
				}
				//-----------------------------------------------------------
				// W test line
				//-----------------------------------------------------------
				else if(strstr(line, ":W/"))
				{	
					strtok((line),delim);
					index=0;
					while(1)
					{
						ptr=strtok(NULL,delim);
						if(ptr == 0) break;
						
						switch(index)
						{
							case 0:
								TestFile->test_item[i].id=i+1;
								TestFile->test_item[i].type='W';
								TestFile->test_item[i].p1=atoi(ptr);
								index++;
								break;
							case 1:
								TestFile->test_item[i].p2=atoi(ptr);
								index++;
								break;
							case 2:
								TestFile->test_item[i].param1=atoi(ptr);
								index++;
								break;
							case 3:
								TestFile->test_item[i].param2=atoi(ptr);
								index++;
								break;
							case 4:
								TestFile->test_item[i].param3=atoi(ptr);
								index++;
								break;
							case 5:
								TestFile->test_item[i].index=atoi(ptr);
								index++;
								break;
							case 6:
								TestFile->test_item[i].plus_tun=atoi(ptr);
								index++;
								break;
							case 7:
								TestFile->test_item[i].plus_type=atoi(ptr);
								index++;
								break;
							case 8:
								TestFile->test_item[i].out_tun=atoi(ptr);
								index++;
								break;
							case 9:
								TestFile->test_item[i].out_type=atoi(ptr);
								index++;
								break;
							default:
								break;
						}
					}
					TestFile->test_item[i].result = 0;
					i++;
					TestFile->item_total = i;
				}
				//-----------------------------------------------------------
				// D test line
				//-----------------------------------------------------------
				else if(strstr(line, ":D/"))
				{	
					strtok((line),delim);
					index=0;
					while(1)
					{
						ptr=strtok(NULL,delim);
						if(ptr == 0) break;
						
						switch(index)
						{
							case 0:
								TestFile->test_item[i].id=i+1;
								TestFile->test_item[i].type='D';
								TestFile->test_item[i].p1=atoi(ptr);
								index++;
								break;
							case 1:
								TestFile->test_item[i].p2=atoi(ptr);
								index++;
								break;
							case 2:
								TestFile->test_item[i].param1=atoi(ptr);
								index++;
								break;
							case 3:
								TestFile->test_item[i].param2=atoi(ptr);
								index++;
								break;
							case 4:
								TestFile->test_item[i].param3=atoi(ptr);
								index++;
								break;
							case 5:
								TestFile->test_item[i].index=atoi(ptr);
								index++;
								break;
							case 6:
								TestFile->test_item[i].plus_tun=atoi(ptr);
								index++;
								break;
							case 7:
								TestFile->test_item[i].plus_type=atoi(ptr);
								index++;
								break;
							case 8:
								TestFile->test_item[i].out_tun=atoi(ptr);
								index++;
								break;
							case 9:
								TestFile->test_item[i].out_type=atoi(ptr);
								index++;
								break;
							default:
								break;
						}
					}
					TestFile->test_item[i].result = 0;
					i++;
					TestFile->item_total = i;
				}		
				//-----------------------------------------------------------
				// C test line
				//-----------------------------------------------------------
				else if(strstr(line, ":C/"))
				{	
					strtok((line),delim);
					index=0;
					while(1)
					{
						ptr=strtok(NULL,delim);
						if(ptr == 0) break;
						
						switch(index)
						{
							case 0:
								TestFile->test_item[i].id=i+1;
								TestFile->test_item[i].type='C';
								TestFile->test_item[i].p1=atoi(ptr);
								index++;
								break;
							case 1:
								TestFile->test_item[i].p2=atoi(ptr);
								index++;
								break;
							case 2:
								TestFile->test_item[i].param1=atoi(ptr);
								index++;
								break;
							case 3:
								TestFile->test_item[i].param2=atoi(ptr);
								index++;
								break;
							case 4:
								TestFile->test_item[i].param3=atoi(ptr);
								index++;
								break;
							case 5:
								TestFile->test_item[i].index=atoi(ptr);
								index++;
								break;
							case 6:
								TestFile->test_item[i].plus_tun=atoi(ptr);
								index++;
								break;
							case 7:
								TestFile->test_item[i].plus_type=atoi(ptr);
								index++;
								break;
							case 8:
								TestFile->test_item[i].out_tun=atoi(ptr);
								index++;
								break;
							case 9:
								TestFile->test_item[i].out_type=atoi(ptr);
								index++;
								break;
							default:
								break;
						}
					}
					TestFile->test_item[i].result = 0;
					i++;
					TestFile->item_total = i;
				}		
				//-----------------------------------------------------------
				// R test line
				//-----------------------------------------------------------
				else if(strstr(line, ":R/"))
				{	
					strtok((line),delim);
					index=0;
					while(1)
					{
						ptr=strtok(NULL,delim);
						if(ptr == 0) break;
						
						switch(index)
						{
							case 0:
								TestFile->test_item[i].id=i+1;
								TestFile->test_item[i].type='R';
								TestFile->test_item[i].p1=atoi(ptr);
								index++;
								break;
							case 1:
								TestFile->test_item[i].p2=atoi(ptr);
								index++;
								break;
							case 2:
								TestFile->test_item[i].param1=atoi(ptr);
								index++;
								break;
							case 3:
								TestFile->test_item[i].param2=atoi(ptr);
								index++;
								break;
							case 4:
								TestFile->test_item[i].param3=atoi(ptr);
								index++;
								break;
							case 5:
								TestFile->test_item[i].index=atoi(ptr);
								index++;
								break;
							case 6:
								TestFile->test_item[i].plus_tun=atoi(ptr);
								index++;
								break;
							case 7:
								TestFile->test_item[i].plus_type=atoi(ptr);
								index++;
								break;
							case 8:
								TestFile->test_item[i].out_tun=atoi(ptr);
								index++;
								break;
							case 9:
								TestFile->test_item[i].out_type=atoi(ptr);
								index++;
								break;
							default:
								break;
						}
					}
					TestFile->test_item[i].result = 0;
					i++;
					TestFile->item_total = i;
				}		
				//-----------------------------------------------------------
				// K test line
				//-----------------------------------------------------------
				else if(strstr(line, ":K/"))
				{	
					strtok((line),delim);
					index=0;
					while(1)
					{
						ptr=strtok(NULL,delim);
						if(ptr == 0) break;
						
						switch(index)
						{
							case 0:
								TestFile->test_item[i].id=i+1;
								TestFile->test_item[i].type='K';
								TestFile->test_item[i].p1=atoi(ptr);
								index++;
								break;
							case 1:
								TestFile->test_item[i].p2=atoi(ptr);
								index++;
								break;
							case 2:
								TestFile->test_item[i].param1=atoi(ptr);
								index++;
								break;
							case 3:
								TestFile->test_item[i].param2=atoi(ptr);
								index++;
								break;
							case 4:
								TestFile->test_item[i].param3=atoi(ptr);
								index++;
								break;
							case 5:
								TestFile->test_item[i].index=atoi(ptr);
								index++;
								break;
							case 6:
								TestFile->test_item[i].plus_tun=atoi(ptr);
								index++;
								break;
							case 7:
								TestFile->test_item[i].plus_type=atoi(ptr);
								index++;
								break;
							case 8:
								TestFile->test_item[i].out_tun=atoi(ptr);
								index++;
								break;
							case 9:
								TestFile->test_item[i].out_type=atoi(ptr);
								index++;
								break;
							default:
								break;
						}
					}
					TestFile->test_item[i].result = 0;
					i++;
					TestFile->item_total = i;
				}								
				//-----------------------------------------------------------
				// Z test line
				//-----------------------------------------------------------
				else if(strstr(line, ":Z/"))
				{	
					strtok((line),delim);
					index=0;
					while(1)
					{
						ptr=strtok(NULL,delim);
						if(ptr == 0) break;
						
						switch(index)
						{
							case 0:
								TestFile->test_item[i].id=i+1;
								TestFile->test_item[i].type='Z';
								TestFile->test_item[i].p1=atoi(ptr);
								index++;
								break;
							case 1:
								TestFile->test_item[i].p2=atoi(ptr);
								index++;
								break;
							case 2:
								TestFile->test_item[i].param1=atoi(ptr);
								index++;
								break;
							case 3:
								TestFile->test_item[i].param2=atoi(ptr);
								index++;
								break;
							case 4:
								TestFile->test_item[i].param3=atoi(ptr);
								index++;
								break;
							case 5:
								TestFile->test_item[i].index=atoi(ptr);
								index++;
								break;
							case 6:
								TestFile->test_item[i].plus_tun=atoi(ptr);
								index++;
								break;
							case 7:
								TestFile->test_item[i].plus_type=atoi(ptr);
								index++;
								break;
							case 8:
								TestFile->test_item[i].out_tun=atoi(ptr);
								index++;
								break;
							case 9:
								TestFile->test_item[i].out_type=atoi(ptr);
								index++;
								break;
							default:
								break;
						}
					}
					TestFile->test_item[i].result = 0;
					i++;
					TestFile->item_total = i;
				}
				//-----------------------------------------------------------
				// End line
				//-----------------------------------------------------------
				else if(strstr(line, ":E"))
				{
					read_ok = 1;
					break;
				}
				continue;	
			}
			//f_close(&file);
			if(1 == read_ok) TestFile->file_status = 1;	//0:not init, 1: init ok, 2: hardware error, 3-no this file
			else TestFile->file_status = 0;
		}
		else	
		{
			TestFile->file_status = 3;	//0:not init, 1: init ok, 2: hardware error, 3-no this file
			return 1;
		}
		f_close(&file);
		return 0;
}

/*
*********************************************************************************************************
*	�� �� ��: LoadFontLib()
*	����˵��: ��U���м����ֿ��ļ���SPI FLASH
*	��    ��: ��   	
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LoadFontLib(void)
{
    FRESULT res;
    FIL file;
    float FinishPecent;
    uint32_t Count = 0;
    //uint16_t y;
    //FONT_T tFont16;			/* ����һ������ṹ���������������������� */
    char cDispBuf[32];
    uint8_t tempbuf[1024]; 
    uint16_t PageSize = 256;
    uint32_t bw;
  
  // Check disc
	if(k_StorageGetStatus(USB_DISK_UNIT) == 0 && store_dev == 0)	//no usb or SD
	{
		return;
	}  

	/* ��1�������Flash�ֿ��Ƿ��Ѹ���************************************************/
    sFLASH_Read(tempbuf, 0, 2);
    if((tempbuf[0] == 0x5A) && (tempbuf[1] == 0xA5)) return;
	
	/* ��2�������ļ� ***************************************************************/
	res = f_open(&file, "0:/LZY_FW/font.bin", FA_OPEN_EXISTING | FA_READ);
	if (res !=  FR_OK)
	{
		//LCD_DispStr(0, y, "open file(font.bin) error!", &tFont16);
		//y += 16;
	}
	
	//�Ȳ�������оƬ
	sFLASH_Erase_Chip();

	/* ��3��������SD�����ֿ��ļ�font.bin��SPI FLASH **********************************/
	for(;;)
	{
		/* ��ȡһ�����������ݵ�buf */
		res = f_read(&file, &tempbuf, PageSize, &bw);
		
		/* ��ȡ������߶�ȡ��ϣ��˳� */
		if ((res != FR_OK)||bw == 0)
		{
			break;
		}
		
        /* д���ݵ�SPI FLASH */
        //ucState = sf_WriteBuffer(tempbuf, Count*g_tSF.PageSize, g_tSF.PageSize);
        sFLASH_Write(tempbuf, sFLASH_ADDR_FONT+Count*PageSize, PageSize);
		
		/* �������0����ʾ����ʧ�� */
//		if(ucState == 0)
//		{
//			LCD_DispStr(0, y, "Coppy error", &tFont16);
//			break;
//		}
		
		/* ��ʾ���ƽ��� */
		Count = Count + 1;
		FinishPecent = (float)(Count* PageSize) / file.fsize;
		printf(cDispBuf, "Current copy is:%02d%%", (uint8_t)(FinishPecent*100));
		//LCD_DispStr(0, y, cDispBuf, &tFont16);
	}
	
    //update flag
    tempbuf[0] = 0x5A;
    tempbuf[1] = 0xA5;
    sFLASH_Write(tempbuf, 0, 2);

    /* �ȴ�2���ʼ��ȥemWin������ */
    osDelay(2000);
}
