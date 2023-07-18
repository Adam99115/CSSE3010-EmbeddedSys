/** 
 **************************************************************
 * @file mylib/s4582547_joystick.h
 * @author Arthur Mitchell - 45825473
 * @date 27/02/2023
 * @brief Joystick peripheral driver header file.
 * REFERENCE: csse3010_mylib_reg_joystick_pushbutton.pdf 
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4582547_reg_joystick_pb_init()- intialise joystick driver.
 * s4582547_reg_joystick_press_get() - Get joyStickPressCounter.
 * s4582547_reg_joystick_pb_isr() - interrupt service routine for joystick press.
 * s4582547_joystick_press_reset() - Reset joystickPressCounter to 0.
 *************************************************************** 
 */



void s4582547_reg_joystick_pb_init();
void s4582547_reg_joystick_init();
void s4582547_reg_joystick_pb_isr();
int s4582547_reg_joystick_press_get();
void s4582547_joystick_press_reset();
int s4582547_joystick_readxy(ADC_HandleTypeDef AdcHandle);

static int joystickPressCounter;

extern ADC_HandleTypeDef AdcxHandle;
extern ADC_HandleTypeDef AdcyHandle;

extern ADC_ChannelConfTypeDef AdcxChanConfig;
extern ADC_ChannelConfTypeDef AdcyChanConfig;

#define S4582547_REG_JOYSTICK_X_READ() s4582547_joystick_readxy(AdcxHandle)
#define S4582547_REG_JOYSTICK_Y_READ() s4582547_joystick_readxy(AdcyHandle)

#define S4582547_REG_JOYSTICK_X_ZERO_CAL_OFFSET 5
#define S4582547_REG_JOYSTICK_Y_ZERO_CAL_OFFSET -10