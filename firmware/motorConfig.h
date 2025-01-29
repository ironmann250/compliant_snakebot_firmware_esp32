#pragma once
#include <Arduino.h>
#include "TrackEncoder.h"
#include <ESP32MotorControl.h>
#include <QuickPID.h>
#define BRAKING_THRESHOLD 2

void motorInit(int enc1A, int enc1B, int enc2A, int enc2B,
              int ain1_1, int ain1_2, int ain2_1, int ain2_2,
              int sleepPin, bool resetCounts, float pulsesPerRev);

class MotorPID {
public:
    // Configuration
    struct Config {
        int motorNum;  // 0 = Motor 1, 1 = Motor 2
        int ain1;
        int ain2;
        float pulsesPerRev;
    };

    // PID Parameters
    float Setpoint = 0.0f;
    float Input = 0.0f;
    float Output = 0.0f;
    float Kp = 1.32f;
    float Ki = 10.28f;
    float Kd = 0.02f;//0.10f;

    void init(const Config& config);
    void update();
    void setSetpointDeg(float degrees);
    void goTo(float var);

private:
    QuickPID pid;
    ESP32MotorControl* motorControl = nullptr; // Initialize to nullptr
    int motorNum = 0; // Default to motor 0
    Config cfg;
    
    void updatePID();
    void controlMotor();
};

// Declare global motor control instance
extern ESP32MotorControl motorControl;