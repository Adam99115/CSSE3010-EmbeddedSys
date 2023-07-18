
/** 
 **************************************************************
 * @file mylib/s4582547_rgb.c
 * @author Arthur Mitchell - 45825473
 * @date 11/03/2023
 * @brief RGB LED peripheral driver header file.
 * REFERENCE: csse3010_mylib_reg_rgb.pdf 
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4582547_reg_rgb_init() - intialise mylib rgb driver
 * s4582547_reg_rgb_brightness_write() - set brightness of RGB LED 
 * s4582547_reg_rgb_brightness_read() - set brightness of RGB LED 
 * s4582547_reg_rgb_colour() - set brightness of RGB LED 
 *************************************************************** 
*/

#include "processor_hal.h"
#include "board.h"
#include "s4582547_rgb.h"
#include "debug_log.h"

#define GPIO_PULLDOWN_SETTING 0x02
#define GPIO_RGB_MODE 0x01
#define GPIO_SPEED_SETTING 0x03
#define S2_BRIGHTNESS_PIN 0x09
#define S2_BRIGHTNESS_PIN_AFHR 0x01

#define TIMER_COUNTER_FREQ  				20000           				//Timer Frequency 
#define PWM_MAX_PULSE_WIDTH_TICKS		    100					            //5ms is the maximum pulse width. 
#define PWM_PULSE_WIDTH_TICKS				200 					        //Period length with max pulse width in timer ticks

/*
 * Initialise the registers & hardware interface
 * for the RGB LED.
 */
void s4582547_reg_rgb_init() {

     //Hardware init for brightness PWM
    __TIM1_CLK_ENABLE();
    GPIOE->MODER |= (0x02 << (S2_BRIGHTNESS_PIN * 2));
    GPIOE->PUPDR &= ~(0x03 << (S2_BRIGHTNESS_PIN * 2));		
    GPIOE->OSPEEDR |= (0x02<<(S2_BRIGHTNESS_PIN * 2));
    GPIOE->AFR[1] &= ~((0x0F) << (S2_BRIGHTNESS_PIN_AFHR * 4));			//Clear Alternate Function for pin (ARFH register)
    GPIOE->AFR[1] |= (GPIO_AF1_TIM1 << (S2_BRIGHTNESS_PIN_AFHR * 4));	//Set Alternate Function for pin (ARFH register) 
    /* Compute the prescaler value
	   Set the clock prescaler to 20KHz
	   SystemCoreClock is the system clock frequency */
    TIM1->PSC = ((SystemCoreClock) / TIMER_COUNTER_FREQ) - 1; 
    TIM1->CR1 &= ~(1 << 4);    // Counting direction: 0 = up-counting,
    TIM1->CR1 |= (TIM_CR1_ARPE); 	// Set Auto-Reload Preload Enable
    TIM1->CR1 |= 0x01;	// Enable the counter

    TIM1->ARR = PWM_PULSE_WIDTH_TICKS; 	//Set period to 10ms as Period = 1/f
    
    TIM1->CCMR1 &= ~(TIM_CCMR1_OC1M); 	// Clear OC1M (Channel 1) 
    TIM1->CCMR1 |= (0x06 << 4); 		// Enable PWM Mode 1, upcounting, on Channel 1
    TIM1->CCMR1 |= (TIM_CCMR1_OC1PE); 	// Enable output preload bit for channel 1
    
    TIM1->CCER |= TIM_CCER_CC1E; 	// Set CC1E Bit 
    TIM1->CCER &= ~TIM_CCER_CC1NE; 	// Clear CC1NE Bit
    TIM1->CCR1 = (uint16_t)  (50 * PWM_MAX_PULSE_WIDTH_TICKS) / 100; 
    
    /* Set Main Output Enable (MOE) bit - Enable clock output
      Set Off-State Selection for Run mode (OSSR) bit - When OFF, OCN is forced with inactive level
      Set Off-State Selection for Idle mode (OSSI) bit  - When IDLE, OCN is forced with idle level
    */
    TIM1->BDTR |= TIM_BDTR_MOE | TIM_BDTR_OSSR | TIM_BDTR_OSSI;


    // Set Output types for GPIO's
    GPIOE->MODER |= (GPIO_RGB_MODE << (11 * 2) | GPIO_RGB_MODE << (13 * 2));

    // Set RBG Outputs for s2, Pullup/down & Output speed reg's
    GPIOE->PUPDR &= ~(0x03 << (11 * 2) | 0x03 << (13 * 2));  
    GPIOE->PUPDR |= (GPIO_PULLDOWN_SETTING << (11 * 2) | GPIO_PULLDOWN_SETTING << (13 * 2));  // Set Pull down for GPIOE
    GPIOE->OSPEEDR |=  (GPIO_SPEED_SETTING << (11 * 2) | GPIO_SPEED_SETTING << (13 * 2));
    GPIOE->OTYPER &=  ~((0 << 11) | (0 << 13)); //For Push Pull


    // Set Output types for GPIOF
    GPIOF->MODER |= (GPIO_RGB_MODE << (14 * 2));
    GPIOF->PUPDR &= ~(0x03 << (14 * 2));  // Set Pull down for GPIOF
    GPIOF->PUPDR |= (GPIO_PULLDOWN_SETTING << (14 * 2));  // Set Pull down for GPIOF
    GPIOF->OSPEEDR |=  (GPIO_SPEED_SETTING << (14 * 2));
    GPIOE->OTYPER &=  ~((0 << 14)); //For Push Pull
}

/*
 * Set the brightness level on the RGB Led
 */
void s4582547_reg_rgb_brightness_write(int level) {
    TIM1->CCR1 = (uint16_t)  (level * PWM_MAX_PULSE_WIDTH_TICKS) / 100; 
}
/*
 * Read RGBLED's Brightness level from PWM
 */
int s4582547_reg_rgb_brightness_read() {
    return TIM1->CCR1;
}

/*
 * Set the RGB LED's colour basedon the given mask
 */
void s4582547_reg_rgb_colour(unsigned char rgb_mask) {
    // RED -> GPIOE11 // D5
    // GREEN -> GPIOF14 // D4
    // BLUE -> GPIOE13 // D3 
    //Clear the data registers prior to setting the colour
    GPIOE->ODR &= ~(0x01 << 11 | 0x01 << 13);
    GPIOF->ODR &= ~(0x01 << 14);
    switch(rgb_mask){
        case 0x00: //BLACK
            GPIOE->ODR &= ~(0x01 << 11 | 0x01 << 13);
            GPIOF->ODR &= ~(0x01 << 14);
            break;
        case 0x01: //Blue
            GPIOE->ODR |= (0x01 << 13);
            break;
        case 0x02: //Green
            GPIOF->ODR |= (0x01 << 14);
            break;
        case 0x03: //Cyan
            GPIOE->ODR |= (0x01 << 13);
            GPIOF->ODR |= (0x01 << 14);
            break;
        case 0x04: //Red
            GPIOE->ODR |= (0x01 << 11);
            break;
        case 0x05: //Magenta
            GPIOE->ODR |= (0x01 << 11 | 0x01 << 13);
            break;
        case 0x06: //Yelow
            GPIOE->ODR |= (0x01 << 11);
            GPIOF->ODR |= (0x01 << 14);
            break;
        case 0x07: //White
            GPIOE->ODR |= (0x01 << 11 | 0x01 << 13);
            GPIOF->ODR |= (0x01 << 14);
            break;
    }
    // debug_log(" Mask %d R %d G %d B %d ", rgb_mask, 1 && GPIOE->ODR >> 11, 1 && GPIOF->ODR >> 14, 1 && GPIOE->ODR >> 13);
}

