#include "bleCom.h"
#include <BLESerial.h>
#include <Embedded_Template_Library.h>
#include <etl/circular_buffer.h>
#include <stdarg.h>

// Add these at the top of your file

// BLE Serial instance
BLESerial<etl::circular_buffer<uint8_t, 255>> SerialBLE;

// Buffer for incoming data
String inputBuffer = "";

void initBLE(const char* deviceName) {
    SerialBLE.begin(deviceName);
}

Command updateBLE() {
    Command result;
    
    while (SerialBLE.available()) {
        char c = SerialBLE.read();
        
        if (c == '[') {
            // Start of new command
            inputBuffer = "";
        } else if (c == ']') {
            // End of command, parse it
            if (inputBuffer.length() >= 2) {
                result.cmd = inputBuffer[0];
                // Start parsing from position 1 to include potential negative sign
                result.value = inputBuffer.substring(1).toFloat();
            }
            inputBuffer = "";
            break;
        } else {
            // Add to buffer, including minus signs
            inputBuffer += c;
        }
    }
    
    return result;
}

void blePrintf(const char *fmt, ...) {
    char buf[128];  // Static buffer for ESP32
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    
    // Write each character using available SerialBLE.write
    for(int i = 0; buf[i] != '\0'; i++) {
        SerialBLE.write(buf[i]);
    }
    SerialBLE.flush();
}