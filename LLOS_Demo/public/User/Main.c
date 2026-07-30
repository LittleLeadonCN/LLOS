#include "CH58x_common.h"
#include "system.h"
/*********************************************************************
 * @fn      main
 *
 * @brief   主函数
 *
 * @return  none
 */
int main()
{
    SetSysClock(CLK_SOURCE_HSE_PLL_78MHz);
    
    GPIOB_SetBits(GPIO_Pin_7);
    GPIOB_ModeCfg(GPIO_Pin_4, GPIO_ModeIN_PU);      // RXD-配置上拉输入
    GPIOB_ModeCfg(GPIO_Pin_7, GPIO_ModeOut_PP_5mA); // TXD-配置推挽输出，注意先让IO口输出高电平
    UART0_DefInit();

    System_Init();
    while(1)
    {
        System_Loop();
    }
}
