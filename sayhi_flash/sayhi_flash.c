#include "m_flash.h"
#include "string.h"

/**	@author	: 	jiangshan 
*	@sayhi	: 	open source code library	 
*	@time	:	2020.01.15
*/


/** @note:本文用于实现对stm32内置flash实现一页存储一个数据，设置数据头和数据尾。
*			从而可以在一页里面实现存储多次数据，延长flash寿命
*/

/****************************

*操作流程：
1、定义一个结构体结点	：	flash_node node_arry;
2、将结点初始化			：	Sayhi_flash_node_init(&node_arry,M_ADR62,0x05050505,0xa0a0a0a0);
3、存数据				：	Sayhi_write_data(node_arry,(unsigned char *)test_arry,6);
4、读数据				：	Sayhi_read_data(node_arry,(uint8_t *)test_arrt_read,6);
*操作例子：				：	test_template

***************************/


/**
* 	注意stm32是32位的机器，所以一个地址的长度是32位，
*  	例如：addr1:0x00000001,但是每一个地址所能够存储的数据大小却是一个字节长度那就是：0x01
*/

/**		测试地址
*/
#define M_ADR61 0x0800F400U	//61页地址five_reset
#define M_ADR62 0x0800F800U	//62页地址
#define M_ADR63 0x0800FC00U	//63页地址



//页大小，默认为2K
//可以在头文件里面修改大小
#ifdef  PAGE_SIZE 
static uint16_t page_size = PAGE_SIZE;
#else
static uint16_t page_size  = 2048;
#endif


/***************************** tool start **********************************/
/**				用于调试打印的工具函数					*/

/** @fun	:	打印数组
*
*/
void printf_arry(uint8_t *arry,uint16_t arry_num)
{
	printf("arry is : ");
	for(int i= 0;i<arry_num;i++)
	{
		printf("%02x",arry[i]);
	}
	printf("\n ");
}
/***************************** tool end **********************************/


/***************************** static fun start **********************************/
/** @fun	:		擦除一页的数据
*	@return : 		一个32位数据
*/
static int erase_page(uint32_t page_addr,uint32_t page_num)
{
	HAL_FLASH_Unlock();
	FLASH_EraseInitTypeDef FlashSet;
	FlashSet.TypeErase = FLASH_TYPEERASE_PAGES;
	FlashSet.PageAddress = page_addr;
	FlashSet.NbPages = page_num;
	uint32_t PageError = 0;
	HAL_FLASHEx_Erase(&FlashSet, &PageError);
	HAL_FLASH_Lock();
	return 1;
}

/** @fun:		读取某个三十二位地址的值
*	@return : 	一个32位数据
*/
static uint32_t read_u32_addr(uint32_t addr)
{
	return *(uint32_t *)addr; 
}

/** @fun:		读取某个三十二位地址的值
*	@return : 	一个8位数据
*/
static uint8_t read_u8_addr(uint32_t addr)
{
	return *(__IO   uint8_t *)addr; 
}

/** @fun:		读取固定字节数量的数据到buffer里面
*	@return : 	1 成功
*   @return :	0 失败
*/
static uint8_t read_u8_buffer(uint32_t addr,uint8_t *buffer,uint8_t byte_num)
{
	for(int i = 0;i<byte_num;i++)
	{
		buffer[i] = read_u8_addr(addr+i);
		//buffer[i] =*(__IO   uint8_t *)(addr+i);
	}
	return 1; 
}

/** @fun	:		读取固定长度大小的数据到flash中
*   @brief  :		node里面的flag为头，宏定义TAIl为尾巴
*	@return :	 	1 成功
*   @return :		0 失败
*/
static uint8_t write_u8_buffer(uint32_t addr,uint8_t *buffer,uint8_t word_num)
{
	for(int i=0,j=0;j<word_num;i+=4,j++)
	{
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD+i,addr,*(buffer+j));
	}
}
/***************************** static fun end **********************************/

/*********************** user funcation begin **********************************/

/** @fun	:		读取固定长度大小的数据到flash中
*   @brief  :		node里面的flag为头，宏定义TAIl为尾巴
*	@return :	 	1 成功
*   @return :		0 失败
*/
uint8_t Sayhi_read_data(flash_node node,uint8_t *buffer,uint8_t byte_num)
{
	uint32_t temp_addr =0;
	uint8_t flag = 0;
	
	for(int i=node.start_page_addr+page_size;i>node.start_page_addr;i-=4)
	{
		if(read_u32_addr(i) == node.tail_flag)
		{
			flag = 1;
			if(read_u32_addr(i - ((byte_num+3)/4)*4 -4) == node.head_flag)
			{
				temp_addr = i - ((byte_num+3)/4)*4;
				break;
			}
			else
			{
				printf("data wrong\n");
				break;
			}
		}
		else
		{
			//printf("no data\n");
			//return 0 ;
		}
	}
	if(flag ==1)
	{
		flag =0;
		//printf("temp_addr is %02x\n",temp_addr);
		//printf("buffer is %02x\n",buffer);
		read_u8_buffer(temp_addr,buffer,byte_num);
		return 1;
	}
	else
	{
		printf("no data\n");
		return 0;
	}
	
}

/** @fun	:		写固定长度大小的数据到flash中
*   @brief  :		添加node里面的flag为头，宏定义TAIl为尾巴
*	@return :	 	1 成功
*   @return :		0 失败
*/
uint8_t Sayhi_write_data(flash_node node,unsigned char *buffer,uint8_t byte_num)
{
	uint32_t temp_addr =0;
	int num = 0;
	int word_num = (byte_num+3)/4;
	for(int i=node.start_page_addr+page_size;i>node.start_page_addr;i-=4)
	{
		if(read_u32_addr(i) == node.tail_flag)
		{
			if(read_u32_addr(i-((byte_num+3)/4)*4 -4) == node.head_flag)
			{
				if((page_size -( i - node.start_page_addr))<byte_num+8)
				{
					printf("该页地址不够，擦除\n");
					erase_page(node.start_page_addr,1);
					temp_addr = node.start_page_addr;
				}
				else
				{
					temp_addr = i + 4;
				}
				break;
			}
		}
		else
		{
			temp_addr =node.start_page_addr ; 
		}
	}
	HAL_FLASH_Unlock();
	
	printf("temp_addr is %02x\n",temp_addr);
	
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,temp_addr,node.head_flag);
	temp_addr+=4;
	for(int i=0,j=0;j<word_num;i+=4,j++)
	{
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,temp_addr+i,((unsigned int *)buffer)[j]);
		num += 4;
	}
	temp_addr = temp_addr +num;
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,temp_addr,node.tail_flag);
	HAL_FLASH_Lock();
	return 1 ;
}

/** @fun				：	node初始化函数
*	@para	node		：	一个存储结点
*	@para	page_addr	：	页地址
*	@para	head		：	数据头
*	@para	tail		：	数据尾
*/
void Sayhi_flash_node_init(flash_node *node,uint32_t page_addr,uint32_t head,uint32_t tail)
{
	node->head_flag 		=	head;
	node->tail_flag  		=	tail;
	node->start_page_addr	=	page_addr;
}

/*********************** user funcation end***************************/





/************************        test begin 		**********************************/
/*					测试	文件			*/

/** @fun	:		测试是否可以存储数组
*	@return :		1:成功   0：失败
*/
uint8_t test_arry(void)
{
	uint8_t test_arry[6] ={1,2,3,4,5,6};
	uint8_t test_arrt_read[6] = {0};
	flash_node node_arry;
	Sayhi_flash_node_init(&node_arry,M_ADR62,0x05050505,0xa0a0a0a0);
	Sayhi_write_data(node_arry,(unsigned char *)test_arry,6);
	Sayhi_read_data(node_arry,test_arrt_read,6);
	if(memcmp(test_arry,test_arrt_read,6) ==0 )
	{
		printf("arry flash test good\n");
		return 1;
	}
	else
	{
		printf("arry flash test fail\n");
		printf_arry(test_arrt_read,6);
		return 0;
	}
}


/** @fun	:		测试是否可以存储数组
*	@return :		1:成功   0：失败
*/
uint8_t test_sturct(void)
{
	struct test_st
	{
		uint8_t a;
		uint16_t b;
		uint8_t  c;
	};
	struct test_st struct_st ;
	struct test_st struct_st_read ;
	struct_st.a = 	0;
	struct_st.b =	1;
	struct_st.c	=   2;
	
	
	flash_node node_st;
	Sayhi_flash_node_init(&node_st,M_ADR62,0x05050505,0xa0a0a0a0);
	
	Sayhi_write_data(node_st,(unsigned char *)&struct_st,sizeof(struct test_st));
	Sayhi_read_data(node_st,(unsigned char *)&struct_st_read,sizeof(struct test_st));
	
	if(memcmp(&struct_st,&struct_st_read,sizeof(struct test_st)) ==0 )
	{
		printf("struct flash test good\n");
		return 1;
	}
	else
	{
		printf("struct flash test fail\n");
		return 0;
	}
}

/** @fun	:		测试是否可以存储数组
*	@return :		1:成功   0：失败
*/
uint8_t test_uint16_arry(void)
{
	uint16_t test_arry[6] ={1,2,3,4,5,6};
	uint16_t test_arrt_read[6] = {0};
	flash_node node_arry;
	Sayhi_flash_node_init(&node_arry,M_ADR62,0x05050505,0xa0a0a0a0);
	Sayhi_write_data(node_arry,(unsigned char *)test_arry,6);
	Sayhi_read_data(node_arry,(uint8_t *)test_arrt_read,6);
	if(memcmp(test_arry,test_arrt_read,6) ==0 )
	{
		printf("u16 arry flash test good\n");
		return 1;
	}
	else
	{
		printf("u16 arry flash test fail\n");
		printf_arry((uint8_t *)test_arrt_read,6);
		return 0;
	}
}

/************************        test end 	       **********************************/

/* template */
uint8_t test_template()
{

	uint16_t test_arry[6] ={1,2,3,4,5,6};
	uint16_t test_arrt_read[6] = {0};
	
	
//	Standard process
	//1:First:  define a node 	
	//首先定义一个结点
	flash_node node_arry;
	//2:Second:init node.set flash addr,data head and data tail
	//2：将这个节点初始化，设置存储地址，数据头和数据尾
	Sayhi_flash_node_init(&node_arry,M_ADR62,0x05050505,0xa0a0a0a0);
	//3:third:save data(must be u8)
	//3:存储数据，必须转换为u8
	Sayhi_write_data(node_arry,(unsigned char *)test_arry,6);
	//4:forth:read data(must be u8)
	//4:读取数据，这个数组必须是u8
	Sayhi_read_data(node_arry,(uint8_t *)test_arrt_read,6);
//	Standard process end
	
	
	if(memcmp(test_arry,test_arrt_read,6) ==0 )
	{
		printf("u16 arry flash test good\n");
		return 1;
	}
	else
	{
		printf("u16 arry flash test fail\n");
		printf_arry((uint8_t *)test_arrt_read,6);
		return 0;
	}	
}


