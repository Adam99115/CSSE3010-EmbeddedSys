/** 
 **************************************************************
 * @file mylib/s4582547_txradio.h
 * @author Arthur Mitchell - 45825473
 * @date 1/05/2023
 * @brief Header file for Radio Trasmitter for ASC Project
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4582547_txradio_init() - intialise mylib TX Radio driver
 *************************************************************** 
*/


#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

typedef struct {
	int messageType;
	int rot;
	int xyz[3];
	char* payLoadString;
} Packet; 

static int TXRadioCurrentState;

#define TXCHECKQUEUE 1
#define TXCONSTRUCT 2
#define TXENCODE 3
#define TXTRANSMIT 4


#define JOINMESSAGE 1
#define XYZMESSAGE 2
#define ROTMESSAGE 3
#define VACUUMMESSAGE 4


QueueHandle_t s4582547_txradio_get_message_queue();
void s4582547_txradio_init();
void s4582547_txradio_task_control_radio();
void txradio_fsmprocessing();
void construct_tx_packet();
