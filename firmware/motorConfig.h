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
    float Kd = 0.02f;

    void init(const Config& config);
    void update();
    void setSetpointDeg(float degrees);
    void goTo(float var);
    
    // Oscillation control interface
    void startSinusoidalOscillation(float frequencyHz, float amplitudeDeg, 
                                   float phaseOffset = 0.0f, 
                                   uint32_t durationMs = 0);
    void stopSinusoidalOscillation();
    bool isOscillating() const { return oscillationActive; }
    
    // Real-time parameter updates
    void setOscillationFrequency(float newFrequencyHz);
    void setOscillationAmplitude(float newAmplitudeDeg);
    void setOscillationPhase(float newPhaseOffsetRad);

private:
    QuickPID pid;
    ESP32MotorControl* motorControl = nullptr;
    int motorNum = 0;
    Config cfg;
    
    // Oscillation parameters with mutex protection
    float oscillationFrequency = 0.0f;
    float oscillationAmplitude = 0.0f;
    float oscillationPhase = 0.0f;
    uint32_t oscillationStartTime = 0;
    uint32_t oscillationDuration = 0;
    bool oscillationActive = false;
    portMUX_TYPE parameterMux = portMUX_INITIALIZER_UNLOCKED;
    
    void updatePID();
    void controlMotor();
};

extern ESP32MotorControl motorControl;
