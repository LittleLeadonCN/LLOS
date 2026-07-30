#include "app_LED.h"
#include "hal_gpio.h"
#include "CH58x_common.h"

static struct ll_device_t *devGPIOB;

void App_LED_Init(void)
{
    devGPIOB = LLOS_Device_Find("GPIOB");
	if(devGPIOB == NULL)
	{
    	LL_LOG_E("%s ", "GPIOB Not Found!\r\n", __FUNCTION__);
		while(1);
	}
}

void App_LED_Ctrl(enum App_LED_color_t color, ll_bit_t newState)
{
    switch (color)
    {
        case App_LED_color_R: LLOS_Device_WritePin(devGPIOB, PIN_LEDR, !newState); break;
        case App_LED_color_G: LLOS_Device_WritePin(devGPIOB, PIN_LEDG, !newState); break;
        case App_LED_color_B: LLOS_Device_WritePin(devGPIOB, PIN_LEDB, !newState); break;
        default: break;
    }
}
