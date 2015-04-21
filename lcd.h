#ifndef LCD_H_
#define LCD_H_

#include "drivers/mss_uart/mss_uart.h"
#include <inttypes.h>


struct Cursor {
	uint8_t x;
	uint8_t y;
};

enum OPTION {
	GLOVE_CONTROL = 0,
	HIGH_FIVE = 1,
	FIST_BUMP = 2,
	COUNT_TO_FIVE = 3,
	ROCK_PAPER_SCISSORS = 4,
	PEACE = 5,
	THUMBS_UP_DOWN = 6,
	HORNS = 7,
	OK = 8,
	JOYSTICK = 9
};

/*
 *
 */
void lcdInit();

/*
 *
 */
void lcdClearScreen();

/*
 *
 */
void lcdMoveCursorTo(uint8_t row, uint8_t col);

/*
 *
 */
void lcdWriteMessage(const char message[]);

/*
 *
 */
uint8_t lcdGetCursorX();

/*
 *
 */
uint8_t lcdGetCursorY();

/*
 *
 */
void lcdNextOption();

/*
 *
 */
void lcdSameOption();

/*
 *
 */
enum OPTION lcdGetCurrentOption();

/*
 *
 */
//void lcdSetSplashScreen();

#endif /* LCD_H_ */
