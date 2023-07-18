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

extern unsigned short two_bit_error_hamming;
extern unsigned short one_bit_error_hamming;
unsigned short s4582547_lib_hamming_byte_encode(unsigned char value); 
unsigned char s4582547_lib_hamming_byte_decode(unsigned char value);
unsigned char hamming_hbyte_encode(unsigned char value);
int s4582547_lib_hamming_parity_error(unsigned char value);