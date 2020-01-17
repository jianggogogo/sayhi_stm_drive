# sayhi_flash

- 一个简单的单页多次存储一个数据的驱动：



# 操作流程：
1、定义一个结构体结点	：	flash_node node_arry;
2、将结点初始化			：	Sayhi_flash_node_init(&node_arry,M_ADR62,0x05050505,0xa0a0a0a0);
3、存数据				：	Sayhi_write_data(node_arry,(unsigned char *)test_arry,6);
4、读数据			Sayhi_read_data(node_arry,(uint8_t *)test_arrt_read,6);


# 操作例子：				
test_template();



