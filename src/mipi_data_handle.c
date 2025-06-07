#include "mipi_data_handle.h"




//W_COM: --- 指令
/*
    //多字节格式
    39 00 xx --- xx 后面命令的字节数，大于等于3，为十六进制

    //双字节格式
    15 00 02 --- 中间为延时时间，十六进制

    //单字节格式
    05 00 01 --- 中间为延时时间，十六进制
*/


/*
函数名称: parse_mipi_init_data
函数功能: 解析mipi初始化序列数据
函数形参： 
    char *raw_buf --- 生缓冲空间地址
    char *cook_buf --- 熟缓冲空间地址
返回值: 
    小于0 --- 解析失败或出错
    等于0 --- 解析成功，并存储到了熟缓冲空间
*/
//解地址修改，会对存储的数据产生影响
//没有去对空行进行处理
int parse_mipi_init_data(char *raw_buf,char *cook_buf){
    /************程序核心************/
    char *cmd = NULL;//指令
    char *cmd_h = NULL;//指令开头
    int count = 0;//命令字节数计数器
    int ret = 0;//解析数据段的判断结果

    cmd_h = raw_buf;
    //目标1：找到数据前面的指令段
    cmd_h = find_data_head(cmd_h);
    if(NULL == cmd_h){
        perror("raw data format error\r\n");
        return RAW_DATA_FORMAT_ERR;
    }
    else{
        //目标2：解析指令段
        count =  count_of_bytes(cmd_h);//计算字节数
        if(0 > count){
            return PTR_NOT_DATA_HEAD;
        }
    }

    /*
        1.多字节格式 05
        2.双字节格式 15
        3.单字节格式 39
    */
    cmd = cook_buf;

    //不用使用找分界函数去找了,上面找过一次，存储到了cmd_h中
    ret = parse_mipi_data(cmd_h,cmd,count);
    if(0 > ret){
        perror("parse mipi data error\r\n");
        return PARSE_RAW_DATA_ERR;
    }

    return 0;
}



/*
函数名称: find_data_head
函数功能：找到指令与数据的分界
函数参数:
    char *cmp --- 比较段地址
返回值:
    char * --- 返回"("或者"{"地址
*/
//只是找到分界点，没有修改
char *find_data_head(char *cmp){
    char *ptr = cmp;
    //指针的值也是值
    while(ptr[0] != '(' && ptr[0] != '{'){
        if(ptr[0] == ')' || ptr[0] == '}' || ptr[0] == ';'){
            perror("data is error format\r\n");
            return NULL;
        }
        ptr++;//地址++
    }
    return ptr;//返回偏移后的地址
}



/*
函数名称：count_of_bytes
函数功能：计算字节数函数
函数参数：
    char *cmp --- 开始段地址
返回值：
    大于等于0 --- 返回命令的字节数
    小于0 --- 失败或出错
*/
//只是计算了数据段字节数，没有修改
int count_of_bytes(char *cmp){
    /*
        解决方法:
            地址从"("开始
            逐渐偏移
            遇到")"结束
            在这之间，遇到"x",计数值++
    */
    int num = 0;
    char *ptr = cmp;

    //判断是否在数据段
    if(ptr[0] != '(' && ptr[0] != '{'){
        //开始段不在数据段头
        perror("ptr is not data head\r\n");
        return PTR_NOT_DATA_HEAD;
    }

    //在数据段头，计数开始
    while(ptr[0] != ')'){
        if(ptr[0] == 'x'){
            //偏移到对应的16进制数据下
            num++;
        }
        ptr++;
    }
    return num;
}





/*
函数名称：parse_mipi_data
函数功能：解析数据段
函数参数：
    char *src --- 解析数据的源地址
    char *dest --- 解析后数据的存储空间地址
    int count --- 数据段字节数
返回值：
    0 --- 解析成功
    小于0 --- 解析失败或出错
*/
/*
处理方法：
对解析后的数据进行拼接
*/
int parse_mipi_data(char *src,char *dest,int count){
    char *sp = src;//接收传进的源地址
    char *dp = dest;//接受传进的目标地址

    //错误判断
    if(sp == NULL || sp[0] == '\0' || dp == NULL){
        perror("src or dest addr is null or empty\r\n");
        return -1;
    }
    if(sp[0] != '(' && sp[0] != '{'){
        perror("data_head is error format\r\n");
        return -2;
    }
    else{
        //先解析指令段
        switch(count){
            case 1:
                strcat(dp,"05 00 01");
                break;
            case 2:
                strcat(dp,"15 00 02");
                break;
            default:
                sprintf(dp,"39 00 %02x",count);
                break;
        }
        dp += 8;

        //格式正确，开始过滤并解析
        //三个退出条件，一个不满足就退出
        while(sp[0] != ')' && sp[0] != '}' && sp[0] != ';'){
            //既不等于')',又不等于'}',且不等于';',开始循环
            //找'x'的位置
            if(sp[0] != 'x' && sp[0] != 'X'){
                //既不是'x'，也不是'X'
                sp++;
            }
            else{
                //找到了'x',或者是'X'
                //解析'x'或者'X'后面的数据
                if(isxdigit(*(sp+1)) && isxdigit(*(sp+2))){//是十六进制数字字符
                    /******************可能是字符串越界访问了******************/
                    sprintf(dp," %c%c",*(sp+1),*(sp+2));
                    dp+=3;
                    sp+=2;//源地址记得偏移，不然可能会导致越界访问
                }
                else{//'x'或者'X',后面不是十六进制字符
                    perror("data is error format\r\n");//报告数据异常格式
                    return -3;
                }
            }
        }
    }
    return 0;
}




/*
函数名称：check_raw_data_format
函数功能：检查生数据格式
函数参数：
    char *raw_buf --- 生数据地址缓冲空间
返回值：
    int 
    1.注释类信息
    2.空行
    3.规范序列格式
    其他 --- 未知格式
*/
int check_raw_data_format(char *raw_buf){

    //判断是否为注释
    if(raw_buf[0] == '/'){
        return 1;
    }
    //判断是否为空行
    else if(raw_buf[0] == '\n'){
        return 2;
    }

    //检查数据是否为延时数据格式
    int ret = check_delay_format(raw_buf);
    if(0 == ret){
        return 4;
    }
    
    //判断是否为标准序列格式
    //检查'('、'{',指令段与数据段份的分界符
    while(*raw_buf != '\0'){
        if(*raw_buf == '(' || *raw_buf == '{'){
            int num = count_of_bytes(raw_buf);
            if(num >= 1){
                return 3;
            }
        }
        raw_buf++;
    }

    return -1;
}



/*
函数名称：check_delay_format
函数功能：检查延时格式
函数参数：
    char *buf --- 数据空间首地址
返回值：
    int
    0 --- 是延时数据格式
    非0 --- 不是延时数据格式
*/
int check_delay_format(char *buf){
    //检查前5个字符，只要带有d、e、l、a、y无论大小写，默认为延时数据格式
    if(buf[0] == 'D' || buf[0] == 'd'){
        if(buf[1] == 'E' || buf[1] == 'e'){
            if(buf[2] == 'L' || buf[2] == 'l'){
                if(buf[3] == 'A' || buf[3] == 'a'){
                    if(buf[4] == 'Y' || buf[4] == 'y'){
                        return 0;
                    }
                }
            }
        }
    }

    return 1;
}



/*
函数名称：parse_delay_time
函数功能：解析延时时间数据
函数参数：
    char *buf --- 数据空间首地址
    char *cook_buf --- 熟缓冲空间
返回值：
    int
    0 --- 解析成功
    非0 --- 解析失败或出错
*/
int parse_delay_time(char *buf,char *cook_buf){
    char dig_buf[4] = {0};//10进制buf
    char dig_buf_16[3] = {0};//16进制buf
    char *digit_ptr = NULL;
    int num = 0;
    //找分界符
    while(*buf != '{' && *buf != '(' && *buf != '\0'){
        buf++;
    }
    if('\0' == *buf){//字符串结束,也未找到'{'、'('分界符
        perror("delay data is end,but not find { or (\r\n");
        return 1;
    }
    else{
        //找到了分界符号
        //检查为10进制还是16进制
        digit_ptr = buf;
        while(digit_ptr[0] != '}' && digit_ptr[0] != ')' && digit_ptr[0] != '\0'){
            if(*digit_ptr == 'x' || *digit_ptr == 'X'){
                //检查到后面的数据为16进制字符
                cook_buf[3] = *(digit_ptr+1);//直接将熟缓冲空间中存储的延时段数据修改掉
                cook_buf[4] = *(digit_ptr+2);
                return 0;
            }
            digit_ptr++;
        }
        if('\0' == digit_ptr[0]){
            //delay数据结束，但是并没有找到'}'或者')'
            perror("delay_data is end,but not find } or )\r\n");
            return 2;
        }
        
        digit_ptr = buf;//指向延时数据的首地址
        //遍历，寻找10进制数字字符
        while(digit_ptr[0] != '}' && digit_ptr[0] != ')' && digit_ptr[0] != '\0'){
            //如果是数字字符，就将其存储到定义的dig_buf中
            if(isdigit(*digit_ptr)){
                strncat(dig_buf,digit_ptr,1);
                num++;
            }
            if(num > 3){
                //超过3位数(0 --- 255)
                perror("delay_time is too long\r\n");
                return 3;
            }
            digit_ptr++;
        }

        transform_to_hex(dig_buf,dig_buf_16);//转化为16进制数字字符串
        cook_buf[3] = dig_buf_16[0];//直接修改值
        cook_buf[4] = dig_buf_16[1];
        return 0;
    }
    return 3;
}



/*
函数名称：transform_to_hex
函数功能：将10进制字符串转化为16进制字符串
函数参数：
    char *buf_10 --- 存储10进制数字字符串的空间
    char *buf_16 --- 存储16进制数字字符串的空间
返回值：
    void
*/
void transform_to_hex(char *buf_10,char *buf_16){
    //将十进制字符串转换为整数 --- strtol函数
    long decimalValue = strtol(buf_10, NULL, 10);

    //利用sprintf函数将整数形式的数据转换为ie16进制数字字符串，并存储
    snprintf(buf_16, 3, "%02lX", decimalValue);
}

/*
参考： --- 输入10进制数字字符串，转化为16进制数字字符串输出
    char hex_buf[3] = {0};
    char num[4] = {0};
    printf("请输入3位以内的10进制数字: \r\n");
    scanf("%s",num);

    //转化为16进制
    //一个字节大小 00 -- ff
    long decimalValue = strtol(num, NULL, 10);
    snprintf(hex_buf, 3, "%02lX", decimalValue);
    printf("转化为16进制数是: %s\r\n",hex_buf);
*/
