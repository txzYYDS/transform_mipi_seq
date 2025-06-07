#ifndef __MIPI_DATA_HANDLE_H__
#define __MIPI_DATA_HANDLE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>  // 包含 isdigit 函数
#include <stdbool.h>


//错误码
#define OPEN_RAW_FILE_ERR -1;
#define CLOSE_RAW_FILE_ERR -2;
#define OPEN_COOKED_FILE_ERR -3;
#define CLOSE_COOKED_FILE_ERR -4;
#define READ_RAW_FILE_ERR -5;
#define PARSE_RAW_DATA_ERR -6;
#define WRITE_COOKED_FILE_ERR -7;
#define PTR_NOT_DATA_HEAD -8;
#define RAW_DATA_FORMAT_ERR -9;
#define COUNT_OR_ADDR_ERR -10;
#define ARGC_NUM_IS_ILLEGAL -11;
#define INPUT_PARAM_IS_NULL -12;


//函数声明:
int count_of_bytes(char *cmp);//计算数据段字节数
char *find_data_head(char *cmp);//找到指令段与数据段的分界点
int parse_mipi_init_data(char *raw_buf,char *cook_buf);//解析生数据
int parse_mipi_data(char *src,char *dest,int count);//解析数据段
int check_raw_data_format(char *raw_buf);//检查数据格式函数
int check_delay_format(char *buf);//检查延时数据格式函数
int parse_delay_time(char *buf,char *cook_buf);//解析延时数据段数据 --- 实际上还是对熟数据空间的修改
void transform_to_hex(char *buf_10,char *buf_16);//将10进制数字字符串转化为16进制数字字符串







#endif
