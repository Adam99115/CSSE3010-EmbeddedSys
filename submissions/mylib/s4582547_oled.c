/** 
 **************************************************************
 * @file mylib/s4582547_oled.c
 * @author Arthur Mitchell - 45825473
 * @date 28/04/2023
 * @brief Source file for OLED Display for csse3010
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
#include "oled_pixel.h"
#include "oled_string.h"
#include "fonts.h"
#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "queue.h"
#include "semphr.h"

#include "s4582547_oled.h"

#define I2C_DEV_SDA_PIN		9 //This is GPIOB9
#define I2C_DEV_SCL_PIN		8 //This is GPIOB8
#define I2C_DEV_GPIO		GPIOB
#define I2C_DEV_GPIO_AF 	GPIO_AF4_I2C1
#define I2C_DEV_GPIO_CLK()	__GPIOB_CLK_ENABLE()

#define I2C_DEV				I2C1
#define I2C_DEV_CLOCKSPEED 	100000

QueueHandle_t SorterQueue;	// Queue used

/*
 *  Initialise the registers & hardware interface
 *  for the RGB LED. (The initialisation of the OLED
	Display was inspired from the following
	sourcelib/examples/peripherals/oled/main.c
 */
void s4582547_oled_init() {

     //Hardware init for brightness PWM
    __TIM1_CLK_ENABLE();
    uint32_t pclk1;
	uint32_t freqrange;

	// Enable GPIO clock
	I2C_DEV_GPIO_CLK();

	//Clear and Set Alternate Function for pin (lower ARF register) 
	MODIFY_REG(I2C_DEV_GPIO->AFR[1], ((0x0F) << ((I2C_DEV_SCL_PIN-8) * 4)) | ((0x0F) << ((I2C_DEV_SDA_PIN-8)* 4)), ((I2C_DEV_GPIO_AF << ((I2C_DEV_SCL_PIN-8) * 4)) | (I2C_DEV_GPIO_AF << ((I2C_DEV_SDA_PIN-8)) * 4)));
	
	//Clear and Set Alternate Function Push Pull Mode
	MODIFY_REG(I2C_DEV_GPIO->MODER, ((0x03 << (I2C_DEV_SCL_PIN * 2)) | (0x03 << (I2C_DEV_SDA_PIN * 2))), ((GPIO_MODE_AF_OD << (I2C_DEV_SCL_PIN * 2)) | (GPIO_MODE_AF_OD << (I2C_DEV_SDA_PIN * 2))));
	
	//Set low speed.
	SET_BIT(I2C_DEV_GPIO->OSPEEDR, (GPIO_SPEED_LOW << I2C_DEV_SCL_PIN) | (GPIO_SPEED_LOW << I2C_DEV_SDA_PIN));

	//Set Bit for Push/Pull output
	SET_BIT(I2C_DEV_GPIO->OTYPER, ((0x01 << I2C_DEV_SCL_PIN) | (0x01 << I2C_DEV_SDA_PIN)));

	//Clear and set bits for no push/pull
	MODIFY_REG(I2C_DEV_GPIO->PUPDR, (0x03 << (I2C_DEV_SCL_PIN * 2)) | (0x03 << (I2C_DEV_SDA_PIN * 2)), (GPIO_PULLUP << (I2C_DEV_SCL_PIN * 2)) | (GPIO_PULLUP << (I2C_DEV_SDA_PIN * 2)));

	// Configure the I2C peripheral
	// Enable I2C peripheral clock
	__I2C1_CLK_ENABLE();

	// Disable the selected I2C peripheral
	CLEAR_BIT(I2C_DEV->CR1, I2C_CR1_PE);

  	pclk1 = HAL_RCC_GetPCLK1Freq();			// Get PCLK1 frequency
  	freqrange = I2C_FREQRANGE(pclk1);		// Calculate frequency range 

  	//I2Cx CR2 Configuration - Configure I2Cx: Frequency range
  	MODIFY_REG(I2C_DEV->CR2, I2C_CR2_FREQ, freqrange);

	// I2Cx TRISE Configuration - Configure I2Cx: Rise Time
  	MODIFY_REG(I2C_DEV->TRISE, I2C_TRISE_TRISE, I2C_RISE_TIME(freqrange, I2C_DEV_CLOCKSPEED));

   	// I2Cx CCR Configuration - Configure I2Cx: Speed
  	MODIFY_REG(I2C_DEV->CCR, (I2C_CCR_FS | I2C_CCR_DUTY | I2C_CCR_CCR), I2C_SPEED(pclk1, I2C_DEV_CLOCKSPEED, I2C_DUTYCYCLE_2));

   	// I2Cx CR1 Configuration - Configure I2Cx: Generalcall and NoStretch mode
  	MODIFY_REG(I2C_DEV->CR1, (I2C_CR1_ENGC | I2C_CR1_NOSTRETCH), (I2C_GENERALCALL_DISABLE| I2C_NOSTRETCH_DISABLE));

   	// I2Cx OAR1 Configuration - Configure I2Cx: Own Address1 and addressing mode
  	MODIFY_REG(I2C_DEV->OAR1, (I2C_OAR1_ADDMODE | I2C_OAR1_ADD8_9 | I2C_OAR1_ADD1_7 | I2C_OAR1_ADD0), I2C_ADDRESSINGMODE_7BIT);

   	// I2Cx OAR2 Configuration - Configure I2Cx: Dual mode and Own Address2
  	MODIFY_REG(I2C_DEV->OAR2, (I2C_OAR2_ENDUAL | I2C_OAR2_ADD2), I2C_DUALADDRESS_DISABLE);

  	// Enable the selected I2C peripheral
	SET_BIT(I2C_DEV->CR1, I2C_CR1_PE);

	ssd1306_Init();	//Initialise SSD1306 OLED.
}

QueueHandle_t s4582547_oled_get_sorter_queue() {
	return SorterQueue;
}	

/*
 * Helper function that draws bounding box / grid of the ASC environment.
 */
void s4582547_oled_draw_grid() {
		//Clear Screen
		ssd1306_Fill(Black);
		//Draw Horizontal lines of boundary box
		for (int i=0; i < 30; i++) {
			ssd1306_DrawPixel(i, 0, SSD1306_WHITE);		//top line
			ssd1306_DrawPixel(i, 30, SSD1306_WHITE);	//bottom line
		}
		//Draw Vertical lines of boundary box
		for (int i=0; i < 31; i++) {
		ssd1306_DrawPixel(0, i, SSD1306_WHITE);			//left line
			ssd1306_DrawPixel(30, i, SSD1306_WHITE);	//right line
		}
}

/*
 * Helper function that draws the current + to describe the position of the ASC.
 */
void s4582547_oled_draw_sorter(SorterCoords_OLED sorter) {
		int scaledX = (int) (sorter.x / 200.0f * 30.0f);
		int scaledY = (int) ((150 - sorter.y) / 200.0f * 30.0f);
		for (int i=0; i < 5; i++) {
				ssd1306_DrawPixel((scaledX + 2) + i, (scaledY + 1) + 2, SSD1306_WHITE);
				ssd1306_DrawPixel((scaledX + 2) + 2 , (scaledY + 1) + i, SSD1306_WHITE);
		}
}

/*
 * Helper function that draws the current Depth of the ASC.
 */
void s4582547_oled_draw_sorter_depth(SorterCoords_OLED sorter) {
	char depth[10];
	sprintf(depth, "Z:%d", sorter.z);
	if (sorter.z == 0) {
		ssd1306_SetCursor(108,3);
	} else {
		ssd1306_SetCursor(102,3);
	}
    ssd1306_WriteString(depth, Font_6x8, SSD1306_WHITE);
}

/*
 * Helper function that draws the current Rotation of the ASC.
 */
void s4582547_oled_draw_sorter_rotation(SorterCoords_OLED sorter) {
	char rotation[10];
	sprintf(rotation, "Rot:%d", sorter.rotation);
	if (sorter.rotation > 90) {
		ssd1306_SetCursor(85,23);
	} else if (sorter.rotation == 0) {
		ssd1306_SetCursor(96,23);
	} else {
		ssd1306_SetCursor(90,23);
	}
    ssd1306_WriteString(rotation, Font_6x8, SSD1306_WHITE);
}

/*
 *  Main Controlling task of the OLED display.
 */
void s4582547_oled_control_task() {

	taskENTER_CRITICAL();
	SorterCoords_OLED sorter;
	s4582547_oled_init();
	SorterQueue = xQueueCreate(10, sizeof(sorter));	
	taskEXIT_CRITICAL();

	for (;;) {
		if (s4582547_oled_get_sorter_queue() != NULL) {
			if (xQueueReceive(s4582547_oled_get_sorter_queue(), &sorter, 10 ) == pdTRUE) {
				taskENTER_CRITICAL();
				s4582547_oled_draw_grid();
				s4582547_oled_draw_sorter(sorter);
				s4582547_oled_draw_sorter_depth(sorter);
				s4582547_oled_draw_sorter_rotation(sorter);
				ssd1306_UpdateScreen();
				taskEXIT_CRITICAL();
			}
		}
	vTaskDelay(50);
	}
}