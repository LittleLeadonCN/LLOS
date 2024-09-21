#ifndef __HAL_SPI1_H
#define __HAL_SPI1_H

// configuration begin
#define CS_DISPLAY1_PORT		GPIOA
#define CS_DISPLAY1_PIN     	GPIO_PIN_15
// configuration end

enum SPI_cmd_CS_t
{
	device_DISPLAY1_Cmd_CS,
};

void LLOS_Device_Register_SPI1(void);

#endif	/* __HAL_SPI1_H */
