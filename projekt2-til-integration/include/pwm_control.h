#ifndef PWM_CONTROL_H
#define PWM_CONTROL_H
#include <Adafruit_PWMServoDriver.h>


// Global PWM object
extern Adafruit_PWMServoDriver pwm;
// preset attribute and function
int selected_preset();
int get_pwm(int channel);
void update_selected_preset();
void update_selected_preset_value(int new_preset);
void update_occupancy_zones(int zones[4]);
void parse_zones_command(String command);
// Function declarations
void setup_pwm();
void fade_led(int channel, int start_value, int end_value, int delay_ms);
void fade_led_async(int channel, int start_value, int end_value, int duration_ms, int step_size = 1);
void update_pwm_fade();
bool is_fade_active();

void stop_fade();


#endif // PWM_CONTROL_H