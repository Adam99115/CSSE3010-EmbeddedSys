/**
  ******************************************************************************
  * @file    s2/main.c
  * @author  Arthur Mitchell
  * @date    11/03/2023
  * @brief   Nucleo429ZI Joystick RGB Controller that displays varying 
             RGB LED Colours based on on X Axes Angle and brightness of the RGB
             based on the Y axis which also displays the brightness level
             as a percentage on the 10 segment display.
  ******************************************************************************
  */

#include "board.h"
#include "processor_hal.h"
#include "debug_log.h"
#include "s4582547_lta1000g.h"
#include "s4582547_joystick.h"
#include "s4582547_rgb.h"


/*
 * Initialise Hardware
 */
void hardware_init() {
    // Enable the GPIO Clock's
    __GPIOA_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();
    __GPIOC_CLK_ENABLE();
    __GPIOE_CLK_ENABLE();
    __GPIOF_CLK_ENABLE();
    BRD_debuguart_init();

    // Clear the Control Registers for GPIO USE
    GPIOA->MODER &= ~(0xA8000000); // Pins 4, 15 LED
    GPIOA->OSPEEDR &= ~(0xAC000000); // Pins 0
    GPIOB->MODER &= ~(0x00000280); // Pins 3, 4, 5, 12, 13, 15 LED
    GPIOC->MODER &= ~(0x00000000); // Pins 6, 7, LED, 0, 3 Joystick XY
    GPIOE->MODER &= ~(0x00000000); // Pins 9 for LED Brightness
    GPIOF->MODER &= ~(0x00000000); // Pins 9 for LED Brightness

    s4582547_reg_lta1000g_init(); 
    s4582547_reg_joystick_init();
    s4582547_reg_rgb_init();
 

}

/*
 * Main program - Initialises Hardware Interfaces, Polls X & Y Readings from
 *                PC0 & PC3 which uses ADC  
 */
int main(void)  {
  unsigned int adcx_value = 0;
  unsigned int adcy_value = 0;

	HAL_Init();
	hardware_init();

	while (1)
	{
    // HAL_Delay(250);
    //Configure x Channel
    int32_t adcx_value = S4582547_REG_JOYSTICK_X_READ() + S4582547_REG_JOYSTICK_X_ZERO_CAL_OFFSET;
    if (adcx_value > 4096) adcx_value = 4096;
    if (adcx_value < 0) adcx_value = 0;
    //Configure Y Channel
    int32_t adcy_value = S4582547_REG_JOYSTICK_Y_READ() + S4582547_REG_JOYSTICK_Y_ZERO_CAL_OFFSET;
    if (adcy_value > 4096) adcy_value = 4096;
    if (adcy_value < 0)  adcy_value = 0;
    uint32_t rgb_to_set = (uint32_t) adcx_value;
    uint32_t brightness_to_set = (uint32_t) adcy_value;
    if (rgb_to_set < 0x471) {    // Black 0V - 0.73V 
      S4582547_REG_RGB_BLACK();
    }
    else if (rgb_to_set >= 0x471 && rgb_to_set < 0x638){ 
      S4582547_REG_RGB_BLUE(); // Blue - 60 degrees - 1.11V (10% either side in ADC is 1138-1592)
    }
    else if (rgb_to_set >= 0x638 && rgb_to_set < 0x78E) {
      S4582547_REG_RGB_GREEN(); // Green - 80 degrees - 1.466V (10% either side in ADC is 1592-2048)
    }
    else if (rgb_to_set >= 0x78E && rgb_to_set < 0x8E3) {
      S4582547_REG_RGB_CYAN(); // Cyan - 90 degrees - 1.83V (10% below and 5% above in ADC is 2048-2389)
    }
    else if (rgb_to_set >= 0x8E3 && rgb_to_set < 0xA38) {
      S4582547_REG_RGB_RED(); // Red - 110 degrees - 2.01V (5% below and 5% above in ADC is 2389-2616)
    }
    else if (rgb_to_set >= 0xA38 && rgb_to_set < 0xB2C) {
      S4582547_REG_RGB_MAGENTA(); // Magenta - 120 degrees - 2.2V (5% below and 5% above in ADC is 2616-2844)
    }
    else if (rgb_to_set >= 0xB2C && rgb_to_set < 0xC00) {
      S4582547_REG_RGB_YELLOW(); // Yellow - 130 degrees - 2.38V (5% below and 5% above in ADC is 2844-3072)
    }
    else if (rgb_to_set >= 0xC00) {
      S4582547_REG_RGB_WHITE(); // White - > 140 degrees - 2.65V (5% below in ADC is 3072-2389)
    }

    //Joystick Y handling LEDBAR
    float pc_dec_value = (float) (adcy_value) / 4.0f / 1024.0f;
    int percentage_value = (int) (pc_dec_value * 100.0f);
    int tens_value = (int) (pc_dec_value * 11.0f) % 11;
    s4582547_reg_lta1000g_write((1 << tens_value) - 1);
    s4582547_reg_rgb_brightness_write(percentage_value);
    // debug_log(" ADCX %d, ADCY %d y_PC, %d y_T %d y_wr %d \n\r", adcx_value, adcy_value, percentage_value, tens_value, (1 << tens_value) - 1);
	}
}
