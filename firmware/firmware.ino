 #define ENCODER1_PIN_A 1
#define ENCODER1_PIN_B 2
#define ENCODER2_PIN_A 11
#define ENCODER2_PIN_B 12
#define RESET_COUNT_ON_BOOT 0

#define LED_PIN 7
#define PWM_2 41
#define AIN_1 38
#define AIN_2 37
#define BIN_1 36
#define BIN_2 35
#define SLEEP_PIN 39

#include <Arduino.h>
#include "motorConfig.h"
#include "tuning.h"
#include "bleCom.h"

MotorPID motor1, motor2;
TuneSet<> tuning;

unsigned long previousMillis = 0;
const long interval = 10;  // interval in milliseconds

void setup() {
    Serial.begin(115200);
    
    // Initialize motor system with correct pin mapping
    motorInit(
        ENCODER1_PIN_A, ENCODER1_PIN_B,
        ENCODER2_PIN_A, ENCODER2_PIN_B,
        AIN_2, AIN_1,  // Motor 1 pins (AIN_2 = 37, AIN_1 = 38)
        BIN_2, BIN_1,  // Motor 2 pins (BIN_2 = 35, BIN_1 = 36)
        SLEEP_PIN,
        RESET_COUNT_ON_BOOT,
        8344
    );

    // Initialize motor controllers with explicit configurations
    motor1.init({0, AIN_2, AIN_1, 8344}); // Motor 1 (num 0) uses AIN_2 & AIN_1
    motor2.init({1, BIN_2, BIN_1, 8344}); // Motor 2 (num 1) uses BIN_2 & BIN_1

    // Tuning setup
    tuning.add("tar1", motor1.Setpoint);
    tuning.add("kp1", motor1.Kp);
    tuning.add("ki1", motor1.Ki);
    tuning.add("kd1", motor1.Kd);
    tuning.add("tar2", motor2.Setpoint);
    tuning.add("kp2", motor2.Kp);
    tuning.add("ki2", motor2.Ki);
    tuning.add("kd2", motor2.Kd);

    motor1.setSetpointDeg(0.0f);
    motor2.setSetpointDeg(0.0f);
    
    // Initialize BLE
    initBLE("SnakeRobot");
    motor1.startSinusoidalOscillation(0.3f, 180.0f);
    Serial.println("Setup complete");
}

void loop() {
    // Get BLE command
    unsigned long currentMillis = millis();
    Command cmd = updateBLE();
    tuning.readSerial();

    // Handle command based on type
    switch(cmd.cmd) {
        case 't':  // Target position
            motor1.Setpoint+=cmd.value;//goTo((int)cmd.value)
            break;
        case 'y':  // Target position
            motor1.Setpoint-=cmd.value;//goTo((int)cmd.value)
            break;
        case 'p':  // Update Kp
            motor2.Setpoint+=cmd.value;
            break;
        case 'o':  // Update Kp
            motor2.Setpoint-=cmd.value;
            break;
        // Add more command handlers as needed
    }
    
    // Update both motors
     if(motor1.isOscillating()) {
        // Update motor (oscillation is handled in update())
        motor1.update();
    }

    // Serial print statements remain unchanged
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        //motor1.update();
        motor2.update();
        debugPrint();
    }
    //delay(10);
}


void debugPrint()
{
    Serial.print(motor1.Setpoint); Serial.print("\t");
    Serial.print(motor1.Input);    Serial.print("\t");
    Serial.print(motor1.Output);   Serial.print("\t");
    Serial.print(motor1.Kp);       Serial.print("\t");
    Serial.print(motor1.Ki);       Serial.print("\t");
    Serial.print(motor1.Kd);       Serial.print("\t");

    Serial.print(motor2.Setpoint); Serial.print("\t");
    Serial.print(motor2.Input);    Serial.print("\t");
    Serial.print(motor2.Output);   Serial.print("\t");
    Serial.print(motor2.Kp);       Serial.print("\t");
    Serial.print(motor2.Ki);       Serial.print("\t");
    Serial.println(motor2.Kd);

    blePrintf("%d,%d\n",motor1.Setpoint,motor2.Setpoint);
}