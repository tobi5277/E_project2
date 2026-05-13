#ifndef PWM_CONTROL_H
#define PWM_CONTROL_H

#include <Adafruit_PWMServoDriver.h>

// Global PWM object
extern Adafruit_PWMServoDriver pwm;
// preset attribute and function
int selected_preset();
int get_pwm(int channel);
void update_selected_preset();
// Function declarations
void setup_pwm();
void fade_led_async(int channel, int start_value, int end_value, int duration_ms, int step_size = 1);
void fade_led_to(int channel, int end_value, int duration_ms);
void update_pwm_fade();
bool is_fade_active(int channel);

void turn_all_lights_off();


#endif // PWM_CONTROL_H