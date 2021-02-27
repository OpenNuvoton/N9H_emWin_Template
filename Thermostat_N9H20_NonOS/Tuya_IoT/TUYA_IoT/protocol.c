/**********************************Copyright (c)**********************************
**                       版权所有 (C), 2015-2020, 涂鸦科技
**
**                             http://www.tuya.com
**
*********************************************************************************/
/**
 * @file    protocol.c
 * @author  涂鸦综合协议开发组
 * @version v2.5.6
 * @date    2020.12.16
 * @brief                
 *                       *******非常重要，一定要看哦！！！********
 *          1. 用户在此文件中实现数据下发/上报功能
 *          2. DP的ID/TYPE及数据处理函数都需要用户按照实际定义实现
 *          3. 当开始某些宏定义后需要用户实现代码的函数内部有#err提示,完成函数后请删除该#err
 */

/****************************** 免责声明 ！！！ *******************************
由于MCU类型和编译环境多种多样，所以此代码仅供参考，用户请自行把控最终代码质量，
涂鸦不对MCU功能结果负责。
******************************************************************************/

/******************************************************************************
                                移植须知:
1:MCU必须在while中直接调用mcu_api.c内的wifi_uart_service()函数
2:程序正常初始化完成后,建议不进行关串口中断,如必须关中断,关中断时间必须短,关中断会引起串口数据包丢失
3:请勿在中断/定时器中断内调用上报函数
******************************************************************************/

#include "wifi.h"

extern void Uart_PutChar(unsigned char value);

extern volatile unsigned char wifi_connected;

#ifdef WEATHER_ENABLE
/**
 * @var    weather_choose
 * @brief  天气数据参数选择数组
 * @note   用户可以自定义需要的参数，注释或者取消注释即可，注意更改
 */
const char *weather_choose[WEATHER_CHOOSE_CNT] = {
    "temp",
    "humidity",
    "condition",
    "pm25",
    /*"pressure",
    "realFeel",
    "uvi",
    "tips",
    "windDir",
    "windLevel",
    "windSpeed",
    "sunRise",
    "sunSet",
    "aqi",
    "so2 ",
    "rank",
    "pm10",
    "o3",
    "no2",
    "co",
    "conditionNum",*/
};
#endif


/******************************************************************************
                              第一步:初始化
1:在需要使用到wifi相关文件的文件中include "wifi.h"
2:在MCU初始化中调用mcu_api.c文件中的wifi_protocol_init()函数
3:将MCU串口单字节发送函数填入protocol.c文件中uart_transmit_output函数内,并删除#error
4:在MCU串口接收函数中调用mcu_api.c文件内的uart_receive_input函数,并将接收到的字节作为参数传入
5:单片机进入while循环后调用mcu_api.c文件内的wifi_uart_service()函数
******************************************************************************/

/******************************************************************************
                        1:dp数据点序列类型对照表
          **此为自动生成代码,如在开发平台有相关修改请重新下载MCU_SDK**         
******************************************************************************/
const DOWNLOAD_CMD_S download_cmd[] =
{
  {DPID_SWITCH, DP_TYPE_BOOL},
  {DPID_TEMP_SET, DP_TYPE_VALUE},
  {DPID_TEMP_CURRENT, DP_TYPE_VALUE},
  {DPID_BACKLIGHT, DP_TYPE_VALUE},
  {DPID_MENU2_TEMP1, DP_TYPE_BOOL},
  {DPID_MENU2_TEMP1_MODBUS, DP_TYPE_BOOL},
  {DPID_MENU1_MODE1, DP_TYPE_BOOL},
  {DPID_MENU1_MODE1_MODE1, DP_TYPE_BOOL},
  {DPID_MENU1_MODE1_MODE2, DP_TYPE_BOOL},
  {DPID_MENU1_MODE1_MODE3, DP_TYPE_BOOL},
  {DPID_MENU, DP_TYPE_VALUE},
  {DPID_MENU3_DATE1, DP_TYPE_BOOL},
  {DPID_MENU3_DATE1_DATE1, DP_TYPE_BOOL},
  {DPID_MENU3_DATE1_DATE2, DP_TYPE_BOOL},
  {DPID_MENU3_DATE1_DATE3, DP_TYPE_BOOL},
  {DPID_MENU1_DEHUMIDIFY, DP_TYPE_BOOL},
  {DPID_MENU1_FAN, DP_TYPE_BOOL},
  {DPID_MENU1_COOL, DP_TYPE_BOOL},
  {DPID_MENU1_HEAT, DP_TYPE_BOOL},
  {DPID_MENU1_ONOFF_LEVEL, DP_TYPE_BOOL},
  {DPID_MENU1_INCREASE_LEVEL, DP_TYPE_VALUE},
  {DPID_MENU1_DECREASE_LEVEL, DP_TYPE_VALUE},
};



/******************************************************************************
                           2:串口单字节发送函数
请将MCU串口发送函数填入该函数内,并将接收到的数据作为参数传入串口发送函数
******************************************************************************/

/**
 * @brief  串口发送数据
 * @param[in] {value} 串口要发送的1字节数据
 * @return Null
 */
void uart_transmit_output(unsigned char value)
{
//    #error "请将MCU串口发送函数填入该函数,并删除该行"
    Uart_PutChar(value);

/*
    //Example:
    extern void Uart_PutChar(unsigned char value);
    Uart_PutChar(value);	                                //串口发送函数
*/
}

/******************************************************************************
                           第二步:实现具体用户函数
1:APP下发数据处理
2:数据上报处理
******************************************************************************/

/******************************************************************************
                            1:所有数据上报处理
当前函数处理全部数据上报(包括可下发/可上报和只上报)
  需要用户按照实际情况实现:
  1:需要实现可下发/可上报数据点上报
  2:需要实现只上报数据点上报
此函数为MCU内部必须调用
用户也可调用此函数实现全部数据上报
******************************************************************************/

//自动化生成数据上报函数

/**
 * @brief  系统所有dp点信息上传,实现APP和muc数据同步
 * @param  Null
 * @return Null
 * @note   此函数SDK内部需调用，MCU必须实现该函数内数据上报功能，包括只上报和可上报可下发型数据
 */

unsigned int menu2settemp_vflag;
unsigned int menu2settemp_v;
unsigned int menu2onoff_vflag;
unsigned int menu2onoff_v;

unsigned int menu2temp1_vflag;
unsigned int menu2temp1_v;
unsigned int menu2temp1modbus_vflag;
unsigned int menu2temp1modbus_v;
unsigned int menu2temp1backlight_v;

unsigned int menu1mode1_vflag;
unsigned int menu1mode1_v;
unsigned int menu1mode1mode1_vflag;
unsigned int menu1mode1mode1_v;
unsigned int menu1mode1mode2_vflag;
unsigned int menu1mode1mode2_v;
unsigned int menu1mode1mode3_vflag;
unsigned int menu1mode1mode3_v;

unsigned int menu3date1_vflag;
unsigned int menu3date1_v;
unsigned int menu3date1date1_vflag;
unsigned int menu3date1date1_v;
unsigned int menu3date1date2_vflag;
unsigned int menu3date1date2_v;
unsigned int menu3date1date3_vflag;
unsigned int menu3date1date3_v;

unsigned int menu123_vflag;
unsigned int menu123_v;

unsigned int menu1dehumidify_vflag;
unsigned int menu1dehumidify_v;
unsigned int menu1fan_vflag;
unsigned int menu1fan_v;
unsigned int menu1cool_vflag;
unsigned int menu1cool_v;
unsigned int menu1heat_vflag;
unsigned int menu1heat_v;
unsigned int menu1onofflevel_vflag;
unsigned int menu1onofflevel_v;
unsigned int menu1increaselevel_vflag;
unsigned int menu1increaselevel_v;
unsigned int menu1decreaselevel_vflag;
unsigned int menu1decreaselevel_v;

void all_data_update(void)
{
//    #error "请在此处理可下发可上报数据及只上报数据示例,处理完成后删除该行"
    /*
    //此代码为平台自动生成，请按照实际数据修改每个可下发可上报函数和只上报函数
    mcu_dp_bool_update(DPID_SWITCH,当前开关); //BOOL型数据上报;
    mcu_dp_value_update(DPID_TEMP_SET,当前目标温度); //VALUE型数据上报;
    mcu_dp_value_update(DPID_TEMP_CURRENT,当前当前温度); //VALUE型数据上报;
    mcu_dp_value_update(DPID_BACKLIGHT,当前背光亮度); //VALUE型数据上报;
    mcu_dp_bool_update(DPID_MENU2_TEMP1,当前Menu2_Temp1); //BOOL型数据上报;
    mcu_dp_bool_update(DPID_MENU2_TEMP1_MODBUS,当前Menu2_Temp1_Modbus); //BOOL型数据上报;
    mcu_dp_bool_update(DPID_MENU1_MODE1,当前Menu1_Mode1); //BOOL型数据上报;
    mcu_dp_bool_update(DPID_MENU1_MODE1_MODE1,当前Menu1_Mode1_Mode1); //BOOL型数据上报;
    mcu_dp_bool_update(DPID_MENU1_MODE1_MODE2,当前Menu1_Mode1_Mode2); //BOOL型数据上报;
    mcu_dp_bool_update(DPID_MENU1_MODE1_MODE3,当前Menu1_Mode1_Mode3); //BOOL型数据上报;
    mcu_dp_value_update(DPID_MENU,当前Menu); //VALUE型数据上报;
    mcu_dp_bool_update(DPID_MENU3_DATE1,当前Menu3_Date1); //BOOL型数据上报;
    mcu_dp_bool_update(DPID_MENU3_DATE1_DATE1,当前Menu3_Date1_Date1); //BOOL型数据上报;
    mcu_dp_bool_update(DPID_MENU3_DATE1_DATE2,当前Menu3_Date1_Date2); //BOOL型数据上报;
    mcu_dp_bool_update(DPID_MENU3_DATE1_DATE3,当前Menu3_Date1_Date3); //BOOL型数据上报;
    mcu_dp_bool_update(DPID_MENU1_DEHUMIDIFY,当前Menu1_Dehumidify); //BOOL型数据上报;
    mcu_dp_bool_update(DPID_MENU1_FAN,当前Menu1_Fan); //BOOL型数据上报;
    mcu_dp_bool_update(DPID_MENU1_COOL,当前Menu1_Cool); //BOOL型数据上报;
    mcu_dp_bool_update(DPID_MENU1_HEAT,当前Menu1_Heat); //BOOL型数据上报;
    mcu_dp_bool_update(DPID_MENU1_ONOFF_LEVEL,当前Menu1_OnOff_Level); //BOOL型数据上报;
    mcu_dp_value_update(DPID_MENU1_INCREASE_LEVEL,当前Menu1_Increase_Level); //VALUE型数据上报;
    mcu_dp_value_update(DPID_MENU1_DECREASE_LEVEL,当前Menu1_Decrease_Level); //VALUE型数据上报;

    */
    menu2onoff_v = 0;
	menu2onoff_vflag = 0;
	menu2temp1_vflag = 0;
	menu2temp1_v = 0;
    menu2temp1modbus_vflag = 0;
	menu2temp1modbus_v = 0;
	menu2settemp_v = 235;
    menu2settemp_vflag = 0;
    menu2temp1backlight_v = 50;
    menu123_vflag = 0;
    menu123_v = 2;

    mcu_dp_bool_update(DPID_SWITCH,menu2onoff_v); //BOOL型数据上报;
    mcu_dp_value_update(DPID_TEMP_SET,menu2settemp_v); //VALUE型数据上报;
    mcu_dp_value_update(DPID_TEMP_CURRENT,menu2settemp_v); //VALUE型数据上报;
    mcu_dp_value_update(DPID_BACKLIGHT,menu2temp1backlight_v); //VALUE型数据上报;
    mcu_dp_value_update(DPID_MENU,menu123_v); //VALUE型数据上报;
}


/******************************************************************************
                                WARNING!!!    
                            2:所有数据上报处理
自动化代码模板函数,具体请用户自行实现数据处理
******************************************************************************/
/*****************************************************************************
函数名称 : dp_download_switch_handle
功能描述 : 针对DPID_SWITCH的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static unsigned char dp_download_switch_handle(const unsigned char value[], unsigned short length)
{
    //示例:当前DP类型为BOOL
    unsigned char ret;
    //0:关/1:开
    unsigned char switch_1;

    menu2onoff_vflag = 1;
    switch_1 = mcu_get_dp_download_bool(value,length);
    menu2onoff_v = switch_1;
    if(switch_1 == 0) {
        //开关关
    }else {
        //开关开
    }
  
    //处理完DP数据后应有反馈
    ret = mcu_dp_bool_update(DPID_SWITCH,switch_1);
    if(ret == SUCCESS)
        return SUCCESS;
    else
        return ERROR;
}
/*****************************************************************************
函数名称 : dp_download_temp_set_handle
功能描述 : 针对DPID_TEMP_SET的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static unsigned char dp_download_temp_set_handle(const unsigned char value[], unsigned short length)
{
    //示例:当前DP类型为VALUE
    unsigned char ret;
    unsigned long temp_set;

    menu2settemp_vflag = 1;
    temp_set = mcu_get_dp_download_value(value,length);
    menu2settemp_v = temp_set;
    /*
    //VALUE类型数据处理
    
    */
    
    //处理完DP数据后应有反馈
    ret = mcu_dp_value_update(DPID_TEMP_SET,temp_set);
    if(ret == SUCCESS)
        return SUCCESS;
    else
        return ERROR;
}
/*****************************************************************************
函数名称 : dp_download_backlight_handle
功能描述 : 针对DPID_BACKLIGHT的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static unsigned char dp_download_backlight_handle(const unsigned char value[], unsigned short length)
{
    //示例:当前DP类型为VALUE
    unsigned char ret;
    unsigned long backlight;
    
    backlight = mcu_get_dp_download_value(value,length);
    /*
    //VALUE类型数据处理
    
    */
    menu2temp1backlight_v = backlight;

    //处理完DP数据后应有反馈
    ret = mcu_dp_value_update(DPID_BACKLIGHT,backlight);
    if(ret == SUCCESS)
        return SUCCESS;
    else
        return ERROR;
}
/*****************************************************************************
函数名称 : dp_download_menu2_temp1_handle
功能描述 : 针对DPID_MENU2_TEMP1的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static unsigned char dp_download_menu2_temp1_handle(const unsigned char value[], unsigned short length)
{
    //示例:当前DP类型为BOOL
    unsigned char ret;
    //0:关/1:开
    unsigned char menu2_temp1;

    menu2temp1_vflag = 1;
    menu2_temp1 = mcu_get_dp_download_bool(value,length);
    menu2temp1_v = menu2_temp1;
    if(menu2_temp1 == 0) {
        //开关关
    }else {
        //开关开
    }
  
    //处理完DP数据后应有反馈
    ret = mcu_dp_bool_update(DPID_MENU2_TEMP1,menu2_temp1);
    if(ret == SUCCESS)
        return SUCCESS;
    else
        return ERROR;
}
/*****************************************************************************
函数名称 : dp_download_menu2_temp1_modbus_handle
功能描述 : 针对DPID_MENU2_TEMP1_MODBUS的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static unsigned char dp_download_menu2_temp1_modbus_handle(const unsigned char value[], unsigned short length)
{
    //示例:当前DP类型为BOOL
    unsigned char ret;
    //0:关/1:开
    unsigned char menu2_temp1_modbus;

    menu2temp1modbus_vflag = 1;
    menu2_temp1_modbus = mcu_get_dp_download_bool(value,length);
    menu2temp1modbus_v = menu2_temp1_modbus;
    if(menu2_temp1_modbus == 0) {
        //开关关
    }else {
        //开关开
    }
  
    //处理完DP数据后应有反馈
    ret = mcu_dp_bool_update(DPID_MENU2_TEMP1_MODBUS,menu2_temp1_modbus);
    if(ret == SUCCESS)
        return SUCCESS;
    else
        return ERROR;
}
/*****************************************************************************
函数名称 : dp_download_menu1_mode1_handle
功能描述 : 针对DPID_MENU1_MODE1的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static unsigned char dp_download_menu1_mode1_handle(const unsigned char value[], unsigned short length)
{
    //示例:当前DP类型为BOOL
    unsigned char ret;
    //0:关/1:开
    unsigned char menu1_mode1;

    menu1mode1_vflag = 1;
    menu1_mode1 = mcu_get_dp_download_bool(value,length);
    menu1mode1_v = menu1_mode1;
    if(menu1_mode1 == 0) {
        //开关关
    }else {
        //开关开
    }
  
    //处理完DP数据后应有反馈
    ret = mcu_dp_bool_update(DPID_MENU1_MODE1,menu1_mode1);
    if(ret == SUCCESS)
        return SUCCESS;
    else
        return ERROR;
}
/*****************************************************************************
函数名称 : dp_download_menu1_mode1_mode1_handle
功能描述 : 针对DPID_MENU1_MODE1_MODE1的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static unsigned char dp_download_menu1_mode1_mode1_handle(const unsigned char value[], unsigned short length)
{
    //示例:当前DP类型为BOOL
    unsigned char ret;
    //0:关/1:开
    unsigned char menu1_mode1_mode1;

    menu1mode1mode1_vflag = 1;
    menu1_mode1_mode1 = mcu_get_dp_download_bool(value,length);
    menu1mode1mode1_v = menu1_mode1_mode1;
    if(menu1_mode1_mode1 == 0) {
        //开关关
    }else {
        //开关开
    }
  
    //处理完DP数据后应有反馈
    ret = mcu_dp_bool_update(DPID_MENU1_MODE1_MODE1,menu1_mode1_mode1);
    if(ret == SUCCESS)
        return SUCCESS;
    else
        return ERROR;
}
/*****************************************************************************
函数名称 : dp_download_menu1_mode1_mode2_handle
功能描述 : 针对DPID_MENU1_MODE1_MODE2的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static unsigned char dp_download_menu1_mode1_mode2_handle(const unsigned char value[], unsigned short length)
{
    //示例:当前DP类型为BOOL
    unsigned char ret;
    //0:关/1:开
    unsigned char menu1_mode1_mode2;

    menu1mode1mode2_vflag = 1;
    menu1_mode1_mode2 = mcu_get_dp_download_bool(value,length);
    menu1mode1mode2_v = menu1_mode1_mode2;
    if(menu1_mode1_mode2 == 0) {
        //开关关
    }else {
        //开关开
    }
  
    //处理完DP数据后应有反馈
    ret = mcu_dp_bool_update(DPID_MENU1_MODE1_MODE2,menu1_mode1_mode2);
    if(ret == SUCCESS)
        return SUCCESS;
    else
        return ERROR;
}
/*****************************************************************************
函数名称 : dp_download_menu1_mode1_mode3_handle
功能描述 : 针对DPID_MENU1_MODE1_MODE3的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static unsigned char dp_download_menu1_mode1_mode3_handle(const unsigned char value[], unsigned short length)
{
    //示例:当前DP类型为BOOL
    unsigned char ret;
    //0:关/1:开
    unsigned char menu1_mode1_mode3;

    menu1mode1mode3_vflag = 1;
    menu1_mode1_mode3 = mcu_get_dp_download_bool(value,length);
    menu1mode1mode3_v = menu1_mode1_mode3;
    if(menu1_mode1_mode3 == 0) {
        //开关关
    }else {
        //开关开
    }
  
    //处理完DP数据后应有反馈
    ret = mcu_dp_bool_update(DPID_MENU1_MODE1_MODE3,menu1_mode1_mode3);
    if(ret == SUCCESS)
        return SUCCESS;
    else
        return ERROR;
}
/*****************************************************************************
函数名称 : dp_download_menu_handle
功能描述 : 针对DPID_MENU的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static unsigned char dp_download_menu_handle(const unsigned char value[], unsigned short length)
{
    //示例:当前DP类型为VALUE
    unsigned char ret;
    unsigned long menu;

    menu123_vflag = 1;
    menu = mcu_get_dp_download_value(value,length);
    menu123_v = menu;
    /*
    //VALUE类型数据处理
    
    */
    
    //处理完DP数据后应有反馈
    ret = mcu_dp_value_update(DPID_MENU,menu);
    if(ret == SUCCESS)
        return SUCCESS;
    else
        return ERROR;
}
/*****************************************************************************
函数名称 : dp_download_menu3_date1_handle
功能描述 : 针对DPID_MENU3_DATE1的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static unsigned char dp_download_menu3_date1_handle(const unsigned char value[], unsigned short length)
{
    //示例:当前DP类型为BOOL
    unsigned char ret;
    //0:关/1:开
    unsigned char menu3_date1;

    menu3date1_vflag = 1;
    menu3_date1 = mcu_get_dp_download_bool(value,length);
    menu3date1_v = menu3_date1;
    if(menu3_date1 == 0) {
        //开关关
    }else {
        //开关开
    }
  
    //处理完DP数据后应有反馈
    ret = mcu_dp_bool_update(DPID_MENU3_DATE1,menu3_date1);
    if(ret == SUCCESS)
        return SUCCESS;
    else
        return ERROR;
}
/*****************************************************************************
函数名称 : dp_download_menu3_date1_date1_handle
功能描述 : 针对DPID_MENU3_DATE1_DATE1的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static unsigned char dp_download_menu3_date1_date1_handle(const unsigned char value[], unsigned short length)
{
    //示例:当前DP类型为BOOL
    unsigned char ret;
    //0:关/1:开
    unsigned char menu3_date1_date1;

    menu3date1date1_vflag = 1;
    menu3_date1_date1 = mcu_get_dp_download_bool(value,length);
    menu3date1date1_v = menu3_date1_date1;
    if(menu3_date1_date1 == 0) {
        //开关关
    }else {
        //开关开
    }
  
    //处理完DP数据后应有反馈
    ret = mcu_dp_bool_update(DPID_MENU3_DATE1_DATE1,menu3_date1_date1);
    if(ret == SUCCESS)
        return SUCCESS;
    else
        return ERROR;
}
/*****************************************************************************
函数名称 : dp_download_menu3_date1_date2_handle
功能描述 : 针对DPID_MENU3_DATE1_DATE2的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static unsigned char dp_download_menu3_date1_date2_handle(const unsigned char value[], unsigned short length)
{
    //示例:当前DP类型为BOOL
    unsigned char ret;
    //0:关/1:开
    unsigned char menu3_date1_date2;

    menu3date1date2_vflag = 1;
    menu3_date1_date2 = mcu_get_dp_download_bool(value,length);
    menu3date1date2_v = menu3_date1_date2;
    if(menu3_date1_date2 == 0) {
        //开关关
    }else {
        //开关开
    }
  
    //处理完DP数据后应有反馈
    ret = mcu_dp_bool_update(DPID_MENU3_DATE1_DATE2,menu3_date1_date2);
    if(ret == SUCCESS)
        return SUCCESS;
    else
        return ERROR;
}
/*****************************************************************************
函数名称 : dp_download_menu3_date1_date3_handle
功能描述 : 针对DPID_MENU3_DATE1_DATE3的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static unsigned char dp_download_menu3_date1_date3_handle(const unsigned char value[], unsigned short length)
{
    //示例:当前DP类型为BOOL
    unsigned char ret;
    //0:关/1:开
    unsigned char menu3_date1_date3;

    menu3date1date3_vflag = 1;
    menu3_date1_date3 = mcu_get_dp_download_bool(value,length);
    menu3date1date3_v = menu3_date1_date3;
    if(menu3_date1_date3 == 0) {
        //开关关
    }else {
        //开关开
    }
  
    //处理完DP数据后应有反馈
    ret = mcu_dp_bool_update(DPID_MENU3_DATE1_DATE3,menu3_date1_date3);
    if(ret == SUCCESS)
        return SUCCESS;
    else
        return ERROR;
}
/*****************************************************************************
函数名称 : dp_download_menu1_dehumidify_handle
功能描述 : 针对DPID_MENU1_DEHUMIDIFY的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static unsigned char dp_download_menu1_dehumidify_handle(const unsigned char value[], unsigned short length)
{
    //示例:当前DP类型为BOOL
    unsigned char ret;
    //0:关/1:开
    unsigned char menu1_dehumidify;

    menu1dehumidify_vflag = 1;
    menu1_dehumidify = mcu_get_dp_download_bool(value,length);
    menu1dehumidify_v = menu1_dehumidify;
    if(menu1_dehumidify == 0) {
        //开关关
    }else {
        //开关开
    }
  
    //处理完DP数据后应有反馈
    ret = mcu_dp_bool_update(DPID_MENU1_DEHUMIDIFY,menu1_dehumidify);
    if(ret == SUCCESS)
        return SUCCESS;
    else
        return ERROR;
}
/*****************************************************************************
函数名称 : dp_download_menu1_fan_handle
功能描述 : 针对DPID_MENU1_FAN的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static unsigned char dp_download_menu1_fan_handle(const unsigned char value[], unsigned short length)
{
    //示例:当前DP类型为BOOL
    unsigned char ret;
    //0:关/1:开
    unsigned char menu1_fan;

    menu1fan_vflag = 1;
    menu1_fan = mcu_get_dp_download_bool(value,length);
    menu1fan_v = menu1_fan;
    if(menu1_fan == 0) {
        //开关关
    }else {
        //开关开
    }
  
    //处理完DP数据后应有反馈
    ret = mcu_dp_bool_update(DPID_MENU1_FAN,menu1_fan);
    if(ret == SUCCESS)
        return SUCCESS;
    else
        return ERROR;
}
/*****************************************************************************
函数名称 : dp_download_menu1_cool_handle
功能描述 : 针对DPID_MENU1_COOL的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static unsigned char dp_download_menu1_cool_handle(const unsigned char value[], unsigned short length)
{
    //示例:当前DP类型为BOOL
    unsigned char ret;
    //0:关/1:开
    unsigned char menu1_cool;

    menu1cool_vflag = 1;
    menu1_cool = mcu_get_dp_download_bool(value,length);
    menu1cool_v = menu1_cool;
    if(menu1_cool == 0) {
        //开关关
    }else {
        //开关开
    }
  
    //处理完DP数据后应有反馈
    ret = mcu_dp_bool_update(DPID_MENU1_COOL,menu1_cool);
    if(ret == SUCCESS)
        return SUCCESS;
    else
        return ERROR;
}
/*****************************************************************************
函数名称 : dp_download_menu1_heat_handle
功能描述 : 针对DPID_MENU1_HEAT的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static unsigned char dp_download_menu1_heat_handle(const unsigned char value[], unsigned short length)
{
    //示例:当前DP类型为BOOL
    unsigned char ret;
    //0:关/1:开
    unsigned char menu1_heat;

    menu1heat_vflag = 1;
    menu1_heat = mcu_get_dp_download_bool(value,length);
    menu1heat_v = menu1_heat;
    if(menu1_heat == 0) {
        //开关关
    }else {
        //开关开
    }
  
    //处理完DP数据后应有反馈
    ret = mcu_dp_bool_update(DPID_MENU1_HEAT,menu1_heat);
    if(ret == SUCCESS)
        return SUCCESS;
    else
        return ERROR;
}
/*****************************************************************************
函数名称 : dp_download_menu1_onoff_level_handle
功能描述 : 针对DPID_MENU1_ONOFF_LEVEL的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static unsigned char dp_download_menu1_onoff_level_handle(const unsigned char value[], unsigned short length)
{
    //示例:当前DP类型为BOOL
    unsigned char ret;
    //0:关/1:开
    unsigned char menu1_onoff_level;

    menu1onofflevel_vflag = 1;
    menu1_onoff_level = mcu_get_dp_download_bool(value,length);
    menu1onofflevel_v = menu1_onoff_level;
    if(menu1_onoff_level == 0) {
        //开关关
    }else {
        //开关开
    }
  
    //处理完DP数据后应有反馈
    ret = mcu_dp_bool_update(DPID_MENU1_ONOFF_LEVEL,menu1_onoff_level);
    if(ret == SUCCESS)
        return SUCCESS;
    else
        return ERROR;
}
/*****************************************************************************
函数名称 : dp_download_menu1_increase_level_handle
功能描述 : 针对DPID_MENU1_INCREASE_LEVEL的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static unsigned char dp_download_menu1_increase_level_handle(const unsigned char value[], unsigned short length)
{
    //示例:当前DP类型为VALUE
    unsigned char ret;
    unsigned long menu1_increase_level;

    menu1increaselevel_vflag = 1;
    menu1_increase_level = mcu_get_dp_download_value(value,length);
    menu1increaselevel_v = menu1_increase_level;
    /*
    //VALUE类型数据处理
    
    */
    
    //处理完DP数据后应有反馈
    ret = mcu_dp_value_update(DPID_MENU1_INCREASE_LEVEL,menu1_increase_level);
    if(ret == SUCCESS)
        return SUCCESS;
    else
        return ERROR;
}
/*****************************************************************************
函数名称 : dp_download_menu1_decrease_level_handle
功能描述 : 针对DPID_MENU1_DECREASE_LEVEL的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static unsigned char dp_download_menu1_decrease_level_handle(const unsigned char value[], unsigned short length)
{
    //示例:当前DP类型为VALUE
    unsigned char ret;
    unsigned long menu1_decrease_level;

    menu1decreaselevel_vflag = 1;
    menu1_decrease_level = mcu_get_dp_download_value(value,length);
    menu1decreaselevel_v = menu1_decrease_level;
    /*
    //VALUE类型数据处理
    
    */
    
    //处理完DP数据后应有反馈
    ret = mcu_dp_value_update(DPID_MENU1_DECREASE_LEVEL,menu1_decrease_level);
    if(ret == SUCCESS)
        return SUCCESS;
    else
        return ERROR;
}




/******************************************************************************
                                WARNING!!!                     
此部分函数用户请勿修改!!
******************************************************************************/

/**
 * @brief  dp下发处理函数
 * @param[in] {dpid} dpid 序号
 * @param[in] {value} dp数据缓冲区地址
 * @param[in] {length} dp数据长度
 * @return dp处理结果
 * -           0(ERROR): 失败
 * -           1(SUCCESS): 成功
 * @note   该函数用户不能修改
 */
unsigned char dp_download_handle(unsigned char dpid,const unsigned char value[], unsigned short length)
{
    /*********************************
    当前函数处理可下发/可上报数据调用                    
    具体函数内需要实现下发数据处理
    完成用需要将处理结果反馈至APP端,否则APP会认为下发失败
    ***********************************/
    unsigned char ret;
    switch(dpid) {
        case DPID_SWITCH:
            //开关处理函数
            ret = dp_download_switch_handle(value,length);
        break;
        case DPID_TEMP_SET:
            //目标温度处理函数
            ret = dp_download_temp_set_handle(value,length);
        break;
        case DPID_BACKLIGHT:
            //背光亮度处理函数
            ret = dp_download_backlight_handle(value,length);
        break;
        case DPID_MENU2_TEMP1:
            //Menu2_Temp1处理函数
            ret = dp_download_menu2_temp1_handle(value,length);
        break;
        case DPID_MENU2_TEMP1_MODBUS:
            //Menu2_Temp1_Modbus处理函数
            ret = dp_download_menu2_temp1_modbus_handle(value,length);
        break;
        case DPID_MENU1_MODE1:
            //Menu1_Mode1处理函数
            ret = dp_download_menu1_mode1_handle(value,length);
        break;
        case DPID_MENU1_MODE1_MODE1:
            //Menu1_Mode1_Mode1处理函数
            ret = dp_download_menu1_mode1_mode1_handle(value,length);
        break;
        case DPID_MENU1_MODE1_MODE2:
            //Menu1_Mode1_Mode2处理函数
            ret = dp_download_menu1_mode1_mode2_handle(value,length);
        break;
        case DPID_MENU1_MODE1_MODE3:
            //Menu1_Mode1_Mode3处理函数
            ret = dp_download_menu1_mode1_mode3_handle(value,length);
        break;
        case DPID_MENU:
            //Menu处理函数
            ret = dp_download_menu_handle(value,length);
        break;
        case DPID_MENU3_DATE1:
            //Menu3_Date1处理函数
            ret = dp_download_menu3_date1_handle(value,length);
        break;
        case DPID_MENU3_DATE1_DATE1:
            //Menu3_Date1_Date1处理函数
            ret = dp_download_menu3_date1_date1_handle(value,length);
        break;
        case DPID_MENU3_DATE1_DATE2:
            //Menu3_Date1_Date2处理函数
            ret = dp_download_menu3_date1_date2_handle(value,length);
        break;
        case DPID_MENU3_DATE1_DATE3:
            //Menu3_Date1_Date3处理函数
            ret = dp_download_menu3_date1_date3_handle(value,length);
        break;
        case DPID_MENU1_DEHUMIDIFY:
            //Menu1_Dehumidify处理函数
            ret = dp_download_menu1_dehumidify_handle(value,length);
        break;
        case DPID_MENU1_FAN:
            //Menu1_Fan处理函数
            ret = dp_download_menu1_fan_handle(value,length);
        break;
        case DPID_MENU1_COOL:
            //Menu1_Cool处理函数
            ret = dp_download_menu1_cool_handle(value,length);
        break;
        case DPID_MENU1_HEAT:
            //Menu1_Heat处理函数
            ret = dp_download_menu1_heat_handle(value,length);
        break;
        case DPID_MENU1_ONOFF_LEVEL:
            //Menu1_OnOff_Level处理函数
            ret = dp_download_menu1_onoff_level_handle(value,length);
        break;
        case DPID_MENU1_INCREASE_LEVEL:
            //Menu1_Increase_Level处理函数
            ret = dp_download_menu1_increase_level_handle(value,length);
        break;
        case DPID_MENU1_DECREASE_LEVEL:
            //Menu1_Decrease_Level处理函数
            ret = dp_download_menu1_decrease_level_handle(value,length);
        break;

        
        default:
        break;
    }
    return ret;
}

/**
 * @brief  获取所有dp命令总和
 * @param[in] Null
 * @return 下发命令总和
 * @note   该函数用户不能修改
 */
unsigned char get_download_cmd_total(void)
{
    return(sizeof(download_cmd) / sizeof(download_cmd[0]));
}


/******************************************************************************
                                WARNING!!!                     
此代码为SDK内部调用,请按照实际dp数据实现函数内部数据
******************************************************************************/

#ifdef SUPPORT_MCU_FIRM_UPDATE
/**
 * @brief  升级包大小选择
 * @param[in] {package_sz} 升级包大小
 * @ref           0x00: 256byte (默认)
 * @ref           0x01: 512byte
 * @ref           0x02: 1024byte
 * @return Null
 * @note   MCU需要自行实现该功能
 */
void upgrade_package_choose(unsigned char package_sz)
{
    #error "请自行实现请自行实现升级包大小选择代码,完成后请删除该行"
    unsigned short send_len = 0;
    send_len = set_wifi_uart_byte(send_len, package_sz);
    wifi_uart_write_frame(UPDATE_START_CMD, MCU_TX_VER, send_len);
}

/**
 * @brief  MCU进入固件升级模式
 * @param[in] {value} 固件缓冲区
 * @param[in] {position} 当前数据包在于固件位置
 * @param[in] {length} 当前固件包长度(固件包长度为0时,表示固件包发送完成)
 * @return Null
 * @note   MCU需要自行实现该功能
 */
unsigned char mcu_firm_update_handle(const unsigned char value[],unsigned long position,unsigned short length)
{
    #error "请自行完成MCU固件升级代码,完成后请删除该行"
    if(length == 0) {
        //固件数据发送完成
      
    }else {
        //固件数据处理
      
    }
    
    return SUCCESS;
}
#endif

#ifdef SUPPORT_GREEN_TIME
/**
 * @brief  获取到的格林时间
 * @param[in] {time} 获取到的格林时间数据
 * @return Null
 * @note   MCU需要自行实现该功能
 */
void mcu_get_greentime(unsigned char time[])
{
//    #error "请自行完成相关代码,并删除该行"
    /*
    time[0] 为是否获取时间成功标志，为 0 表示失败，为 1表示成功
    time[1] 为年份，0x00 表示 2000 年
    time[2] 为月份，从 1 开始到12 结束
    time[3] 为日期，从 1 开始到31 结束
    time[4] 为时钟，从 0 开始到23 结束
    time[5] 为分钟，从 0 开始到59 结束
    time[6] 为秒钟，从 0 开始到59 结束
    */
    if(time[0] == 1) {
        //正确接收到wifi模块返回的格林数据
//        sysprintf("mcu_get_greentime success\n");
//        sysprintf("%d/%02d/%02d %02d:%02d:%02d\n",time[1], time[2], time[3], time[4], time[5], time[6]);
    }else {
        //获取格林时间出错,有可能是当前wifi模块未联网
//        sysprintf("mcu_get_greentime failure\n");
    }
}
#endif

#ifdef SUPPORT_MCU_RTC_CHECK
/**
 * @brief  MCU校对本地RTC时钟
 * @param[in] {time} 获取到的格林时间数据
 * @return Null
 * @note   MCU需要自行实现该功能
 */
extern void NVT_UpdateDate(unsigned char time[]);

void mcu_write_rtctime(unsigned char time[])
{
//    #error "请自行完成RTC时钟写入代码,并删除该行"
    /*
    Time[0] 为是否获取时间成功标志，为 0 表示失败，为 1表示成功
    Time[1] 为年份，0x00 表示 2000 年
    Time[2] 为月份，从 1 开始到12 结束
    Time[3] 为日期，从 1 开始到31 结束
    Time[4] 为时钟，从 0 开始到23 结束
    Time[5] 为分钟，从 0 开始到59 结束
    Time[6] 为秒钟，从 0 开始到59 结束
    Time[7] 为星期，从 1 开始到 7 结束，1代表星期一
   */
    if(time[0] == 1) {
        //正确接收到wifi模块返回的本地时钟数据
//        sysprintf("mcu_write_rtctime success\n");
//        sysprintf("20%d/%02d/%02d %02d:%02d:%02d\n",time[1], time[2], time[3], time[4], time[5], time[6]);
        NVT_UpdateDate(time);
    }else {
        //获取本地时钟数据出错,有可能是当前wifi模块未联网
//        sysprintf("mcu_write_rtctime failure\n");
    }
}
#endif

#ifdef WIFI_TEST_ENABLE
/**
 * @brief  wifi功能测试反馈
 * @param[in] {result} wifi功能测试结果
 * @ref       0: 失败
 * @ref       1: 成功
 * @param[in] {rssi} 测试成功表示wifi信号强度/测试失败表示错误类型
 * @return Null
 * @note   MCU需要自行实现该功能
 */
void wifi_test_result(unsigned char result,unsigned char rssi)
{
//    #error "请自行实现wifi功能测试成功/失败代码,完成后请删除该行"
    if(result == 0) {
        //测试失败
        if(rssi == 0x00) {
            //未扫描到名称为tuya_mdev_test路由器,请检查
        }else if(rssi == 0x01) {
            //模块未授权
        }
    }else {
        //测试成功
        //rssi为信号强度(0-100, 0信号最差，100信号最强)
    }
}
#endif

#ifdef WEATHER_ENABLE
/**
* @brief  mcu打开天气服务
 * @param  Null
 * @return Null
 */
void mcu_open_weather(void)
{
    int i = 0;
    char buffer[13] = {0};
    unsigned char weather_len = 0;
    unsigned short send_len = 0;
    
    weather_len = sizeof(weather_choose) / sizeof(weather_choose[0]);
      
    for(i=0;i<weather_len;i++) {
        buffer[0] = sprintf(buffer+1,"w.%s",weather_choose[i]);
        send_len = set_wifi_uart_buffer(send_len, (unsigned char *)buffer, buffer[0]+1);
    }
    
    #error "请根据提示，自行完善打开天气服务代码，完成后请删除该行"
    /*
    //当获取的参数有和时间有关的参数时(如:日出日落)，需要搭配t.unix或者t.local使用，需要获取的参数数据是按照格林时间还是本地时间
    buffer[0] = sprintf(buffer+1,"t.unix"); //格林时间   或使用  buffer[0] = sprintf(buffer+1,"t.local"); //本地时间
    send_len = set_wifi_uart_buffer(send_len, (unsigned char *)buffer, buffer[0]+1);
    */
    
    buffer[0] = sprintf(buffer+1,"w.date.%d",WEATHER_FORECAST_DAYS_NUM);
    send_len = set_wifi_uart_buffer(send_len, (unsigned char *)buffer, buffer[0]+1);
    
    wifi_uart_write_frame(WEATHER_OPEN_CMD, MCU_TX_VER, send_len);
}

/**
 * @brief  打开天气功能返回用户自处理函数
 * @param[in] {res} 打开天气功能返回结果
 * @ref       0: 失败
 * @ref       1: 成功
 * @param[in] {err} 错误码
 * @return Null
 * @note   MCU需要自行实现该功能
 */
void weather_open_return_handle(unsigned char res, unsigned char err)
{
    #error "请自行完成打开天气功能返回数据处理代码,完成后请删除该行"
    unsigned char err_num = 0;
    
    if(res == 1) {
        //打开天气返回成功
    }else if(res == 0) {
        //打开天气返回失败
        //获取错误码
        err_num = err; 
    }
}

/**
 * @brief  天气数据用户自处理函数
 * @param[in] {name} 参数名
 * @param[in] {type} 参数类型
 * @ref       0: int 型
 * @ref       1: string 型
 * @param[in] {data} 参数值的地址
 * @param[in] {day} 哪一天的天气  0:表示当天 取值范围: 0~6
 * @ref       0: 今天
 * @ref       1: 明天
 * @return Null
 * @note   MCU需要自行实现该功能
 */
void weather_data_user_handle(char *name, unsigned char type, const unsigned char *data, char day)
{
    #error "这里仅给出示例，请自行完善天气数据处理代码,完成后请删除该行"
    int value_int;
    char value_string[50];//由于有的参数内容较多，这里默认为50。您可以根据定义的参数，可以适当减少该值
    
    my_memset(value_string, '\0', 50);
    
    //首先获取数据类型
    if(type == 0) { //参数是INT型
        value_int = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
    }else if(type == 1) {
        my_strcpy(value_string, data);
    }
    
    //注意要根据所选参数类型来获得参数值！！！
    if(my_strcmp(name, "temp") == 0) {
        printf("day:%d temp value is:%d\r\n", day, value_int);          //int 型
    }else if(my_strcmp(name, "humidity") == 0) {
        printf("day:%d humidity value is:%d\r\n", day, value_int);      //int 型
    }else if(my_strcmp(name, "pm25") == 0) {
        printf("day:%d pm25 value is:%d\r\n", day, value_int);          //int 型
    }else if(my_strcmp(name, "condition") == 0) {
        printf("day:%d condition value is:%s\r\n", day, value_string);  //string 型
    }
}
#endif

#ifdef MCU_DP_UPLOAD_SYN
/**
 * @brief  状态同步上报结果
 * @param[in] {result} 结果
 * @ref       0: 失败
 * @ref       1: 成功
 * @return Null
 * @note   MCU需要自行实现该功能
 */
void get_upload_syn_result(unsigned char result)
{
    #error "请自行完成状态同步上报结果代码,并删除该行"
      
    if(result == 0) {
        //同步上报出错
    }else {
        //同步上报成功
    }
}
#endif

#ifdef GET_WIFI_STATUS_ENABLE
/**
 * @brief  获取 WIFI 状态结果
 * @param[in] {result} 指示 WIFI 工作状态
 * @ref       0x00: wifi状态 1 smartconfig 配置状态
 * @ref       0x01: wifi状态 2 AP 配置状态
 * @ref       0x02: wifi状态 3 WIFI 已配置但未连上路由器
 * @ref       0x03: wifi状态 4 WIFI 已配置且连上路由器
 * @ref       0x04: wifi状态 5 已连上路由器且连接到云端
 * @ref       0x05: wifi状态 6 WIFI 设备处于低功耗模式
 * @ref       0x06: wifi状态 7 WIFI 设备处于smartconfig&AP配置状态
 * @return Null
 * @note   MCU需要自行实现该功能
 */
void get_wifi_status(unsigned char result)
{
    static int get_wifi_status_v;
//  #error "请自行完成获取 WIFI 状态结果代码,并删除该行"
    wifi_connected = result;
//    if (get_wifi_status_v == 0)
//    sysprintf("wifi_connected = %d\n", wifi_connected);

    switch(result) {
        case 0:
            //wifi工作状态1
        break;
    
        case 1:
            //wifi工作状态2
        break;
        
        case 2:
            //wifi工作状态3
        break;
        
        case 3:
            //wifi工作状态4
        break;
        
        case 4:
            //wifi工作状态5
            if (get_wifi_status_v == 0)
            {
                get_wifi_status_v = 1;
//                sysprintf("The router is connected and connected to the cloud\n");
            }
        break;
        
        case 5:
            //wifi工作状态6
        break;
      
        case 6:
            //wifi工作状态7
        break;
        
        default:break;
    }
}
#endif

#ifdef WIFI_STREAM_ENABLE
/**
 * @brief  流服务发送结果
 * @param[in] {result} 结果
 * @ref       0x00: 成功
 * @ref       0x01: 流服务功能未开启
 * @ref       0x02: 流服务器未连接成功
 * @ref       0x03: 数据推送超时
 * @ref       0x04: 传输的数据长度错误
 * @return Null
 * @note   MCU需要自行实现该功能
 */
void stream_trans_send_result(unsigned char result)
{
    #error "这里仅给出示例，请自行完善流服务发送结果处理代码,完成后请删除该行"
    switch(result) {
        case 0x00:
            //成功
        break;
        
        case 0x01:
            //流服务功能未开启
        break;
        
        case 0x02:
            //流服务器未连接成功
        break;
        
        case 0x03:
            //数据推送超时
        break;
        
        case 0x04:
            //传输的数据长度错误
        break;
        
        default:break;
    }
}

/**
 * @brief  多地图流服务发送结果
 * @param[in] {result} 结果
 * @ref       0x00: 成功
 * @ref       0x01: 失败
 * @return Null
 * @note   MCU需要自行实现该功能
 */
void maps_stream_trans_send_result(unsigned char result)
{
    #error "这里仅给出示例，请自行完善多地图流服务发送结果处理代码,完成后请删除该行"
    switch(result) {
        case 0x00:
            //成功
        break;
        
        case 0x01:
            //失败
        break;
        
        default:break;
    }
}
#endif

#ifdef WIFI_CONNECT_TEST_ENABLE
/**
 * @brief  路由信息接收结果通知
 * @param[in] {result} 模块是否成功接收到正确的路由信息
 * @ref       0x00: 失败
 * @ref       0x01: 成功
 * @return Null
 * @note   MCU需要自行实现该功能
 */
void wifi_connect_test_result(unsigned char result)
{
    #error "请自行实现wifi功能测试成功/失败代码,完成后请删除该行"
    if(result == 0) {
        //路由信息接收失败，请检查发出的路由信息包是否是完整的JSON数据包
    }else {
        //路由信息接收成功，产测结果请注意WIFI_STATE_CMD指令的wifi工作状态
    }
}
#endif

#ifdef GET_MODULE_MAC_ENABLE
/**
 * @brief  获取模块mac结果
 * @param[in] {mac} 模块 MAC 数据
 * @ref       mac[0]: 为是否获取mac成功标志，0x00 表示成功，0x01 表示失败
 * @ref       mac[1]~mac[6]: 当获取 MAC地址标志位如果mac[0]为成功，则表示模块有效的MAC地址
 * @return Null
 * @note   MCU需要自行实现该功能
 */
void mcu_get_mac(unsigned char mac[])
{
    #error "请自行完成mac获取代码,并删除该行"
    /*
    mac[0]为是否获取mac成功标志，0x00 表示成功，为0x01表示失败
    mac[1]~mac[6]:当获取 MAC地址标志位如果mac[0]为成功，则表示模块有效的MAC地址
   */
   
    if(mac[0] == 1) {
        //获取mac出错
    }else {
        //正确接收到wifi模块返回的mac地址
    }
}
#endif

#ifdef GET_IR_STATUS_ENABLE
/**
 * @brief  获取红外状态结果
 * @param[in] {result} 指示红外状态
 * @ref       0x00: 红外状态 1 正在发送红外码
 * @ref       0x01: 红外状态 2 发送红外码结束
 * @ref       0x02: 红外状态 3 红外学习开始
 * @ref       0x03: 红外状态 4 红外学习结束
 * @return Null
 * @note   MCU需要自行实现该功能
 */
void get_ir_status(unsigned char result)
{
    #error "请自行完成红外状态代码,并删除该行"
    switch(result) {
        case 0:
            //红外状态 1
        break;
      
        case 1:
            //红外状态 2
        break;
          
        case 2:
            //红外状态 3
        break;
          
        case 3:
            //红外状态 4
        break;
          
        default:break;
    }
    
    wifi_uart_write_frame(GET_IR_STATUS_CMD, MCU_TX_VER, 0);
}
#endif

#ifdef IR_TX_RX_TEST_ENABLE
/**
 * @brief  红外进入收发产测结果通知
 * @param[in] {result} 模块是否成功接收到正确的信息
 * @ref       0x00: 失败
 * @ref       0x01: 成功
 * @return Null
 * @note   MCU需要自行实现该功能
 */
void ir_tx_rx_test_result(unsigned char result)
{
    #error "请自行实现红外进入收发产测功能测试成功/失败代码,完成后请删除该行"
    if(result == 0) {
        //红外进入收发产测成功
    }else {
        //红外进入收发产测失败，请检查发出的数据包
    }
}
#endif

#ifdef FILE_DOWNLOAD_ENABLE
/**
 * @brief  文件下载包大小选择
 * @param[in] {package_sz} 文件下载包大小
 * @ref       0x00: 256 byte (默认)
 * @ref       0x01: 512 byte
 * @ref       0x02: 1024 byte
 * @return Null
 * @note   MCU需要自行实现该功能
 */
void file_download_package_choose(unsigned char package_sz)
{
    #error "请自行实现请自行实现文件下载包大小选择代码,完成后请删除该行"
    unsigned short send_len = 0;
    send_len = set_wifi_uart_byte(send_len, package_sz);
    wifi_uart_write_frame(FILE_DOWNLOAD_START_CMD, MCU_TX_VER, send_len);
}

/**
 * @brief  文件包下载模式
 * @param[in] {value} 数据缓冲区
 * @param[in] {position} 当前数据包在于文件位置
 * @param[in] {length} 当前文件包长度(长度为0时,表示文件包发送完成)
 * @return 数据处理结果
 * -           0(ERROR): 失败
 * -           1(SUCCESS): 成功
 * @note   MCU需要自行实现该功能
 */
unsigned char file_download_handle(const unsigned char value[],unsigned long position,unsigned short length)
{
    #error "请自行完成文件包下载代码,完成后请删除该行"
    if(length == 0) {
        //文件包数据发送完成
        
    }else {
        //文件包数据处理
      
    }
    
    return SUCCESS;
}
#endif

#ifdef MODULE_EXPANDING_SERVICE_ENABLE
/**
 * @brief  打开模块时间服务通知结果
 * @param[in] {value} 数据缓冲区
 * @param[in] {length} 数据长度
 * @return Null
 * @note   MCU需要自行实现该功能
 */
void open_module_time_serve_result(const unsigned char value[], unsigned short length)
{
    #error "请自行实现模块时间服务通知结果代码,完成后请删除该行"
    unsigned char sub_cmd = value[0];
    
    switch(sub_cmd) {
        case 0x01: { //子命令  打开模块时间服务通知
            if(0x02 != length) {
                //数据长度错误
                return;
            }
            
            if(value[1] == 0) {
                //服务开启成功
            }else {
                //服务开启失败
            }
        }
        break;
        
        case 0x02: {  //子命令  模块时间服务通知
            if(0x09 != length) {
                //数据长度错误
                return;
            }
            
            unsigned char time_type = value[1]; //0x00:格林时间  0x01:本地时间
            unsigned char time_data[7];
            
            my_memcpy(time_data, value + 2, length - 2);
            /*
            Data[0]为年份, 0x00表示2000年
            Data[1]为月份，从1开始到12结束
            Data[2]为日期，从1开始到31结束
            Data[3]为时钟，从0开始到23结束
            Data[4]为分钟，从0开始到59结束
            Data[5]为秒钟，从0开始到15结束
            Data[6]为星期，从1开始到7结束，1代表星期一
            */
            
            //在此处添加时间数据处理代码，time_type为时间类型
            
            unsigned short send_len = 0;
            send_len = set_wifi_uart_byte(send_len,sub_cmd);
            wifi_uart_write_frame(MODULE_EXTEND_FUN_CMD, MCU_TX_VER, send_len);
        }
        break;
        
        case 0x03: {  //子命令  主动请求天气服务数据
            if(0x02 != length) {
                //数据长度错误
                return;
            }
            
            if(value[1] == 0) {
                //成功
            }else {
                //失败
            }
        }
        break;
        
        case 0x04: {  //子命令  打开模块重置状态通知
            if(0x02 != length) {
                //数据长度错误
                return;
            }
            
            if(value[1] == 0) {
                //成功
            }else {
                //失败
            }
        }
        break;
        
        case 0x05: {  //子命令  模块重置状态通知
            if(0x02 != length) {
                //数据长度错误
                return;
            }
            
            switch(value[1]) {
                case 0x00:
                    //模块本地重置
                    
                break;
                case 0x01:
                    //APP远程重置
                    
                break;
                case 0x02:
                    //APP恢复出厂重置
                    
                break;
                default:break;
            }
            
            unsigned short send_len = 0;
            send_len = set_wifi_uart_byte(send_len, sub_cmd);
            wifi_uart_write_frame(MODULE_EXTEND_FUN_CMD, MCU_TX_VER, send_len);
        }
        break;
        
        default:break;
    }
}
#endif

#ifdef BLE_RELATED_FUNCTION_ENABLE
/**
 * @brief  蓝牙功能性测试结果
 * @param[in] {value} 数据缓冲区
 * @param[in] {length} 数据长度
 * @return Null
 * @note   MCU需要自行实现该功能
 */
void BLE_test_result(const unsigned char value[], unsigned short length)
{
    #error "请自行实现蓝牙功能性测试结果代码,完成后请删除该行"
    unsigned char sub_cmd = value[0];
    
    if(0x03 != length) {
        //数据长度错误
        return;
    }
    
    if(0x01 != sub_cmd) {
        //子命令错误
        return;
    }
    
    unsigned char result = value[1];
    unsigned char rssi = value[2];
        
    if(result == 0) {
        //测试失败
        if(rssi == 0x00) {
            //未扫描到名称为 ty_mdev蓝牙信标,请检查
        }else if(rssi == 0x01) {
            //模块未授权
        }
    }else if(result == 0x01) {
        //测试成功
        //rssi为信号强度(0-100, 0信号最差，100信号最强)
    }
}
#endif

#ifdef VOICE_MODULE_PROTOCOL_ENABLE
/**
 * @brief  获取语音状态码结果
 * @param[in] {result} 语音状态码
 * @ref       0x00: 空闲
 * @ref       0x01: mic静音状态
 * @ref       0x02: 唤醒
 * @ref       0x03: 正在录音
 * @ref       0x04: 正在识别
 * @ref       0x05: 识别成功
 * @ref       0x06: 识别失败
 * @return Null
 * @note   MCU需要自行实现该功能
 */
void get_voice_state_result(unsigned char result)
{
    #error "请自行实现获取语音状态码结果处理代码,完成后请删除该行"
    switch(result) {
        case 0:
            //空闲
        break;
    
        case 1:
            //mic静音状态
        break;
        
        case 2:
            //唤醒
        break;
        
        case 3:
            //正在录音
        break;
        
        case 4:
            //正在识别
        break;
    
        case 5:
            //识别成功
        break;
        
        case 6:
            //识别失败
        break;
        
      default:break;
    }
}

/**
 * @brief  MIC静音设置结果
 * @param[in] {result} 语音状态码
 * @ref       0x00: mic 开启
 * @ref       0x01: mic 静音
 * @return Null
 * @note   MCU需要自行实现该功能
 */
void set_voice_MIC_silence_result(unsigned char result)
{
    #error "请自行实现MIC静音设置处理代码,完成后请删除该行"
    if(result == 0) {
        //mic 开启
    }else {
        //mic 静音
    }
}

/**
 * @brief  speaker音量设置结果
 * @param[in] {result} 音量值
 * @ref       0~10: 音量范围
 * @return Null
 * @note   MCU需要自行实现该功能
 */
void set_speaker_voice_result(unsigned char result)
{
    #error "请自行实现speaker音量设置结果处理代码,完成后请删除该行"
    
}

/**
 * @brief  音频产测结果
 * @param[in] {result} 音频产测状态
 * @ref       0x00: 关闭音频产测
 * @ref       0x01: mic1音频环路测试
 * @ref       0x02: mic2音频环路测试
 * @return Null
 * @note   MCU需要自行实现该功能
 */
void voice_test_result(unsigned char result)
{
    #error "请自行实现音频产测结果处理代码,完成后请删除该行"
    if(result == 0x00) {
        //关闭音频产测
    }else if(result == 0x01) {
        //mic1音频环路测试
    }else if(result == 0x02) {
        //mic2音频环路测试
    }
}

/**
 * @brief  唤醒产测结果
 * @param[in] {result} 唤醒返回值
 * @ref       0x00: 唤醒成功
 * @ref       0x01: 唤醒失败(10s超时失败)
 * @return Null
 * @note   MCU需要自行实现该功能
 */
void voice_awaken_test_result(unsigned char result)
{
    #error "请自行实现唤醒产测结果处理代码,完成后请删除该行"
    if(result == 0x00) {
        //唤醒成功
    }else if(result == 0x01) {
        //唤醒失败
    }
}

/**
 * @brief  语音模组扩展功能
 * @param[in] {value} 数据缓冲区
 * @param[in] {length} 数据长度
 * @return Null
 * @note   MCU需要自行实现该功能
 */
void voice_module_extend_fun(const unsigned char value[], unsigned short length)
{
    unsigned char sub_cmd = value[0];
    unsigned char play;
    unsigned char bt_play;
    unsigned short send_len = 0;
  
    switch(sub_cmd) {
        case 0x00: { //子命令  MCU功能设置
            if(0x02 != length) {
                //数据长度错误
                return;
            }
            
            if(value[1] == 0) {
                //成功
            }else {
                //失败
            }
        }
        break;
        
        case 0x01: {  //子命令  状态通知
            if(0x02 > length) {
                //数据长度错误
                return;
            }
            
            unsigned char play = 0xff;
            unsigned char bt_play = 0xff;
            
            const char *str_buff = (const char *)&value[1];
            const char *str_result = NULL;
            
            str_result = strstr(str_buff,"play") + my_strlen("play") + 2;
            if(NULL == str_result) {
                //数据错误
                goto ERR_EXTI;
            }
            
            if(0 == memcmp(str_result, "true", my_strlen("true"))) {
                play = 1;
            }else if(0 == memcmp(str_result, "false", my_strlen("false"))) {
                play = 0;
            }else {
                //数据错误
                goto ERR_EXTI;
            }
            
            str_result = strstr(str_buff,"bt_play") + my_strlen("bt_play") + 2;
            if(NULL == str_result) {
                //数据错误
                goto ERR_EXTI;
            }
            
            if(0 == memcmp(str_result, "true", my_strlen("true"))) {
                bt_play = 1;
            }else if(0 == memcmp(str_result, "false", my_strlen("false"))) {
                bt_play = 0;
            }else {
                //数据错误
                goto ERR_EXTI;
            }
            
            #error "请自行实现语音模组状态通知处理代码,完成后请删除该行"
            //MCU设置暂仅支持”播放/暂停” ”蓝牙开关”
            //play    播放/暂停功能  1(播放) / 0(暂停)
            //bt_play 蓝牙开关功能   1(开)   / 0(关)
            
            
            
            send_len = 0;
            send_len = set_wifi_uart_byte(send_len, sub_cmd);
            send_len = set_wifi_uart_byte(send_len, 0x00);
            wifi_uart_write_frame(MODULE_EXTEND_FUN_CMD, MCU_TX_VER, send_len);
        }
        break;

        default:break;
    }
    
    return;

ERR_EXTI:
    send_len = 0;
    send_len = set_wifi_uart_byte(send_len, sub_cmd);
    send_len = set_wifi_uart_byte(send_len, 0x01);
    wifi_uart_write_frame(MODULE_EXTEND_FUN_CMD, MCU_TX_VER, send_len);
    return;
}
#endif




