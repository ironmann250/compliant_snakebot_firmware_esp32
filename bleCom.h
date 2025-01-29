#pragma once
#include <BLESerial.h>
#include <etl/circular_buffer.h>
#include <mutex>

class MotorPID;
extern MotorPID motor1, motor2;

class BLECom {
public:
    static void init();
    static void update();

private:
    static BLESerial<etl::circular_buffer<uint8_t, 255>> SerialBLE;
    static std::mutex ble_mutex;  // Mutex for thread safety
    static String buffer;
    static constexpr float MAX_SETPOINT = 360.0f;
    static constexpr float MIN_SETPOINT = -360.0f;
    
    static void handleCommand(const String& command);
    static void processCharacter(char c);
};