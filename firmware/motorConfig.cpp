#include "motorConfig.h"

// Global motor control and encoder instances
TrackEncoder* trackEncoder = nullptr;
ESP32MotorControl motorControl; // <-- Global instance defined here

void motorInit(int enc1A, int enc1B, int enc2A, int enc2B,
              int ain1_1, int ain1_2, int ain2_1, int ain2_2,
              int sleepPin, bool resetCounts, float pulsesPerRev) {
    // Update TrackEncoder instantiation
    trackEncoder = new TrackEncoder(enc1A, enc1B, enc2A, enc2B, "encoderStorage", pulsesPerRev);
    trackEncoder->begin(200);
    
    if(resetCounts) {
        trackEncoder->resetCounts();
    }

    // Initialize motor control with correct pin assignments
    motorControl.attachMotors(ain1_1, ain1_2, ain2_1, ain2_2);
    pinMode(sleepPin, OUTPUT);
    digitalWrite(sleepPin, HIGH);
}

void MotorPID::init(const Config& config) {
    cfg = config;
    motorControl = &::motorControl; // <-- Critical fix: Assign global instance
    motorNum = config.motorNum;     // 0 = Motor 1, 1 = Motor 2

    // PID initialization
    pid = QuickPID(&Input, &Output, &Setpoint, Kp, Ki, Kd,
                  QuickPID::pMode::pOnError,
                  QuickPID::dMode::dOnMeas,
                  QuickPID::iAwMode::iAwClamp,
                  QuickPID::Action::direct);
    pid.SetOutputLimits(-100, 100);
    pid.SetSampleTimeUs(10000);
    pid.SetMode(QuickPID::Control::automatic);
}

void MotorPID::update() {
    Input = (motorNum == 0) ? trackEncoder->getEncoder2Count() 
                            : trackEncoder->getEncoder1Count();
    updatePID();
    controlMotor();
}

void MotorPID::setSetpointDeg(float degrees) {
    Setpoint = (degrees * cfg.pulsesPerRev) / 360.0f;
}

void MotorPID::goTo(float var) {
    Setpoint = var;
    update();
}

void MotorPID::updatePID() {
    if(pid.GetKp() != Kp || pid.GetKi() != Ki || pid.GetKd() != Kd) {
        pid.SetTunings(Kp, Ki, Kd);
    }
    pid.Compute();
}

void MotorPID::controlMotor() {
    float error = Setpoint - Input;
    
    // Use valid motorControl pointer
    if(abs(error) <= BRAKING_THRESHOLD) {
        motorControl->motorStop(motorNum);
    } else if(Output > 0) {
        motorControl->motorForward(motorNum, abs(Output));
    } else {
        motorControl->motorReverse(motorNum, abs(Output));
    }
}