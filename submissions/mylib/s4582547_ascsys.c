/** 
 **************************************************************
 * @file mylib/s4582547_ascsys.c
 * @author Arthur Mitchell - 45825473
 * @date 1/05/2023
 * @brief Source file for system control for csse3010
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4582547_ascsys_run() - Starter function that starts all other main task for ASC.
 *************************************************************** 
*/

#include "board.h"
#include "processor_hal.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "debug_log.h"
#include "semphr.h"
#include "event_groups.h"

#include "s4582547_keypad.h"
#include "s4582547_hamming.h"
#include "s4582547_oled.h"
#include "s4582547_txradio.h"
#include "s4582547_ascsys.h"
#include "s4582547_ascext.h"


#define HIGHEST_PRIORITY			( tskIDLE_PRIORITY + 3 )
#define MEDIUM_PRIORITY			( tskIDLE_PRIORITY + 2 )
#define LOWEST_PRIORITY			( tskIDLE_PRIORITY + 1 )

#define SMALL_STACK_SIZE		((configMINIMAL_STACK_SIZE * 4))
#define MEDIUM_STACK_SIZE		(configMINIMAL_STACK_SIZE * 8) 
#define LARGE_STACK_SIZE		((configMINIMAL_STACK_SIZE * 16)) 
#define HUGE_STACK_SIZE		(configMINIMAL_STACK_SIZE  * 64) 

TaskHandle_t txradio_join_handle;
TaskHandle_t oled_task_handle;
TaskHandle_t keypad_task_set_on_handle;
TaskHandle_t keypad_task_act_on_handle;
TaskHandle_t LED_task_handle;
TaskHandle_t txradio_task_handle;
TaskHandle_t asc_task_list[6];

SemaphoreHandle_t latestTransmissionPacketSemaphore; // Semaphore for Transmission Packets as they are pointers to structs.

int currentSorterDepth = 0;
int currentSorterRotation = 0;
int currentSorterX = 0;
int currentSorterY = 0;
int vacuumOn = 0;

SorterCoords_OLED lastSorterCoordsOLED;
Packet* latestTransmissionPacket; 

#define KEYPAD_EVENT		    0xFFFF	    //Control Event Group Mask
#define XYZ_EVENT           0x0FFE       //XYZ Event Group Mask
#define VAC_EVENT           0x01        //VACUUM Event Group Mask
#define ROT_EVENT           0x1000        //VACUUM Event Group Mask
#define TRANS_EVENT         XYZ_EVENT | VAC_EVENT | ROT_EVENT

/*
 * Main Task that is transmits or interacts with the ASC depending on the keypad key pressed.
 */
void ascsys_task_act_on_keypad() {
  EventBits_t keypadBits;

  for (;;) {

  if (s4582547_reg_get_keypad_event_handle() != NULL
        && s4582547_ascext_get_event_handle()) {
    keypadBits = xEventGroupWaitBits(s4582547_reg_get_keypad_event_handle(), KEYPAD_EVENT, pdTRUE, pdFALSE, 10);
    int transmissionRequired = 0;
    if  ((keypadBits & (TRANS_EVENT)) != 0) {
      xSemaphoreTake(latestTransmissionPacketSemaphore, 10 );
      latestTransmissionPacket = pvPortMalloc(sizeof(Packet));
    }
    if ((keypadBits & XYZ_EVENT) != 0) {
      latestTransmissionPacket->xyz[0] = currentSorterX;
      latestTransmissionPacket->xyz[1] = currentSorterY;
      latestTransmissionPacket->xyz[2] = currentSorterDepth;
      latestTransmissionPacket->rot = currentSorterRotation;
      latestTransmissionPacket->messageType = 1;
      latestTransmissionPacket->payLoadString = "XYZ";
    }
    if ((keypadBits & KEYPAD_EVENT_1) != 0) {
      lastSorterCoordsOLED.x = 0;
      lastSorterCoordsOLED.y = 150;
      latestTransmissionPacket->xyz[0] = 0;
      latestTransmissionPacket->xyz[1] = 150;
    } else if ((keypadBits & KEYPAD_EVENT_2) != 0) {
      lastSorterCoordsOLED.x = 75;
      lastSorterCoordsOLED.y = 150;
      latestTransmissionPacket->xyz[0] = 75;
      latestTransmissionPacket->xyz[1] = 150;
    } else if ((keypadBits & KEYPAD_EVENT_3) != 0) {
      lastSorterCoordsOLED.x = 150;
      lastSorterCoordsOLED.y = 150;
      latestTransmissionPacket->xyz[0] = 150;
      latestTransmissionPacket->xyz[1] = 150;
    } else if ((keypadBits & KEYPAD_EVENT_A) != 0) {
      if (currentSorterDepth > 0) {
      currentSorterDepth = currentSorterDepth - 10;
      lastSorterCoordsOLED.z = lastSorterCoordsOLED.z - 10;
      }
    } else if ((keypadBits & KEYPAD_EVENT_4) != 0) {
      lastSorterCoordsOLED.x = 0;
      lastSorterCoordsOLED.y = 75;
      latestTransmissionPacket->xyz[0] = 0;
      latestTransmissionPacket->xyz[1] = 75;
    } else if ((keypadBits & KEYPAD_EVENT_5) != 0) {
      lastSorterCoordsOLED.x = 75;
      lastSorterCoordsOLED.y = 75;
      latestTransmissionPacket->xyz[0] = 75;
      latestTransmissionPacket->xyz[1] = 75;
    } else if ((keypadBits & KEYPAD_EVENT_6) != 0) {
      lastSorterCoordsOLED.x = 150;
      lastSorterCoordsOLED.y = 75;
      latestTransmissionPacket->xyz[0] = 150;
      latestTransmissionPacket->xyz[1] = 75;
    } else if ((keypadBits & KEYPAD_EVENT_B) != 0) {
      if (currentSorterDepth < 90) {
      currentSorterDepth = currentSorterDepth + 10;
      lastSorterCoordsOLED.z = lastSorterCoordsOLED.z + 10;
      }
    } else if ((keypadBits & KEYPAD_EVENT_7) != 0) {
      lastSorterCoordsOLED.x = 0;
      lastSorterCoordsOLED.y = 0;
      latestTransmissionPacket->xyz[0] = 0;
      latestTransmissionPacket->xyz[1] = 0;
    } else if ((keypadBits & KEYPAD_EVENT_8) != 0) {
      lastSorterCoordsOLED.x = 75;
      lastSorterCoordsOLED.y = 0;
      latestTransmissionPacket->xyz[0] = 75;
      latestTransmissionPacket->xyz[1] = 0;
    } else if ((keypadBits & KEYPAD_EVENT_9) != 0) {
      lastSorterCoordsOLED.x = 150;
      lastSorterCoordsOLED.y = 0;
      latestTransmissionPacket->xyz[0] = 150;
      latestTransmissionPacket->xyz[1] = 0;
    } else if ((keypadBits & KEYPAD_EVENT_C) != 0) {
      latestTransmissionPacket->payLoadString = "ROT";
      currentSorterRotation = (currentSorterRotation + 10) % 190;
      
      latestTransmissionPacket->messageType = 2;
      latestTransmissionPacket->rot = currentSorterRotation;
      lastSorterCoordsOLED.rotation = currentSorterRotation;
    } else if ((keypadBits & KEYPAD_EVENT_0) != 0) {
      latestTransmissionPacket->messageType = 3;
      if (vacuumOn) {
        vacuumOn = 0;
        latestTransmissionPacket->payLoadString = "VOFF";
      } else {
        vacuumOn = 1;
        latestTransmissionPacket->payLoadString = "VON";
      }
    } else if ((keypadBits & KEYPAD_EVENT_F) != 0) {
      //New
      xEventGroupSetBits(s4582547_ascext_get_event_handle(), EXT_EVENT_NEW);
    } else if ((keypadBits & KEYPAD_EVENT_E) != 0) {
      xEventGroupSetBits(s4582547_ascext_get_event_handle(), EXT_EVENT_DEL);
    } else if ((keypadBits & KEYPAD_EVENT_D) != 0) {
      //Sys
      xEventGroupSetBits(s4582547_ascext_get_event_handle(), EXT_EVENT_SYS);
  
    }
    currentSorterX = lastSorterCoordsOLED.x;
    currentSorterY = lastSorterCoordsOLED.y;
    if ((keypadBits & (TRANS_EVENT)) != 0) {
      xQueueSendToBack(s4582547_txradio_get_message_queue(), ( void * ) &latestTransmissionPacket, ( portTickType ) 10 );
      xSemaphoreGive(latestTransmissionPacketSemaphore);
    }

    xQueueSendToBack(s4582547_oled_get_sorter_queue(), ( void * ) &lastSorterCoordsOLED, ( portTickType ) 10 );
  }
  vTaskDelay(75);

  }
}

/*
 *  Task for handling the blue user button being pushed and consequently 
 *  causing a JOIN packet to be queued for the radio. 
 */
void ascsys_task_control_join() {

  taskENTER_CRITICAL();

  uint8_t pressed_state = 0;
  uint32_t user_button_prev_tick = HAL_GetTick();
  __GPIOC_CLK_ENABLE();
  GPIOC->PUPDR &= ~(0x03 << (2 * 13));
  GPIOC->PUPDR |= (0x00 << (2 * 13));

  taskEXIT_CRITICAL();
  for (;;) {
      if (HAL_GetTick() - user_button_prev_tick > (uint32_t)300 && !pressed_state) { //Software Debounce
        if ((GPIOC->IDR >> 13) & 0x01 ) { //Check if button pressed

          if( xSemaphoreTake(latestTransmissionPacketSemaphore, 10 ) == pdTRUE ) { //Get semaphore for packet pointer
            latestTransmissionPacket = pvPortMalloc(sizeof(Packet));
            latestTransmissionPacket->messageType = 0;
            latestTransmissionPacket->payLoadString = "JOIN";
            xQueueSendToBack(s4582547_txradio_get_message_queue(), ( void * ) &latestTransmissionPacket, ( portTickType ) 10 );
            pressed_state = 1;

            #ifdef DEBUG
              debug_log("JOIN Pushed\n\r");
            #endif

            xSemaphoreGive(latestTransmissionPacketSemaphore);
          }
        } 

        user_button_prev_tick = HAL_GetTick(); //Software Debounce
      }

      if (!((GPIOC->IDR >> 13) & 0x01)) {
        pressed_state = 0;
      } 
    vTaskDelay(20);
  }
}

/*
 *  Task for controlling the green LED by toggling it every second
 */
void ascsys_task_control_led() {
  taskENTER_CRITICAL();
  uint32_t prev_green_led = HAL_GetTick();
  taskEXIT_CRITICAL();
  for (;;) {
    if ((HAL_GetTick() - prev_green_led) > (uint32_t)1000) {	// Check if semaphore exists
        BRD_LEDGreenToggle();
        prev_green_led = HAL_GetTick();
		}
    vTaskDelay(20);
  }
}

/*
 * Starter function called by Main.c that initialises all other tasks.
 * It is called by main.c
 */
void s4582547_ascsys_run() {


  lastSorterCoordsOLED = (SorterCoords_OLED) {.x = 0, .y = 0, .z = 0, .rotation = 0};
	BRD_LEDInit();				//Initialise Blue LED  

  latestTransmissionPacketSemaphore = xSemaphoreCreateMutex();
  xSemaphoreGive(latestTransmissionPacketSemaphore);

  s4582547_txradio_init();
  debug_log("\n\n");
  xTaskCreate((void *) &ascsys_task_act_on_keypad,(const signed char *) "KeypadActor", MEDIUM_STACK_SIZE, NULL, MEDIUM_PRIORITY, &keypad_task_act_on_handle);
  xTaskCreate((void *) &s4582547_reg_task_set_on_keypad,(const signed char *) "KeypadReader", MEDIUM_STACK_SIZE, NULL, MEDIUM_PRIORITY, &keypad_task_set_on_handle);
  xTaskCreate((void *) &s4582547_oled_control_task,(const signed char *) "OLED", LARGE_STACK_SIZE, NULL, MEDIUM_PRIORITY, &oled_task_handle);

  xTaskCreate((void *) &s4582547_txradio_task_control_radio,(const signed char *) "Radio", MEDIUM_STACK_SIZE, NULL, HIGHEST_PRIORITY, &txradio_task_handle);
  xTaskCreate((void *) &s4582547_ascext_control,(const signed char *) "ASCEXT", MEDIUM_STACK_SIZE, NULL, MEDIUM_PRIORITY, &LED_task_handle);

  xTaskCreate((void *) &ascsys_task_control_join,(const signed char *) "JOIN", SMALL_STACK_SIZE, NULL, MEDIUM_PRIORITY, &txradio_join_handle);

  xTaskCreate((void *) &ascsys_task_control_led,(const signed char *) "LED", SMALL_STACK_SIZE, NULL, LOWEST_PRIORITY, &LED_task_handle);
  
  TaskHandle_t task_list[6] =  {txradio_task_handle, txradio_join_handle, oled_task_handle, keypad_task_set_on_handle, keypad_task_act_on_handle, LED_task_handle };
  for (int i = 0; i < 6; i++) {
    asc_task_list[i] = task_list[i];
  }

  vTaskStartScheduler();
}