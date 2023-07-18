 /** 
 **************************************************************
 * @file mylib/s4582547_hamming.c
 * @author Arthur Mitchell - 45825473
 * @date 23/03/2023
 * @brief 
 * REFERENCE: csse3010_mylib_reg_hamming.pdf 
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4582547_lib_hamming_byte_encode(unsigned char value) - Return the 16bit encoded value of a byte
 * s4582547_lib_hamming_byte_decode() - Return the decoded half byte from a byte
 * s4582547_lib_hamming_parity_error() - Return if a parity error has occurred, else 0. 
 *************************************************************** 
 */

#include "processor_hal.h"
#include "board.h"
#include "s4582547_hamming.h"
#include "debug_log.h"

// #define DEBUG 1

unsigned short two_bit_error_hamming;
unsigned short one_bit_error_hamming;
/**
  * Implement Hamming Code + parity checking
  * Hamming code is based on 4/7 
  */
unsigned short s4582547_lib_hamming_byte_encode(unsigned char value) {

	uint8_t first_byte = hamming_hbyte_encode(value & 0xF);
	uint8_t second_byte = hamming_hbyte_encode(value >> 4);
	uint16_t out = (first_byte | (second_byte << 8));

	return out;

}

/**
  * 
  * Decodes the given byte into it's respective data bits based on 4/7 
  * hamming encoding. 
  * 
  */
unsigned char s4582547_lib_hamming_byte_decode(unsigned char value) {

	uint8_t sent_parity_0, sent_parity_1, sent_parity_2, sent_main_parity;
	uint8_t expected_main_parity, parity_0, parity_1, parity_2;
	uint8_t data_0, data_1, data_2, data_3;
	data_0 = !!(value & 0x10);
	data_1 = !!(value & 0x20);
	data_2 = !!(value & 0x40);
	data_3 = !!(value & 0x80);
	sent_main_parity = !!(value & 0x01);
	sent_parity_0 = !!(value & 0x2);
	sent_parity_1 = !!(value & 0x4);
	sent_parity_2 = !!(value & 0x8);

	//If sent parity is same as parity calculated 
	//from data bits then the parity will remain 0
	//If calculated bit is different from what is sent
	//then assume that parity bits are correct and flip data bits  
	parity_0 = sent_parity_0 ^ data_1 ^ data_2 ^ data_3; 
	parity_1 = sent_parity_1 ^ data_0 ^ data_2 ^ data_3;
	parity_2 = sent_parity_2 ^ data_0 ^ data_1 ^ data_3;
	
	one_bit_error_hamming = 1;
	if (parity_0 & parity_1 & parity_1) {
		data_3 = !!!data_3;
	} else  if (parity_0 & parity_1 & !parity_2) {
		data_2 = !!!data_2;
	} else 	if (parity_0 & !parity_1 & parity_2) {
		data_1 = !!!data_1;
	} else 	if (!parity_0 & parity_1 & parity_2) {
		data_0 = !!!data_0;
	} else if (!parity_0 & !parity_1 & !parity_2) {
		one_bit_error_hamming = 0;
	}
	
	sent_main_parity = sent_main_parity ^ parity_0 ^ parity_1 ^ parity_2 ^ data_0 ^ data_1 ^ data_2 ^ data_3;
	unsigned char decoded_char = (data_3 << 3) | (data_2 << 2) | (data_1 << 1) | (data_0 << 0); 

	if (sent_main_parity) {
		two_bit_error_hamming = 1;
	} else {
		two_bit_error_hamming = 0;
	}

	return decoded_char;
}

/**
  * Internal helper function to encode a half byte into a 4/7 hamming full byte.
  */
unsigned char hamming_hbyte_encode(unsigned char value) {
	
    uint8_t data_0, data_1, data_2, data_3;
	uint8_t p0 = 0, parity_0, parity_1, parity_2;
	uint8_t z;
	uint8_t out;

	/* extract bits */
	data_0 = !!(value & 0x1);
	data_1 = !!(value & 0x2);
	data_2 = !!(value & 0x4);
	data_3 = !!(value & 0x8);

	/* calculate hamming parity bits */
	parity_0 = data_1 ^ data_2 ^ data_3;
	parity_1 = data_0 ^ data_2 ^ data_3;
	parity_2 = data_0 ^ data_1 ^ data_3;

	/* generate out byte without parity bit P0 */
	out = (parity_0 << 1) | (parity_1 << 2) | (parity_2 << 3) |
		(data_0 << 4) | (data_1 << 5) | (data_2 << 6) | (data_3 << 7);

	/* calculate even parity bit */
	for (z = 1; z<8; z++)
		p0 = p0 ^ !!(out & (1 << z));

	out |= p0;

	return(out);
}

/**
  * External function that determines whether a parity error has occured
  * within the given byte provided to the function. Returns 1 if an error has occured.
  */
int s4582547_lib_hamming_parity_error(unsigned char value) {
	uint8_t sent_parity = value & 0x01;
	uint8_t expected_parity = (value << 7) ^ (value << 6) ^ (value << 5) ^ (value << 4) ^ (value << 3) ^ (value << 2) ^ (value << 1);
	if (sent_parity != expected_parity) {
		return 1;
	} else {
		return 0;
	}
}