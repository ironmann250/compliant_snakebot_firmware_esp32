#include "bleCom.h"
#include "motorConfig.h"
#include <Arduino.h>

bool BLECom::debugEnabled = true;  // Debug enabled by default

extern MotorPID motor1, motor2;

BLESerial<etl::circular_buffer<uint8_t, 255>> BLECom::SerialBLE;
std::mutex BLECom::ble_mutex;
String BLECom::buffer = "";

void BLECom::init() {
    SerialBLE.begin("MotorController-BLE");
    if(debugEnabled) Serial.println("BLE Initialized");
}

void BLECom::update() {
    // Process only 1 character per call to prevent blocking
    if (SerialBLE.available()) {
        char c = SerialBLE.read();
        processCharacter(c);
    }
}

void BLECom::processCharacter(char c) {
    // Handle various line endings
    if (c == '\r' || c == '\n' || c == ';') {
        if (buffer.length() > 0) {
            std::lock_guard<std::mutex> lock(motor_mutex);
            if(debugEnabled) Serial.printf("[BLE] Processing command: '%s'\n", buffer.c_str());
            handleCommand(buffer);
            buffer = "";
        }
        return;
    }
    
    if (isPrintable(c) && buffer.length() < 32) {
        buffer += c;
    }
}

void BLECom::handleCommand(const String& command) {
   if(motor_mutex.try_lock_for(std::chrono::milliseconds(10))) {
    float value = 0;
    int motorNum = -1;
    const char* cmd = command.c_str();
    
    // Strict pattern matching
    if (sscanf(cmd, "tar1=%f", &value) == 1) {
        motorNum = 0;
    } 
    else if (sscanf(cmd, "tar2=%f", &value) == 1) {
        motorNum = 1;
    }

    if (motorNum != -1) {
        value = constrain(value, MIN_SETPOINT, MAX_SETPOINT);
        MotorPID* target = (motorNum == 0) ? &motor1 : &motor2;
        
        // Debug before/after values
        if(debugEnabled) {
            Serial.printf("[MOTOR%d] Previous Setpoint: %.2f\n", 
                         motorNum+1, target->Setpoint);
        }
        
        target->setSetpointDeg(value);
        
        if(debugEnabled) {
            Serial.printf("[MOTOR%d] New Setpoint: %.2f\n", 
                         motorNum+1, target->Setpoint);
        }
        
        SerialBLE.printf("OK %s=%.2f\n", 
                        (motorNum == 0) ? "tar1" : "tar2", value);
    } else {
        SerialBLE.println("ERR: Invalid format");
        if(debugEnabled) Serial.printf("[BLE] Rejected command: '%s'\n", cmd);
    }

     motor_mutex.unlock();
    } else {
        SerialBLE.println("ERR: System busy");
    }
}