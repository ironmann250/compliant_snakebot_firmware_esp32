#include "bleCom.h"

BLESerial<etl::circular_buffer<uint8_t, 255>> SerialBLE;

// Packet parsing state machine
// Packet constants
constexpr uint8_t HEADER = 0xA5;
constexpr uint8_t FOOTER = 0x5A;

void initBLE(const char* deviceName) {
    SerialBLE.begin(deviceName);
}


Command updateBLE() {
    Command result;
    static enum {
        WAIT_HEADER,
        COLLECT_DATA,
        CHECK_FOOTER
    } state = WAIT_HEADER;
    
    static uint8_t dataIndex = 0;
    static uint8_t dataBuffer[MSG_BYTE_LEN];
    static uint8_t receivedChecksum = 0;

    while (SerialBLE.available()) {
        uint8_t c = SerialBLE.read();

        switch (state) {
            case WAIT_HEADER:
                if (c == HEADER) {
                    state = COLLECT_DATA;
                    dataIndex = 0;
                }
                break;

            case COLLECT_DATA:
                if (dataIndex < MSG_BYTE_LEN) {
                    dataBuffer[dataIndex++] = c;
                    //Serial.printf("%02x",c); Serial.print(",");
                } else {
                   // get CHECKSUM complete
                    receivedChecksum = c;
                    state = CHECK_FOOTER; //check footer
                    //Serial.printf("%02x",c);Serial.println();
                }
                break;

            case CHECK_FOOTER:
                if (c == FOOTER) {
                    // Calculate checksum (sum of data bytes)
                    uint8_t calculatedChecksum = 0;
                    for (size_t i = 0; i < MSG_BYTE_LEN; i++) {
                        calculatedChecksum += dataBuffer[i];
                        //Serial.printf("%02x",dataBuffer[i]); Serial.print(",");
                    }
                    calculatedChecksum &= 0xFF;
                    //Serial.println(calculatedChecksum);
                    // Validate and populate result
                    result.isvalid = (calculatedChecksum == receivedChecksum);
                    memcpy(result.bytes, dataBuffer, MSG_BYTE_LEN);
                }
                state = WAIT_HEADER; // Reset for next packet
                break;
        }
    }

    return result;
}

// blePrintf remains unchanged


void blePrintf(const char *fmt, ...) {
    char buf[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    
    for(int i = 0; buf[i] != '\0'; i++) {
        SerialBLE.write(buf[i]);
        SerialBLE.flush();
    }
}