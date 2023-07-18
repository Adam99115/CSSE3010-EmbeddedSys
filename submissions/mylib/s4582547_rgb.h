/** 
 **************************************************************
 * @file mylib/s4582547_rgb.h
 * @author Arthur Mitchell - 45825473
 * @date 10/03/2023
 * @brief RBG Led peripheral driver header file.
 * REFERENCE: csse3010_mylib_reg_rgb.pdf 
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4582547_reg_rgb_init() - intialise mylib rgb driver
 * s4582547_reg_rgb_brightness_write() - set brightness of RGB LED 
 * s4582547_reg_rgb_brightness_read() - set brightness of RGB LED 
 * s4582547_reg_rgb_colour() - set RGB LED colour 
 *************************************************************** 
*/



void s4582547_reg_rgb_init();
void s4582547_reg_rgb_brightness_write(int level);
int  s4582547_reg_rgb_brightness_read();
void s4582547_reg_rgb_colour(unsigned char rgb_mask);

#define S4582547_REG_RGB_BLACK() s4582547_reg_rgb_colour(0x00);
#define S4582547_REG_RGB_BLUE() s4582547_reg_rgb_colour(0x01);
#define S4582547_REG_RGB_GREEN() s4582547_reg_rgb_colour(0x02);
#define S4582547_REG_RGB_CYAN() s4582547_reg_rgb_colour(0x03);
#define S4582547_REG_RGB_RED() s4582547_reg_rgb_colour(0x04);
#define S4582547_REG_RGB_MAGENTA() s4582547_reg_rgb_colour(0x05);
#define S4582547_REG_RGB_YELLOW() s4582547_reg_rgb_colour(0x06);
#define S4582547_REG_RGB_WHITE() s4582547_reg_rgb_colour(0x07);
