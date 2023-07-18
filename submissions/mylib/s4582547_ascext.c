
/** 
 **************************************************************
 * @file mylib/s4582547_ascext.c
 * @author Arthur Mitchell - 45825473
 * @date 1/05/2023
 * @brief Source file for ext system control for csse3010
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4582547_ascext_get_event_handle() - get event group handler
 * s4582547_ascext_control() - Controller task for ASC's ext functionality
 *************************************************************** 
*/

#include "processor_hal.h"
#include "board.h"
#include "debug_log.h"

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "queue.h"

#include "s4582547_txradio.h"
#include "s4582547_ascext.h"

#define HIGHEST_PRIORITY			( tskIDLE_PRIORITY + 3 )
#define MEDIUM_PRIORITY			( tskIDLE_PRIORITY + 2 )
#define LOWEST_PRIORITY			( tskIDLE_PRIORITY + 1 )

#define SMALL_STACK_SIZE		((configMINIMAL_STACK_SIZE * 4))
#define MEDIUM_STACK_SIZE		(configMINIMAL_STACK_SIZE * 8) 
#define LARGE_STACK_SIZE		((configMINIMAL_STACK_SIZE * 16)) 
#define HUGE_STACK_SIZE		(configMINIMAL_STACK_SIZE  * 64) 

EventGroupHandle_t extctrlEventGroup = NULL;	
#define ASC_EVENT_MASK		    0xFF	    //Control Event Group Mask

int radioTaskDeleted = 0;

/*
 * Helper function that handles creating a new task for the radio.
 */
void ascext_new() {
    if (radioTaskDeleted) {
        asc_task_list[0] = txradio_task_handle;
        xTaskCreate((void *) &s4582547_txradio_task_control_radio,(const signed char *) "Radio", MEDIUM_STACK_SIZE, NULL, HIGHEST_PRIORITY, &txradio_task_handle);
        radioTaskDeleted = 0;
    }
}

/*
 * Helper function that handles deleting a new task for the radio.
 */
void ascext_del() {
    if (!radioTaskDeleted) {
        vTaskDelete(txradio_task_handle);
        txradio_task_handle = NULL;
        asc_task_list[0] = NULL;
        radioTaskDeleted = 1;
    }
}

/*
 * Helper function that displays task statistics & statuses.
 */
void ascext_sys() {
      TaskStatus_t task_status;
      int taskMemAllocations[6] = {MEDIUM_STACK_SIZE, SMALL_STACK_SIZE, LARGE_STACK_SIZE, MEDIUM_STACK_SIZE, MEDIUM_STACK_SIZE, SMALL_STACK_SIZE};
      for (int i = 0; i < 6; i++){
        TaskHandle_t task = asc_task_list[i];
        debug_log("Task: \n\r", task_status.pcTaskName);

        if (task != NULL) {
            vTaskGetInfo(task, &task_status, pdTRUE, eInvalid);
            debug_log("\tName : %s \n\r", task_status.pcTaskName);
            debug_log("\tTask ID : %d \n\r", task_status.xTaskNumber);
            debug_log("\tTask State : %d \n\r", task_status.eCurrentState);
            debug_log("\tTask Priority : %d \n\r", task_status.uxCurrentPriority);
            debug_log("\tTask Mem Usage : %d / %d \n\r", task_status.usStackHighWaterMark, taskMemAllocations[i]);

          
        } 
      }
}

/*
 * External function that returns the event handle for the event bits to be used.
 */
EventGroupHandle_t s4582547_ascext_get_event_handle() {
    return extctrlEventGroup;
}

/*
 * External function that acts as the controlling task for ascsys's external functions.
 */
void s4582547_ascext_control(TaskHandle_t tasks[]) {

  taskENTER_CRITICAL();
  extctrlEventGroup = xEventGroupCreate();
  taskEXIT_CRITICAL();
  EventBits_t extBits;

  for(;;) {
    if (s4582547_ascext_get_event_handle() != NULL) {
        extBits = xEventGroupWaitBits(s4582547_ascext_get_event_handle(), ASC_EVENT_MASK, pdTRUE, pdFALSE, 10);
        
        if  ((extBits & (EXT_EVENT_NEW)) != 0) {
            ascext_new();

        }
        if  ((extBits & (EXT_EVENT_DEL)) != 0) {
            ascext_del();
        }
        if  ((extBits & (EXT_EVENT_SYS)) != 0) {
            taskENTER_CRITICAL();
            ascext_sys();
            taskEXIT_CRITICAL();

        }
    }
  }
  vTaskDelay(50);
}
