/**
  ******************************************************************************
  * @file    s4/main.c
  * @author  Arthur Mitchell
  * @date    22/03/2023
  * @brief   Stage 4 for CSSE3010 S1/2023
  ******************************************************************************
  */

#include "board.h"
#include "processor_hal.h"
#include "s4582547_lta1000g.h"
#include "s4582547_joystick.h"
#include "s4582547_keypad.h"
#include "s4582547_hamming.h"

// #define DEBUG 0
#define GPIO_OUTPUT 0x01
#define GPIO_SPEED 0x03
#define GPIO_PULLDOWN_SET 0x02

#ifdef DEBUG
  #include "debug_log.h"
#endif

/*
 * Main program - Initialises Hardware Interfaces, 
 */
int main(void)  {
 
	HAL_Init();
	xTaskCreate(myTsk, "TASK1", STACK_SIZE, NULL, PRIORITY, NULL);
  xTaskCreate(myTsk, "TASK2", STACK_SIZE, NULL, PRIORITY, NULL);
  xTaskCreate(myTsk, "TASK3", STACK_SIZE, NULL, PRIORITY, NULL);
  vTaskStartScheduler();

  return 1;

}

void task1() {
  // task1_hw_init();
  // Enable the GPIO Clock's
  __GPIOD_CLK_ENABLE();
    GPIOD->MODER |=  (GPIO_OUTPUT << 2 * 7) | (GPIO_OUTPUT << 2 * 6) | (GPIO_OUTPUT << 2 * 5);

    // Set Output types for GPIO's
    GPIOD->OTYPER |=  (0 << 7) | (0 << 6) | (0 << 5);

    // Clear Output Speed Registers
    GPIOD->OSPEEDR &= ~(0x00000000); // Pins 4, 15
    // Set Output types for GPIO's
    GPIOD->OSPEEDR |=  (GPIO_SPEED << 4) | (GPIO_SPEED << 15);
    // Set Joystick button Control, Pullup/down & Output speed reg's
    GPIOD->PUPDR |= (GPIO_PULLDOWN_SETTING << (3));  // Set Pull down for GPIOA
    GPIOD->OSPEEDR |=  (GPIO_SPEED_SETTING << (3 * 2));
    S4582547_REG_SYSMON_CHAN0_CLR();
  for (;;) {
    S4582547_REG_SYSMON_CHAN0_SET();
    vTaskDelay(3);
    S4582547_REG_SYSMON_CHAN0_CLR();
  }
}

void task2() {
  task2_hw_init();
  for (;;) {
    S4582547_REG_SYSMON_CHAN1_SET();
    vTaskDelay(3);
    S4582547_REG_SYSMON_CHAN1_CLR();
  }
}

void task3() {
  task3_hw_init();
  for (;;) {
    S4582547_REG_SYSMON_CHAN3_SET();
    vTaskDelay(3);
    S4582547_REG_SYSMON_CHAN3_CLR();
  }
}