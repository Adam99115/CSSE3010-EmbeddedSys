/**
  ******************************************************************************
  * @file    s3/main.c
  * @author  Arthur Mitchell
  * @date    22/03/2023
  * @brief   
  ******************************************************************************
  */

#include "board.h"
#include "processor_hal.h"
#include "s4582547_lta1000g.h"
#include "s4582547_joystick.h"
#include "s4582547_keypad.h"
#include "s4582547_hamming.h"

// #define DEBUG 0

#ifdef DEBUG
  #include "debug_log.h"
#endif

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
    GPIOA->MODER &= ~(0xA8000000); // Pins 4, 15 LED Pings 3 JoystickPB
    GPIOA->OSPEEDR &= ~(0xAC000000); // Pins 0
    GPIOB->MODER &= ~(0x00000280); // Pins 3, 4, 5, 12, 13, 15 LED
    GPIOC->MODER &= ~(0x00000000); // Pins 6, 7, LED, 
    GPIOE->MODER &= ~(0x00000000); // Pins 8, 7, 10, 12, 14, 15 

    s4582547_reg_lta1000g_init(); 
    s4582547_reg_joystick_pb_init();
    s4582547_reg_keypad_init();

    //Hardware inits for onboard User Button
    GPIOC->PUPDR &= ~(0x03 << (2 * 13));
    GPIOC->PUPDR |= (0x00 << (2 * 13));

}

/*
 * Main program - Initialises Hardware Interfaces, 
 */
int main(void)  {
 
  uint8_t encode_mode_on = 1;
  BRD_LEDInit();
  BRD_LEDBlueOn();
  uint8_t pressed_state = 0;


	HAL_Init();
	hardware_init();

  uint32_t user_button_prev_tick = HAL_GetTick();
  uint32_t fsm_poll = HAL_GetTick();

  uint8_t lower_byte_keypad = 0;
  uint8_t decoded_lower_keypad_byte = 0;

  uint8_t upper_byte_keypad = 0;
  uint8_t decoded_upper_keypad_byte = 0;
  uint8_t encoded_upper_led_byte = 0;
  uint8_t encoded_lower_led_byte = 0;
  uint8_t display_byte= 0;

  #ifdef DEBUG
    debug_log("NEW RUN \n\r");
  #endif

	while (1)
	{
   
    //FSM Polling
    if (HAL_GetTick() - fsm_poll > 50) {
      s4582547_reg_keypad_fsmprocessing();
      fsm_poll = HAL_GetTick();
    }

    //Handling Keypad Input
    if (s4582547_reg_keypad_read_status()) {
      switch(s4582547_reg_keypad_read_key()){
        case 1:
          upper_byte_keypad ^= (1 << 7);
          break;
        case 2:
          upper_byte_keypad ^= (1 << 6);
          break;
        case 3:
          upper_byte_keypad ^= (1 << 5);
          break;
        case 10: //A
          upper_byte_keypad ^= (1 << 4);
          break;
        case 4:
          upper_byte_keypad ^= (1 << 3);
          break;
        case 5:
          upper_byte_keypad ^= (1 << 2);
          break;
        case 6:
          upper_byte_keypad ^= (1 << 1);
          break;
        case 11: //B
          upper_byte_keypad ^= (1 << 0);
          break;
        case 7:
          lower_byte_keypad ^= (1 << 7);
          break;
        case 8:
          lower_byte_keypad ^= (1 << 6);
          break;
        case 9:
          lower_byte_keypad ^= (1 << 5);
          break;
        case 12: //C
          lower_byte_keypad ^= (1 << 4);
          break;
        case 0: //C
          lower_byte_keypad ^= (1 << 0);
          break;
        case 13: //E
          lower_byte_keypad ^= (1 << 3);
          break;
        case 14: //D
          lower_byte_keypad ^= (1 << 2);
          break;
        case 15: //F
          lower_byte_keypad ^= (1 << 1);
          break;
      }
    }

   
     // Joystick Toggling
    if (HAL_GetTick() - user_button_prev_tick > (uint32_t)300 && !pressed_state) {
      if ((GPIOC->IDR >> 13) & 0x01 ) {
        BRD_LEDBlueToggle();
        pressed_state = 1;
        encode_mode_on = !!!encode_mode_on;
      } 
      user_button_prev_tick = HAL_GetTick();
    }

    if (!((GPIOC->IDR >> 13) & 0x01)) {
      pressed_state = 0;
    } 

    // Lower Byte LED Processing
    if (s4582547_reg_joystick_press_get() == 1) {
      if (encode_mode_on) {
        //Encode Upper byte
        encoded_lower_led_byte = s4582547_lib_hamming_byte_encode(upper_byte_keypad) & 0xFF;
        display_byte = encoded_lower_led_byte;
      } else {
        //Decode Lower Byte
        decoded_lower_keypad_byte = s4582547_lib_hamming_byte_decode(lower_byte_keypad);
        display_byte = decoded_lower_keypad_byte;
      }
      BRD_LEDRedOff(); 
    } else { // Higher Byte LED Processing
    
      //Turn on Red LED
      BRD_LEDRedOn();
      if (s4582547_reg_joystick_press_get() == 2) s4582547_joystick_press_reset();
      if (encode_mode_on) {
        //Encode Upper byte
        encoded_upper_led_byte = s4582547_lib_hamming_byte_encode(upper_byte_keypad) >> 8;
        display_byte = encoded_upper_led_byte;
      } else {
        //Decode Upper byte
        decoded_upper_keypad_byte = s4582547_lib_hamming_byte_decode(upper_byte_keypad);
        display_byte = decoded_upper_keypad_byte;
      }
    }
    #ifdef DEBUG
      debug_log("KpValue %x Upper byte %u Lower Byte %u One-Bit E %d Two-Bit E %d BRUH %d\n\r", KeypadValue, upper_byte_keypad, lower_byte_keypad, one_bit_error_hamming, two_bit_error_hamming, s4582547_lib_hamming_parity_error(display_byte));
    #endif
    #ifdef DEBUG
      debug_log("KpValue %x Encoded Upper byte %x Encoded Lower byte %x Decoded Upper Byte %x Decoded Lower byte %x \n\r", KeypadValue, encoded_upper_led_byte, encoded_lower_led_byte, decoded_upper_keypad_byte, decoded_lower_keypad_byte);
    #endif

    if (s4582547_lib_hamming_parity_error(display_byte) || one_bit_error_hamming) {
      s4582547_reg_lta1000g_write(display_byte + (!!one_bit_error_hamming == 1 ? 256 : 0) + (!!two_bit_error_hamming == 1 ? 512: 0));
    } else {
      s4582547_reg_lta1000g_write(display_byte);
    }

    #ifdef DEBUG
      debug_log("Display Byte %x encode mode %d Display lower led %d \n\r", display_byte, encode_mode_on, s4582547_reg_joystick_press_get() == 1); //
    #endif
	}
}