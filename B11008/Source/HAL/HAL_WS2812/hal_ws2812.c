/************************************************************************************************************************************************
* 文件名称: hal_ws2812.c
* 文件描述: ws2812相关驱动封装函数
* 维护人员: bo.liangmin
* 版本信息: 20240406--代码创建
************************************************************************************************************************************************/



/************************************************************************************************************************************************
* include
************************************************************************************************************************************************/
#include "string.h"
#include "led_strip.h"
#include "sys_common.h"
#include "hal_ws2812.h"



/************************************************************************************************************************************************
* define
************************************************************************************************************************************************/
/* WS2812灯带控制句柄 */
static led_strip_handle_t hal_ws2812_handle[HAL_WS2812_CHANNELS];

/* WS2812灯带控制管脚 */
static uint8_t hal_ws2812_rgbPin[HAL_WS2812_CHANNELS] =
{
    SYS_RGB_CLK_PIN, SYS_RGB_HT_PIN, SYS_RGB_DAY_PIN, SYS_RGB_FUNC_PIN
};

/* WS2812灯带灯珠数量 */
static uint8_t hal_ws2812_rgbNum[HAL_WS2812_CHANNELS] =
{
    HAL_WS2812_CLK_RGB_NUM, HAL_WS2812_HT_RGB_NUM, HAL_WS2812_DAY_RGB_NUM, HAL_WS2812_FUN_RGB_NUM
};

/* WS2812灯带显示矩阵 */
static uint8_t hal_ws2812_clk_showMatrix[HAL_WS2812_CLK_RGB_NUM] = { 0 };
static uint8_t hal_ws2812_ht_showMatrix[HAL_WS2812_HT_RGB_NUM]   = { 0 };
static uint8_t hal_ws2812_day_showMatrix[HAL_WS2812_HT_RGB_NUM]  = { 0 };
static uint8_t hal_ws2812_fun_showMatrix[HAL_WS2812_FUN_RGB_NUM] = { 0 };

/* 数字显示矩阵 */
const uint8_t hal_ws2812_numMatrix[10][7] = 
{
/*   0  1  2  3  4  5  6   */
    {1, 1, 1, 1, 1, 0, 1},       // num 0
    {0, 0, 0, 0, 1, 0, 1},       // num 1
    {1, 1, 0, 1, 1, 1, 0},       // num 2
    {1, 0, 0, 1, 1, 1, 1},       // num 3
    {0, 0, 1, 0, 1, 1, 1},       // num 4
    {1, 0, 1, 1, 0, 1, 1},       // num 5
    {1, 1, 1, 1, 0, 1, 1},       // num 6
    {0, 0, 0, 1, 1, 0, 1},       // num 7
    {1, 1, 1, 1, 1, 1, 1},       // num 8
    {1, 0, 1, 1, 1, 1, 1},       // num 9
};



/************************************************************************************************************************************************
* 函数名称: void HAL_WS2812_Init(void)
* 函数说明: 设备初始化
* 输入参数: 无
* 输出参数: 无
* 返回数值: 无
************************************************************************************************************************************************/
void HAL_WS2812_Init(void)
{
    uint8_t i = 0;
    led_strip_config_t strip_config   = { 0 };
    led_strip_rmt_config_t rmt_config = { 0 };

    rmt_config.resolution_hz = 10 * 1000 * 1000;        // 10MHz

    for(i=0; i<HAL_WS2812_CHANNELS; i++)
    {
        strip_config.strip_gpio_num = hal_ws2812_rgbPin[i];
        strip_config.max_leds       = hal_ws2812_rgbNum[i];

        led_strip_new_rmt_device(&strip_config, &rmt_config, &hal_ws2812_handle[i]);
        led_strip_clear(hal_ws2812_handle[i]);
    }
}

/************************************************************************************************************************************************
* 函数名称: void HAL_WS2812_SetShowType(show_type_t show_type)
* 函数说明: 设置中央区域显示时间or日期
* 输入参数: 无
* 输出参数: 无
* 返回数值: 无
************************************************************************************************************************************************/
void HAL_WS2812_SetShowType(show_type_t show_type)
{
    if(TYPE_DATE == show_type)
    {
        hal_ws2812_clk_showMatrix[HAL_WS2812_MIDDLE_ADDR    ] = 1;
        hal_ws2812_clk_showMatrix[HAL_WS2812_MIDDLE_ADDR + 1] = 0;
        hal_ws2812_clk_showMatrix[HAL_WS2812_MIDDLE_ADDR + 2] = 0;
    }
    else if(TYPE_TIME == show_type)
    {
        hal_ws2812_clk_showMatrix[HAL_WS2812_MIDDLE_ADDR    ] = 0;
        hal_ws2812_clk_showMatrix[HAL_WS2812_MIDDLE_ADDR + 1] = 1;
        hal_ws2812_clk_showMatrix[HAL_WS2812_MIDDLE_ADDR + 2] = 1;
    }
    else
    {
        hal_ws2812_clk_showMatrix[HAL_WS2812_MIDDLE_ADDR    ] = 0;
        hal_ws2812_clk_showMatrix[HAL_WS2812_MIDDLE_ADDR + 1] = 0;
        hal_ws2812_clk_showMatrix[HAL_WS2812_MIDDLE_ADDR + 2] = 0;
    }
}

/************************************************************************************************************************************************
* 函数名称: void HAL_WS2812_SetShowNum(uint8_t addr, uint8_t showNum)
* 函数说明: 数字显示
* 输入参数: 无
* 输出参数: 无
* 返回数值: 无
************************************************************************************************************************************************/
void HAL_WS2812_SetShowNum(uint8_t addr, uint8_t showNum)
{
    uint8_t i = 0;

    for(i=0; i<7; i++)
    {
        hal_ws2812_clk_showMatrix[addr + i] = hal_ws2812_numMatrix[showNum][i];
    }
//
//    memcpy(&hal_ws2812_clk_showMatrix[addr], &hal_ws2812_numMatrix[showNum], 7);
}

/************************************************************************************************************************************************
* 函数名称: void HAL_WS2812_ClkFresh(ws2812_color_t color)
* 函数说明: 中央CLK区域显示
* 输入参数: 无
* 输出参数: 无
* 返回数值: 无
************************************************************************************************************************************************/
void HAL_WS2812_ClkFresh(ws2812_color_t color)
{
    uint8_t i = 0 ;

    for(i=0; i<HAL_WS2812_CLK_RGB_NUM; i++)
    {
        if(1 == hal_ws2812_clk_showMatrix[i])
        {
            led_strip_set_pixel(hal_ws2812_handle[WS2812_CHANNEL_CLK], i, color.red, color.green, color.blue);
        }
        else
        {
            led_strip_set_pixel(hal_ws2812_handle[WS2812_CHANNEL_CLK], i, 0, 0, 0);
        }
    }

    led_strip_refresh(hal_ws2812_handle[WS2812_CHANNEL_CLK]);
}






/************************************************************************************************************************************************
* 函数名称: void HAL_WS2812_ShowDay(uint8_t week_channel, uint8_t day_channel, week_day_t day, ws2812_color_t color)
* 函数说明: 工作日显示
* 输入参数: 无
* 输出参数: 无
* 返回数值: 无
************************************************************************************************************************************************/
void HAL_WS2812_ShowDay(uint8_t week_channel, uint8_t day_channel, week_day_t day, ws2812_color_t color)
{
    uint8_t i = 0;

    for(i=0; i<hal_ws2812_rgbNum[week_channel]; i++)
    {
        led_strip_set_pixel(hal_ws2812_handle[week_channel], i, color.red, color.green, color.blue);
    }
    led_strip_refresh(hal_ws2812_handle[week_channel]);

    for(i=0; i<hal_ws2812_rgbNum[day_channel]; i++)
    {
        if(day == i)
        {
            led_strip_set_pixel(hal_ws2812_handle[day_channel], i, color.red, color.green, color.blue);
        }
        else
        {
            led_strip_set_pixel(hal_ws2812_handle[day_channel], i, 0, 0, 0);
        }
    }
    led_strip_refresh(hal_ws2812_handle[day_channel]);
}

/************************************************************************************************************************************************
* 函数名称: void HAL_WS2812_ShowFun(uint8_t channel, uint8_t showNum)
* 函数说明: 功能显示
* 输入参数: 无
* 输出参数: 无
* 返回数值: 无
************************************************************************************************************************************************/
void HAL_WS2812_ShowFun(uint8_t channel, uint8_t showNum)
{

}

/************************************************************************************************************************************************
* 函数名称: void HAL_WS2812_Close(uint8_t channel)
* 函数说明: 通道关闭
* 输入参数: 无
* 输出参数: 无
* 返回数值: 无
************************************************************************************************************************************************/
void HAL_WS2812_Close(uint8_t channel)
{

}



/************************************************************************************************************************************************
* end
************************************************************************************************************************************************/


