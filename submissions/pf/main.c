/**
  ******************************************************************************
  * @file    pf/main.c
  * @author  Arthur Mitchell
  * @date    1/05/2023
  * @brief   Main file for CSSE3010 ASC Project S1/2023
  ******************************************************************************
  */

#include "board.h"
#include "processor_hal.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "debug_log.h"

#include "s4582547_ascsys.h"

// #define DEBUG 0
#define GPIO_OUTPUT 0x01
#define GPIO_SPEED 0x03
#define GPIO_PULLDOWN_SET 0x02

#include "debug_log.h"

/*
 * Main program - Inits hardware & calls ascsys_run which
 * creates all necessary tasks to run the ASC. 
 */
int main(void)  {
 
	HAL_Init();
  BRD_debuguart_init();
  s4582547_ascsys_run();

  return 1;

}

