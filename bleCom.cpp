#include "bleCom.h"

BLESerial<etl::circular_buffer<uint8_t, 255>> SerialBLE;

void initBLE() {
    String device_name = "ESP32-BLE-Slave";
    SerialBLE.begin(device_name);
}

void bleUpdate(MotorPID &motor1, MotorPID &motor2) {
    if (SerialBLE.available()) {
        String command = SerialBLE.readStringUntil('\n');
        Serial.println("Received: " + command);

        if (command.startsWith("tar1=")) {
            float setpoint = command.substring(5).toFloat();
            motor1.setSetpointDeg(setpoint);
            Serial.println("Motor 1 setpoint updated to: " + String(setpoint));
        } else if (command.startsWith("tar2=")) {
            float setpoint = command.substring(5).toFloat();
            motor2.setSetpointDeg(setpoint);
            Serial.println("Motor 2 setpoint updated to: " + String(setpoint));
        }
    }
}
