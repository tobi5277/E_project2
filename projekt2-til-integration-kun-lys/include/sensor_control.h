#ifndef SENSOR_CONTROL_H
#define SENSOR_CONTROL_H

#include <Adafruit_TSL2591.h>
#include <Adafruit_PWMServoDriver.h>

// Global sensor object
extern Adafruit_TSL2591 tsl;
// Global PWM object
extern Adafruit_PWMServoDriver pwm;
// Function declarations
void init_light_sensor();
float read_lux_value();
int calculate_pwm_duty_cycle(float lux);
uint16_t integration_time_to_ms(tsl2591IntegrationTime_t integration_time);
void sensor_control_update();
#endif // SENSOR_CONTROL_H