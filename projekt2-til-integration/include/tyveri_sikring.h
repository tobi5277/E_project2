#ifndef TYVERI_SIKRING_H
#define TYVERI_SIKRING_H
#include "pwm_control.h"
#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>

extern Adafruit_PWMServoDriver pwm; 
extern bool all_lights_off_bool;

// State machine for burglary security
enum TyveriState {
  TYVERI_IDLE,      // Burglary mode disabled
  TYVERI_WAITING,   // Waiting 1 hour before triggering lights
  TYVERI_LIGHTS_ON  // Lights on, will turn off after 5 minutes
};

struct TyveriSikring {
  bool tyveri_on;              // Enable/disable burglary mode
  TyveriState state;           // Current state
  unsigned long phase_start_time; // When the current phase started
};

extern struct TyveriSikring tyveri_sikring;

void update_tyveri_sikring();
void parse_burglary_command(String command);
#define TYVERI_SIKRING_H
