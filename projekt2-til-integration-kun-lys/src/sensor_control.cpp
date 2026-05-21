#include "sensor_control.h"
#include "pwm_control.h"
#include <Arduino.h>


float ambient_offset = 0;
bool ambient_calibrated = false;

// timer variable for millis() comparisons
static unsigned long last_sample_time = 0;
// static unsigned long integration_start_time = 0;

// global variable for integration time for comparisons
static uint16_t integration_time_ms = 100;

// How often to resample ambient light
static const unsigned long SAMPLE_INTERVAL_MS = 500;

/* Calibrate sensor for current ambient lighting. 
   Only run at initialization of the sensor. 
   Here we simply use an offset of ambient light,
   since light is additive, we can later subtract
   this offset from our measurement to */
void calibrate_ambient(){

  delay(200); // let sensor settle
  ambient_offset = read_lux_value();
  ambient_calibrated = true;

  Serial.print("Calibrating ambient offset: ");
  Serial.println(ambient_offset);

}

// Function definitions
void init_light_sensor() {
  if (!tsl.begin(&Wire)) {
    Serial.println("TSL2591 not found!");
    while (1); // Halt if sensor not found
  }

  // Configure sensor gain and integration time.
  tsl.setGain(TSL2591_GAIN_MED);      // Medium gain
  tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS); // 100ms integration time

  // get actual integration time in ms
  tsl2591IntegrationTime_t it = tsl.getTiming();
  integration_time_ms = integration_time_to_ms(it);
  delay(integration_time_ms + 50);

  

  // Optional debug
  Serial.print("TSL2591 integration time (ms): ");
  Serial.println(integration_time_ms);

  calibrate_ambient();
  // 
  last_sample_time = millis();

  Serial.println("TSL2591 initialized!");
}


float read_lux_value() {
  uint32_t full_lum = tsl.getFullLuminosity();
  uint16_t ir = full_lum >> 16; // high word is infrared
  uint16_t full = full_lum & 0xFFFF; // low word is visible 

  float lux = tsl.calculateLux(full, ir);
  return lux;
}

// Convert integration time to ms.
uint16_t integration_time_to_ms(tsl2591IntegrationTime_t integration_time){
  switch (integration_time){
    case TSL2591_INTEGRATIONTIME_100MS: return 100;
    case TSL2591_INTEGRATIONTIME_200MS: return 200;
    case TSL2591_INTEGRATIONTIME_300MS: return 300;
    case TSL2591_INTEGRATIONTIME_400MS: return 400;
    case TSL2591_INTEGRATIONTIME_500MS: return 500;
    case TSL2591_INTEGRATIONTIME_600MS: return 600;
    default: return 100;
  }
}

int calculate_pwm_duty_cycle(float lux) {
  // Map lux values to PWM duty cycle (0-4095)
  // Lower lux should produce brighter output, higher lux should dim the LEDs.
  float min_lux = 0;
  float max_lux = 1000;

  // clamp lux to a valid range (0 - 4095)
  lux = constrain(lux, min_lux, max_lux); 

  // normalize lux to range [0,1]
  float normalized = (lux - min_lux) / (max_lux - min_lux);

  // invert the response (high pwm -> low LED), and scale to pwm
  int pwm = (1.0 - normalized) * 4095;
  return pwm;
}

/* called in loop(): updates sensor values after designated time
has passed, and update LED pwm correspondingly. */
void sensor_control_update(){
  unsigned long now = millis();
  if (now - last_sample_time >= SAMPLE_INTERVAL_MS) {
    last_sample_time = now;

    float raw = read_lux_value();

    float corrected = raw - ambient_offset;
    if (corrected < 0) corrected = 0.0;

    // Debug output
    /* Serial.println("LUX MEASUREMENT AND CORRECTION:");
    Serial.print("Raw: ");
    Serial.print(raw);
    Serial.print(" | Ambient: ");
    Serial.print(ambient_offset);
    Serial.print(" | Corrected: ");
    Serial.println(corrected); */

    int target_pwm = calculate_pwm_duty_cycle(corrected);

    for (int i = 0; i < 4; i++) {
      int current_pwm = get_pwm(i);
      if (current_pwm == target_pwm) continue;
      
      int difference = abs(target_pwm - current_pwm);
      int steps = 50;
      int step_size = difference / steps;
      
      if (step_size < 1) step_size = 1;
      fade_led_async(i, current_pwm, target_pwm, 500, step_size);
    }
  }
}