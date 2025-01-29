#include "bleCom.h"
#include "motorConfig.h"
#include <Arduino.h>

extern MotorPID motor1, motor2;

BLESerial<etl::circular_buffer<uint8_t, 255>> BLECom::SerialBLE;
std::mutex BLECom::ble_mutex;
String BLECom::buffer = "";

void BLECom::init() {
    SerialBLE.begin("MotorController-BLE");
    Serial.println("BLE Initialized");
}

void BLECom::update() {
    while (SerialBLE.available()) {
        char c = SerialBLE.read();
        processCharacter(c);
    }
}

void BLECom::processCharacter(char c) {
    // Handle backspace for simple editing
    if (c == '\b' && buffer.length() > 0) {
        buffer.remove(buffer.length()-1);
        return;
    }

    // Accept only printable ASCII characters
    if (isPrintable(c)) {
        if (c == '\n' || c == ';') {  // Accept both newline and semicolon as terminators
            if (buffer.length() > 0) {
                std::lock_guard<std::mutex> lock(ble_mutex);
                handleCommand(buffer);
                buffer = "";
            }
        }
        else if (buffer.length() < 32) {
            buffer += c;
        }
    }
}

void BLECom::handleCommand(const String& command) {
    float value = 0;
    int motorNum = -1;
    const char* cmd = command.c_str();
    
    // Use sscanf for safe parsing
    if (sscanf(cmd, "tar1=%f", &value) == 1) {
        motorNum = 0;
    } 
    else if (sscanf(cmd, "tar2=%f", &value) == 1) {
        motorNum = 1;
    }

    if (motorNum != -1) {
        value = constrain(value, MIN_SETPOINT, MAX_SETPOINT);
        MotorPID* target = (motorNum == 0) ? &motor1 : &motor2;
        
        // Protected motor access
        target->setSetpointDeg(value);
        
        // Send confirmation
        SerialBLE.printf("OK %s=%.2f\n", (motorNum == 0) ? "tar1" : "tar2", value);
        Serial.printf("[BLE] Set motor%d to %.2f°\n", motorNum+1, value);
    } else {
        SerialBLE.println("ERR: Invalid format. Use 'tar1=<value>' or 'tar2=<value>'");
        Serial.printf("[BLE] Invalid command: %s\n", cmd);
    }
}