#ifndef GYRO_H_
#define GYRO_H_

#include "inttypes.h"

/*
 *
 */
void gyroInit();

/*
 *
 */
void gyroCalibrate();

/*
 *
 */
int32_t gyroGetY();

/*
 *
 */
int32_t gyroCalcY();

/*
 *
 */
int32_t gyroGetYangle();

#endif /* GYRO_H_ */
