#ifndef BLE_COM_H
#define BLE_COM_H

#include <Arduino.h>
#include <Embedded_Template_Library.h>
#include <BLESerial.h>
#include <etl/circular_buffer.h>
#include <stdarg.h>

#ifndef MSG_BYTE_LEN
#define MSG_BYTE_LEN 3
#endif

struct Command {
    bool isvalid;                   // Checksum validation flag
    uint8_t bytes[MSG_BYTE_LEN];    // Raw data bytes (header/footer excluded)
    Command() : isvalid(false) {
        memset(bytes, 0, sizeof(bytes));
    }
};

void initBLE(const char* deviceName);
Command updateBLE();
void blePrintf(const char *fmt, ...);

#endif