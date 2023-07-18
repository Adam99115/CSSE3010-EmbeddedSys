
/** 
 **************************************************************
 * @file mylib/s4582547_oled.h
 * @author Arthur Mitchell - 45825473
 * @date 28/04/2023
 * @brief Header file for OLED Display for csse3010
 * REFERENCE: csse3010_mylib_oled.pdf 
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4582547_oled_control_task() - main controlling task for the OLED driver
 *************************************************************** 
*/

#include "processor_hal.h"
#include "board.h"
#include "debug_log.h"


 typedef struct {
  int x;
  int y;
  int z;
  int rotation;
} SorterCoords_OLED;

void s4582547_oled_init();
void s4582547_oled_draw_grid();
void s4582547_oled_draw_sorter(SorterCoords_OLED sorter);
void s4582547_oled_draw_sorter_depth(SorterCoords_OLED sorter);
void s4582547_oled_draw_sorter_rotation(SorterCoords_OLED sorter);
void s4582547_oled_control_task();
QueueHandle_t s4582547_oled_get_sorter_queue();