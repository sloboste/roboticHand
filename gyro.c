#include "gyro.h"
#include "drivers/mss_spi/mss_spi.h"
#include "inttypes.h"
#include <stdio.h>

static int32_t gyroOffset = 0;
static double gyroAngle = 0.00;

/*
 *
 */
static uint8_t read(uint8_t reg) {
	// Calculate tx frame
	uint16_t master_tx_frame = ((uint16_t) (reg) << 8) | (uint16_t) 0x8000;
	uint8_t  rx_buffer;

	// SPI read gyroscope
	MSS_SPI_set_slave_select( &g_mss_spi1, MSS_SPI_SLAVE_0 );
	rx_buffer = MSS_SPI_transfer_frame( &g_mss_spi1, master_tx_frame );
	MSS_SPI_clear_slave_select( &g_mss_spi1, MSS_SPI_SLAVE_0 );

	// Debugging
	//printf("master_tx_frame = %x\n\r", master_tx_frame);
	//printf("rx_buffer = %x\n\r", rx_buffer);

	return rx_buffer;
}

/*
 *
 */
static void write(uint8_t reg, uint8_t val) {
	// Calculate tx frame
	uint16_t master_tx_frame = ((uint16_t) (reg) << 8) | (uint16_t) (val);

	// SPI write gyroscope
	MSS_SPI_set_slave_select( &g_mss_spi1, MSS_SPI_SLAVE_0 );
	MSS_SPI_transfer_frame( &g_mss_spi1, master_tx_frame );
	MSS_SPI_clear_slave_select( &g_mss_spi1, MSS_SPI_SLAVE_0 );

	// Debugging
	//printf("master_tx_frame = %x\n\r", master_tx_frame);

}

void gyroInit(){
	// Set up SPI communication
	MSS_SPI_init(&g_mss_spi1);
	MSS_SPI_configure_master_mode
	(
		&g_mss_spi1,
		MSS_SPI_SLAVE_0,
		MSS_SPI_MODE3,
		MSS_SPI_PCLK_DIV_256,
		16 //frame size
	);
	// Verify whoami
	printf("I am %x\r\n", read(0x0f));
	printf("I should be 0xD7\r\n");
	// Set control reg1 valsss
	write(0x20, 0x0F); // Enable x,y,z, power on, set odr 95 hz, set cutoff 12.5hz
	write(0x23, 0x10); // Set sensitivity to 500DPS
}

void gyroCalibrate() {
	// Reset offset
	gyroOffset = 0;
	// Calculate mean of lots of samples
	int numSamples = 1000;
	long long average = 0;
	int i;
	int x=0;
	for (i = 0; i < numSamples; ++i) {
		int temp = gyroGetY();
		if( temp <0 ){
			average += temp;
			++x;
		}
	}
	average /= x;
	gyroOffset = (int32_t) average;
	gyroAngle = 0;
}


int32_t gyroGetY() {
	// read y from gyro
	uint16_t yl = (uint16_t) read(0x2A);
	//printf("Y_h = %d\n\r", yl);
	uint16_t yh = (uint16_t) read(0x2B);
	//printf("Y_l = %d\n\r", yh);
	int16_t y = yl;
	y += yh << 8;
	//printf("Y = %d\n\r", y);
	return y;
}


int32_t gyroCalcY() {
	int32_t y = gyroGetY();
	//printf("Y = %d\n\r",y);
	int32_t result = (y - gyroOffset) / 7.00; //sensitivity multiplier
	printf("Y (degrees/s) = %d\n\r",result);
	return result;
}


int32_t gyroGetYangle() {
	//int32_t avg =0;
	int i;
	int32_t y = gyroCalcY();
	double dAngle = (double) y / 256;
	printf("dAngle = %g\r\n", dAngle);
	gyroAngle += dAngle;
	//printf("gyroOffset = %d\r\n", gyroOffset);
	return (int32_t) gyroAngle;
}
