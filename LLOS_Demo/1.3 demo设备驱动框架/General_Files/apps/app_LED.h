#ifndef APP_LED_H
#define APP_LED_H

#include "llos.h"

enum App_LED_color_t
{
    App_LED_color_R,
    App_LED_color_G,
    App_LED_color_B,
};

void App_LED_Init(void);
void App_LED_Ctrl(enum App_LED_color_t color, ll_bit_t newState);

#endif
