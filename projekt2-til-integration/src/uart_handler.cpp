#include "uart_handler.h"
#include "tyveri_sikring.h"
#include "pwm_control.h"
#include <Arduino.h>

/**
 * Handle burglary security commands.
 */
void handle_burglary_command(String command) {
  Serial.println(command);
  parse_burglary_command(command);
}

/**
 * Handle light preset commands.
 */
void handle_preset_command(String command) {
  Serial.println(command);
  command = command.substring(6);
  int new_preset = command.toInt();
  if (new_preset >= 1 && new_preset <= 4) {
    update_selected_preset_value(new_preset);
    Serial.print("Preset set to: ");
    Serial.println(new_preset);
  } else {
    Serial.println("Invalid preset. Use 1-4.");
  }
}
/**
 * Handle occupancy zones commands.
 */
void handle_zones_command(String command){
  parse_zones_command(command);
}

/**
 * Route incoming command to appropriate handler.
 */
void route_command(String command) {
  command.trim();
  //Serial.println(command);
  if (command.startsWith("TyvAlarm")) {
    handle_burglary_command(command);
  } 
  else if (command.startsWith("Preset")){
    handle_preset_command(command);
  }
  else if (command.startsWith("Zones")){
    handle_zones_command(command);
  }
  else if (command.startsWith("ShutDown")){
    return;
  }
}

/**
 * Main UART update function.
 * Reads serial input and routes to appropriate handlers.
 */
void uart_handler_update() {
  if (Serial1.available()) {
    String rx_input = Serial1.readStringUntil('\n');
    route_command(rx_input);
  }
  if (Serial2.available()) {
    String rx_input = Serial2.readStringUntil('\n');
    route_command(rx_input);
  }
}
