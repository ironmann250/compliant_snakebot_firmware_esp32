#ifndef BLE_COM_H
#define BLE_COM_H

#include <Arduino.h>
#include <Embedded_Template_Library.h>

struct Command {
    char cmd;
    float value;
    Command(char c = 0, float v = 0.0f) : cmd(c), value(v) {}
};

void initBLE(const char* deviceName);
Command updateBLE();

#endif
