/** 
 **************************************************************
 * @file mylib/s4582547_txradio.c
 * @author Arthur Mitchell - 45825473
 * @date 1/05/2023
 * @brief Header file for Radio Trasmitter for ASC Project
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4582547_txradio_init() - intialise mylib TX Radio driver
 * s4582547_txradio_task_control_radio - Main controlling task
 *************************************************************** 
*/
#include "board.h"
#include "processor_hal.h"
#include "debug_log.h"

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "queue.h"
#include "semphr.h"

#include <string.h>
#include "s4582547_hamming.h"
#include "s4582547_txradio.h"
#include "nrf24l01plus.h"

// #define EXTRARADIODEBUG 1

Packet* transmitPacket;
int packetBytes[16];
uint8_t encodedPacketBytes[32];
QueueHandle_t radioTxQueue;	// Queue used

/*
 * Initialisation function for the NRFl2401Plus Radio Transmitter
 */
void s4582547_txradio_init() {
    nrf24l01plus_init();

	radioTxQueue = xQueueCreate(30, sizeof(Packet));
}


/*
 * Getter function that can be called to return the queue.
 */
QueueHandle_t s4582547_txradio_get_message_queue() {
	return radioTxQueue;
}

/*
 * Helper function to construct packets ready for transmission.
 * Packets are first constructed in BigEndian byte order then reverse at the end.
 */
void construct_tx_packet() {
	int type = transmitPacket->messageType;
	uint8_t senderAddress[] = {0x45, 0x82, 0x54, 0x73};
	// uint8_t senderAddress[] = {0x54, 0x28, 0x45, 0x73};
	
	for (int addrIndex = 0; addrIndex < 4; addrIndex++) {
		packetBytes[1+addrIndex] = senderAddress[addrIndex];
	}
	for (unsigned int payLoadStrIndex = 0; payLoadStrIndex < strlen(transmitPacket->payLoadString); payLoadStrIndex++) {
		packetBytes[5+payLoadStrIndex] = transmitPacket->payLoadString[payLoadStrIndex];
	}
	int paddingCount = 0;
	int paddingOffset = 0;

	switch (type) {
		case 0:
			//JOIN
			packetBytes[0] = 0x20;
			paddingCount = 11;
			paddingOffset = 9;
			break;
		case 1:
			packetBytes[0] = 0x22;
			for (int xyzOffset = 0; xyzOffset < 3; xyzOffset++){
				int tempxyz = transmitPacket->xyz[xyzOffset];
				for (int charoffset = 2; charoffset >= 0; charoffset--) {
					if (xyzOffset == 2 && charoffset == 2) {
						continue;
					}
					int digit = tempxyz % 10;
					packetBytes[8 + (xyzOffset * 3) + charoffset] = digit + 48; //ASCII Offset
					tempxyz = tempxyz / 10;
					//Following if loop needed as the z loop only has two digits
					
				}
			}
			break;
		case 2:
			packetBytes[0] = 0x23;
			int rotTemp = transmitPacket->rot;
			for (int charoffset = 2; charoffset >= 0; charoffset--) {
					int digit = rotTemp % 10;
					packetBytes[8 + (charoffset)] = digit + 48; 
					rotTemp = rotTemp / 10;
			}
			paddingCount = 8;
			paddingOffset = 11;
			break;
		case 3:
			packetBytes[0] = 0x24;
			switch ((int) strlen(transmitPacket->payLoadString)) {
				case 4:
					paddingCount = 10;
					paddingOffset = 9;
					break;
				case 3:
					paddingCount = 11;
					paddingOffset = 8;
					break;
			}
			break;
	}
	for (int pad = 0; pad < paddingCount; pad++) {
		packetBytes[paddingOffset + pad] = 0x00;
	}
	
}

/*
 *  Finite State Machine for the NRFl2401Plus Radio
 */
void txradio_fsmprocessing() {
	switch(TXRadioCurrentState) {
		case TXCHECKQUEUE:
			
				if (s4582547_txradio_get_message_queue() != NULL) {	// Check if queue exists 
					if (xQueueReceive(s4582547_txradio_get_message_queue(), &transmitPacket, 10 ) == pdTRUE) {
						// debug_log("PACKET RECEIVED ?\n\r");
						TXRadioCurrentState = TXCONSTRUCT;
					}
				}
			break;
		case TXCONSTRUCT:
			construct_tx_packet();
			#ifdef EXTRARADIODEBUG
				debug_log("Raw Packet Char ");
				for (int i = 0; i < 16; i++) {
					debug_log("%c, ", packetBytes[i]);
				}
				debug_log("\n\r");
				debug_log("Raw Packet Hex ");
				for (int i = 0; i < 16; i++) {
					debug_log("%x, ", packetBytes[i]);
				}
				debug_log("\n\r");
			#endif
			TXRadioCurrentState = TXENCODE;
			break;
		case TXENCODE:
			for (int endianIndex = 0; endianIndex < 16; endianIndex++){
				uint8_t firsthalf = packetBytes[endianIndex] >> 4;
				uint8_t secondhalf = packetBytes[endianIndex] << 4;
				uint8_t littleEndian = firsthalf | secondhalf;
				packetBytes[endianIndex] = littleEndian;
			}
			for (int encodedIndex = 0; encodedIndex < 32; encodedIndex+=2) {
				unsigned short byteToBeEncoded= s4582547_lib_hamming_byte_encode(packetBytes[encodedIndex / 2]);
				//WILL NEED TO CHECK IF ENDIANESS IS CORRECT, IF WRONG SWITCH INDEX THAT GETS >>
				encodedPacketBytes[encodedIndex] = byteToBeEncoded >> 8;
				encodedPacketBytes[encodedIndex+1] = byteToBeEncoded;
			}
			#ifdef EXTRARADIODEBUG
				debug_log("Encoded Packet ");
				for (int i = 0; i < 32; i++) {
					debug_log("%x, ", encodedPacketBytes[i]);
				}
				debug_log("\n\r");
			#endif
			TXRadioCurrentState = TXTRANSMIT;
			break;
		case TXTRANSMIT:
			taskENTER_CRITICAL();

  			nrf24l01plus_init();
			vTaskDelay(50);

			BRD_LEDBlueToggle();

			nrf24l01plus_send(encodedPacketBytes);

			vPortFree(transmitPacket);
			TXRadioCurrentState = TXCHECKQUEUE;
			
			taskEXIT_CRITICAL();
			break;
	}

}

/*
 *  Main processing task for the radio, is called by s4582547_ascsys.c and turned on / off
 *  By ascext.c depending on whether E or F has been pressed on the keypad.
 */
void s4582547_txradio_task_control_radio() {
taskENTER_CRITICAL();
TXRadioCurrentState = TXCHECKQUEUE;
taskEXIT_CRITICAL();

  for (;;) {
    txradio_fsmprocessing();

    vTaskDelay(25);
  }
}
