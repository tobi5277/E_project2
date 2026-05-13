#include "sensor_control.h"
#include "pwm_control.h"
#include <Arduino.h>

// State machine enum for lux sensor state.
enum LuxSampleState {
  LUX_WAITING, LUX_IDLE
};
// initial lux sampling state
static LuxSampleState lux_state = LUX_IDLE;

// timer variable for millis() comparisons
static unsigned long last_sample_time = 0;
static unsigned long integration_start_time = 0;

// global variable for integration time for comparisons
static uint16_t integration_time_ms = 100;

// How often to resample ambient light
static const unsigned long SAMPLE_INTERVAL_MS = 1000;


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

  // 
  last_sample_time = millis();

  // Optional debug
  Serial.print("TSL2591 integration time (ms): ");
  Serial.println(integration_time_ms);


  Serial.println("TSL2591 initialized!");
}


float read_lux_value() {
  uint32_t full_lum = tsl.getFullLuminosity();
  uint16_t ir = full_lum >> 16;
  uint16_t full = full_lum & 0xFFFF;
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

  uint8_t lux_case;
  if (lux < 50) {
    lux_case = 0;
  } else if (lux < 200) {
    lux_case = 1;
  } else if (lux < 500) {
    lux_case = 2;
  } else if (lux < 1000) {
    lux_case = 3;
  } else {
    lux_case = 4;
  }

  switch (lux_case) {
    case 0: // very dark
      return 4095;
    case 1: // dim indoor
      return 3072;
    case 2: // normal indoor
      return 2048;
    case 3: // bright indoor
      return 1024;
    default: // outdoors / very bright
      return 256;
  }
}

/* called in loop(): updates sensor values after designated time
has passed, and update LED pwm correspondingly. */

void sensor_control_update(){
  unsigned long now = millis();

  switch (lux_state) {
    case LUX_IDLE:
      if (now - last_sample_time >= SAMPLE_INTERVAL_MS) {
        last_sample_time = now;

        // Turn off all LED's
        for (int i = 0; i < 16; i++) {
          pwm.setPWM(i, 0, 0);
        }
        integration_start_time = now;
        lux_state = LUX_WAITING;
        Serial.println("Measuring LUX");
      }
      break;

    case LUX_WAITING:
      if (now - integration_start_time >= integration_time_ms) {

        float lux = read_lux_value();
        uint16_t new_pwm = (uint16_t)calculate_pwm_duty_cycle(lux);

        /*Set all LED's new pwm to the calculated pwm based on room luminosity. */
        for (int i = 0; i < 16; i++) {
          uint8_t current_pwm = pwm.getPWM(i);
          uint8_t step_size = abs(new_pwm - current_pwm) / 500;
          if (step_size < 1) step_size = 1;
          fade_led_async(i, current_pwm, new_pwm, 500, step_size);
        }
        lux_state = LUX_IDLE;
        Serial.print("PWM updated: ");
        Serial.println(new_pwm);
      }
      break;
  }
}