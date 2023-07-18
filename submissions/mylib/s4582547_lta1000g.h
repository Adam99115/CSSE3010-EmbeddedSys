/** 
 **************************************************************
 * @file mylib/s4582547_lta1000g.h
 * @author Arthur Mitchell - 45825473
 * @date 27/02/2023
 * @brief LED Light Bar peripheral driver header.
 * REFERENCE: REFERENCE: csse3010_mylib_reg_lta1000g.pdf 
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4582547_reg_lta1000g_init() - Intialise LEDBAR driver.
 * s4582547_reg_lta1000g_write() - Write number segment to LEDBAR.
 * s4582547_lta_lta1000g_invert() - Write inverted value with mask to LEDBAR. 
 *************************************************************** 
 */

void s4582547_reg_lta1000g_init();
void s4582547_reg_lta1000g_write(unsigned short value);
void s4582547_lta_lta1000g_invert(unsigned short value, unsigned short mask);
void lta1000g_set_set(int segment, unsigned char value);
