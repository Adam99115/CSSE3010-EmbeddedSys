/** 
 **************************************************************
 * @file mylib/s4582547_ascext.h
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

void ascext_new();
void scext_del();
void ascext_sys();
void s4582547_ascext_control();
EventGroupHandle_t s4582547_ascext_get_event_handle();

extern TaskHandle_t txradio_task_handle;
extern TaskHandle_t asc_task_list[6];

#define EXT_EVENT_NEW 1 << 0
#define EXT_EVENT_DEL 1 << 1
#define EXT_EVENT_SYS 1 << 2

