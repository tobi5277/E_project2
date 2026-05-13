#include "pwm_control.h"
#include <Arduino.h>


#define NUM_CHANNELS 4
#define NUM_ZONES 4

static int current_preset = 4;
static unsigned long lights_off_since = 0;
bool all_lights_off_bool = true;
static int occupancy_zones[4] = {1,1,1,1};
static bool force_off[NUM_CHANNELS] = {false};

static struct Fadestate {
  bool active = false;
  int current;
  int end;
  int step;
  unsigned long interval;  // Interval between updates in microseconds
  unsigned long last_update;  // Last update time in microseconds
} fade_state[NUM_CHANNELS];

// Function definitions
int selected_preset() {
  return current_preset;
}

int get_pwm(int chnl){
  if (chnl < 0 || chnl >= NUM_CHANNELS) return 0;
  return fade_state[chnl].current;
}

void setup_pwm() {
  // This function is called in setup()
  pwm.begin();
  // Set PWM frequency to 1000 Hz (good for LED control)
  pwm.setPWMFreq(1000);

  // Set all outputs to off initially
  for (int i = 0; i < NUM_CHANNELS; i++) {
    pwm.setPWM(i, 0, 0);
  }
}

void parse_zones_command(String command){
  command = command.substring(5); // first 5 is "Zones"
  // rest is the occupancy array
  int new_zones[4] = {
    command[0] - '0', 
    command[1] - '0', 
    command[2] - '0', 
    command[3] - '0'};
  update_occupancy_zones(new_zones);
}

void update_occupancy_zones(int zones[4]){
  for (int i = 0; i < NUM_ZONES; i++){
    occupancy_zones[i] = zones[i];
  }
}

void update_selected_preset_value(int new_preset) {
  // Validate the preset value (1-4)
  if (new_preset >= 1 && new_preset <= 4) {
    current_preset = new_preset;
  }
}

void update_selected_preset(){
  if (Serial.available()){
    String rx_input = Serial.readStringUntil('\n');  // Read until newline
    rx_input.trim();  // Remove whitespace

    int new_preset = rx_input.toInt();

    // Validate the preset value (1-4)
    if (new_preset >= 1 && new_preset <= 4) {
      current_preset = new_preset;
      Serial.print("Preset set to: ");
      Serial.println(current_preset);
    } else {
      Serial.println("Invalid preset. Use 1-4.");
    }
  }
}

void get_current_saving(){
  int cur_pwms_sum = 0;
  for (int i = 0; i < NUM_CHANNELS; i++){
    cur_pwms_sum += get_pwm(i);
  }
  int saving = 100 - ceil((cur_pwms_sum/(NUM_CHANNELS * 4095)) * 100);
  Serial.println(saving);
}

/* 
Starts an asynchronous fade operation for the specified PWM channel.
This function initiates a non-blocking fade from start_value to end_value
over the given duration in milliseconds. The fade progresses in the background
and does not block the main loop. Call update_pwm_fade() in loop() to advance the fade.
step_size controls the increment per update (default 1 for smooth fades). 
*/
void fade_led_async(int channel, int start_value, int end_value, int duration_ms, int step_size) {
  
  // Ignore any ON request while forced OFF
  if (force_off[channel] && end_value > 0) return;


  if (channel < 0 || channel >=NUM_CHANNELS) return;
  // Ensure step_size is positive
  if (step_size <= 0) step_size = 1;

  Fadestate &fs = fade_state[channel];
  end_value = floor(end_value * (current_preset/4.0));

  /* If start and end values are the same or duration is invalid, 
  set PWM directly and deactivate fade */
  if (start_value == end_value || duration_ms <= 0) {
    pwm.setPWM(channel, 0, end_value);
    fs.active = false;
    return;
  }

  // Activate the fade and store the fade parameters
  fs.active = true;  // Mark the fade as active
  fs.current = start_value;  // Starting PWM value
  fs.end = end_value;  // Target PWM value
  fs.step = (end_value > start_value) ? step_size : -step_size;  // Step size and direction


  // Calculate the number of steps and interval between updates
  int total_range = abs(end_value - start_value);  // Total PWM range
  int steps = total_range / step_size;  // Number of updates needed
  if (steps == 0) steps = 1;  // Ensure at least one step

  fs.interval = (duration_ms * 1000UL) / steps;  // Time per step in microseconds
  if (fs.interval < 1) {
    fs.interval = 1;  // Minimum interval
  }

  // Initialize timing and set initial PWM value
  fs.last_update = micros();  // Record start time in microseconds
  pwm.setPWM(channel, 0, fs.current);  // Set PWM to starting value
}

void fade_led_to(int channel, int end_value, int duration_ms){
  int start = fade_state[channel].current;
  fade_led_async(channel, start, end_value, duration_ms);
}

/* 
Updates the asynchronous fade operation.
This function should be called repeatedly in the main loop to advance
the fade process. It checks if enough time has passed since the last update
and increments the PWM value accordingly. Non-blocking and returns immediately
if no update is needed or if no fade is active.
*/
void update_pwm_fade() {
  /*#######################
    Overvej at give aktive zoner som parameter her (og i fade),
    så vi ved hvilke zoner der skal opdateres ift. hvordan det svarer
    til bestemte LED'er. Evt. kunne det bare være et array, eks.
    [0,1,1,0] som representerer zoner der skal belyses.
    ####################### */
  unsigned long now = micros();  // Get current time in microseconds
  bool all_off = true; // temp check for burglary security.

  for (int ch = 0; ch < NUM_CHANNELS; ch++){
    Fadestate &fs = fade_state[ch];
    force_off[ch] = !occupancy_zones[ch];

    // If zone is inactive -> force fade to 0
    if (!occupancy_zones[ch]) {
      // enforce target = off
      fs.end = 0;
      if (!fs.active && fs.current > 0) {
        fade_led_async(ch, fs.current, 0, 500, 1);
      }
      continue;
    }

    // If no fade is currently active, do nothing
    if (!fs.active) { continue; }

    // Check if enough time has passed since the last update
    if (now - fs.last_update < fs.interval) { continue; }  // Not enough time has passed, wait for next call}
    
    // Update the timing and advance the fade
    fs.last_update = now;  // Record this update time
    fs.current += fs.step;  // Increment or decrement the current PWM value

    // Check if the fade has reached or overshot the target value
    if ((fs.step > 0 && fs.current >= fs.end) ||
        (fs.step < 0 && fs.current <= fs.end)) {
      fs.current = fs.end;  // Clamp to target value
      fs.active = false;  // Deactivate the fade when complete
    }
    pwm.setPWM(ch, 0, fs.current);  // Apply the new PWM value
  }
  for (int i = 0; i < NUM_CHANNELS; i++) {
      if (fade_state[i].active || get_pwm(i) > 0) {
        all_off = false;
        break;
      }
  }

  if (all_off && !all_lights_off_bool) {
    lights_off_since = millis();
  }
  all_lights_off_bool = all_off;
  
  
}

bool is_fade_active(int channel) {
  if (channel < 0 || channel >= NUM_CHANNELS) return false;
  return fade_state[channel].active;
}

void turn_all_lights_off(){
  for (int i = 0; i < NUM_CHANNELS; i++) {
    fade_led_async(i, get_pwm(i), 0, 500, 1);
  }
}

