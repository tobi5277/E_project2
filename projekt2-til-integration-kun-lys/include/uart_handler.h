#ifndef UART_HANDLER_H
#define UART_HANDLER_H

#include <Arduino.h>

/**
 * Main UART update function.
 * Reads serial input and dispatches to appropriate handlers.
 * Call once per loop iteration.
 * 
 * Supports commands:
 * - BURGLARY_ON, BURGLARY_OFF, BURGLARY_STATUS (burglary security)
 * - 1, 2, 3, 4 (light presets)
 */
void route_command(String command);
void uart_handler_update();
void handle_zones_command(String command);
void handle_preset_command(String command);
void handle_burglary_command(String command);

#endif // UART_HANDLER_H
