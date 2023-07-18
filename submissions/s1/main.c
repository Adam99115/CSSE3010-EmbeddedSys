/**
  ******************************************************************************
  * @file    s1/main.c
  * @author  Arthur Mitchell
  * @date    27/02/2023
  * @brief   Nucleo429ZI binary counter via joystick button example. Uses
             LEDBAR to visually represent the number in the binary counter.
  ******************************************************************************
  */

#include "board.h"
#include "processor_hal.h"
#include "s4582547_lta1000g.h"
#include "s4582547_joystick.h"

void hardware_init(void);

/*
 * Main program - Program represents a binary counter via joystick button
 */
int main(void)  {

  uint32_t prev_tick = HAL_GetTick();

	HAL_Init();			//Initalise Board
	hardware_init();	//Initalise hardware modules
  
	// Main processing loop
  while (1) {
    if (HAL_GetTick() - prev_tick > 5) {
      s4582547_reg_lta1000g_write(s4582547_reg_joystick_press_get());
      s4582547_lta_lta1000g_invert(s4582547_reg_joystick_press_get(), 0x001F);
      prev_tick = HAL_GetTick();
    }
	}

  return 0;
}

/*
 * Initialise Hardware
 */
void hardware_init(void) {
    // Enable the GPIO Clock's
    __GPIOA_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();
    __GPIOC_CLK_ENABLE();

    // Clear the Control Registers for GPIO USE
    GPIOA->MODER &= ~(0xA8000000); // Pins 4, 15 LED
    GPIOB->MODER &= ~(0x00000280); // Pins 3, 4, 5, 12, 13, 15 LED
    GPIOC->MODER &= ~(0x00000000); // Pins 6, 7, LED, 0, 3 Joystick XY
    
  s4582547_reg_lta1000g_init();
  s4582547_reg_joystick_pb_init();
}

