/********************************txz_yyds的mipi初始化序列转化程序*********************************/
#include "transform_mipi.h"

/*
编写思路:
    1.以只读方式打开提供的mipi初始化序列文件
    2.以可读可写、创建、覆盖写的方式打开存储转化后数据的熟文件
    while(1){
        //文件数据扫描部分
        3.读取一行数据存储到raw_buf
        4.再读取一行数据存储到next_buf
        5.判断是否读到文件结尾，到结尾则退出
        6.文件光标偏移确定下一次第一次读的是上一次第二次读的初始位置
        
        //数据格式检查
        7.检查第一行raw_buf数据格式
        8.检查第二行next_buf数据格式

        //根据数据检查部分的结果进行相应处理

        //清空raw_buf和next_buf缓冲空间
    }
    9.关闭熟文件
    10.关闭生文件
*/


//缓冲空间
char raw_buf[2048];//生缓冲空间 --- 第一行读取
char cooked_buf[2048];//写入到熟文件中的数据存储空间
char next_buf[2048];//第二行数据存储的缓冲空间
char temp_buf[2048];//中间出现空格后临时存储第一行解析后的熟数据缓冲空间
//标志位 --------- 注意标志位置一后，一定要清零
char white_coming = 0;//进入空白区域


//示例:./transform_mipi xxx.txt
//传入程序的第一个参数为mipi初始化序列文件名
int main(int argc,char *argv[]){
    int p_ret;//mipi生数据解析结果
    
    if(argc!= 2){
        perror("program argc is illegal\r\n");
        return ARGC_NUM_IS_ILLEGAL;
    }
    if(strcmp("",argv[1]) == 0){
        perror("input param is null\r\n");
        return INPUT_PARAM_IS_NULL;
    }
    FILE *raw = fopen(argv[1],"r+");
    if(NULL == raw){
        perror("open mipi init file error\r\n");
        return OPEN_RAW_FILE_ERR;
    }
    //输入到创建文件
    /*
    FILE *cook = fopen("./output/cooked_mipi_init.txt","w+");
    if(NULL == cook){
        perror("open or create cooked mipi init file error\r\n");
        return OPEN_COOKED_FILE_ERR;
    }
    */
   FILE *cook = stdout;//输出到终端

    while(1){
        /*****************************一次扫描两行*********************************/
        fgets(raw_buf,sizeof(raw_buf)-1,raw);//读取第一行
        //printf("raw_buf:%s\r\n",raw_buf);
        char *ret2 = fgets(next_buf,sizeof(next_buf)-1,raw);//读取第二行
        if(NULL == ret2){
            if(feof(raw)){
                break;//文件已经到达结尾
            }
            else{
                perror("read file error\r\n");
                return READ_RAW_FILE_ERR;
            }
        }
        //printf("next_buf:%s\r\n",next_buf);
        long position = ftell(raw);
        fseek(raw, position - strlen(next_buf), SEEK_SET);

        //printf("****两次扫描完成****\r\n");

        /*********************************数据格式检查************************************/
        /*
            1.注释类的信息
            2.空行
            3.规范序列格式
            4.延时数据格式
        */
        //第一行数据格式检查不能影响第二行格式数据检查
        int check_ret1 = check_raw_data_format(raw_buf);//第一次检查raw_buf中的数据格式
        int check_ret2 = check_raw_data_format(next_buf);//进行第二行格式检测
        
        //printf("check_ret1:%d\r\n",check_ret1);
        //printf("check_ret2:%d\r\n",check_ret2);
        /*************************后面根据前两行的格式检测进行数据的写入*************************/
        /*
        情况判断：0---规范序列 1---注释或空格 2---延时格式
        真值表：
            0,0 --- 第一行解析，第二行不解析,且正常写入数据
            0,1 --- 第一行解析，第二行不解析，且将第一行解析后的数据存储temp_buf空间
            0,2 --- 第一行解析，第二行解析，写入

            1,1 --- 第一行不解析，第二行不解析,不写入
            1,0 --- 第一行不解析，第二行不解析,不写入
            1,2 --- 第一行不解析，第二行解析，并写入

            2,0 --- 第一行不解析，第二行不解析,不写入
            2,1 --- 第一行不解析，第二行不解析,不写入
        */
        if(check_ret1 == 3 && check_ret2 == 3){//0,0
            /***第一行规范序列格式，第二行规范序列格式***/
            p_ret = parse_mipi_init_data(raw_buf,cooked_buf);
            if(0 > p_ret){
                perror("parse raw mipi init data error\r\n");
                return PARSE_RAW_DATA_ERR;
            }

            strcat(cooked_buf,"\n");
            fputs(cooked_buf,cook);
            //printf("***规范序列，规范序列***\r\n");
        }
        else if(check_ret1 == 3 && (check_ret2 == 1 || check_ret2 == 2)){//0,1
            /***第一行规范序列格式，第二行注释或空格***/
            p_ret = parse_mipi_init_data(raw_buf,cooked_buf);
            if(0 > p_ret){
                perror("parse raw mipi init data error\r\n");
                return PARSE_RAW_DATA_ERR;
            }

            strcpy(temp_buf,cooked_buf);//记住这一行的解析数据
            white_coming = 1;//进入空白
            //printf("***规范序列，注释空白***\r\n");
            /*............................................*/
        }
        else if(check_ret1 == 3 && check_ret2 == 4){//0,2
            /***第一行规范序列，第二行延时格式***/
            p_ret = parse_mipi_init_data(raw_buf,cooked_buf);
            if(0 > p_ret){
                perror("parse raw mipi init data error\r\n");
                return PARSE_RAW_DATA_ERR;
            }
            parse_delay_time(next_buf,cooked_buf);

            strcat(cooked_buf,"\n");
            fputs(cooked_buf,cook);
            //printf("***规范序列,延时格式***\r\n");
        }
        else if((check_ret1 == 1 || check_ret1 == 2) && check_ret2 == 4){//1,2
            /***第一行注释或空格，第二行延时格式***/
            //第一行不解析，第二行解析
            if(white_coming == 1){//判断进入空白标志位是否置一
                parse_delay_time(next_buf,temp_buf);//temp_buf记录着规格序列的那一行数据
                
                strcat(temp_buf,"\n");
                fputs(temp_buf,cook);
                memset(temp_buf,0,sizeof(temp_buf));
                white_coming = 0;//复位
                //printf("***注释空格，延时格式***\r\n");
            }
            /*........................................*/
        }
        else if((check_ret1 == 1 || check_ret1 == 2) && check_ret2 == 3){//1,0
            /***第一行为空行或注释，第二行为规范序列***/
            //判断进入空白标志位是否置一
            if(white_coming == 1){
                strcat(temp_buf,"\n");
                fputs(temp_buf,cook);
                memset(temp_buf,0,sizeof(temp_buf));
                white_coming = 0;//复位
                //printf("***注释空格,规范序列***\r\n");
            }
        }
        /*
        else if((check_ret1 == 1 || check_ret1 == 2) && (check_ret2 == 1 || check_ret2 == 2)){//1,1
            //第一行，第二行都为空行或注释，不做任何处理
        }
        else if(check_ret1 == 4 && check_ret2 == 3){//2,0
            //第一行延时，第二行规范序列
        }
        else if(check_ret1 == 4 && (check_ret2 == 1 || check_ret2 == 2)){//2,1
            //第一行延时格式，第二行空行或注释
        }
        */

        memset(raw_buf,0,sizeof(raw_buf));
        memset(cooked_buf,0,sizeof(cooked_buf));
    }
    /***********mipi初始化序列转换完成***********/
    //打开mipi默认屏参文件
    


    //关闭默认屏参文件



    //关闭转化好的文件
    if(0 != fclose(cook)){
        perror("close cooked mipi init file error");
        return CLOSE_COOKED_FILE_ERR;
    }
    //关闭mipi出是序列文件
    if(0 != fclose(raw)){
        perror("close mipi init file error");
        return CLOSE_RAW_FILE_ERR;
    }

    return 0;
}
