#ifndef _SAYHI_FLASH_H_
#define _SAYHI_FLASH_H_

#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

/**@brief:结点结构：
*	@start_page_addr:存储页地址
@	@start head_flag:数据头
@	@start tail_flag:数据尾
*/
typedef struct Flash_node_struct
{
	uint32_t start_page_addr;
	uint32_t head_flag;		//数据头
	uint32_t tail_flag;		//数据尾
}flash_node;


/** @brief:				node初始化函数
*	@para node: 		结构体指针
*	@para page_addr: 	页地址
*	@para head: 		数据头（32位）
*	@para tail: 		数据尾（32位）
*/
void Sayhi_flash_node_init(flash_node *node,uint32_t page_addr,uint32_t head,uint32_t tail);


/** @fun	:		读取固定长度大小的数据到flash中
*   @brief  :		node里面的flag为头，宏定义TAIl为尾巴
*	@return :	 	1 成功
*   @return :		0 失败
*/
uint8_t Sayhi_read_data(flash_node node,uint8_t *buffer,uint8_t byte_num);

/** @fun	:		写固定长度大小的数据到flash中
*   @brief  :		添加node里面的flag为头，宏定义TAIl为尾巴
*	@return :	 	1 成功
*   @return :		0 失败
*/
uint8_t Sayhi_write_data(flash_node node,unsigned char *buffer,uint8_t byte_num);



//*********************            test         *******************************//

/** @fun	:		测试是否可以存储数组
*	@return :		1:成功   0：失败
*/
uint8_t test_arry(void);

/** @fun	:		测试是否可以存储数组
*	@return :		1:成功   0：失败
*/
uint8_t test_sturct(void);

/** @fun	:		测试是否可以存储数组
*	@return :		1:成功   0：失败
*/
uint8_t test_uint16_arry(void);
/************************        test end 	       **********************************/

/* template */
uint8_t test_template();
#endif
