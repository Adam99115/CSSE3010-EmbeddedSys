 /** 
 **************************************************************
 * @file mylib/s4582547_joystick.c
 * @author Arthur Mitchell - 45825473
 * @date 27/02/2023
 * @brief Joystick peripheral driver source file.
 * REFERENCE: csse3010_mylib_reg_joystick_pushbutton.pdf 
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4582547_reg_joystick_pb_init()- Intialise joystick press button interface.
 * s4582547_reg_joystick_pb_init() - Initial joystick xy interface.
 * s4582547_reg_joystick_press_get() - Get joyStickPressCounter.
 * s4582547_reg_joystick_pb_isr() - interrupt service routine for joystick press.
 * s4582547_joystick_press_reset() - Reset joystickPressCounter to 0.
 *************************************************************** 
 */

#include "processor_hal.h"
#include "board.h"
#include "debug_log.h"
#include "s4582547_joystick.h"

#define GPIO_PULLDOWN_SETTING 0x02
#define GPIO_SPEED_SETTING 0x03
#define GPIO_ANALOG_MODE 0x03
#define GPIO_ANALOG_PUPD 0x03

static int joystickPressCounter;
static uint32_t last_isr_call = 0;

ADC_HandleTypeDef AdcxHandle;
ADC_HandleTypeDef AdcyHandle;

ADC_ChannelConfTypeDef AdcxChanConfig;
ADC_ChannelConfTypeDef AdcyChanConfig;

/*
 * Initialise Hardware
 */
void s4582547_reg_joystick_pb_isr() {
    if ((HAL_GetTick() - last_isr_call) > (uint32_t)200) {
        if (GPIOA->IDR & (0x0001 << 3) != 1) {
            joystickPressCounter += 1;   
        } 
    } 
        last_isr_call = HAL_GetTick();
}

/*
 * Return the number of counts for Joystick Presses
 */
int s4582547_reg_joystick_press_get() {
    return joystickPressCounter;
}

/*
 * Reset the count for the number of Joystick Presses
 */
void s4582547_joystick_press_reset() {
    joystickPressCounter = 0;
}

/*
 * Initialise the hardware registers for the Joystick's push button.
 */
void s4582547_reg_joystick_pb_init() {

    s4582547_joystick_press_reset();
   
    
    // Set Joystick button Control, Pullup/down & Output speed reg's
    GPIOA->PUPDR |= (GPIO_PULLDOWN_SETTING << (3));  // Set Pull down for GPIOA
    GPIOA->OSPEEDR |=  (GPIO_SPEED_SETTING << (3 * 2));

	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;   // Enable EXTI clock

	//select trigger source (port A, pin 3) on EXTICR1. 
	SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI3;
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI3_PA;

	EXTI->RTSR &= ~EXTI_RTSR_TR3;	//Disable rising edge
	EXTI->FTSR |= EXTI_FTSR_TR3;	//Enable falling edge
	EXTI->IMR |= EXTI_IMR_IM3;		//Enable external interrupt

	HAL_NVIC_SetPriority(EXTI3_IRQn, 10, 0);
    HAL_NVIC_SetPriority(SysTick_IRQn, 6, 0);
	HAL_NVIC_EnableIRQ(EXTI3_IRQn);
  
}

/*
 * Initialise the hardware registers for the XY functionality of
*  the Joystick
 */
void s4582547_reg_joystick_init() {
    // Set Joystick XY Control
    GPIOC->MODER |= (GPIO_ANALOG_MODE << (3 * 2) | GPIO_ANALOG_MODE << (0 * 2));
    GPIOC->OSPEEDR &= ~(GPIO_SPEED_SETTING<<(3 * 2) | GPIO_SPEED_SETTING<<(0 * 2));
	GPIOC->OSPEEDR |= (0x02<<(3 * 2) |  0x02<<(0 * 2)); // Fast speed
	GPIOC->PUPDR &= ~(GPIO_ANALOG_PUPD << (3) | GPIO_ANALOG_PUPD << (0));			//Clear bits for no push/pull

    //Joystick ADC
    __ADC1_CLK_ENABLE();						                            //Enable ADC1 clock
    AdcxHandle.Instance = (ADC_TypeDef *)(ADC1_BASE);						//Use ADC2 (For PC0 or A1)
    AdcxHandle.Init.ClockPrescaler        = ADC_CLOCKPRESCALER_PCLK_DIV2;	//Set clock prescaler
    AdcxHandle.Init.Resolution            = ADC_RESOLUTION12b;				//Set 12-bit data resolution
    AdcxHandle.Init.ScanConvMode          = DISABLE;
    AdcxHandle.Init.ContinuousConvMode    = DISABLE;
    AdcxHandle.Init.DiscontinuousConvMode = DISABLE;
    AdcxHandle.Init.NbrOfDiscConversion   = 0;
    AdcxHandle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;	//No Trigger
    AdcxHandle.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T1_CC1;    
    AdcxHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;				
    AdcxHandle.Init.NbrOfConversion       = 1;
    AdcxHandle.Init.DMAContinuousRequests = DISABLE;
    AdcxHandle.Init.EOCSelection          = DISABLE;
    HAL_ADC_Init(&AdcxHandle);
    AdcxChanConfig.Channel = ADC_CHANNEL_10;					//PC0 has Analog Channel 10 connected
    AdcxChanConfig.Rank         = 1;
    AdcxChanConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
    AdcxChanConfig.Offset       = 0;      
    HAL_ADC_ConfigChannel(&AdcxHandle, &AdcxChanConfig);		//Initialise ADC Channel 

    __ADC2_CLK_ENABLE();						                            //Enable ADC1 clock
    AdcyHandle.Instance = (ADC_TypeDef *)(ADC2_BASE);						//Use ADC2 (For PC3 or A2)
    AdcyHandle.Init.ClockPrescaler        = ADC_CLOCKPRESCALER_PCLK_DIV2;	//Set clock prescaler
    AdcyHandle.Init.Resolution            = ADC_RESOLUTION12b;				//Set 12-bit data resolution
    AdcyHandle.Init.ScanConvMode          = DISABLE;
    AdcyHandle.Init.ContinuousConvMode    = DISABLE;
    AdcyHandle.Init.DiscontinuousConvMode = DISABLE;
    AdcyHandle.Init.NbrOfDiscConversion   = 0;
    AdcyHandle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;	//No Trigger
    AdcyHandle.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T1_CC1;	//No Trigger
    AdcyHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;			//Right align data
    AdcyHandle.Init.NbrOfConversion       = 1;
    AdcyHandle.Init.DMAContinuousRequests = DISABLE;
    AdcyHandle.Init.EOCSelection          = DISABLE;
    HAL_ADC_Init(&AdcyHandle);
    AdcyChanConfig.Channel = ADC_CHANNEL_13;					            //PC3 has Analog Channel 13 connected
    AdcyChanConfig.Rank         = 1;
    AdcyChanConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
    AdcyChanConfig.Offset       = 0;
    HAL_ADC_ConfigChannel(&AdcyHandle, &AdcyChanConfig);		            //Initialise ADC Channel
  
}

/*
 * Read the specified ADC value from input being X or Y
 * depending on the specified adc handle passed.
 */
int s4582547_joystick_readxy(ADC_HandleTypeDef AdcHandle) {
  
    HAL_ADC_Start(&AdcHandle);                                              // Start ADC conversion
    while (HAL_ADC_PollForConversion(&AdcHandle, 10) != HAL_OK);
    uint32_t adc_value = AdcHandle.Instance->DR;		                    //Read ADC1 Data Register for converted value
    return adc_value;
}

/*
 * Interrupt handler (ISR) for EXTI3
 */ 
void EXTI3_IRQHandler(void) {
	NVIC_ClearPendingIRQ(EXTI3_IRQn);
	// PR: Pending register
	if ((EXTI->PR & EXTI_PR_PR3) == EXTI_PR_PR3) {
		// cleared by writing a 1 to this bit
        s4582547_reg_joystick_pb_isr();
		EXTI->PR |= EXTI_PR_PR3;	//Clear interrupt flag.
	}
}