 /** 
 **************************************************************
 * @file mylib/s4582547_lta1000g.c
 * @author Arthur Mitchell - 45825473
 * @date 27/02/2023
 * @brief LED Light Bar peripheral driver source.
 * REFERENCE: csse3010_mylib_reg_lta1000g.pdf 
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4582547_reg_lta1000g_init() - Intialise LEDBAR driver.
 * s4582547_reg_lta1000g_write() - Write number segment to LEDBAR.
 * s4582547_lta_lta1000g_invert() - Write inverted value with mask to LEDBAR. 
 *************************************************************** 
 */

#include "processor_hal.h"
#include "board.h"
#include "s4582547_lta1000g.h"

#define GPIO_OUTPUT 0x01
#define GPIO_SPEED 0x03
#define GPIO_PULLDOWN_SET 0x02
#define GPIO_UP 1
#define GPIO_DOWN 0

/*
 * Initialise the registers for the 10 segment LEDBAR
 */
void s4582547_reg_lta1000g_init() {
  
    // Set Mode in Control Registers
    GPIOA->MODER |=  (GPIO_OUTPUT << 2 * 4) | (GPIO_OUTPUT << 2 * 15);
    GPIOB->MODER |=  (GPIO_OUTPUT << 2 * 3) | (GPIO_OUTPUT << 2 * 4) | (GPIO_OUTPUT << 2 * 5) | \
                        (GPIO_OUTPUT << 2 * 12) |  (GPIO_OUTPUT << 2 * 13) | (GPIO_OUTPUT << 2 * 15);
    GPIOC->MODER |=  (GPIO_OUTPUT << 2 * 6) | (GPIO_OUTPUT << 2 * 7);

    // Set Output types for GPIO's
    GPIOA->OTYPER |=  (0 << 4) | (0 << 15);
    GPIOB->OTYPER |=  (0 << 3) | (0 << 4) | (0 << 5) | (0 << 12) |  (0 << 13) | (0 << 15);
    GPIOC->OTYPER |=  (0 << 6) | (0 << 7);

    // Clear Output Speed Registers
    GPIOA->OSPEEDR &= ~(0xAC000000); // Pins 4, 15
    GPIOB->OSPEEDR &= ~(0x000000C0); // Pins 3, 4, 5, 12, 13, 15, 
    GPIOC->OSPEEDR &= ~(0x00000000); // Pins 6, 7

    // Set Output types for GPIO's
    GPIOA->OSPEEDR |=  (GPIO_SPEED << 4) | (GPIO_SPEED << 15);
    GPIOB->OSPEEDR |=  (GPIO_SPEED << 3) | (GPIO_SPEED << 4) | (GPIO_SPEED << 5) | \
                            (GPIO_SPEED << 12) |  (GPIO_SPEED << 13) | (GPIO_SPEED << 15);
    GPIOC->OSPEEDR |=  (GPIO_SPEED << 6) | (GPIO_SPEED << 7);

    // Clear pull up / pull down register
    GPIOA->PUPDR &= ~(0x64000000); 
    GPIOB->PUPDR &= ~(0x00000100);
    GPIOC->PUPDR &= ~(0x00000000); 

    // Set pull up / pull down register for GPIO's
    GPIOA->PUPDR |=  (GPIO_PULLDOWN_SET << 4) | (GPIO_PULLDOWN_SET << 15);
    GPIOB->PUPDR |=  (GPIO_PULLDOWN_SET << 3) | (GPIO_PULLDOWN_SET << 4) | (GPIO_PULLDOWN_SET << 5) | \
                            (GPIO_PULLDOWN_SET << 12) |  (GPIO_PULLDOWN_SET << 13) | (GPIO_PULLDOWN_SET << 15);
    GPIOC->PUPDR |=  (GPIO_PULLDOWN_SET << 6) | (GPIO_PULLDOWN_SET << 7);
}

/*
 * Set the specified segment to be on
 */
void lta1000g_set_seg(int segment, unsigned char value) {
    switch (segment) {
        case 0:
            GPIOC->ODR &= ~(GPIO_UP << 6);
            GPIOC->ODR |= (value << 6);
            break;
        case 1:
            GPIOB->ODR &= ~(GPIO_UP << 15);
            GPIOB->ODR |= (value << 15);
            break;
        case 2:
            GPIOB->ODR &= ~(GPIO_UP << 13);
            GPIOB->ODR |= (value << 13);
            break;
        case 3:
            GPIOB->ODR &= ~(GPIO_UP << 12);
            GPIOB->ODR |= (value << 12);
            break;
        case 4:
            GPIOA->ODR &= ~(GPIO_UP << 15);
            GPIOA->ODR |= (value << 15);
            break;
        case 5:
            GPIOC->ODR &= ~(GPIO_UP << 7);
            GPIOC->ODR |= (value << 7);
            break;
        case 6:
            GPIOB->ODR &= ~(GPIO_UP << 5);
            GPIOB->ODR |= (value << 5);
            break;
        case 7:
            GPIOB->ODR &= ~(GPIO_UP << 3);
            GPIOB->ODR |= (value << 3);
            break;
        case 8:
            GPIOA->ODR &= ~(GPIO_UP << 4);
            GPIOA->ODR |= (value << 4);
            break;
        case 9:
            GPIOB->ODR &= ~(GPIO_UP << 4);
            GPIOB->ODR |= (value << 4);
            break;
    }
}

/*
 * Write specified value to the 10 segment LEDBAR
 */
void s4582547_reg_lta1000g_write(unsigned short value) {
    for (int i = 0; i < 10; i++) {
        if (value >> i & 0x01)  {
            lta1000g_set_seg(i, 1);
        } else {
            lta1000g_set_seg(i, 0);
        }
    }
}

/*
 * Write a specified value that is inverted and bit masked to the 10 segment LEDBAR  
 */
void s4582547_lta_lta1000g_invert(unsigned short value, unsigned short mask) {
    unsigned short inverted = ~value;
    inverted = inverted & mask;
    for (int i = 0; i < 10; i++) {
        if (mask >> i & 0x01) {
            if (inverted >> i & 0x01)  {
                lta1000g_set_seg(i, 1);
            } else {
                lta1000g_set_seg(i, 0);
            } 
        }
      
    }
}

