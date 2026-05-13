#include "tyveri_sikring.h"
#include "pwm_control.h"

#define MAX_TYVERI_TIME 3600000UL   // 1 hour
#define MAX_TYV_ON_TIME 300000UL    // 5 minutes

struct TyveriSikring tyveri_sikring = {
  .tyveri_on = false,
  .state = TYVERI_IDLE,
  .phase_start_time = 0
};

void update_tyveri_sikring() {
  unsigned long now = millis();

  // If burglary mode is disabled, ensure we're in IDLE state
  if (!tyveri_sikring.tyveri_on) {
    if (tyveri_sikring.state != TYVERI_IDLE) {
      // Turn off all lights if transitioning to idle
      for (int i = 0; i < 4; i++) {
          fade_led_async(i, pwm.getPWM(i), 0, 500, 1);
        }
      tyveri_sikring.state = TYVERI_IDLE;
    }
    if (!all_lights_off_bool){
        tyveri_sikring.state = TYVERI_IDLE;
    }
    
    return;
  }

  // State machine for burglary security
  switch (tyveri_sikring.state) {
    case TYVERI_IDLE:
      // Transition from disabled to enabled
      if (tyveri_sikring.tyveri_on) {
        tyveri_sikring.state = TYVERI_WAITING;
        tyveri_sikring.phase_start_time = now;
        Serial.println("Burglary mode: entering WAITING phase (1 hour)");
      }
      break;

    case TYVERI_WAITING:
      // Wait 1 hour before triggering lights
      if (now - tyveri_sikring.phase_start_time >= MAX_TYVERI_TIME) {
        // Transition to LIGHTS_ON phase
        tyveri_sikring.state = TYVERI_LIGHTS_ON;
        tyveri_sikring.phase_start_time = now;
        
        // Fade all 4 lights on
        for (int i = 0; i < 4; i++) {
          fade_led_async(i, 0, 4095, 500, 1);
        }
        Serial.println("Burglary mode: WAITING timeout reached, lights ON for 5 minutes");
      }
      break;

    case TYVERI_LIGHTS_ON:
      // Keep lights on for 5 minutes, then turn off
      if (now - tyveri_sikring.phase_start_time >= MAX_TYV_ON_TIME) {
        // Fade all 4 lights off
        for (int i = 0; i < 4; i++) {
          fade_led_async(i, pwm.getPWM(i), 0, 500, 1);
        }
        // Transition back to WAITING
        tyveri_sikring.state = TYVERI_WAITING;
        tyveri_sikring.phase_start_time = now;
        Serial.println("Burglary mode: LIGHTS_ON timeout reached, cycling back to WAITING");
      }
      break;
  }
}

void parse_burglary_command(String command) {
  command.toUpperCase();
  command.trim();

  if (command == "TyvAlarmOn") {
    if (!tyveri_sikring.tyveri_on) {
      tyveri_sikring.tyveri_on = true;
      tyveri_sikring.state = TYVERI_IDLE;
      Serial.println("Burglary mode ENABLED");
    }
  }
  else if (command == "TyvAlarmOff") {
    if (tyveri_sikring.tyveri_on) {
      tyveri_sikring.tyveri_on = false;
      Serial.println("Burglary mode DISABLED");
    }
  }
/*   else if (command == "BURGLARY_STATUS") {
    Serial.print("Burglary mode: ");
    Serial.print(tyveri_sikring.tyveri_on ? "ON" : "OFF");
    Serial.print(" | State: ");
    switch (tyveri_sikring.state) {
      case TYVERI_IDLE:
        Serial.println("IDLE");
        break;
      case TYVERI_WAITING:
        Serial.println("WAITING");
        break;
      case TYVERI_LIGHTS_ON:
        Serial.println("LIGHTS_ON");
        break;
    }
  } */
}
