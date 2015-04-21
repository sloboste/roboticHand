#include <inttypes.h>
#include <stdio.h>
#include "drivers/mss_ace/mss_ace.h"
#include "drivers/mss_gpio/mss_gpio.h"
#include "drivers/mss_i2c/mss_i2c.h"
#include "drivers/mss_spi/mss_spi.h"
#include "gyro.h"
#include "lcd.h"
#include "mytimer.h"


#define target 0b1001000 // for ADC chip

/*
SLAVE0 = POLLING TIMER
timer_overflow= 0x40050000
timer_counter= 0x40050004
timer_control= 0x40050008
timer_compare= 0x4005000C
timer_interrupt = 0x40050010

SLAVE1 = CONTROL
servo0= 0x40050100 thumb
servo1= 0x40050104 index
servo2= 0x40050108 middle
servo3= 0x4005010C ring
servo4= 0x40050110 pinky
servo5= 0x40050114 bend
servo6= 0x40050118 rotate
joystickPress= 0x4005011C


SLAVE2 = EXTRA TIMER
timer_overflow= 0x40050200
timer_counter= 0x40050204
timer_control= 0x40050208
timer_compare= 0x400503=20C
timer_interrupt = 0x40050210
 */

const uint32_t SERVO_REG_BASE = (uint32_t) (0x40050100);
enum servoEnum {THUMB = 0, INDEX = 1, MIDDLE = 2, RING = 3, PINKY = 4, BEND = 5, ROTATE = 6};
static enum servoEnum current_servo = THUMB;

enum inputModeEnum {GLOVE, JOYSTCK, NONE};
static enum inputModeEnum input_mode = NONE;

uint16_t adc_prev1[6] = {0,0,0,0,0,0};
uint16_t adc_prev2[6] = {0,0,0,0,0,0};
uint16_t adc_prev3[6] = {0,0,0,0,0,0};
uint16_t adc_prev4[6] = {0,0,0,0,0,0};
uint16_t adc_prev5[6] = {0,0,0,0,0,0};
uint32_t joy_prev[3] = {0,0,0};



// ACE channel handlers
ace_channel_handle_t adc_handler2;
ace_channel_handle_t adc_handler3;
ace_channel_handle_t adc_handler4;
ace_channel_handle_t adc_handler5;
ace_channel_handle_t adc_handler6;
ace_channel_handle_t adc_handler7;

// Functions
void nextServo();
uint32_t calcServoVal(int servoNum, float voltage);
void doHighFive();
void doFistBump();
void doCountToFive();
void doRPS();
void doJoystick();
void setServoPosition(enum servoEnum servo, int angle);
void handToFist();
void handToPaper();
void handToScissors();
void doPeace();
void doOK();
void doThumb();
void doHorns();
void wait(double seconds);

volatile int waiting = 0;


///////////////////////////////////////////////////////////////////////////////////////

__attribute__ ((interrupt)) void Fabric_IRQHandler( void )
{
	// Polling Timer interrupt
	printf("Polling timer interrupt (FABINT) occurred at %lu\n\r", MYTIMER_getCounterVal(1));

	if(input_mode == GLOVE){
		uint16_t adc_data[6];
		// Poll flex sensors from ADCs
		int i;
		for (i = 0; i < 6; ++i) {
			switch(i) {
			case THUMB:
				adc_data[i] = ACE_get_ppe_sample(adc_handler2);
				break;
			case INDEX:
				adc_data[i] = ACE_get_ppe_sample(adc_handler3);
				break;
			case MIDDLE:
				adc_data[i] = ACE_get_ppe_sample(adc_handler4);
				break;
			case RING:
				adc_data[i] = ACE_get_ppe_sample(adc_handler5);
				break;
			case PINKY:
				adc_data[i] = ACE_get_ppe_sample(adc_handler6);
				break;
			default: //BEND
				adc_data[i] = ACE_get_ppe_sample(adc_handler7);
				break;
			}
			//printf("adc%d = %d\n\r", i+2, adc_data[i]);
		}

		// Poll gyro and write to gyro
		int yAngle = gyroGetYangle();
		printf("gyro yAngle = %d \n\r", yAngle);
		printf("\r\n\r\n");
		setServoPosition(ROTATE, 90 + yAngle);

		// Calculate voltages and write to servos
		uint32_t * servoReg = (uint32_t*) SERVO_REG_BASE;
		float flexV[6];
		float avg[6];
		for (i = 0; i < 6; ++i) {
			//calculates average
			avg[i]=(adc_data[i]+adc_prev1[i]+adc_prev2[i]+adc_prev3[i]+adc_prev4[i]+adc_prev5[i])/6;
			flexV[i] = (float) (2.56) * (float) (avg[i]) / (float) (4096);
			printf("Flex sensor on ADC %d: Voltage = %f Volts\n\r", i+2, flexV[i]);
			*(servoReg + i) = calcServoVal(i, flexV[i]);

			//shifts prev data backwards
			adc_prev5[i]=adc_prev4[i];
			adc_prev4[i]=adc_prev3[i];
			adc_prev3[i]=adc_prev2[i];
			adc_prev2[i]=adc_prev1[i];
			adc_prev1[i]=adc_data[i];
		}

	} else { // Analog Joystick Control
		uint8_t receive_buf[2];
		float value = 0;
		//uint32_t timeDiff;
		uint32_t * servoReg = (uint32_t*) SERVO_REG_BASE;

		//check if button has been pressed from hardware
		int button = *(servoReg + 7);

		//check if button pressed
		if(button == 1){
			//increment to next finger or joint
			nextServo();
			lcdClearScreen();
			switch(current_servo){
			case THUMB:
				lcdWriteMessage("Thumb");
				break;
			case INDEX:
				lcdWriteMessage("Index");
				break;
			case MIDDLE:
				lcdWriteMessage("Middle");
				break;
			case RING:
				lcdWriteMessage("Ring");
				break;
			case PINKY:
				lcdWriteMessage("Pinky");
				break;
			case ROTATE:
				lcdWriteMessage("Rotate");
				break;
			}
			joy_prev[2]=0;
			joy_prev[1]=0;
			joy_prev[0]=0;
		}

		//read value from buffer
		MSS_I2C_read
		(
			&g_mss_i2c1,
			target,
			receive_buf,
			sizeof(receive_buf),
			MSS_I2C_RELEASE_BUS
		 );

		//MSS_I2C_wait_complete(&g_mss_i2c1, MSS_I2C_NO_TIMEOUT);

		printf("receive_buf[1] = %d\r\n", receive_buf[1]);
		printf("voltage[1] = %f\r\n", (float) (receive_buf[1] * 3.3 / 256));

		//calculates servo pwm value
		value=(float) receive_buf[1]*3.3/256;
		value=value*59375;
		value+=70000;
		printf("pulsewidth = %f\r\n", value);
		printf("current joing = %d\r\n", current_servo);
		uint32_t avg = (value+joy_prev[0]+joy_prev[1]+joy_prev[2])/4;
		//Writes to servo
		*(servoReg + current_servo)= avg;
		//prevtime=time;
		joy_prev[2]=joy_prev[1];
		joy_prev[1]=joy_prev[0];
		joy_prev[0]=(uint32_t) value;

	}
	// Clear interrupt
    NVIC_ClearPendingIRQ( Fabric_IRQn );
}

///////////////////////////////////////////////////////////////////////////////////////

__attribute__ ((interrupt))  void GPIO0_IRQHandler( void )
{
	// Push-button 0 interrupt (next_option button)
	printf("Push-button_0 interrupt (MSS_GPIO_0) occurred\n\r");

	// Display the next option on the LCD
	lcdNextOption();

	// Clear interrupt
	MSS_GPIO_clear_irq( MSS_GPIO_0 );
}

///////////////////////////////////////////////////////////////////////////////////////

__attribute__ ((interrupt))  void GPIO1_IRQHandler( void )
{
	// Push-button 1 interrupt (select_option button)
	printf("Push-button_1 interrupt (MSS_GPIO_1) occurred\n\r");

	// Select the next option on the LCD
	lcdClearScreen();
	int i;
	switch(lcdGetCurrentOption()) {
	case GLOVE_CONTROL:
		//lcdWriteMessage("                    ");
		lcdWriteMessage("Activating glove    ");
		lcdWriteMessage("control!            ");
		// Delay
		for(i=0;i<10000000;++i) 1;
		lcdClearScreen();
		//lcdWriteMessage("                    ");
		lcdWriteMessage("Calibrating         ");
		lcdWriteMessage("gyroscope. Lay arm  ");
		lcdWriteMessage("on table and hold   ");
		lcdWriteMessage("still.");
		// Delay
		for(i=0;i<10000000;++i) 1;
		lcdClearScreen();
		// Calibrate
		gyroCalibrate();
		//lcdWriteMessage("                    ");
		lcdWriteMessage("Calibrated!         ");
		lcdWriteMessage("Glove control       ");
		lcdWriteMessage("Activated!");
		// Delay
		for(i=0;i<10000000;++i) 1;
		lcdClearScreen();
		// Activate glove control
		doGloveControl();
		break;
	case HIGH_FIVE:
		lcdWriteMessage("High five me!");
		// Do high five motion
		doHighFive();
		break;
	case FIST_BUMP:
		lcdWriteMessage("Fist bump me!");
		// Do fist bump motion
		doFistBump();
		break;
	case COUNT_TO_FIVE:
		lcdWriteMessage("I can count to five!");
		// Do count to five motion
		doCountToFive();
		break;
	case ROCK_PAPER_SCISSORS:
		//lcdWriteMessage("                    ");
		lcdWriteMessage("Let's play rock     ");
		lcdWriteMessage("paper scissors!");
		// Do rps motion
		doRPS();
		break;
	case PEACE:
			//lcdWriteMessage("                    ");
			lcdWriteMessage("Peace out!");
			// Do Peace motion
			doPeace();
			break;
	case HORNS:
			//lcdWriteMessage("                    ");
			lcdWriteMessage("Heck Yea!!");
			// Do Horns motion
			doHorns();
			break;
	case THUMBS_UP_DOWN:
			//lcdWriteMessage("                    ");
			lcdWriteMessage("Judging........");
			// Do thumb motion
			doThumb();
			break;
	case OK:
			//lcdWriteMessage("                    ");
			lcdWriteMessage("OkeyDokey");
			// Do Loser motion
			doOK();
			break;
	case JOYSTICK:
		lcdWriteMessage("Joystick Control    ");
		lcdWriteMessage("activated! ");
		// Delay
		int i;
		for (i = 0; i < 10000000; ++i) 1;
		doJoystick();
		break;
	}

	//Display an option
	lcdSameOption();

	// Clear interrupt
	MSS_GPIO_clear_irq( MSS_GPIO_1 );
}

///////////////////////////////////////////////////////////////////////////////////////

__attribute__ ((interrupt))  void GPIO2_IRQHandler( void ) {
	// Extra timer interrupt
	uint32_t time = MYTIMER_getCounterVal(2);
	printf("Extra timer interrupt (MSS_GPIO_2) occurred at %lu\n\r", time);

	// Disable timer
	MYTIMER_disable(2);

	// Stop waiting
	waiting = 0;

	// Clear interrupt
	MSS_GPIO_clear_irq( MSS_GPIO_2 );
}

///////////////////////////////////////////////////////////////////////////////////////

__attribute__ ((interrupt))  void GPIO3_IRQHandler( void ) {
	// Extra timer interrupt
	uint32_t time = MYTIMER_getCounterVal(2);
	printf("Joystick click interrupt (MSS_GPIO_3) occurred at %lu\n\r", time);

	// Change menu option


	// Clear interrupt
	MSS_GPIO_clear_irq( MSS_GPIO_3 );
}

///////////////////////////////////////////////////////////////////////////////////////

int main()
{
	// Setup Interrupt handlers
	NVIC_EnableIRQ(Fabric_IRQn);
	MSS_GPIO_config( MSS_GPIO_0, MSS_GPIO_INPUT_MODE | MSS_GPIO_IRQ_EDGE_POSITIVE ); // config 0 for interrupts
	MSS_GPIO_config( MSS_GPIO_1, MSS_GPIO_INPUT_MODE | MSS_GPIO_IRQ_EDGE_POSITIVE ); // config 1 for interrupts
	MSS_GPIO_config( MSS_GPIO_2, MSS_GPIO_INPUT_MODE | MSS_GPIO_IRQ_EDGE_POSITIVE ); // config 2 for interrupts
	MSS_GPIO_config( MSS_GPIO_3, MSS_GPIO_INPUT_MODE | MSS_GPIO_IRQ_EDGE_POSITIVE ); // config 3 for interrupts
	MSS_GPIO_enable_irq( MSS_GPIO_0 );
	MSS_GPIO_enable_irq( MSS_GPIO_1 );
	MSS_GPIO_enable_irq( MSS_GPIO_2 );
	MSS_GPIO_enable_irq( MSS_GPIO_3 );

	// Setup Polling Timer
	MYTIMER_init(1);
	MYTIMER_setOverflowVal(1, 390625); // Set this to determine sensor poll rate: 256 Hz
	MYTIMER_enable_overflowInt(1);
    MYTIMER_enable_allInterrupts(1);

    // Setup Extra Timer
    MYTIMER_init(2);
    MYTIMER_enable_overflowInt(2);
    MYTIMER_enable_allInterrupts(2);


	// Init LCD
	lcdInit();

	// Declare and initialize ADCs
	ACE_init();
	adc_handler2 = ACE_get_channel_handle((const uint8_t *)"ADCDirectInput_2");
	adc_handler3 = ACE_get_channel_handle((const uint8_t *)"ADCDirectInput_3");
	adc_handler4 = ACE_get_channel_handle((const uint8_t *)"ADCDirectInput_4");
	adc_handler5 = ACE_get_channel_handle((const uint8_t *)"ADCDirectInput_5");
	adc_handler6 = ACE_get_channel_handle((const uint8_t *)"ADCDirectInput_6");
	adc_handler7 = ACE_get_channel_handle((const uint8_t *)"ADCDirectInput_7");

	// Init gyro
	gyroInit();

	//I2C Analog Joystick init
	uint8_t transmit_buf[]=   {0x00};
	MSS_I2C_init(&g_mss_i2c1 , target, MSS_I2C_PCLK_DIV_256 );
	MSS_I2C_write
			(
				&g_mss_i2c1,
				target,
				transmit_buf,
				sizeof(transmit_buf),
				MSS_I2C_RELEASE_BUS
			);
	MSS_I2C_wait_complete(&g_mss_i2c1, MSS_I2C_NO_TIMEOUT);

	// Busy loop
	printf("Entering busy loop\n\r");
	while(1) {
		// Do nothing
		//wait(1);
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////

void nextServo() {
	switch (current_servo) {
	case THUMB:
		current_servo = INDEX;
		break;
	case INDEX:
		current_servo = MIDDLE;
		break;
	case MIDDLE:
		current_servo = RING;
		break;
	case RING:
		current_servo = PINKY;
		break;
	case PINKY:
		current_servo = ROTATE;
		break;
	/*
	case BEND:
		current_servo = ROTATE;
		break;
	 */
	default: /* ROTATE */
		current_servo = THUMB;
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

uint32_t calcServoVal(int servoNum, float voltage) {
	// Value to count to for pwm in servo
	float servoVal;
	// Voltage to servo value equations
	switch (servoNum) {
	case 0: // Thumb
		servoVal = (float) (-430839) * voltage + (float) (925647);
		break;
	case 1: // Index finger
		servoVal = (float) (-311475) * voltage + (float) (700180);
		break;
	case 2: // Middle finger
		servoVal = (float) (-395833) * voltage + (float) (897291);
		break;
	case 3: // Ring finger
		servoVal = (float) (-380000) * voltage + (float) (868000);
		break;
	case 4: // Pinky finger
		servoVal = (float) (-287878) * voltage + (float) (677423);
		break;
	case 5: // Wrist bend
		servoVal = (float) (-380000) * voltage + (float) (1006000);
		break;
	default: //6 Wrist rotate
		servoVal = 0;
		break;
	}
	// Debugging output
	//printf("Servo value %d\r\n", (int) (servoVal));
	return (uint32_t) (servoVal);
}
///////////////////////////////////////////////////////////////////////////////////////
void doPeace(){
	MYTIMER_disable(1);

	setServoPosition(THUMB, 180);
	setServoPosition(INDEX, 0);
	setServoPosition(MIDDLE, 0);
	setServoPosition(RING, 180);
	setServoPosition(PINKY, 180);
	setServoPosition(ROTATE, 90);

	int i;
	for(i=0;i<30000000;++i) 1;
	// Reset hand
	handToPaper();
}
///////////////////////////////////////////////////////////////////////////////////////

void doHorns(){
	MYTIMER_disable(1);

	setServoPosition(THUMB, 0);
	setServoPosition(INDEX, 0);
	setServoPosition(MIDDLE, 180);
	setServoPosition(RING, 180);
	setServoPosition(PINKY, 0);
	setServoPosition(ROTATE, 90);

	int i;
	for(i=0;i<30000000;++i) 1;
	// Reset hand
	handToPaper();
}
///////////////////////////////////////////////////////////////////////////////////////

void doThumb(){
	MYTIMER_disable(1);

	setServoPosition(THUMB, 0);
	setServoPosition(INDEX, 180);
	setServoPosition(MIDDLE, 180);
	setServoPosition(RING, 180);
	setServoPosition(PINKY, 180);
	setServoPosition(ROTATE, 90);

	int i;
	//rotate wrist
	for (i = 0; i < 5000000; ++i) 1;
	setServoPosition(ROTATE, 135);
	for (i = 0; i < 5000000; ++i) 1;
	setServoPosition(ROTATE, 45);
	for (i = 0; i < 5000000; ++i) 1;
	setServoPosition(ROTATE, 135);
	for (i = 0; i < 5000000; ++i) 1;
	setServoPosition(ROTATE, 45);
	for (i = 0; i < 5000000; ++i) 1;
	setServoPosition(ROTATE, 135);
	for (i = 0; i < 5000000; ++i) 1;
	setServoPosition(ROTATE, 45);

	lcdClearScreen();
	int action = rand() % 3;
	if (action == 0) {
		lcdWriteMessage(" (Average)");
		setServoPosition(ROTATE,90);
	} else if (action == 1) {
		setServoPosition(ROTATE,180);
		lcdWriteMessage(" (Good)");
	} else {
		setServoPosition(ROTATE,0);
		lcdWriteMessage(" (Bad) ");
	}

	for(i=0;i<30000000;++i) 1;
	// Reset hand
	handToPaper();
}
///////////////////////////////////////////////////////////////////////////////////////
void doOK(){
	MYTIMER_disable(1);

	setServoPosition(THUMB, 150);
	setServoPosition(INDEX, 180);
	setServoPosition(MIDDLE, 0);
	setServoPosition(RING, 0);
	setServoPosition(PINKY, 0);
	setServoPosition(ROTATE, 90);

	int i;
	for(i=0;i<30000000;++i) 1;
	// Reset hand
	handToPaper();

}

///////////////////////////////////////////////////////////////////////////////////////

void doJoystick(){
	handToPaper(); //Reset hand
	input_mode = JOYSTICK;
	lcdClearScreen();
		switch(current_servo){
		case THUMB:
			lcdWriteMessage("Thumb");
			break;
		case INDEX:
			lcdWriteMessage("Index");
			break;
		case MIDDLE:
			lcdWriteMessage("Middle");
			break;
		case RING:
			lcdWriteMessage("Ring");
			break;
		case PINKY:
			lcdWriteMessage("Pinky");
			break;
		case ROTATE:
			lcdWriteMessage("Rotate");
			break;
		}
	MYTIMER_enable(1);

}

///////////////////////////////////////////////////////////////////////////////////////

void doGloveControl() {
	handToPaper(); //Reset hand
	input_mode = GLOVE;
	MYTIMER_enable(1);
}

///////////////////////////////////////////////////////////////////////////////////////

void doHighFive() {
	// Disable glove control
	MYTIMER_disable(1);
	// Reset hand to open palm
	handToPaper();
	// Delay
	int i;
	for(i=0;i<10000000;++i) 1;
}

///////////////////////////////////////////////////////////////////////////////////////

void doFistBump() {
	// Disable glove control
	MYTIMER_disable(1);
	// Reset hand
	handToPaper();
	// Hand to fist
	handToFist();
	// Delay
	int i;
	for(i=0;i<30000000;++i) 1;
	// Reset hand
	handToPaper();
}

///////////////////////////////////////////////////////////////////////////////////////

void doCountToFive() {
	// Disable glove control
	MYTIMER_disable(1);
	// Reset hand
	handToPaper();
	// Hand to fist
	handToFist();
	// Delay
	int i;
	for (i = 0; i < 10000000; ++i) 1;
	// Open finger by finger
	//lcdWriteMessage("                    ");
	lcdClearScreen();
	lcdWriteMessage("One!                ");
	setServoPosition(INDEX, 0);
	for (i = 0; i < 10000000; ++i) 1;
	//lcdWriteMessage("                    ");
	lcdClearScreen();
	lcdWriteMessage("TWO!                ");
	setServoPosition(MIDDLE, 10);
	for (i = 0; i < 10000000; ++i) 1;
	//lcdWriteMessage("                    ");
	lcdClearScreen();
	lcdWriteMessage("Three!              ");
	setServoPosition(RING, 0);
	for (i = 0; i < 10000000; ++i) 1;
	//lcdWriteMessage("                    ");
	lcdClearScreen();
	lcdWriteMessage("Four!               ");
	setServoPosition(PINKY, 0);
	for (i = 0; i < 10000000; ++i) 1;
	//lcdWriteMessage("                    ");
	lcdClearScreen();
	lcdWriteMessage("Five!               ");
	setServoPosition(THUMB, 0);
	for(i=0;i<10000000;++i) 1;
	// Reset hand
	handToPaper();
}

///////////////////////////////////////////////////////////////////////////////////////

void doRPS() {
	// Disable glove control
	MYTIMER_disable(1);
	// Clear screen
	lcdClearScreen();
	// Reset hand
	handToPaper();
	// Hand to fist
	handToFist();
	//lcdWriteMessage("                    ");
	lcdWriteMessage("Rock!               ");
	// Delay
	setServoPosition(RING,0);
	setServoPosition(MIDDLE,0);
	setServoPosition(INDEX,0);

	int i;
	for (i = 0; i < 10000000; ++i) 1;
	// Hand to paper
	setServoPosition(RING,180);
	//lcdWriteMessage("                    ");
	lcdWriteMessage("Paper!              ");
	// Delay
	for (i = 0; i < 10000000; ++i) 1;
	// Hand to scissors
	setServoPosition(MIDDLE,180);
	//lcdWriteMessage("                    ");
	lcdWriteMessage("Scissors!           ");
	// Delay
	for (i = 0; i < 10000000; ++i) 1;
	// Shoot!
	int action = rand() % 3;
	if (action == 0) {
		handToFist();
	} else if (action == 1) {
		handToPaper();
	} else {
		handToScissors();
	}
	//lcdWriteMessage("                    ");
	lcdWriteMessage("Shoot! ");
	if (action == 0) {
		setServoPosition(ROTATE,180);
		lcdWriteMessage("(Rock)");
	} else if (action == 1) {
		lcdWriteMessage("(Paper)");
	} else {
		setServoPosition(ROTATE,180);
		lcdWriteMessage("(Scissors)");
	}
	// Delay
	for(i=0;i<10000000;++i) 1;
	// Reset hand
	handToPaper();
}

///////////////////////////////////////////////////////////////////////////////////////

void setServoPosition(enum servoEnum servo, int angle) {
	uint32_t * servoReg = (uint32_t*) SERVO_REG_BASE;
	// Servo range:
	// 0   degrees =  70,000
	// 180 degrees = 260,000
	if (angle > 180) {
		angle = 180;
	} else if (angle < 0) {
		angle = 0;
	}
	*(servoReg + servo) = (1056) * angle + 70000;
}

///////////////////////////////////////////////////////////////////////////////////////

void handToFist() {
	setServoPosition(THUMB, 180);
	setServoPosition(INDEX, 180);
	setServoPosition(MIDDLE, 180);
	setServoPosition(RING, 180);
	setServoPosition(PINKY, 180);
	setServoPosition(ROTATE, 90);
}

///////////////////////////////////////////////////////////////////////////////////////

void handToPaper() {
	setServoPosition(THUMB, 0);
	setServoPosition(INDEX, 0);
	setServoPosition(MIDDLE, 0);
	setServoPosition(RING, 0);
	setServoPosition(PINKY, 0);
	setServoPosition(ROTATE, 90);
}

///////////////////////////////////////////////////////////////////////////////////////

void handToScissors() {
	setServoPosition(THUMB, 180);
	setServoPosition(INDEX, 0);
	setServoPosition(MIDDLE, 0);
	setServoPosition(RING, 180);
	setServoPosition(PINKY, 180);
	setServoPosition(ROTATE, 90);
}

///////////////////////////////////////////////////////////////////////////////////////

void wait(double seconds) {
	unsigned int cycles = seconds * 100e6;
	MYTIMER_setOverflowVal(2, cycles);
	waiting = 1;
	MYTIMER_enable(2);
	while (waiting != 0) {
		//printf("waiting\r\n");
	}
}
