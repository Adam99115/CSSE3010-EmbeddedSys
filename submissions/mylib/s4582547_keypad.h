 /** 
 **************************************************************
 * @file mylib/s4582547_keypad.h
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


#define KEYPAD_COL1()
#define KEYPAD_COL2()
#define KEYPAD_COL3()
#define KEYPAD_COL4()

static int KeypadFsmCurrentState;
static int KeypadStatus;
static unsigned char KeypadValue;

void s4582547_reg_keypad_init();
void s4582547_reg_keypad_fsmprocessing();
uint8_t  keypad_readrow();
int s4582547_reg_keypad_read_status();
uint8_t s4582547_reg_keypad_read_key();
void s4582547_reg_task_set_on_keypad();
void keypad_writecol(unsigned char colval);
EventGroupHandle_t s4582547_reg_get_keypad_event_handle();

#define INIT_STATE 0
#define RSCAN1_STATE 1
#define RSCAN2_STATE 2
#define RSCAN3_STATE 3 
#define RSCAN4_STATE 4

#define KEYPAD_EVENT_0			1 << 0		//Keypad Button 0 Event Flag
#define KEYPAD_EVENT_1			1 << 1		//Keypad Button 1 Event Flag
#define KEYPAD_EVENT_2		    1 << 2		//Keypad Button 2 Event Flag
#define KEYPAD_EVENT_3		  	1 << 3		//Keypad Button 3 Event Flag
#define KEYPAD_EVENT_4		    1 << 4		//Keypad Button 4 Event Flag
#define KEYPAD_EVENT_5		  	1 << 5		//Keypad Button 5 Event Flag
#define KEYPAD_EVENT_6		    1 << 6		//Keypad Button 6 Event Flag
#define KEYPAD_EVENT_7			1 << 7		//Keypad Button 7 Event Flag
#define KEYPAD_EVENT_8		    1 << 8		//Keypad Button 8 Event Flag
#define KEYPAD_EVENT_9			1 << 9		//Keypad Button 9 Event Flag
#define KEYPAD_EVENT_A		    1 << 10		//Keypad Button A Event Flag
#define KEYPAD_EVENT_B		    1 << 11		//Keypad Button B Event Flag
#define KEYPAD_EVENT_C		    1 << 12		//Keypad Button C Event Flag
#define KEYPAD_EVENT_D		    1 << 13		//Keypad Button D Event Flag
#define KEYPAD_EVENT_E		    1 << 14		//Keypad Button E Event Flag
#define KEYPAD_EVENT_F		    1 << 15		//Keypad Button F Event Flag

#define KEYPAD_EVENT		    0xFFFF	    //Control Event Group Mask
