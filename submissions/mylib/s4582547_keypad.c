 /** 
 **************************************************************
 * @file mylib/s4582547_keypad.c
 * @author Arthur Mitchell - 45825473
 * @date 22/03/2023
 * @brief csse3010-keypad-mylib-register-driver
 * REFERENCE: s4582547_keypad
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4582547_reg_keypad_init() - init PGPIO & timer ISR, sets keypad FSM to init
 * s4582547_reg_keypad_fsmprocessing() - Keypad FSM processing loop (called from main())
 * s4582547_reg_keypad_read_status() - Return keyPad status variable
 * s4582547_reg_keypad_read_key() - Return current hex value of the keypad using Keypad Value
 *************************************************************** 
 */

#include "processor_hal.h"
#include "board.h"

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#include "s4582547_keypad.h"

// #define DEBUG 1
#ifdef DEBUG
    #include "debug_log.h"
#endif

#define GPIO_OUTPUT 0x01
#define GPIO_INPUT 0x00
#define GPIO_OPUDPR 0x01 // Pull down
#define GPIO_IPUDPR 0x01 // Pull UP
#define GPIO_Speed 0x03

#define ROW_1_IM 0x0E
#define ROW_2_IM 0x0D
#define ROW_3_IM 0x0B
#define ROW_4_IM 0x07

#define COL_1 0x01
#define COL_2 0x02
#define COL_3 0x03
#define COL_4 0x04

EventGroupHandle_t keypadctrlEventGroup;	
uint8_t prevKeypadValue;

int lastRowPressed = 0;
int lastRowPressedIsReleased = 0;

/*
 * 
 * Hardware init for reg_keypad 
 *
 */
void s4582547_reg_keypad_init() {

    __GPIOE_CLK_ENABLE();

    GPIOB->MODER &= ~(0x00000280); // Pins 3, 4, 5, 12, 13, 15 LED
    GPIOE->MODER &= ~(0x00000000); // Pins 8, 7, 10, 12, 14, 15 


    //Output for rows GPIOE pins 8, 7, 10, 12 correspond to column 4, 3, 2, 1
    GPIOE->MODER |= (GPIO_OUTPUT << (2 * 8)) | (GPIO_OUTPUT << (2 * 7)) | (GPIO_OUTPUT << (2 * 10)) | (GPIO_OUTPUT << (2 * 12));
    GPIOE->OSPEEDR |= (GPIO_Speed << (2 * 8)) | (GPIO_Speed << (2 * 7)) | (GPIO_Speed << (2 * 10)) | (GPIO_Speed << (2 * 12));
    GPIOE->OTYPER &= ~((1 << 8) | (1 << 7) | (1 << 10) | (1 << 12));
    //Input for columns GPIOE pins 14, 15 GPIOB pins 10, 11 correspond to rows 4, 3, 2, 1
    GPIOE->MODER |= (GPIO_INPUT << (2 * 0)) | (GPIO_INPUT << (2 * 14)) |  (GPIO_INPUT << (2 * 2)) | ((GPIO_INPUT << (2 * 15)));

    GPIOE->PUPDR &= ~((GPIO_IPUDPR << (2 * 0)) | (GPIO_IPUDPR << (2 * 14)) | (GPIO_IPUDPR << (2 * 2)) | (GPIO_IPUDPR << (2 * 15)));

    GPIOE->PUPDR |= (GPIO_IPUDPR << (2 * 0)) | (GPIO_IPUDPR << (2 * 14)) | (GPIO_IPUDPR << (2 * 2)) | (GPIO_IPUDPR << (2 * 15));
    GPIOE->ODR |= (0x01 << 8 | 0x01 << 7 | 0x01 << 10 | 0x01 << 12);

    KeypadFsmCurrentState = 0;
    KeypadStatus = 0;
    KeypadValue = -1;
	
}

/*
 *
 * Finite state machine that handles 4 main states that handle
 * setting columns and reading row values.
 *
 */
void s4582547_reg_keypad_fsmprocessing(){
    // BRD_LEDBlueToggle();

    int ROW_PRESSED = keypad_readrow();
    int NextKeyPadState = 1;
   
    switch(KeypadFsmCurrentState) {
        case INIT_STATE: 
            keypad_writecol(COL_1);
            NextKeyPadState = RSCAN1_STATE;
            break;
        case RSCAN1_STATE:
            switch(ROW_PRESSED){
                case ROW_1_IM:
                    KeypadValue = 0x01;
                    break;
                case ROW_2_IM:
                    KeypadValue = 0x04;
                    break;
                case ROW_3_IM:
                    KeypadValue = 0x07;
                    break;
                case ROW_4_IM:
                    KeypadValue = 0x00;
                    break;
            }
            keypad_writecol(COL_2);
            NextKeyPadState = RSCAN2_STATE;
            break;
        case RSCAN2_STATE:
            switch(ROW_PRESSED){
                case ROW_1_IM:
            
                    KeypadValue = 0x02;
                    break;
                case ROW_2_IM:
                    KeypadValue = 0x05;
                    break;
                case ROW_3_IM:
                    KeypadValue = 0x08;
                    break;
                case ROW_4_IM:
                    KeypadValue = 0x0F;
                    break;
            }
            keypad_writecol(COL_3);
            NextKeyPadState = RSCAN3_STATE;
            break;
        case RSCAN3_STATE:
            switch(ROW_PRESSED){
                case ROW_1_IM:
                    KeypadValue = 0x03;
                    break;
                case ROW_2_IM:
                    KeypadValue = 0x06;
                    break;
                case ROW_3_IM:
                    KeypadValue = 0x09;
                    break;
                case ROW_4_IM:
                    KeypadValue = 0x0E;
                    break;
            }
            keypad_writecol(COL_4);
            NextKeyPadState = RSCAN4_STATE;
            break;
        case RSCAN4_STATE:
            switch(ROW_PRESSED){
                case ROW_1_IM:
                    KeypadValue = 0x0A;
                    break;
                case ROW_2_IM:
                    KeypadValue = 0x0B;
                    break;
                case ROW_3_IM:
                    KeypadValue = 0x0C;
                    break;
                case ROW_4_IM:
                    KeypadValue = 0x0D;
                    break;
            }
            keypad_writecol(COL_1);
            NextKeyPadState = RSCAN1_STATE;
            break;
    }

    //The following pair of if statements, combined with the vTaskDelay, debounces the keypad.
    if (ROW_PRESSED == 0x0F && NextKeyPadState - 1 == lastRowPressed) {
        lastRowPressedIsReleased = 1;
    } else if (ROW_PRESSED != 0x0F && lastRowPressedIsReleased) {
        KeypadStatus = 1; //0x0F is bitmask for when no button has been pressed on any row.
        prevKeypadValue = KeypadValue;
        lastRowPressed = NextKeyPadState - 1;
        lastRowPressedIsReleased = 0;
        BRD_LEDRedToggle();
    } 

  
    KeypadFsmCurrentState = NextKeyPadState;
}

/*
 * Helper function that checks GPIOE & GPIOB registers to 
 * Determine whether or not any button on a particular row has been pressed.
 */
uint8_t keypad_readrow(){
    uint8_t row_mask = (((GPIOE->IDR >> 14) & 0x01) << 3) | (((GPIOE->IDR >> 15) & 0x01) << 2) | \
        (((GPIOE->IDR >> 0) & 0x01) << 1) | (((GPIOE->IDR >> 2) & 0x01) << 0); 

    return row_mask;
}

/*
 * Helper function that sets GPIOE registers to correctly 
 * output to the keypad and enable certain columns.
 */
void keypad_writecol(unsigned char colval) {
    GPIOE->ODR |= (0x01 << 8 | 0x01 << 7 | 0x01 << 10 | 0x01 << 12); //GPIO Pins 8, 7, 10, 12 representing cols 4, 3, 2, 1
    switch(colval){
        case COL_4:
            GPIOE->ODR &= ~(0x01 << 8);
            break;
        case COL_3:
            GPIOE->ODR &= ~(0x01 << 7);
            break;
        case COL_2:
            GPIOE->ODR &= ~(0x01 << 10);
            break;
        case COL_1:
            GPIOE->ODR &= ~(0x01 << 12);
            break;
    }
}

/*
 * Helper function that returns the KeyPadStatus
 * indicating of whether the Keypad has been pressed or not.
 */
int s4582547_reg_keypad_read_status() {
    return KeypadStatus;
}

/*
 * Helper function that returns the latest Keypad Value;
 */
uint8_t s4582547_reg_keypad_read_key() {
    KeypadStatus = 0;
    return KeypadValue;
}

EventGroupHandle_t s4582547_reg_get_keypad_event_handle() {
    return keypadctrlEventGroup;
}

/* 
 * Keypad Task that sets event bits.
 */
void s4582547_reg_task_set_on_keypad() {
  // Enable the GPIO Clock's
  
  taskENTER_CRITICAL();
  s4582547_reg_keypad_init();
  keypadctrlEventGroup = xEventGroupCreate();
  taskEXIT_CRITICAL();
  EventBits_t keypadBits;
  uint8_t val = 0;
  for (;;) {
    s4582547_reg_keypad_fsmprocessing();
    if (s4582547_reg_keypad_read_status()) {
      val = s4582547_reg_keypad_read_key();
      switch(val) {
        case 1:
		  keypadBits = xEventGroupSetBits(keypadctrlEventGroup, KEYPAD_EVENT_1);
          break;
        case 2:
		  keypadBits = xEventGroupSetBits(keypadctrlEventGroup, KEYPAD_EVENT_2);
          break;
        case 3:
		  keypadBits = xEventGroupSetBits(keypadctrlEventGroup, KEYPAD_EVENT_3);
          break;
        case 10: //A
		  keypadBits = xEventGroupSetBits(keypadctrlEventGroup, KEYPAD_EVENT_A);
          break;
        case 4:
		  keypadBits = xEventGroupSetBits(keypadctrlEventGroup, KEYPAD_EVENT_4);
          break;
        case 5:
		  keypadBits = xEventGroupSetBits(keypadctrlEventGroup, KEYPAD_EVENT_5);
          break;
        case 6:
		  keypadBits = xEventGroupSetBits(keypadctrlEventGroup, KEYPAD_EVENT_6);
          break;
        case 11: //B
		  keypadBits = xEventGroupSetBits(keypadctrlEventGroup, KEYPAD_EVENT_B);
          break;
        case 7:
		  keypadBits = xEventGroupSetBits(keypadctrlEventGroup, KEYPAD_EVENT_7);
          break;
        case 8:
		  keypadBits = xEventGroupSetBits(keypadctrlEventGroup, KEYPAD_EVENT_8);
          break;
        case 9:
		  keypadBits = xEventGroupSetBits(keypadctrlEventGroup, KEYPAD_EVENT_9);
          break;
        case 12: //C
		  keypadBits = xEventGroupSetBits(keypadctrlEventGroup, KEYPAD_EVENT_C);
          break;
        case 0: //0
		  keypadBits = xEventGroupSetBits(keypadctrlEventGroup, KEYPAD_EVENT_0);
          break;
        case 15: //F
		  keypadBits = xEventGroupSetBits(keypadctrlEventGroup, KEYPAD_EVENT_F);
          break;
        case 14: //E
		  keypadBits = xEventGroupSetBits(keypadctrlEventGroup, KEYPAD_EVENT_E);
          break;
        case 13: //D
		  keypadBits = xEventGroupSetBits(keypadctrlEventGroup, KEYPAD_EVENT_D);
          break;
      }
    }
    vTaskDelay(25);
  }
}
