/**
  ******************************************************************************
  * @progect LZY Wire Cube Tester
	* @file    malloc.c
  * @author  LZY Zhang Manxin
  * @version V1.0.0
  * @date    2016-08-11
  * @brief   This file provides the malloc functions
  ******************************************************************************
  */
#include "wt_bsp.h"
#include "malloc.h"	  

/* Critical section management. */
extern void vPortEnterCritical( void );
extern void vPortExitCritical( void );


//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//�ڴ���� ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/15
//�汾��V1.2
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved
//********************************************************************************
//V1.1 20140706
//1,�޸Ĳ��ֺ�������,��ֹ��������������ֳ�ͻ(����LWIP).
//2,�޸��ڴ��Ϊ32�ֽڶ���
//V1.2 20140716
//1,�޸�my_mem_malloc����,���ڴ��ڴ��������bug.
////////////////////////////////////////////////////////////////////////////////// 	 

#define MEM_SDRAM_BASE 0xD0000000

//�ڴ��(32�ֽڶ���)
//-----------------------------------------------------------------------------------------------------
#if   (defined ( __CC_ARM ))
__align(32) uint8_t mem1base[MEM1_MAX_SIZE];													        //�ڲ�SRAM�ڴ��
__align(32) uint8_t mem2base[MEM2_MAX_SIZE] __attribute__((at(MEM_SDRAM_BASE)));  //�ⲿSDRAM�ڴ��
#elif (defined (__ICCARM__))
#pragma pack(4)
__no_init uint8_t mem1base[MEM1_MAX_SIZE]; //�ڲ�SRAM�ڴ��   
__no_init uint8_t mem2base[MEM2_MAX_SIZE] @(MEM_SDRAM_BASE);  
#pragma pack()
#elif defined   (  __GNUC__  )
mem_TypeDef memory_pool __attribute__((section(".ExtRAMData"))); 
#endif
//-----------------------------------------------------------------------------------------------------

//�ڴ�����
//-----------------------------------------------------------------------------------------------------
#if   (defined ( __CC_ARM ))
uint16_t mem1mapbase[MEM1_ALLOC_TABLE_SIZE];													                      //�ڲ�SRAM�ڴ��MAP
uint16_t mem2mapbase[MEM2_ALLOC_TABLE_SIZE] __attribute__((at(MEM_SDRAM_BASE+MEM2_MAX_SIZE)));  //�ⲿSDRAM�ڴ��MAP
#elif (defined (__ICCARM__))
__no_init uint16_t mem1mapbase[MEM1_ALLOC_TABLE_SIZE];
__no_init uint16_t mem2mapbase[MEM2_ALLOC_TABLE_SIZE] @(MEM_SDRAM_BASE+MEM2_MAX_SIZE);
#elif defined   (  __GNUC__  )
__align(32) uint16_t mem1mapbase[MEM1_ALLOC_TABLE_SIZE];
__align(32) uint16_t mem2mapbase[MEM2_ALLOC_TABLE_SIZE]  __attribute__((section(".ExtRAMData")));
#endif
//-----------------------------------------------------------------------------------------------------


//�ڴ�������	   
const uint32_t memtblsize[SRAMBANK]={MEM1_ALLOC_TABLE_SIZE,MEM2_ALLOC_TABLE_SIZE};  //�ڴ���С
const uint32_t memblksize[SRAMBANK]={MEM1_BLOCK_SIZE,MEM2_BLOCK_SIZE};					    //�ڴ�ֿ��С
const uint32_t memsize[SRAMBANK]={MEM1_MAX_SIZE,MEM2_MAX_SIZE};							        //�ڴ��ܴ�С


//�ڴ���������
struct _m_mallco_dev mallco_dev=
{
	my_mem_init,						//�ڴ��ʼ��
	my_mem_perused,			    //�ڴ�ʹ����
	mem1base,mem2base,			//�ڴ��
	mem1mapbase,mem2mapbase,//�ڴ����״̬��
	0,0,	 					        //�ڴ����δ����
};

//�����ڴ�
//*des:Ŀ�ĵ�ַ
//*src:Դ��ַ
//n:��Ҫ���Ƶ��ڴ泤��(�ֽ�Ϊ��λ)
void mymemcpy(void *des,void *src,uint32_t n)  
{  
  uint8_t *xdes=des;
  uint8_t *xsrc=src; 
  while(n--)*xdes++=*xsrc++;  
}  
//�����ڴ�
//*s:�ڴ��׵�ַ
//c :Ҫ���õ�ֵ
//count:��Ҫ���õ��ڴ��С(�ֽ�Ϊ��λ)
void mymemset(void *s,uint8_t c,uint32_t count)  
{  
  uint8_t *xs = s;  
  while(count--)*xs++=c;  
}	   
//�ڴ�����ʼ��  
//memx:�����ڴ��
void my_mem_init(uint8_t memx)  
{  
  mymemset(mallco_dev.memmap[memx], 0,memtblsize[memx]*2);//�ڴ�״̬����������  
  mymemset(mallco_dev.membase[memx], 0,memsize[memx]);	  //�ڴ��������������  
  mallco_dev.memrdy[memx]=1;								              //�ڴ�����ʼ��OK  
}  
//��ȡ�ڴ�ʹ����
//memx:�����ڴ��
//����ֵ:ʹ����(0~100)
uint8_t my_mem_perused(uint8_t memx)  
{  
  uint32_t used=0;  
  uint32_t i;  
  for(i=0;i<memtblsize[memx];i++)  
  {  
    if(mallco_dev.memmap[memx][i])used++; 
  } 
  return (used*100)/(memtblsize[memx]);  
}  
//�ڴ����(�ڲ�����)
//memx:�����ڴ��
//size:Ҫ������ڴ��С(�ֽ�)
//����ֵ:0XFFFFFFFF,�������;����,�ڴ�ƫ�Ƶ�ַ 
uint32_t my_mem_malloc(uint8_t memx,uint32_t size)  
{  
  signed long offset=0;  
  uint32_t nmemb;	//��Ҫ���ڴ����  
  uint32_t cmemb=0;//�������ڴ����
  uint32_t i;  
  if(!mallco_dev.memrdy[memx])mallco_dev.init(memx);//δ��ʼ��,��ִ�г�ʼ�� 
  if(size==0)return 0XFFFFFFFF;//����Ҫ����
  nmemb=size/memblksize[memx];  	//��ȡ��Ҫ����������ڴ����
  if(size%memblksize[memx])nmemb++;  
  for(offset=memtblsize[memx]-1;offset>=0;offset--)//���������ڴ������  
  {     
    if(!mallco_dev.memmap[memx][offset])cmemb++;//�������ڴ��������
    else cmemb=0;								//�����ڴ������
    if(cmemb==nmemb)							//�ҵ�������nmemb�����ڴ��
    {
      for(i=0;i<nmemb;i++)  					//��ע�ڴ��ǿ� 
      {  
        mallco_dev.memmap[memx][offset+i]=nmemb;  
      }  
      return (offset*memblksize[memx]);//����ƫ�Ƶ�ַ  
    }
  }  
  return 0XFFFFFFFF;//δ�ҵ����Ϸ����������ڴ��  
}  
//�ͷ��ڴ�(�ڲ�����) 
//memx:�����ڴ��
//offset:�ڴ��ַƫ��
//����ֵ:0,�ͷųɹ�;1,�ͷ�ʧ��;  
uint8_t my_mem_free(uint8_t memx,uint32_t offset)  
{  
  int i;  
  if(!mallco_dev.memrdy[memx])//δ��ʼ��,��ִ�г�ʼ��
  {
    mallco_dev.init(memx);    
    return 1;//δ��ʼ��  
  }  
  if(offset<memsize[memx])//ƫ�����ڴ����. 
  {  
    int index=offset/memblksize[memx];			//ƫ�������ڴ�����  
    int nmemb=mallco_dev.memmap[memx][index];	//�ڴ������
    for(i=0;i<nmemb;i++)  						//�ڴ������
    {  
      mallco_dev.memmap[memx][index+i]=0;  
    }  
    return 0;  
  }else return 2;//ƫ�Ƴ�����.  
}  
//�ͷ��ڴ�(�ⲿ����) 
//memx:�����ڴ��
//ptr:�ڴ��׵�ַ 
void myfree(uint8_t memx,void *ptr)  
{  
  uint32_t offset;  
  if(ptr==NULL)return;//��ַΪ0.  
  //printf("free:%d,%x\r\n",memx,ptr);
  vPortEnterCritical();		//�����ٽ���(�޷����жϴ��)     
  offset=(uint32_t)ptr-(uint32_t)mallco_dev.membase[memx];  
  my_mem_free(memx,offset);	//�ͷ��ڴ�     
  vPortExitCritical();			//�˳��ٽ���(���Ա��жϴ��) 
}  
//�����ڴ�(�ⲿ����)
//memx:�����ڴ��
//size:�ڴ��С(�ֽ�)
//����ֵ:���䵽���ڴ��׵�ַ.
void *mymalloc(uint8_t memx,uint32_t size)  
{  
  uint32_t offset;  
  //printf("m:%d,%d\r\n",memx,size);
  vPortEnterCritical();		//�����ٽ���(�޷����жϴ��)   
  offset=my_mem_malloc(memx,size);  	   	
  vPortExitCritical();			//�˳��ٽ���(���Ա��жϴ��) 			   
  if(offset==0XFFFFFFFF)return NULL;  
  else return (void*)((uint32_t)mallco_dev.membase[memx]+offset);  
}  
//���·����ڴ�(�ⲿ����)
//memx:�����ڴ��
//*ptr:���ڴ��׵�ַ
//size:Ҫ������ڴ��С(�ֽ�)
//����ֵ:�·��䵽���ڴ��׵�ַ.
void *myrealloc(uint8_t memx,void *ptr,uint32_t size)  
{  
  uint32_t offset; 
  vPortEnterCritical();		//�����ٽ���(�޷����жϴ��)   
  offset=my_mem_malloc(memx,size);  
  vPortExitCritical();			//�˳��ٽ���(���Ա��жϴ��) 		
  if(offset==0XFFFFFFFF)return NULL;     
  else  
  {  									   
    mymemcpy((void*)((uint32_t)mallco_dev.membase[memx]+offset),ptr,size);	//�������ڴ����ݵ����ڴ�   
    myfree(memx,ptr);  											  		//�ͷž��ڴ�
    return (void*)((uint32_t)mallco_dev.membase[memx]+offset);  				//�������ڴ��׵�ַ
  }  
}
