#ifndef BLE_COM_H
#define BLE_COM_H

#include <BLESerial.h>
#include "motorConfig.h"

// Initialize BLESerial
extern BLESerial<etl::circular_buffer<uint8_t, 255>> SerialBLE;

void initBLE();
void bleUpdate(MotorPID &motor1, MotorPID &motor2);

#endif // BLE_COM_H
