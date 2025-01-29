#include "motorConfig.h"
#include <mutex>
// Global motor control and encoder instances
std::timed_mutex motor_mutex; // Update mutex type
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
    // Use try_lock_for to prevent deadlocks
    if(motor_mutex.try_lock_for(std::chrono::milliseconds(10))) {
        Input = (motorNum == 0) ? trackEncoder->getEncoder2Count() 
                               : trackEncoder->getEncoder1Count();
        updatePID();
        controlMotor();
        motor_mutex.unlock();
    }
}

void MotorPID::setSetpointDeg(float degrees) {
    std::lock_guard<std::mutex> lock(motor_mutex);
    
    float newSetpoint = (degrees * cfg.pulsesPerRev) / 360.0f;
    Serial.printf("[Motor%d] Setpoint update:\n", motorNum+1);
    Serial.printf("  Degrees: %.2f → Pulses: %.2f (PPR: %.2f)\n", 
                 degrees, newSetpoint, cfg.pulsesPerRev);
    
    Setpoint = newSetpoint;
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