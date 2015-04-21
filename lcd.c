#include "drivers/mss_uart/mss_uart.h"
#include <inttypes.h>
#include "lcd.h"


static struct Cursor lcdCursor;
static enum OPTION currentOption;

void lcdInit() {
	// Init cursor
	lcdCursor.x = 0;
	lcdCursor.y = 0;
	// Init OPTION
	currentOption = GLOVE_CONTROL;
	// Init uart
	MSS_UART_init
	(
	    &g_mss_uart1,
	    MSS_UART_9600_BAUD,
	    MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT
	);
	// Set brightness full
	uint8_t cmd[2];
	cmd[0] = 0x7C; // Brightness char
	cmd[1] = 157; // Full bright
    MSS_UART_polled_tx( &g_mss_uart1, cmd, sizeof(cmd) );
    // Clear Screen
    lcdClearScreen();
    // Display welcome message
    //lcdWriteMessage("                    ");
    lcdWriteMessage("    Robotic Hand    ");
    lcdWriteMessage("    EECS 373 F14    ");
    lcdWriteMessage("   Steven Sloboda   ");
    lcdWriteMessage("     Loren Wang     ");
    // Delay
    int i; for(i=0;i<10000000;++i) 1;
    //Display an option
    lcdNextOption();
}

void lcdClearScreen() {
	// Send clear screen command
	uint8_t message[2];
	message[0]=254; // Command char
	message[1]=1; // Clear screen
    MSS_UART_polled_tx( &g_mss_uart1, message, sizeof(message) );
    // Note Cursor value is 0 0
    lcdCursor.x = 0;
    lcdCursor.y = 0;
    // Delay
    int j; for(j=0;j<1000000;++j) 1;
}

void lcdMoveCursorTo(uint8_t row, uint8_t col) {

}

void lcdWriteMessage(const char message[]) {
	// Copy message into correct format
	uint8_t m[strlen(message)];
	int i;
	for (i = 0; i < strlen(message); ++i) {
		m[i] = message[i];
	}
	// Send message
	MSS_UART_polled_tx( &g_mss_uart1, m, sizeof(m) );
}

uint8_t lcdGetCursorX() {
	return lcdCursor.x;
}

uint8_t lcdGetCursorY() {
	return lcdCursor.y;
}

void lcdNextOption(){
	lcdClearScreen();
	switch(currentOption) {
	case GLOVE_CONTROL:
		++currentOption;
		lcdWriteMessage("High Five");
		break;
	case HIGH_FIVE:
			++currentOption;
			lcdWriteMessage("Fist Bump");
			break;
	case FIST_BUMP:
			++currentOption;
			lcdWriteMessage("Count to Five");
			break;
	case COUNT_TO_FIVE:
			++currentOption;
			lcdWriteMessage("Rock Paper Scissors");
			break;
	case ROCK_PAPER_SCISSORS:
			++currentOption;
			lcdWriteMessage("Peace");
			break;
	case PEACE:
			++currentOption;
			lcdWriteMessage("Good or Bad");
			break;
	case THUMBS_UP_DOWN:
			++currentOption;
			lcdWriteMessage("Horns");
			break;
	case HORNS:
			++currentOption;
			lcdWriteMessage("OK");
			break;
	case OK:
			++currentOption;
			lcdWriteMessage("Analog Joystick");
			break;
	case JOYSTICK:
			currentOption = GLOVE_CONTROL;
			lcdWriteMessage("Glove Control");
			break;
	}
}

void lcdSameOption() {
	lcdClearScreen();
		switch(currentOption) {
		case HIGH_FIVE:
			lcdWriteMessage("High Five");
			break;
		case FIST_BUMP:
				lcdWriteMessage("Fist Bump");
				break;
		case COUNT_TO_FIVE:
				lcdWriteMessage("Count to Five");
				break;
		case ROCK_PAPER_SCISSORS:
				lcdWriteMessage("Rock Paper Scissors");
			break;
		case GLOVE_CONTROL:
			lcdWriteMessage("Glove Control");
			break;
		case PEACE:
			lcdWriteMessage("Peace");
			break;
		case THUMBS_UP_DOWN:
					lcdWriteMessage("Good or Bad");
					break;
		case HORNS:
					lcdWriteMessage("Horns");
					break;
		case OK:
					lcdWriteMessage("OK");
					break;
		case JOYSTICK:
					lcdWriteMessage("Thumb");
					break;

		}


}

enum OPTION lcdGetCurrentOption(){
	return currentOption;
}

/*void lcdSetSplashScreen() {
	lcdWriteMessage("  EECS 373 Project  ");
	lcdWriteMessage(" sloboste &  wloren ");
	uint8_t c[2];
	c[0] = 124;
	c[1] = 10;
	MSS_UART_polled_tx( &g_mss_uart1, c, sizeof(c) );
}*/
